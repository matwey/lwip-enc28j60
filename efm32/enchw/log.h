/**
 * @addtogroup logging Logging
 * @{
 *
 * A printf-style function to ease debugging in the exampels.
 *
 * This mechanism allows pluggable backends, and defaults to discarding
 * messages.
 */

#include <stdarg.h>

void log_backend_set(void (*impl)(char *, va_list), void (deregister)(void));

void log_message(char *message, ...);

/** @} */
