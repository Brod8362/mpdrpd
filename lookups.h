#ifndef MPDRPD_LOOKUPS_H
#define MPDRPD_LOOKUPS_H

struct state_strings {
    const char* state_string;
    const char* small_image_key;
};

struct state_strings state_lookup_table[] = {
    {.state_string = "Unknown", "unk"}, //Unknown
    {.state_string = "Stopped", "stop"}, //Stop
    {.state_string = "Playing", "play"}, //Play
    {.state_string = "Paused", "pause"}  //Pause
};

#endif