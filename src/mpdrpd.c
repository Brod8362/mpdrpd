#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "discord.h"
#include "configuration.h"
#include <pthread.h>

#include <mpd/client.h>
#include <mpd/connection.h>

#include <getopt.h>

struct thread_arg {
    struct mpd_connection* mpd;
    const uint32_t flags;
};

void* mpd_thread(void* arg_raw) {
    struct thread_arg* arg = (struct thread_arg*)arg_raw;

    struct mpd_connection* mpd = arg->mpd;
    const uint32_t mpdrpd_flags = arg->flags;

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
    uint32_t mpdrpd_flags = 0;
    char* host = "127.0.0.1";
    int port = 6600;

    char opt;
    while ((opt = getopt(argc, argv, "sSp:h:")) != -1) {
        switch (opt) {
            case 'p': //port
                port = atoi(optarg);
                break;
            case 'h': //host
                host = optarg;
                break;
            case 's':
                mpdrpd_flags |= MPDRPD_HIDE_PAUSED;
                break;
            case 'S':
                mpdrpd_flags ^= MPDRPD_HIDE_PAUSED;
                break;
            case '?':
                printf("huh??\n");
                break;
            default:
                abort();
        }
    }

    printf("connecting to mpd @ %s:%d\n", host, port);
    struct mpd_connection* mpd = mpd_connection_new(host, port, 5000);

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

    struct thread_arg ta = {
        .mpd = mpd,
        .flags = mpdrpd_flags
    };

    pthread_t mpd_thr;
    pthread_create(&mpd_thr, NULL, mpd_thread, (void*)&ta);
    pthread_join(mpd_thr, NULL);

    Discord_Shutdown();
    mpd_connection_free(mpd);
    return 0;
}  