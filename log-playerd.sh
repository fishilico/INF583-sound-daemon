#!/bin/sh
# Watch playerd daemon log when it is running

exec tail -F "daemon.log"
