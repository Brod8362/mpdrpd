#include "discord.h"
#include "lookups.h"

#include <stdio.h>
#include <string.h>

int mpdrpd_discord_update(struct mpd_status* status, struct mpd_song* song, enum mpd_state state) {
    DiscordRichPresence rp;

    if (state == MPD_STATE_STOP || state == MPD_STATE_UNKNOWN) {
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
        
        rp.details = "Song Title - Song Artist";
        rp.startTimestamp = epoch_started;
        rp.endTimestamp = epoch_will_end;
        rp.largeImageKey = "placeholder";
        rp.largeImageText = "Song Title - Song Artist";
        rp.smallImageKey= state_str.small_image_key;
        rp.smallImageText = state_str.state_string;
        rp.partySize = song_index+1;
        rp.partyMax = queue_length;
        rp.state = state_str.state_string;

        Discord_UpdatePresence(&rp);
        printf("[discord] updated\n");
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