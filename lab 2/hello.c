#include <stdio.h>
#include <unistd.h>

int main()
{
    char c[12] = "HelloWorld\n";
    // Default file descriptor
    write(1, c, 11);
    return 0;
}