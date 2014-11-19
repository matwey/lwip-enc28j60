/** 
 * @addtogroup logging Logging
 * @{
 */

#include "log.h"

#include <stdio.h>

#include <em_cmu.h>
#include <core_cm3.h>

static void (*current_log_implementation)(char *message, va_list argp) = NULL;
static void (*current_log_deregisterer)(void) = NULL;

void log_backend_set(void (*impl)(char *, va_list), void (deregister)(void))
{
	if (current_log_deregister != NULL)
		current_log_deregister();
	current_log_implementation = impl;
	current_log_deregister = deregister
}

void log_message(char *message, ...)
{
	if (current_log_implementation == NULL) return;

	va_list argp;
	va_start(argp, message);

	current_log_implementation(message, argp);

	va_end(argp);
}

/** Enhanced version of the libopencm3 default interrupt handler, which
 * logs the offending interrupt. */
void blocking_handler_implementation(void)
{
	register uint32_t IPSR;
	__asm__ ("MRS %0, IPSR"  : "=r" (IPSR));
	log_message("Unhandled interrupt %#x (IRQ %d), entering endless loop.\n", IPSR & 0xff, (IPSR & 0xff) - 16);
	for(;;);
}

/** @} */
