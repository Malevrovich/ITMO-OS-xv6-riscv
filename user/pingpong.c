#include "kernel/types.h"
#include "user/user.h"

int
main(void) 
{
    int p[2];
    char buf[5];

    pipe(p);

    if (fork() == 0) {
        read(p[0], buf, sizeof(buf));
        printf("%d: got %s\n", getpid(), buf);

        write(p[1], "pong", sizeof(buf));
    } else {
        write(p[1], "ping", 5);

        read(p[0], buf, sizeof(buf));
        printf("%d: got %s\n", getpid(), buf);
    }

    exit(0);
}