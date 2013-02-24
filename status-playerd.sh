#!/bin/sh
# Get playerd daemon status

PIDFILE="daemon.pid"
if [ ! -f "$PIDFILE" ]
then
    echo "PID file $PIDFILE does not exist"
    exit 1
fi

PID=`cat "$PIDFILE"`
if kill -0 $PID 2> /dev/null
then
    echo "Daemon is running with PID $PID"
else
    echo "Daemon is stopped"
fi
