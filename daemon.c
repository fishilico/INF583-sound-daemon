#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "daemon.h"

/**
 * Detach from current process and run in background
 * @return 0 for the daemon, 1 for the parent, -1 is an error occured
 */
int daemon_fork()
{
    // First fork
    int pid = fork();
    int status;
    switch (pid) {
        case -1:
            perror("Fork failed");
            return -1;

        case 0:
            // Child
            break;

        default:
            // Parent
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                return -1;
            }
            if (status != 0) {
                fprintf(stderr, "Error status from child process: %d\n", status);
                return -1;
            }
            return 1;
    }

    // Second fork
    pid = fork();
    switch (pid) {
        case -1:
            perror("Fork failed");
            return -1;

        case 0:
            // Child which is the daemon
            break;

        default:
            // Daemon's parent, who dies without waiting for its child
            //printf("[daemon_fork] Parent of the child process now exits\n");
            exit(0);
    }
    return 0;
}

/**
 * Lock a file
 * @return FD to locked file if successful, -2 if the file is already locked,
 * -1 otherwise
 */
int daemon_lock(const char* lockfile)
{
    int l_fd = open(lockfile, O_RDWR | O_CREAT, 0640);
    if (l_fd == -1) {
        perror("open");
        return -1;
    }
    if (flock(l_fd, LOCK_EX | LOCK_NB) == -1) {
        if (errno == EWOULDBLOCK) {
            return -2;
        } else {
            perror("flock");
        }
        return -1;
    }
    return l_fd;
}

/**
 * Create a new session
 */
int daemon_new_session()
{
    // Create a new session
    pid_t sid = setsid();
    if (sid == -1) {
        perror("setsid");
        return -1;
    }
    if (sid != getpid()) {
        fprintf(stderr, "Session creation failed: sessions %d for PID %d\n",
            sid, getpid());
        return -1;
    }
    return 0;
}

/**
 * Dissociate from terminal
 */
int daemon_dissociate_term(const char *logfile)
{
    int ret = 0;
    int logfd = open(logfile, O_WRONLY | O_CREAT | O_TRUNC);
    if (logfd == -1) {
        perror("open(log)");
        return -1;
    }
    int zerofd = open("/dev/zero", O_RDONLY);
    if (zerofd == -1) {
        perror("open(/dev/zero)");
        close(logfd);
        return -1;
    }


    if (dup2(zerofd, 0) == -1) { // IN
        perror("dup2(in)");
        ret = -1;
    } else if (dup2(logfd, 1) == -1) { // OUT
        perror("dup2(out)");
        ret = -1;
    } else if (dup2(logfd, 2) == -1) { // ERR
        perror("dup2(err)");
        ret = -1;
    }
    close(zerofd);
    close(logfd);
    return ret;
}

/**
 * Create a PID file
 */
int daemon_create_pid_file(const char *pidfile)
{
    char buffer[50];
    int len = sprintf(buffer, "%d\n", getpid());
    FILE *f = fopen(pidfile, "w");
    if (!f) {
        perror("fopen(pidfile)");
        return -1;
    }
    if (fwrite(buffer, 1, len, f) != len) {
        perror("fwrite(pidfile)");
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

/**
 * Call all other functions
 * Return 0 for the daemon, 1 for the parent, -1 if fork failed.
 */
int daemonize(const char* dirpath, const char* lockfile,
              const char *logfile, const char *pidfile)
{
    // Try to lock file to see if there already is a daemon
    int lockfd = daemon_lock(lockfile);
    if (lockfd == -1) {
        return -1;
    } else if (lockfd == -2) {
        printf("Daemon is already running, don't start it.\n");
        return 1;
    }

    // Fork
    int ret = daemon_fork();
    if (ret == -1) return -1;
    else if (ret == 0) {
        // Set up daemon-specific things
        // Set current working directory
        if (dirpath != NULL && chdir(dirpath) == -1) {
            perror("chdir");
            exit(1);
        }
        // Save PID
        if (pidfile != NULL && daemon_create_pid_file(pidfile) == -1) {
            exit(1);
        }
        // Create a new session
        if (daemon_new_session() == -1) {
            exit(1);
        }
        // Dissociate from terminal
        if (daemon_dissociate_term(logfile ? logfile : "/dev/null") == -1) {
            exit(1);
        }
    } else {
        printf("Daemon is started.\n");
    }
    return ret;
}

