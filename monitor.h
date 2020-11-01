/**
 @file    monitor.h
 @author  AVI-crak
 @version V-51%
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



char     m_ms_buf[(eb_buf_zize_in & 0xFFFFFFFC)];

/// печать сообщение в EB monitor
/// \n новая строка, \t табуляция, \f очистка
/// \b позиция минус 1 символ, \r позиция строка с нуля.
void monitor_print (char* text);

/// Принять сообщение из EB monitor \n enter
char *monitor_scan (void);

uint32_t monitor_balance(void);




#ifdef __cplusplus
}
#endif /* __cplusplus  */
#endif /* _MONITOR_ */


