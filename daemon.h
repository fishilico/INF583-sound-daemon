#ifndef DAEMON_H
#define DAEMON_H

int daemon_fork();
int daemon_lock(const char* lockfile);
int daemon_new_session();
int daemon_dissociate_term(const char *logfile);
int daemon_create_pid_file(const char *pidfile);
int daemonize(const char* dirpath, const char* lockfile,
              const char *logfile, const char *pidfile);

#endif /* DAEMON_H */
