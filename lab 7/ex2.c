#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

// Initializam semaforul si mutex-ul
sem_t sem;
pthread_mutex_t mtx;
int maxThreads;
int threads;

void barrier_point()
{
    // Inchidem mutex-ul
    pthread_mutex_lock(&mtx);
    // Sectiunea critica
    threads++;
    if(threads == maxThreads) {
        // Creste valoarea lui S cu o unitate
        if(sem_post(&sem)) {
            perror(NULL);
            exit(errno);
        }
    }
    // Deblocam mutex-ul
    pthread_mutex_unlock(&mtx);

    // Scade valoarea lui S cu o unitate
    if(sem_wait(&sem)) {
        perror(NULL);
        exit(errno);
    }

    // Creste valoarea lui S cu o unitate
    // Dupa incrementare verifica daca sunt thread-uri blocate de semafor
    // Si elibereaza thread-ul care asteapta de cel mai mult timp in coada
    if(sem_post(&sem)) {
        perror(NULL);
        exit(errno);
    }
}

void *tfun(void *v)
{
    int *tid = (int *)v;

    printf("%d reached the barrier\n", *tid);
    barrier_point();
    printf("%d passed the barrier\n", *tid);
    
    free(tid);
    return NULL;
}

pthread_t T[1005];

void init(int N)
{
    // Initializam semaforul cu S = 0
    if(sem_init(&sem, 0, 0)) {
        perror(NULL);
        exit(errno);
    }

    // Cream un obiect de tip mutex
    if(pthread_mutex_init(&mtx, NULL)) {
        perror(NULL);
        exit(errno);
    }
    maxThreads = N;
}

// Functie care creeaza un nou fir de executie plecand de la functia tfun
pthread_t create_process(int res)
{
    pthread_t ret;
    int *x = (int *)malloc(sizeof(int));
    *x = res;
    if(pthread_create(&ret, NULL, tfun, x)) {
        perror(NULL);
        exit(errno);
    }
    return ret;
}

void *result;

int main()
{
    int N;
    printf("NTHRS=");
    scanf("%d", &N);

    // Functie care creeaza semaforul si mutex-ul
    init(N);

    for(int i = 0; i < N; i++) {
        // Functie care creeaza un nou fir de executie
        T[i] = create_process(i);
    }

    for(int i = 0; i < N; i++) {
        // Diferit de wait, asteapta explicit firul de executie din variabila thread
        // Punem la adresa indicata rezultatul functiei proc din create_process
        if(pthread_join(T[i], &result)) {
            perror(NULL);
            return errno;
        }
    }

    // Eliberam semaforul
    sem_destroy(&sem);
    // Eliberam mutex-ul
    pthread_mutex_destroy(&mtx);
    return 0;
}