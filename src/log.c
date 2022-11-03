#include "log.h"
#include <stdio.h>
#include <time.h>

static const char* log_lookup_table[5] = {
    "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"
};

static int current_log_level = LOG_LEVEL_INFO;

void mpdrpd_log(const int level, char* msg) {
    if (level >= current_log_level) {
        char time_str[64];
        time_t current_time = time(NULL);
        struct tm* tm = localtime(&current_time);
        strftime(time_str, sizeof(time_str), "%m/%d/%y %H:%M:%S", tm);
        printf("%s [%s] %s\n", time_str, log_lookup_table[level], msg);
    }
}

void mpdrpd_set_log_level(const int level) {
    current_log_level = level;
}