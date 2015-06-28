#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <syslog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <linux/input.h>
#include <fcntl.h>
#include <stdbool.h>

#include <mpd/client.h>
#include <mpd/status.h>
#include <mpd/entity.h>

#define MOUSEFILE "/dev/input/event0"

#define LOG_ERROR(x, ...) \
{\
    syslog(LOG_ERR, "ERROR " x "\n", __VA_ARGS__);          \
}

static bool mpd_is_playing(struct mpd_connection *conn) {
    struct mpd_status *status;

    bool playing;
    status = mpd_run_status(conn);
    if (!status) {
        LOG_ERROR("%s", mpd_connection_get_error_message(conn));
        playing = false;
    }
    else {
        playing = mpd_status_get_state(status) == MPD_STATE_PLAY
            ? true
            : false;

        mpd_status_free(status);
        mpd_response_finish(conn);
    }

    return playing;
}


static int mpd_toggle_play(struct mpd_connection *conn) {
    if (mpd_is_playing(conn))
        mpd_run_stop(conn);
    else
        mpd_run_play(conn);
    mpd_response_finish(conn);

    return 0;
}

static int mpd_next(struct mpd_connection *conn) {
      struct mpd_status *status;

      status = mpd_run_status(conn);
      if (!status) {
          LOG_ERROR("MPD: %s", mpd_connection_get_error_message(conn));
          return -1;
      }

      unsigned l = mpd_status_get_queue_length(status);
      if (l == 0) {
          LOG_ERROR("%s", "Queue is empty");
          mpd_status_free(status);
          mpd_response_finish(conn);
          return -1;
      }

      unsigned p = 0;
      if (mpd_status_get_state(status) == MPD_STATE_PLAY
              || mpd_status_get_state(status) == MPD_STATE_PAUSE) {

          p = mpd_status_get_song_pos(status);
      }
      mpd_status_free(status);

      if (p+1 < l)
          mpd_run_next(conn);
      else
          mpd_run_play_pos(conn, 0);


      mpd_response_finish(conn);

      return 0;
}

static int mpd_connect(struct mpd_connection **conn) {
    *conn = mpd_connection_new(NULL, 0, 30000);
    if (*conn == NULL) {
        LOG_ERROR("MPD: %s", "Out of memory");
        return -1;
    }
    if (mpd_connection_get_error(*conn) != MPD_ERROR_SUCCESS) {
        LOG_ERROR("MPD: %s", mpd_connection_get_error_message(*conn));
        mpd_connection_free(*conn);
        *conn = NULL;
        return -1;
    }
    return 0;
}



int main() {
    int fd;
    struct input_event ie;
    struct timeval timeout;
    struct timeval tStart;
    struct timeval tDiff;

    daemon(0, 0);
    openlog ("radio-controller", LOG_PID, LOG_DAEMON);

    if ((fd = open(MOUSEFILE, O_RDONLY)) == -1) {
        syslog(LOG_ERR, "opening device");
        exit(EXIT_FAILURE);
    }

    struct mpd_connection *conn = NULL;
    if ((mpd_connect(&conn)) == -1) {
        syslog(LOG_ERR, "could not connect to mpd");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_INFO, "MPD is %s\n", mpd_is_playing(conn) ? "playing" : "stopped");

    syslog(LOG_INFO, "Waiting for input...");
    fd_set master_set;
    fd_set read_set;
    int rv;
    FD_ZERO(&master_set);
    FD_SET(fd, &master_set);
    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);
    int clicks;

    while (read(fd, &ie, sizeof(struct input_event))) {
        if (ie.type == EV_KEY && ie.code == BTN_LEFT && ie.value == 1) {
            clicks = 1;
            timerclear(&tStart);
            timeradd(&tStart, &ie.time, &tStart);
            tStart.tv_sec = ie.time.tv_sec;
            tStart.tv_usec = ie.time.tv_usec;


            do {
                // linux modifies the timeout parameter, so lets reset it
                timeout.tv_sec = 0;
                timeout.tv_usec = 300000; // 300k microseconds
                FD_ZERO(&read_set);
                FD_SET(fd, &read_set);

                // wait until next event or timeout
                rv = select(FD_SETSIZE, &read_set, NULL, NULL, &timeout);

                if (rv == -1) {
                    syslog(LOG_ERR, "select");
                    break;
                }
                else if (rv == 0) {
                    /* syslog(LOG_INFO, "timeout"); */
                    break;
                }

                read(fd, &ie, sizeof(struct input_event));
                timerclear(&tDiff);
                timersub(&ie.time, &tStart, &tDiff);

                if (timercmp(&tDiff, &timeout, <)) {
                    // check for more clicks
                    if (ie.type == EV_KEY && ie.code == BTN_LEFT
                            && ie.value == 1) {
                        clicks++;
                    }
                }
                else {
                    break;
                }
            } while (rv > 0);


            mpd_run_status(conn);
            if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
                syslog(LOG_INFO, "reconnecting to MPD");
                mpd_connection_free(conn);
                mpd_connect(&conn);

            }
            if (clicks > 1) {
                if (mpd_is_playing(conn)) {
                    syslog(LOG_INFO, "playing next song");
                    mpd_next(conn);
                }
                else {
                    syslog(LOG_INFO, "MPD is not playing");
                }
            }
            else {
                if (mpd_is_playing(conn)) {
                    syslog(LOG_INFO, "stopping playback");
                }
                else {
                    syslog(LOG_INFO, "starting playback");
                }
                mpd_toggle_play(conn);
            }
        }
    }
    return 0;
}
