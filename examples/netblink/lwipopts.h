#ifndef MY__LWIPOPTS_H__
#define MY__LWIPOPTS_H__

/** 
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS                          1

/**
 * NO_SYS_NO_TIMERS==1: Drop support for sys_timeout when NO_SYS==1
 * Mainly for compatibility to old versions.
 */
/* i'll use them, but first i want things to work as they were */
#define NO_SYS_NO_TIMERS                1

/* Disable the more complex APIs */
#define LWIP_SOCKET 0
#define LWIP_NETCONN 0

#endif /* MY__LWIPOPTS_H__ */
