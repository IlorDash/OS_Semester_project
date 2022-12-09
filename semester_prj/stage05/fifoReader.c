#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    int fd;
    char *myfifo = "button_data_22";
    mkfifo(myfifo, 0666);
    char str[80];
    fd = open(myfifo, O_RDONLY);
    read(fd, str, 80);
    printf("Reader get: %s\n", str);
    close(fd);
    return 0;
}

