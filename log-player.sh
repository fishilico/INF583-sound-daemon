#!/bin/sh
# Watch player daemon log when it is running

exec tail -F "daemon.log"
