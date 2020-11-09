#include <stdio.h>
#include <stdarg.h>

#include "debug.h"

const char *kLogLevelNames[] = {
    LOG_LEVELS_LIST(MAKE_STRING)
};

void q_log(enum log_levels lev, char *subsystem, char *format, ...) {
  va_list args;
  va_start (args, format);
  fprintf(stdout, "[%s/%s]: ", subsystem, kLogLevelNames[lev]);
  vfprintf(stdout, format, args);
  putc('\n', stdout);
  va_end(args);
}
