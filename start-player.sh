#!/bin/sh
# Start player daemon and interface
# Use Readline wrapper and ALSA-OSS programs

export RLWRAP_HOME=`pwd`
exec rlwrap aoss ./player
