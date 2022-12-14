#ifndef MPDRPD_DISCORD_H
#define MPDRPD_DISCORD_H

#include <discord_rpc.h>
#include <mpd/status.h>
#include <mpd/song.h>
#include <stdint.h>

#define MPDRPD_APPLICATION_ID "514608595028148234"

int mpdrpd_discord_update(struct mpd_status* status, struct mpd_song* song, enum mpd_state state, const uint32_t flags);

void handle_discord_ready(const DiscordUser* connected_user);

void handle_discord_disconnected(int errcode, const char* message);

void handle_discord_error(int errcode, const char* message);

void handle_discord_join(const char* secret);

void handle_discord_spectate(const char* secret);

void handle_discord_join_request(const DiscordUser* request);

#endif