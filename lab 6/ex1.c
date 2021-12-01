#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

// Trebuie sa respecte prototipul start_routine
void *strrev(void *v)
{
    char *str = (char*)v;
    int L = strlen(str);
    // Alocam spatiu pentru L + 1 caractere de tip char
    char *ret = malloc(L + 1);
    // Realizam rasturnatul
    for(int i = 0; i < L; i++) {
        ret[i] = str[L - i - 1];
    }
    return ret;
}

int main(int argc, char *argv[])
{
    // Daca nu avem exact doua argumente => eroare
    if(argc != 2) {
        printf("Invalid number of arguments\n");
        return 1;
    }

    // Definim noul fir de executie
    pthread_t thr;
    // Initializam thr cu noul fir de executie
    // Diferenta intre fork si thread este aceea ca
    // Thread-ul porneste de la o functie data
    // In cazul nostru, strrev, care primeste argumentul argv[1]
    if(pthread_create(&thr, NULL, strrev, argv[1])) {
        perror(NULL);
        return errno;
    }

    // Diferit de wait, asteapta explicit firul de executie din variabila thread
    void *result;
    // Punem la adresa indicata rezultatul functiei strrev
    if(pthread_join(thr, &result)) {
        perror(NULL);
        return errno;
    }

    // Afisam rezultatul
    printf("%s\n", (char*)result);
    // dezalocam zona de memorie
    free(result);

    return 0;
}