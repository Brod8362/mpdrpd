#include <stdio.h>
#include <stdlib.h>

#include "discord.h"

#include <mpd/client.h>
#include <mpd/connection.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s [hostname] [port]\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[2]);

    struct mpd_connection* mpd = mpd_connection_new(argv[1], port, 5000);

    if (mpd == NULL) {
        fprintf(stderr, "failed to connect to mpd\n");
        return 1;
    }

    mpdrpd_discord_init();

    //TODO: is this okay?
    int error = 0;
    while (mpd_send_idle(mpd) && !error) {
        enum mpd_idle event = mpd_recv_idle(mpd, 1);
        switch (event) {
            case MPD_IDLE_PLAYER:
                struct mpd_status* status = mpd_run_status(mpd);
                if (status == NULL) {
                    error = 1;
                    break;
                }
                struct mpd_song* song = mpd_run_current_song(mpd);
                if (song == NULL) {
                    error = 1;
                    mpd_status_free(status);
                    break;
                }
                enum mpd_state state = mpd_status_get_state(status);

                mpdrpd_discord_update(status, song, state);
                
                //cleanup allocated memory
                mpd_status_free(status);
                mpd_song_free(song);
                break;
            default:
                break;
        }
    }

    mpd_connection_free(mpd);
    return 0;
}  