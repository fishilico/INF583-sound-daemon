#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
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
int read_line(int fd, char *buffer, size_t bufsize, const char *prompt)
{
    char c;
    int index = 0;
    int ret;
    if (prompt != NULL) {
        fprintf(stdout, prompt);
        fflush(stdout);
    }
    while ((ret = read(fd, &c, 1)) == 1) {
        if (c == '\n' || c == '\r') {
            // Read an end of line
            if (index > 0) {
                buffer[index] = 0;
                return 0;
            }
            // Drop empty line by doing nothing but rewrite the prompt
            if (prompt != NULL) {
                fprintf(stdout, prompt);
                fflush(stdout);
            }
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
 * Entry point of the program. Try to launch a daemon and then connect to it.
 */
int main(int argc, char **argv)
{
    int ret = 0;

    // Invoke a daemon. prog = 0 in daemon, 1 in parent process (= interface)
    int prog = daemonize(DAEMON_DIRECTORY, DAEMON_LOCKFILE, DAEMON_LOGFILE, DAEMON_PIDFILE);
    if (prog == -1) {
        fprintf(stderr, "Daemonization failed\n");
        return 1;
    }


    // Both daemon and parent try to create a named pipe.
    // Only the first mkfifo succeeds.
    if (mkfifo(DAEMON_FIFOFILE, S_IRUSR|S_IWUSR) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            return 1;
        }
    }

    if (prog == 0) {
        // Daemon process
        printf("[main] >> Daemon is running :)\n");
        fflush(stdout);

        // Main loop
        char line[LINE_MAXLEN + 1];
        int running = 1;
        while (ret == 0 && running) {
            // Open the FIFO
            int fifo = open(DAEMON_FIFOFILE, O_RDONLY);
            if (fifo == -1) {
                perror("open(fifo)");
                ret = 1;
                break;
            }

            // Read lines
            int fifo_status = 0;
            while ((fifo_status = read_line(fifo, line, LINE_MAXLEN, NULL)) == 0) {
                printf("Reading line: %s\n", line);
                fflush(stdout);

                if (!strcasecmp(line, "exit")) {
                    running = 0;
                    break;
                }
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
    } else {
        // Interface process
        printf("[main] >> Interface is running :)\n");
        fflush(stdout);

        // Open the FIFO in write only
        int fifo = open(DAEMON_FIFOFILE, O_WRONLY);
        if (fifo == -1) {
            perror("open(fifo)");
            ret = 1;
        }

        // Read commands from standard input
        int stdin_status = 0;
        char line[LINE_MAXLEN + 1];
        const char *prompt = "Player> ";
        while ((stdin_status = read_line(STDIN_FILENO, line, LINE_MAXLEN, prompt)) == 0) {
            if (line[0] != '!') {
                // Send command
                int len = strlen(line);
                // As len < LINE_MAXLEN, len + 1 < sizeof(line) and there is no overflow
                line[len] = '\n';
                line[len + 1] = 0;
                write(fifo, line, len + 1);

                if (!strcasecmp(line, "exit\n")) {
                    // Quit everything
                    break;
                }
            } else if (!strcasecmp(line, "!exit")) {
                // Detach the interface by closing the FIFO
                break;
            }
        }

        if (stdin_status == -1) {
            fprintf(stderr, "Interface exits now because read_line had a problem\n");
            ret = 1;
        }
        close(fifo);
    }

    return ret;
}
