#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int n, i;

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
    n = atoi(argv[1]);
    pid_t pid;
    printf("Starting parent %d\n", getppid());
    for(i = 1; i < argc; i++)
    {
        // Cream un proces nou pentru fiecare dintre argumentele date
        pid = fork();
        if(pid == 0)
        {
            // Comenzile copilului
            collatz(atoi(argv[i]));
            printf("Done Parent %d Me %d\n", getppid(), getpid());
            return 0;
        }
    }

    for(i = 1; i < argc; i++)
    wait(NULL);

    printf("Done Parent %d Me %d\n", getppid(), getpid());
    return 0;
}