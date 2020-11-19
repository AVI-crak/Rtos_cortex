/**
 @file    monitor.c
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


#include <stdint.h>
#include "stm32f7xx.h"
#include "monitor.h"
#include "RtoS_.h"
//#include "sPrint.h"


 char    _std_out_buffer[eb_buf_zize_out] __attribute__ ((aligned (4)));
 char    _std_in_buffer[eb_buf_zize_in] __attribute__ ((aligned (4)));



static struct _stdout
{
    const uint16_t              length;
    const volatile uint16_t     tail;
    volatile uint16_t           head;
    volatile uint16_t           mode;
    volatile char               *ptr;
}_eb_monitor_stdout ={eb_buf_zize_out,0,0,0,&_std_out_buffer[0]};

static struct _stdin
{
    const uint16_t              length;
    volatile uint16_t           tail;
    const volatile uint16_t     head;
    volatile uint16_t           mode;
    const volatile char         *ptr;
}_eb_monitor_stdin ={eb_buf_zize_in,0,0,0,&_std_in_buffer[0]};

__attribute__( ( always_inline ) ) static inline void delay( uint32_t volatile time_tmp){do {time_tmp--;} while ( time_tmp );};

/// print text
/// os_pass() - Command for OS, task forced switching
/// os_data.system_us - System time, step 1ms
void monitor_print (char* text)
{
    static uint8_t status = 0;
    if ( (text[0] == '\0') || (status == 2) ) return;
    while (status == 1)os_pass(); /// busy there
    status = 1;
    char* in_txt;
    uint32_t temp_h, temp_t;
    int32_t temp_tim;
    temp_tim = os_data.system_us + 2000;
    temp_tim = 0 - temp_tim;
    temp_h = _eb_monitor_stdout.head + 1;
    if (temp_h == eb_buf_zize_out) temp_h = 0;
    temp_t = _eb_monitor_stdout.tail;
    in_txt = text;
    while (*in_txt != '\0')
    {
        do{ temp_t = _eb_monitor_stdout.tail; delay(50);}
            while (temp_t != _eb_monitor_stdout.tail);

        if (temp_h == temp_t)
        {
            if ((os_data.system_us + temp_tim) > 1)
            {
                status = 2; return;
            }else os_pass(); /// busy there
        }else
        {
            _eb_monitor_stdout.ptr[temp_h] = *in_txt++;
            _eb_monitor_stdout.head = temp_h; temp_h++;
            if (temp_h == eb_buf_zize_out) temp_h = 0;

            __DSB();
        };
    };
    status = 0;
};

uint32_t monitor_balance(void)
{
    uint32_t temp_h, temp_t, out;
    do{ temp_t = _eb_monitor_stdout.tail; delay(50);}
        while (temp_t != _eb_monitor_stdout.tail);
    temp_h = _eb_monitor_stdout.head;
    if (temp_h < temp_t) out = temp_t - temp_h - 1;
        else out = temp_t + eb_buf_zize_out - 1 - temp_h;
    return out;
}

char *monitor_scan (void)
{
    uint32_t temp_x = 0;
    uint32_t tail = _eb_monitor_stdin.tail;
    while (tail != _eb_monitor_stdin.head)
        {
            tail = (tail + 1) & (eb_buf_zize_in - 1);
            m_ms_buf[temp_x++] = _eb_monitor_stdin.ptr[tail];
            _eb_monitor_stdin.tail = tail;
        };
        return m_ms_buf;


};


