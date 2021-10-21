/**
 @file    monitor.h
 @author  AVI-crak
 @version V-52%
 @date    25-января-2017
 @brief   Аxis sRtoS, Cortex-M7 ARM GCC EmBitz

 license
 Million helicopters and one cent.

 discussion forum
 http://forum.ixbt.com/topic.cgi?id=48:11735

 repository
 https://github.com/AVI-crak/Rtos_cortex
*/


#ifndef _MONITOR_
#define _MONITOR_

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

#define eb_buf_zize_out     256
#define eb_buf_zize_in      64



extern char m_ms_buf[eb_buf_zize_in];

/// Print message to EB monitor
/// \n newline, \t tab, \f clear
/// \b position minus 1 character, \r position zero-based string.
void monitor_print (char* text);

/// Print without OS
void monitor_print2 (char* text);

/// Receive message from EB monitor
char *monitor_scan (void);

/// Проверка свободного места
uint32_t monitor_balance(void);




#ifdef __cplusplus
}
#endif /* __cplusplus  */
#endif /* _MONITOR_ */


