#include "configuration.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "log.h"

#define SUB_BUFFER_SIZE 96

#define MIN(a,b) (a < b ? a : b)

const char* config_file_paths[] = {
	"/etc/mpdrpd.conf",
	"$HOME/.config/mpdrpd/mpdrpd.conf"
};

const size_t config_file_path_size = 2;

struct key_flag_pair {
	const char* key;
	uint32_t flag;
};

static const struct key_flag_pair config_pairs[] = {
	{ .key = "hide_on_pause", .flag = MPDRPD_HIDE_PAUSED }
};


static int is_empty(const char* c) {
	for (size_t i = 0; i < strlen(c); i++) {
		if (!isspace(c[i])) {
			return 0;
		}
	}
	return 1;
}

static char* string_strip(char* src) {
	const size_t length = strlen(src);
	size_t beginning = 0, end = length-1;
	while (isspace(src[beginning]) && beginning < length) beginning++;
	while (isspace(src[end]) && end > beginning) end--;
	src[end+1] = 0;
	return src+beginning;
}

static int split_key_value(char* src, char** key, char** value) {
	size_t src_length = strlen(src);
	for (size_t i = 0; i < src_length; i++) {
		if (src[i] == '=') {
			src[i] = 0;
			*key = string_strip(src);
			*value = string_strip(src+i+1);
			return 0;
		}
	}
	*key = NULL;
	*value = NULL;
	return -1;
}

int parse_config(FILE* fd, uint32_t* flags) {
	char* line_buffer = NULL;
	size_t buffer_size = 0;
	char* key = NULL;
	char* value = NULL;
	
	while (getline(&line_buffer, &buffer_size, fd) > 0) {
		if (is_empty(line_buffer)) continue;
		if (split_key_value(line_buffer, &key, &value) != 0) {
			//failed to parse line
			mpdrpd_log(LOG_LEVEL_WARNING, "failed to parse config line");
			continue;
		}
		
		for (size_t i = 0; i < sizeof(config_pairs)/sizeof(config_pairs[0]); i++) {
			const struct key_flag_pair* kf = &config_pairs[i];
			if (strcmp(key, kf->key) == 0) {
				if (strcmp(value, "true") == 0) { //set flag
					(*flags) |= kf->flag;
				} else if ((strcmp(value, "false") == 0)) { //clear flag
					(*flags) &= ~(kf->flag);
				} else {
					mpdrpd_log(LOG_LEVEL_WARNING, "invalid config value");
				}
			}
		}	
	}
	free(line_buffer);
	return 0;		
}
