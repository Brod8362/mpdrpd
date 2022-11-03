#ifndef MPDRPD_CONFIG_H
#define MPDRPD_CONFIG_H

#define MPDRPD_HIDE_PAUSED 1

#include <stdint.h>
#include <stdio.h>

extern const char* config_file_paths[];
extern const size_t config_file_path_size;

int parse_config(FILE* fd, uint32_t* flags);

#endif
