/*
 * FIFO example using select.
 *
 * $ mkfifo /tmp/fifo
 * $ clang -Wall -o test ./test.c
 * $ ./test &
 * $ echo 'hello' > /tmp/fifo
 * $ echo 'hello world' > /tmp/fifo
 * $ killall test
 */

#include <sys/types.h>
#include <sys/select.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd;
    int n;
    fd_set set;
    ssize_t bytes;
    size_t total_bytes;
    char buf[1024];

    fd = open("/tmp/fifo", O_RDWR | O_NONBLOCK);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    FD_ZERO(&set);
    FD_SET(fd, &set);

    for (;;) {
        n = select(fd+1, &set, NULL, NULL, NULL);
        if (!n)
            continue;
        if (n == -1) {
            perror("select");
            return EXIT_FAILURE;
        }
        if (FD_ISSET(fd, &set)) {
            printf("Descriptor %d is ready.\n", fd);
            total_bytes = 0;
            for (;;) {
                bytes = read(fd, buf, sizeof(buf));
                if (bytes > 0) {
                    total_bytes += (size_t)bytes;
                } else {
                    if (errno == EWOULDBLOCK) {
                        /* Done reading */
                        printf("done reading (%lu bytes)\n", total_bytes);
                        break;
                    } else {
                        perror("read");
                        return EXIT_FAILURE;
                    }
                }
            }
        }
    }

    return EXIT_SUCCESS;
}