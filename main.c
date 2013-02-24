#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "daemon.h"

int main(int argc, char **argv) {
    if (daemonize(".", "daemon.lock", "daemon.log") == -1) {
        fprintf(stderr, "Daemonization failed\n");
        return 1;
    }
    printf("[main] >> Daemon is running :)\n");
    sleep(10);
    printf("[main] << Daemon now exits\n");
    return 0;
}
