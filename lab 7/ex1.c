#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_RESOURCES 5

int available_resources = MAX_RESOURCES;

// mutex - prescurtat de la mutual exclusive
pthread_mutex_t mtx;

int decrease_count(int count)
{
    // Inchidem mutex-ul
    pthread_mutex_lock(&mtx);
    // Tot ce urmeaza pana la deblocare se numeste sectiunea critica
    if(available_resources < count) {
         pthread_mutex_unlock(&mtx);
        return -1;
    }
    available_resources -= count;
    // Deblocam mutex-ul
    pthread_mutex_unlock(&mtx);
    return 0;
}

int increase_count(int count)
{
    // Blocam mutex-ul
    pthread_mutex_lock(&mtx);
    // Sectiunea critica
    available_resources += count;
    // Deblocam mutex-ul
    pthread_mutex_unlock(&mtx);
    return 0;
}

void *proc(void *v)
{
    int *p = (int *)v;
    int resources = *p;

    for(int i = 1; i <= 10000; i++) {

    }
    
    int res = decrease_count(resources);

    if(res == - 1) {
        return "ERROR";
    }

    printf("Got %d resources %d remaining\n", resources, available_resources);
    
    for(int i = 1; i <= 10000; i++) {

    }

    increase_count(resources);

    printf("Released %d resources %d remaining\n", resources, available_resources);

    free(p);
    return NULL;
}

// Functie pentru crearea unui fir de executie plecand de la functia prod
pthread_t create_process(int res)
{
    pthread_t ret;
    int *x = (int *)malloc(sizeof(int));
    *x = res;
    if(pthread_create(&ret, NULL, proc, x)) {
        perror(NULL);
        exit(errno);
    }
    return ret;
}

void *result;

int main()
{
    // Cream un obiect de tip mutex
    if(pthread_mutex_init(&mtx, NULL)) {
        perror(NULL);
        return errno;
    }

    // Folosim functia creata pentru realizarea unui nou fir de executie
    pthread_t t1 = create_process(2);
   
   // Diferit de wait, asteapta explicit firul de executie din variabila thread
   // Punem la adresa indicata rezultatul functiei proc din create_process
    if(pthread_join(t1, &result)) {
        perror(NULL);
        return errno;
    }

    pthread_t t2 = create_process(3);

    if(pthread_join(t2, &result)) {
        perror(NULL);
        return errno;
    }

    t1 = create_process(2);
    t2 = create_process(3);

    if(pthread_join(t1, &result)) {
        perror(NULL);
        return errno;
    }

    if(pthread_join(t2, &result)) {
        perror(NULL);
        return errno;
    }

    // Nu mai este nevoie de obiectul mutex, eliberam resursele ocupate
    pthread_mutex_destroy(&mtx);
    return 0;
}