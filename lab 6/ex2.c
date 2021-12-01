#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

int M[4][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}};
int N[3][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};
int P[4][4];
pthread_t T[4][4];

// Trebuie sa respecte prototipul start_routine
void *compute_product(void *v)
{
    int *p = (int *)v;
    int i = p[0];
    int j = p[1];
    //printf("in %d %d\n", i, j);
    // Realizam inmultirea matricelor
    for(int k = 0; k < 3; k++) {
        P[i][j] += M[i][k] * N[k][j]; 
    }
    
    free(p);
    //printf("out %d %d\n", i, j);
    return NULL;
}

int main()
{
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            // Aloc un vector cu doua elemente
            int *x = (int*)malloc(sizeof(int) * 2);
            // In x memorez noile pozitii din matricea rezultata
            x[0] = i;
            x[1] = j;
            // Initializam noul fir de executie plecand de la functia compute_product
            if(pthread_create(&T[i][j], NULL, compute_product, x)) {
                perror(NULL);
                return errno;
            }
        }
    }

    for(int i = 0 ; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            // Diferit de wait, asteapta explicit firul de executie din variabila thread
            void *result;
            // Punem la adresa indicata rezultatul functiei compute_product
            if(pthread_join(T[i][j], &result)) {
                perror(NULL);
                return errno;
            }
        }
    }
    
    // Afisam rezultatul
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            printf("%d ", P[i][j]);
        }
        printf("\n");
    }
    return 0;
}