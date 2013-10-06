#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/input.h>
#include <fcntl.h>
#include <stdbool.h>

#define MOUSEFILE "/dev/input/event0"

int main() {
    int fd;
    bool running = false;
    struct input_event ie;
    struct timeval t;
    if ((fd = open(MOUSEFILE, O_RDONLY)) == -1) {
        perror("opening device");
        exit(EXIT_FAILURE);
    }

    fd_set set;
    int rv;
    FD_ZERO(&set);
    FE_SET(fd, &fdset);
    t.tv_sec = 0;
    t.tv_usec = 200000; // 200k microseconds

    while (read(fd, &ie, sizeof(struct input_event))) {

        if (ie.type == EV_KEY && ie.code == BTN_LEFT && ie.value == 1) {

            rv = select(1, fd, NULL, NULL, t);
            if(rv == -1) { // error
                perror("select"); 
            }
            else if(rv == 0) { // timeout
            }
            else {
                (read(fd, &ie, sizeof(struct input_event))) {
            }
            printf("input time: %ld.%03ld, type: %d, code: %d, value: %d\n",
                   ie.time.tv_sec, ie.time.tv_usec,
                   ie.type, ie.code, ie.value);

            if (running == false) {
                running = true;
            }
            else {
                running = false;
            }
            printf("running %s\n", running ? "playing" : "stopped");
        }

    }
    return 0;
}
