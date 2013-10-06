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

    if ((fd = open(MOUSEFILE, O_RDONLY)) == -1) {
        perror("yo opening device");
        exit(EXIT_FAILURE);
    }

    while (read(fd, &ie, sizeof(struct input_event))) {

        // printf("input type: %d, code: %ld, value: %d\n",
        //        ie.type, ie.code, ie.value);

        if (ie.type == EV_KEY && ie.code == BTN_LEFT && ie.value == 1) {
             if (running == false) {
                 running = (system("mpc -q play") == 0) ? true : false;
             }
             else {
                 system("mpc -q stop");
                 running = false;
             }
             printf("BBC 6 Music %s\n", running ? "playing" : "stopped");
        }

    }
    return 0;
}
