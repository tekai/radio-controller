#!/bin/sh
### BEGIN INIT INFO
# Provides:          radio-controller
# Required-Start:    $local_fs $network
# Required-Stop:     $local_fs
# Should-Start:      mpd
# Should-Stop:       mpd
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: radio-controller
# Description:       control mpd via mouse
### END INIT INFO
#PID_FILE=/var/run/radio-controller
CMD_FILE=/usr/local/bin/radio-controller

case "$1" in
    start)
        # Starts the deamon
        echo "Starting radio-controller"
        $CMD_FILE
        ;;

    stop)
        # stops the daemon
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
