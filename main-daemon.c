#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "daemon.h"

#define DAEMON_DIRECTORY "."
#define DAEMON_LOCKFILE "daemon.lock"
#define DAEMON_LOGFILE "daemon.log"
#define DAEMON_PIDFILE "daemon.pid"

int main(int argc, char **argv) {
    if (daemonize(DAEMON_DIRECTORY, DAEMON_LOCKFILE, DAEMON_LOGFILE, DAEMON_PIDFILE) == -1) {
        fprintf(stderr, "Daemonization failed\n");
        return 1;
    }
    printf("[main] >> Daemon is running :)\n");
    sleep(10);
    printf("[main] << Daemon now exits\n");
    return 0;
}
