#!/bin/sh
#
# Startup script for the radio controller daemon
#
# chkconfig: – 50 50
# description: control mpd via mouse
# processname: collectd
# pidfile: /opt/collectd/var/run/collectd.pid
# command: /usr/local/bin/radio-controller
#PID_FILE=/var/run/radio-controller
CMD_FILE=/usr/local/bin/radio-controller

case "$1" in
    start)
        # Starts the collectd deamon
        echo "Starting radio-controller"
        $CMD_FILE
        ;;

    stop)
        # stops the daemon bt cat’ing the pidfile
        echo "stopping radio-controller"
        killall -9 radio-controller
        ;;

    restart)
        ## Stop the service regardless of whether it was
        ## running or not, start it again.
        echo "Restarting radio-controller"
        $0 stop
        $0 start
        ;;

    *)
        echo "Usage: radio-controller (start|stop|restart)"
        exit 1
        ;;
esac
