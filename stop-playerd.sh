#!/bin/sh
# Stop playerd daemon

PIDFILE="daemon.pid"
if [ ! -f "$PIDFILE" ]
then
    echo "PID file $PIDFILE does not exist"
    exit
fi

PID=`cat "$PIDFILE"`
if ! kill -0 $PID 2> /dev/null
then
    echo "Daemon is no longer running"
else
    echo "Sending SIGTERM to the daemon"
    kill -TERM $PID
fi
