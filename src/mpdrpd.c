#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "discord.h"
#include "configuration.h"
#include "log.h"

#include <mpd/client.h>
#include <mpd/connection.h>

#include <getopt.h>
#include <wordexp.h>

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
    mpdrpd_log(LOG_LEVEL_INFO, "entering mpd idle loop");
    while (mpd_send_idle(mpd) && !error) {
        enum mpd_idle event = mpd_recv_idle(mpd, 1);
        mpdrpd_log(LOG_LEVEL_DEBUG, "event received");
        
        if (event != MPD_IDLE_PLAYER) 
            continue;

        status = mpd_run_status(mpd);
        if (status == NULL) {
            mpdrpd_log(LOG_LEVEL_ERROR, "failed to get status in event loop");
            error = 1;
            return NULL;
        }
        song = mpd_run_current_song(mpd);
        if (song == NULL) {
            mpdrpd_log(LOG_LEVEL_ERROR, "failed to get current song in event loop");
            error = 1;
            mpd_status_free(status);
            return NULL;
        }
        enum mpd_state state = mpd_status_get_state(status);

        int update_errcode = mpdrpd_discord_update(status, song, state, mpdrpd_flags);
        if (update_errcode != 0) {
            mpdrpd_log(LOG_LEVEL_ERROR, "failed to update presence in event loop");
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

	for (size_t i = 0; i < config_file_path_size; i++) {
		wordexp_t we;
		const char* file_path = config_file_paths[i];
		if (wordexp(file_path, &we, 0) != 0) {
			mpdrpd_log(LOG_LEVEL_ERROR, "error performing shell expansion");
			wordfree(&we);
			return 1;
		}
		mpdrpd_log(LOG_LEVEL_DEBUG, "trying config file...");
		FILE* fd = fopen(we.we_wordv[0], "r");
		wordfree(&we);
		if (fd != NULL) {
			if (parse_config(fd, &mpdrpd_flags) != 0) {
				mpdrpd_log(LOG_LEVEL_ERROR, "error parsing config file at");
			}
            fclose(fd);
			break;
		}
	}

    char* host = "127.0.0.1";
    int port = 6600;

    char opt;
    while ((opt = getopt(argc, argv, "sSlp:h:")) != -1) {
        switch (opt) {
            case 'p': //port
                port = atoi(optarg);
                break;
            case 'h': //host
                host = optarg;
                break;
            case 's': //hide on pause
                mpdrpd_flags |= MPDRPD_HIDE_PAUSED;
                break;
            case 'S': //don't hide on pause (default)
                mpdrpd_flags &= ~(MPDRPD_HIDE_PAUSED);
                break;
            case 'l': // log level
                mpdrpd_set_log_level(atoi(optarg));
                break;
            case '?':
                printf("huh??\n");
                break;
            default:
                abort();
        }
    }
    mpdrpd_log(LOG_LEVEL_INFO, "connecting to mpd");
    // printf("connecting to mpd @ %s:%d\n", host, port);
    struct mpd_connection* mpd = mpd_connection_new(host, port, 5000);

    if (mpd == NULL) {
        mpdrpd_log(LOG_LEVEL_CRITICAL, "failed to connect to mpd");
        return 1;
    }
    mpdrpd_log(LOG_LEVEL_INFO, "connected to mpd");

    DiscordEventHandlers handlers = {
        .disconnected = handle_discord_disconnected,
        .errored = handle_discord_error,
        .joinGame = handle_discord_join,
        .joinRequest = handle_discord_join_request,
        .ready = handle_discord_ready,
        .spectateGame = handle_discord_spectate
    };

    Discord_Initialize(MPDRPD_APPLICATION_ID, &handlers, 1, NULL);
    mpdrpd_log(LOG_LEVEL_INFO, "discord initialized");

    struct thread_arg ta = {
        .mpd = mpd,
        .flags = mpdrpd_flags
    };

    pthread_t mpd_thr;
    mpdrpd_log(LOG_LEVEL_DEBUG, "spawning mpd worker thread");
    pthread_create(&mpd_thr, NULL, mpd_thread, (void*)&ta);
    pthread_join(mpd_thr, NULL);

    mpdrpd_log(LOG_LEVEL_INFO, "discord shutting down");
    Discord_Shutdown();
    mpd_connection_free(mpd);
    return 0;
}  
