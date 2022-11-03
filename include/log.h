#ifndef MPDRPD_LOG_H
#define MPDRPD_LOG_H

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_CRITICAL 4

void mpdrpd_log(const int level, char* msg);

void mpdrpd_set_log_level(const int level);


#endif