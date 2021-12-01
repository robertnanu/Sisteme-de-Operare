#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    pid_t pid = fork();
    if(pid < 0) {
        return errno;
    }
    else if(pid == 0) {
        // Instructiunile copilului
        char *argv[] = {"ls", NULL};
        // Daca argumentul este "ls" realizam urmatoarea operatie
        execve("/bin/ls", argv, NULL);
        perror(NULL);
    }
    else {
        // Instructiunile parintelui
        printf("My PID=%d, Child PID=%d\n", getpid(), pid);
        wait(NULL);
        printf("Child %d finished\n", getpid());
    }
    return 0;
}