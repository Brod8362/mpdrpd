#include "configuration.h"
#include <string.h>
#include <stdlib.h>

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

static struct key_flag_pair config_pairs[] = {
	{ .key = "hide_on_pause", .flag = MPDRPD_HIDE_PAUSED }
};


static void split_key_value(const char* src, char* key_buf, size_t key_size, char* value_buf, size_t value_size) {
	size_t src_length = strlen(src);
	for (size_t i = 0; i < src_length; i++) {
		if (src[i] == '=') {
			strncpy(key_buf, src, MIN(key_size, i)); 
			(void)value_buf; //temporary
			(void)value_size;
			//TODO copy other
			break;
		}
	}
}

int parse_config(FILE* fd, uint32_t* flags) {
	char* line_buffer = NULL;
	size_t buffer_size = 0;
	char key_buffer[SUB_BUFFER_SIZE];
	char value_buffer[SUB_BUFFER_SIZE];
	memset(key_buffer, 0, SUB_BUFFER_SIZE);
	memset(value_buffer, 0, SUB_BUFFER_SIZE);
	
	while (getline(&line_buffer, &buffer_size, fd) > 0) {
			split_key_value(line_buffer, key_buffer, SUB_BUFFER_SIZE, value_buffer, SUB_BUFFER_SIZE);
			printf("k:%s, v:%s\n", key_buffer, value_buffer);
	}
	free(line_buffer);
	return 0;		
}
