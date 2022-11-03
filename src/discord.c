#include "discord.h"
#include "lookups.h"

#include <stdio.h>
#include <string.h>
#include "configuration.h"

static void ellipse_copy(char* dest, const char* src, size_t output_size) {
    if (strlen(src) >= output_size) {
        memcpy(dest, src, output_size-4);
        strcat(dest, "...");
    } else {
        memset(dest, 0, output_size);
        strncpy(dest, src, output_size);
    }
}

int mpdrpd_discord_update(struct mpd_status* status, struct mpd_song* song, enum mpd_state state, const uint32_t flags) {
    DiscordRichPresence rp;
    char song_details[128];
    char song_final[62];
    char artist_final[62];
    
    if (state == MPD_STATE_STOP || state == MPD_STATE_UNKNOWN) {
        Discord_ClearPresence();
    } else if (state == MPD_STATE_PAUSE && (flags & MPDRPD_HIDE_PAUSED) == 1) {
        Discord_ClearPresence();
    } else {
        memset(&rp, 0, sizeof(rp));
        time_t epoch = time(NULL);
        unsigned int elapsed = mpd_status_get_elapsed_time(status);
        unsigned int duration = mpd_song_get_duration(song);

        unsigned long epoch_started = epoch-elapsed;
        unsigned long epoch_will_end = epoch_started+duration;

        struct state_strings state_str = state_lookup_table[state];

        unsigned int queue_length = mpd_status_get_queue_length(status);
        unsigned int song_index = mpd_status_get_song_pos(status);
        
        const char* song_title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
        const char* song_artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);

        if (song_title == NULL || song_artist == NULL) {
            ellipse_copy(song_details, mpd_song_get_uri(song), 128);
        } else {
            ellipse_copy(song_final, song_title, 62);
            ellipse_copy(artist_final, song_artist, 62);
            snprintf(song_details, 128, "%s - %s", song_final, artist_final);
        }

        rp.details = song_details;
        rp.largeImageKey = "placeholder";
        rp.largeImageText = "using mpdrpd";
        rp.smallImageKey= state_str.small_image_key;
        rp.smallImageText = state_str.state_string;
        if (state == MPD_STATE_PLAY) {
            rp.startTimestamp = epoch_started;
            rp.endTimestamp = epoch_will_end;
        } else if (state == MPD_STATE_PAUSE) {
            rp.endTimestamp = 0;
            rp.startTimestamp = 0;
        }
        rp.partySize = song_index+1;
        rp.partyMax = queue_length;
        rp.state = state_str.state_string;

        Discord_UpdatePresence(&rp);
    }

    return 0;
}

void handle_discord_ready(const DiscordUser* connected_user) {
    printf("[discord] connected (%s#%s)\n", connected_user->username, connected_user->discriminator);
}

void handle_discord_disconnected(int errcode, const char* message) {
    printf("[discord] disconnected (%d) %s", errcode, message);
}

void handle_discord_error(int errcode, const char* message) {
    printf("[discord] error (%d) %s\n", errcode, message);
}

void handle_discord_join(const char* _secret) {
    //to suppress "unused parameter" warnings from gcc
    (void)_secret;
}

void handle_discord_spectate(const char* _secret) {
    (void)_secret;
}

void handle_discord_join_request(const DiscordUser* _request) {
    (void)_request;
}