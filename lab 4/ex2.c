#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int n;

void collatz(int n)
{
    printf("%d: ", n);
    while(n != 1) {
        printf("%d ", n);
        if(n % 2 == 0) {
            n /= 2;
        }
        else {
            n = 3 * n + 1;
        }
    }
    printf("%d\n", n);
}

int main(int argc, char *argv[])
{
    // Transformam din string in numar
    n = atoi(argv[1]);
    // Cream un proces nou
    pid_t pid = fork();
    if(pid == 0) {
        // Comenzile fiului
        collatz(n);
    }
    else {
        // Redam controlul parintelui odata ce iese unul dintre fii sai
        wait(NULL);
        printf("Child %d finished\n", pid);
    }
    return 0;
}