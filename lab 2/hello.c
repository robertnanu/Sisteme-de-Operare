#include <stdio.h>
#include <unistd.h>

int main()
{
    char c[12] = "HelloWorld\n";
    // read(2) cites, te dintr-un descriptor d
    // in buferul buf un numa ̆r dat de nbytes.
    write(1, c, 11);
    return 0;
}