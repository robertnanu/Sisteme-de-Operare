#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    // Cream un proces nou
    pid_t pid = fork();
    if(pid < 0) {
        return errno;
    }
    else if(pid == 0) {
        // Instructiunile copilului
        // Argumentele sunt puse in argv, respectand conventia obisnuita 
        // din C: prima pozitie - calea absoluta
        // a doua pozitie - argumentele
        // Lista se incheie cu NULL
        char *argv[] = {"ls", NULL};
        // Daca argumentul este "ls" realizam urmatoarea operatie
        execve("/bin/ls", argv, NULL);
        perror(NULL);
    }
    else {
        // Instructiunile parintelui
        printf("My PID=%d, Child PID=%d\n", getpid(), pid);
        // Reda controlul parintelui odata ce iese oricare dintre fii sai
        wait(NULL);
        printf("Child %d finished\n", pid);
    }
    return 0;
}