#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

void encript(int v[], int n)
{
    // Foloseste ceasul intern al computerului pentru a controla alegerea seed-urilor
	srand(time(NULL));
	for(int i = n-1; i; --i)
	{
		int j = rand() % (i+1);
		int aux = v[i];
        v[i] = v[j];
        v[j] = aux;
	}
}

int main(int argc, char *argv[])
{
    // Obtinem dimensiunea paginii
    int page_size = getpagesize();

    // Daca avem exact doua argumente in apel, deci vrem sa criptam un anumit fisier
    if(argc == 2)
    {
        char permsFile[] = "permutari";
        char writeFile[strlen(argv[1])];
        strcpy(writeFile, argv[1]);
        strcpy(writeFile + strlen(argv[1]) - 4, "Encripted.txt");

        // Deschidem pentru citire
        FILE *fin = fopen(argv[1], "r");
        if(fin == NULL)
        {
            perror("Errore la deschiderea fisierului de citire");
            return 0;
        }

        // Deschidem pentru scriere
        FILE *foutPerms = fopen(permsFile, "w");
        if(foutPerms == NULL)
        {
            perror("Eroare la deschiderea fisierului de permutari");
            return 0;
        }

        // Deschidem pentru scriere
        FILE *fout = fopen(writeFile, "w");
        if(fout == NULL)
        {
            perror("Eroare la deschiderea fisierului de scriere");
            return 0;
        }

        // Calculez numarul de cuvinte aflate in fisierul de intrare
        int nrcuv = 1;
        char c;
        while((c = fgetc(fin)) != EOF)
            if(c == ' ')
                nrcuv++;
        fseek(fin, 0, SEEK_SET);

        char shm_name[] = "shmem";
        int shm_fd = shm_open(shm_name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
        if(shm_fd < 0)
        {
            perror(NULL);
            return 0;
        }
        // Dimensiunea trebuie sa fie multiplu de pagini
        size_t shm_size = page_size * nrcuv;
        if(ftruncate(shm_fd, shm_size) == -1)
        {
            perror(NULL);
            shm_unlink(shm_name);
            return 0;
        }

        char *shm_ptr;
        for(int i = 0; i < nrcuv; ++i)
        {
            shm_ptr = mmap(0, page_size, PROT_WRITE, MAP_SHARED, shm_fd, page_size*i);
            if(shm_ptr == MAP_FAILED)
            {
                perror(NULL);
                shm_unlink(shm_name);
                return errno;
            }

            // Folosim fgetc() pentru a obtine dintr-un fisier cate un caracter pe rand
            while((c = fgetc(fin)) != EOF)
				if(c == ' ') break;
				else shm_ptr += sprintf(shm_ptr, "%c", c);
        }

        for(int i = 0; i < nrcuv; ++i)
        {
            shm_ptr = mmap(0, page_size, PROT_READ, MAP_SHARED, shm_fd, page_size*i);
            pid_t pid = fork();
            if(pid < 0) return errno;
            else if(pid == 0)
            {
                // Instructiuni copil
                // Prieiau lungimea cuvantului
                int len_cuv = strlen(shm_ptr);
				int perm[len_cuv];
                // Pe toata lungimea acestuia realizez o permutare random prin functia encript
				for(int j = 0; j < len_cuv; ++j)
                    perm[j] = j;
				encript(perm, len_cuv);

                // Reconstruiesc cuvantul in functie de criptarea rezultata
				char ecripted[len_cuv+1];
				for(int j = 0; j < len_cuv; ++j)
                    ecripted[j] = shm_ptr[perm[j]];

                // Incarc toate informatiile (cuvantul criptat + permutarile folosite in criptare) in fisierele de iesire
				for(int j = 0; j < len_cuv; ++j)
				{
					fprintf(foutPerms, "%d ", perm[j]);
					fprintf(fout, "%c", ecripted[j]);
				}
                // La finalul cuvantului, in fisierul permutarile realizez un endline si in cel al mesajului criptat pun un ' '
                if(i != nrcuv-1)
                {
                    fprintf(foutPerms, "\n");
                    fprintf(fout, " ");
                }
				exit(0);
            }
            else wait(NULL);
            munmap(shm_ptr, page_size);
        }

        printf("Fisierul trimis a fost criptat cu succes!\n");
        shm_unlink(shm_name);
        fclose(fin);
        fclose(foutPerms);
        fclose(fout);
    }
    else if(argc == 3) // Vrem sa decriptam un anumit fisier cunoscand permutarile criptarii initiale
    {
        char writeFile[strlen(argv[1])];
        strcpy(writeFile, argv[1]);
        strcpy(writeFile+strlen(argv[1])-4, "Decripted.txt");

        FILE *fin = fopen(argv[1], "r");
        if(fin == NULL)
        {
            perror("Errore la deschiderea fisierului de citire");
            return 0;
        }

        FILE *finPerms = fopen(argv[2], "r");
        if(finPerms == NULL)
        {
            perror("Eroare la deschiderea fisierului de permutari");
            return 0;
        }

        FILE *fout = fopen(writeFile, "w");
        if(fout == NULL)
        {
            perror("Eroare la deschiderea fisierului de scriere");
            return 0;
        }

        // Aflu numarul de cuvinte si lungimea maxima a unui cuvant
        int nrcuv = 1, len = 0, maxLen = 0;
        char c;
        while((c = fgetc(fin)) != EOF)
        {
            if(c == ' ')
            {
                nrcuv++;
                if(len > maxLen) maxLen = len;
                len = 0;
                continue;
            }
            len++;
        }
        fseek(fin, 0, SEEK_SET);
        // Matrice a permutarilor
        int perms[nrcuv+1][maxLen+1];

        char shm_name[] = "shmem";
        int shm_fd = shm_open(shm_name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
        if(shm_fd < 0)
        {
            perror(NULL);
            return 0;
        }
        size_t shm_size = page_size * nrcuv;
        if(ftruncate(shm_fd, shm_size) == -1)
        {
            perror(NULL);
            shm_unlink(shm_name);
            return 0;
        }

        char *shm_ptr;
        for(int i = 0; i < nrcuv; ++i)
        {
            shm_ptr = mmap(0, page_size, PROT_WRITE, MAP_SHARED, shm_fd, page_size*i);
            if(shm_ptr == MAP_FAILED)
            {
                perror(NULL);
                shm_unlink(shm_name);
                return errno;
            }

            // Iau caracter cu caracter primul cuvant si-i aflu si lungimea
            int len_cuv = 0;
            while((c = fgetc(fin)) != EOF)
            {
				if(c == ' ')
                    break;
				else
                {
                    shm_ptr += sprintf(shm_ptr, "%c", c);
                    len_cuv++;
                }
            }

            // Citesc permutarile din fisierul finPerms si le stochez in perms pe pozitia [i][j] a matricei
            for(int j = 0; j < len_cuv; ++j)
                fscanf(finPerms, "%d", &perms[i][j]);
        }

        for(int i = 0; i < nrcuv; ++i)
		{
			shm_ptr = mmap(0, page_size, PROT_READ, MAP_SHARED, shm_fd, page_size*i);
			pid_t pid = fork();
			if(pid < 0) return errno;
			else if(pid == 0)
			{
				int len_cuv = strlen(shm_ptr);

                // Simpla decriptare
				char decripted[len_cuv+1];
				for(int j = 0; j < len_cuv; ++j)
                    decripted[perms[i][j]] = shm_ptr[j];

                // Incarc decriptarea in fisierul de iesire
				for(int j = 0; j < len_cuv; ++j)
                    fprintf(fout, "%c", decripted[j]);
                // La finalul cuvantului adaug un spatiu in fisierul de iesire
				if(i != nrcuv-1)
                    fprintf(fout, " ");
                exit(0);
			}
            else wait(NULL);
			munmap(shm_ptr, page_size);
		}

        printf("Fisierul trimis a fost decriptat cu succes, folosind permutarile date!\n");
        shm_unlink(shm_name);
        fclose(fin);
        fclose(finPerms);
        fclose(fout);
    }
    else
    {
        printf("Pentru functia de criptare va rog folositi urmatorul apel: ./criptare [nume_fisier]\n");
        printf("Pentru functia de decriptare va rog folositi urmatorul apel: ./decriptare [nume_fisier] [nume_fisier_permutari]\n");
    }
    return 0;
}