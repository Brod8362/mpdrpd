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

static void string_strip(char* src, const size_t length) {
	size_t effective_length = length == 0 ? strlen(src): length;
	size_t beginning = 0, end = length;
	//find first non-space character
	for (size_t i = 0; i < effective_length; i++) {
		if (!isspace(src[i])) {
			beginning = i;
			break;
		}
	}
	//find end
	for (size_t i = effective_length-1; i >= beginning; i--) {
		if (!isspace(src[i])) {
			end = i;
			break;
		}
	}
	memmove(src, src+beginning, end-beginning+1);
	src[end-beginning+1] = 0;
}


static void split_key_value(const char* src, char* key_buf, size_t key_size, char* value_buf, size_t value_size) {
	size_t src_length = strlen(src);
	memset(key_buf, 0,key_size);
	memset(value_buf, 0, value_size);
	for (size_t i = 0; i < src_length; i++) {
		if (src[i] == '=') {
			strncpy(key_buf, src, i);
			strncpy(value_buf, src+i+1, src_length-i);
			string_strip(key_buf, 0);
			string_strip(value_buf, 0);
			break;
		}
	}
}

int parse_config(FILE* fd, uint32_t* flags) {
	char* line_buffer = NULL;
	size_t buffer_size = 0;
	char key_buffer[SUB_BUFFER_SIZE];
	char value_buffer[SUB_BUFFER_SIZE];
	
	while (getline(&line_buffer, &buffer_size, fd) > 0) {
		if (is_empty(line_buffer)) continue;
		split_key_value(line_buffer, key_buffer, SUB_BUFFER_SIZE, value_buffer, SUB_BUFFER_SIZE);
		for (size_t i = 0; i < sizeof(config_pairs)/sizeof(config_pairs[0]); i++) {
			const struct key_flag_pair* kf = &config_pairs[i];
			if (strcmp(key_buffer, kf->key) == 0) {
				if (strcmp(value_buffer, "true") == 0) { //set flag
					(*flags) |= kf->flag;
				} else if (strcmp(value_buffer, "false") == 0) { //clear flag
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
