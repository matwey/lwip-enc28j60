/**
 * @addtogroup logging Logging
 * @{
 * @addtogroup logging-itm EFM32 ITM implementation
 * @{
 *
 * FIXME update description
 *
 * An implementation of the interface described in `logging.h` for EMLIB
 * (Energy Micro EFM32) devices, using the ARM SWO interface as suggested in
 * the eACommander utility.
 */

#ifndef LOGSIZE
/** Buffer size (maximum length of messages that can be printed) */
#define LOGSIZE 92
#endif

void logitm_start(void);

