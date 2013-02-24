#!/bin/sh
# Run strace on the daemon, for debugging purpose

PIDFILE="daemon.pid"
if [ ! -f "$PIDFILE" ]
then
    echo "PID file $PIDFILE does not exist"
    exit 1
fi

exec strace -p `cat "$PIDFILE"`
