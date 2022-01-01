/**
 @file    monitor.h
 @author  AVI-crak
 @version V-52%
 @date    2021:11:21
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

/// eb_buf 256,128,64,32,16,8,4
#define eb_buf_zize_out     256
#define eb_buf_zize_in      64



extern char m_ms_buf[eb_buf_zize_in];

/// Print message to EB monitor
/// \n newline, \t tab, \f clear
/// \b position minus 1 character, \r position zero-based string.

/// print text no os
void M_print (char* text);

/// Print without OS
void M_print_OS (char* text);

/// Receive message from EB monitor
char *M_scan (void);

/// Free space check
uint32_t M_balance(void);




#ifdef __cplusplus
}
#endif /* __cplusplus  */
#endif /* _MONITOR_ */


