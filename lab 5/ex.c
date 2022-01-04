#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

int n, i;
char shm[] = "salut";
int shm_fd;

int itoa2(int n, char *ptr)
{
    // Transforma din int in str
    // ret reprezinta numarul de cifre ale int-ului initial
    char c[13];
    int ret = 0;
    while(n != 0) {
        c[ret++] = '0' + n % 10;
        n /= 10;
    }
    for(int i = ret - 1; i >= 0; i--) {
        // Incarc in zona de memorie de la adresa ptr fiecare caracter
        *ptr = c[i];
        // Trec la urmatorul byte
        ptr++;
    }
    return ret;
}

void collatz(int n, char *ptr)
{
    // Shiftez la dreapta cu ret pozitii
    ptr += itoa2(n, ptr);
    // Incarc in zona de memorie caracterul ':'
    *ptr = ':';
    // Trec la urmatorul byte
    ptr++;
    while(n != 1) {
        // Incarc in zona de memorie pe n
        ptr += itoa2(n, ptr);
        // Incarc spatiul
        *ptr = ' ';
        // Trec la urmatorul byte
        ptr++;
        if(n % 2 == 0) {
            n /= 2;
        }
        else {
            n = 3 * n + 1;
        }
    }
    // Incarc ultima valoare a lui n, probabil 1
    ptr += itoa2(n, ptr);
    // Incarc new line-ul
    *ptr = '\n';
    // Trec la urmatorul byte
    ptr++;
}

int main(int argc, char *argv[])
{
    pid_t pid;
    printf("Starting parent %d\n", getpid());

    // Creem un obiect de memorie partajata
    // Cu drepturi de read si write
    // Cu W and R mask for the owner
    shm_fd = shm_open(shm, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(shm_fd < 0) {
        perror(NULL);
        return errno;
    }

    // Dimensiunea trebuie sa fie multiplu de pagini
    size_t shm_size = (argc - 1) * getpagesize();

    // Definim dimensiunea
    if(ftruncate(shm_fd, shm_size) == -1) {
        perror(NULL);
        // Stergem obiectul creat anterior cu functia shm_open
        shm_unlink(shm);
        return errno;
    }
    
    // Incarcam memoria partajata in spatiul procesului
    // 0 - adresa la care sa fie incarcata in proces
    // shm_size - dimensiunea memoriei incarcate
    // PROT_WRITE | PROT_READ - drepturile de acces
    // MAP_SHARED - tipul de memorie (MAP SHARED astfel incat modificarile facute de catre
    // proces sa fie vizibile si in celelalte)
    // shm_fd - descriptorul obiectului de memorie
    // 0 - locul in obiectul de memorie partajata de la care sa fie incarcat in spatiul procesului
    char *shm_ptr = mmap(0, shm_size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
    // initializam fiecare element cu 0
    memset(shm_ptr, 0, shm_size);

    for(i = 1; i < argc; i++) {
        // Cream un proces nou
        pid = fork();
        if(pid == 0) {
            // Comenzile copilului
            collatz(atoi(argv[i]), shm_ptr + (i - 1) * getpagesize());
            printf("Done Parent %d Me %d\n", getppid(), getpid());
            return 0;
        }
    }

    for(int i = 1; i < argc; i++) {
        // Redam controlul parintelui odata ce iese unul dintre fii sai
        wait(NULL);
    }

    for(int i = 1; i < argc; i++) {
        // Afisez fiecare pagina
        printf("%s", shm_ptr + (i - 1) * getpagesize());
    }

    printf("Done Parent %d Me %d\n", getppid(), getpid());
    // Cand nu mai este nevoie de zona de memorie partajata
    // Aceasta se elibereaza
    munmap(shm_ptr, shm_size);
    // Sterge obiectul creat anterior de shm_open
    shm_unlink(shm);
    return 0;
}