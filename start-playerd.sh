#!/bin/sh
# Start playerd daemon

PIDFILE="daemon.pid"
if ! ./playerd
then
    echo >&2 "Daemon failed to start"
    exit 1
fi
echo "Daemon started with PID "`cat "$PIDFILE"`
