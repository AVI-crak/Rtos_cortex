/**
 @file    monitor.h
 @author  AVI-crak
 @version V-90%
 @date    january 2022
 @brief   Cortex ARM, GCC, EmBitz
 license MIT (Massachusetts Institute of Technology)

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

/// recommended size 256,128,64,
#define eb_buf_zize_out     256
#define eb_buf_zize_in      64



/// Print message to EB monitor
/// \n newline, \t tab, \f clear
/// \b position minus 1 character, \r position zero-based string.

/// send string without OS
void M_print (char* text);

/// send character without OS
void M_print_ch (char letter);

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


