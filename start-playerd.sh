#!/bin/sh
# Start playerd daemon

PIDFILE="daemon.pid"
if ! ./playerd
then
    echo >&2 "Daemon failed to start"
fi
echo "Daemon started with PID "`cat "$PIDFILE"`
