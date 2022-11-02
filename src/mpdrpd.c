#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "discord.h"
#include "configuration.h"
#include <pthread.h>

#include <mpd/client.h>
#include <mpd/connection.h>

void* mpd_thread(void* mpd_conn) {
    struct mpd_connection* mpd = (struct mpd_connection*)mpd_conn;

    uint32_t mpdrpd_flags = 1;
    // mpdrpd_flags |= MPDRPD_HIDE_PAUSED;

    int error = 0;
    struct mpd_status* status = NULL;
    struct mpd_song* song = NULL;
    while (mpd_send_idle(mpd) && !error) {
        enum mpd_idle event = mpd_recv_idle(mpd, 1);
        
        if (event != MPD_IDLE_PLAYER) 
            continue;

        status = mpd_run_status(mpd);
        if (status == NULL) {
            error = 1;
            return NULL;
        }
        song = mpd_run_current_song(mpd);
        if (song == NULL) {
            error = 1;
            mpd_status_free(status);
            return NULL;
        }
        enum mpd_state state = mpd_status_get_state(status);

        int update_errcode = mpdrpd_discord_update(status, song, state, mpdrpd_flags);
        if (update_errcode != 0) {
            printf("[mpdrpd] error when updating discord: %d\n", update_errcode);
            error = 2;
        }
        
        //cleanup allocated memory
        mpd_status_free(status);
        status = NULL;
        mpd_song_free(song);
        song = NULL;
    }
    return NULL;
}

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

    DiscordEventHandlers handlers = {
        .disconnected = handle_discord_disconnected,
        .errored = handle_discord_error,
        .joinGame = handle_discord_join,
        .joinRequest = handle_discord_join_request,
        .ready = handle_discord_ready,
        .spectateGame = handle_discord_spectate
    };

    Discord_Initialize(MPDRPD_APPLICATION_ID, &handlers, 1, NULL);
    printf("[discord] initialized\n");

    pthread_t mpd_thr;
    pthread_create(&mpd_thr, NULL, mpd_thread, (void*)mpd);
    pthread_join(mpd_thr, NULL);

    Discord_Shutdown();
    mpd_connection_free(mpd);
    return 0;
}  