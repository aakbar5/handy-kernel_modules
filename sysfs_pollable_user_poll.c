#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LEN 32
#define SYSFS_ENTRY_PATH "/sys/test/root/readwrite"

int main(int argc, char **argv)
{
    int fd;
    int ret;
    int cnt;
    char buf[LEN];
    struct pollfd pfds;
    int timeout_in_ms = 1000 * 15;

    fd = open(SYSFS_ENTRY_PATH, O_RDWR);
    if (fd < 0) {
        printf("Failed to open %s\n", SYSFS_ENTRY_PATH);
        return -1;
    }

    pfds.fd = fd;
    pfds.events = POLLPRI | POLLERR;

    // Dummry read required before polling and after opening
    read(fd, buf, LEN);
    pfds.revents = 0;

    printf("Going to poll....\n");
    ret = poll(&pfds, 1, timeout_in_ms);
    if (ret == 0) {
        printf("Polling timeout...\n");
    }
    else if (ret < 0) {
        printf("Polling error...\n");
    }
    else {
        printf("Polling is triggered\n");

        // Seek to the start to read real data
        ret = lseek(pfds.fd, 0, SEEK_SET);
        if (ret < 0) {
            printf("lseek error...\n");
        }
        else {
            memset(buf, 0, LEN);
            ret = read(pfds.fd, buf, LEN);
            if (ret) {
                printf("buf: 0x%08X, %d\n", buf, atoi(buf));
            }
            else {
                printf("read error...\n");
            }
        }
    }

    close(fd);
    return 0;
}
