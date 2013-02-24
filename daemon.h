#ifndef DAEMON_H
#define DAEMON_H

int daemonize(const char* dirpath, const char* lockfile, const char *logfile);

#endif /* DAEMON_H */
