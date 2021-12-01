#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    // Daca nu avem 3 argumente STOP
    if(argc != 3) {
        printf("WRONG USAGE!\n");
        return -1;
    }

    int bufSize = 100;
    char buf[bufSize + 2];

    // Punem 0 pe toate pozitiile in buf
    memset(buf, 0, sizeof(buf));

    // Deschidem fisierul din primul argument
    // Daca nu exista il cream pentru read si write
    // Cu drepturi de administrator
    int fdIN = open(argv[1], O_CREAT | O_RDWR, S_IRWXU);

    if(fdIN < 0) {
        perror("read file");
        return errno;
    }

    // Deschidem si al doilea fisier cu aceleasi proprietati ca si primul
    int fdOUT = open(argv[2], O_CREAT | O_RDWR, S_IRWXU);

    if(fdOUT < 0) {
        perror("write file");
    }

    int bytesRead = 0, bytesWritten = 0;

    // Citesc pe rand cate bufSize caractere si le scriu cu write in fdOUT
    for(bytesRead = read(fdIN, buf, bufSize); bytesRead > 0; bytesRead = read(fdIN, buf, bufSize)) {
        bytesWritten = write(fdOUT, buf, bytesRead);
        if(bytesWritten < 0) {
            perror("write buf");
            return errno;
        }
    }

    if(bytesRead < 0) {
        perror("read buf");
        return errno;
    }

    // Inchidem fisiere
    close(fdIN);
    close(fdOUT);
	return 0;
}