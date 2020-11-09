#ifndef DEBUG_H
#define DEBUG_H

#include "general.h"
#include <string.h>
#include <errno.h>

#define LOG_LEVELS_LIST(o) \
  o(ERROR)		   \
  o(WARNING)		   \
  o(INFO)

enum log_levels {
    LOG_LEVELS_LIST(MAKE_ENUM)
};

extern const char *kLogLevelNames[];

void q_log(enum log_levels lev, char *subsystem, char *pattern, ...);

#define q_error(subsys, format, ...)  q_log(ERROR, subsys, format, ##__VA_ARGS__)
#define q_warning(subsys, format, ...)  q_log(WARNING, subsys, format, ##__VA_ARGS__)
#define q_info(subsys, format, ...)  q_log(INFO, subsys, format, ##__VA_ARGS__)
#define q_perror(subsys, format, ...)  q_log(ERROR, subsys, format ": %s",  ##__VA_ARGS__, strerror(errno))
#endif
