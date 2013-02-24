#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "daemon.h"

#define DAEMON_DIRECTORY "."
#define DAEMON_LOCKFILE "daemon.lock"
#define DAEMON_LOGFILE "daemon.log"
#define DAEMON_PIDFILE "daemon.pid"
#define DAEMON_FIFOFILE "daemon.fifo"

#define LINE_MAXLEN 1024

/**
 * Read a line from the file descriptor into the buffer of size bufsize
 *
 * Strip final End-Of-Line character and fill buffer with a 0-terminated string.
 * Empty lines or too-long lines are dropped.
 * Return -1 if something went wrong, 0 if data in buffer is valid.
 * Return 1 if end of file is reached.
 */
int read_line(int fd, char *buffer, size_t bufsize)
{
    char c;
    int index = 0;
    int ret;
    while ((ret = read(fd, &c, 1)) == 1) {
        if (c == '\n' || c == '\r') {
            // Read an end of line
            if (index > 0) {
                buffer[index] = 0;
                return 0;
            }
            // Drop empty line by doing nothing
        } else if (index >= bufsize - 1) {
            // Drop too-long lines
            buffer[bufsize - 1] = 0;
            fprintf(stderr, "read_line: dropping line which is too long.\n"
                    "line was: %s\n", buffer);
            index = 0;
        } else {
            buffer[index] = c;
            index ++;
        }
    }
    // Something went wrong here, end of file or error
    if (ret == -1) {
        perror("read");
        return -1;
    } else if (index > 0) {
        // Return last line before end of file
        buffer[index] = 0;
        return 0;
    }
    // Return end of file code
    return 1;
}

/**
 * Daemon main function
 */
int main(int argc, char **argv)
{
    int ret = 0;

    // Daemonize program
    if (daemonize(DAEMON_DIRECTORY, DAEMON_LOCKFILE, DAEMON_LOGFILE, DAEMON_PIDFILE) == -1) {
        fprintf(stderr, "Daemonization failed\n");
        return 1;
    }

    // Create a FIFO, deleting an already-existing one
    ret = mkfifo(DAEMON_FIFOFILE, S_IRUSR|S_IWUSR);
    if (ret == -1 && errno == EEXIST) {
        unlink(DAEMON_FIFOFILE);
        ret = mkfifo(DAEMON_FIFOFILE, S_IRUSR|S_IWUSR);
    }
    if (ret == -1) {
        perror("mkfifo");
        return 1;
    }

    printf("[main] >> Daemon is running :)\n");
    fflush(stdout);

    // Main loop
    char line[LINE_MAXLEN + 1];
    while (ret == 0) {
        // Open the FIFO
        int fifo = open(DAEMON_FIFOFILE, O_RDONLY);
        if (fifo == -1) {
            perror("open(fifo)");
            ret = 1;
            break;
        }

        // Read lines
        int fifo_status = 0;
        while ((fifo_status = read_line(fifo, line, sizeof(line))) == 0) {
            printf("Reading line: %s\n", line);
            fflush(stdout);
        }

        if (fifo_status == -1) {
            fprintf(stderr, "Daemon exits now because read_line had a problem\n");
            ret = 1;
        }

        // Got an error or an end of file, close FIFO
        close(fifo);
    }

    unlink(DAEMON_FIFOFILE);
    printf("[main] << Daemon now exits with value %d\n", ret);
    unlink(DAEMON_PIDFILE);
    unlink(DAEMON_LOCKFILE);
    return ret;
}
