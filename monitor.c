/**
 @file    monitor.c
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


#include <stdint.h>
#include "monitor.h"

#include "RtoS_.h"
/// os_pass() - Command for OS, task forced switching
/// os_data.system_us - System time, step 1ms

#if (!((eb_buf_zize_out < 257) && (eb_buf_zize_out & (eb_buf_zize_out - 1)) == 0))
#error "eb_buf_zize_out!! 256,128,64,32,16,8,4"
#endif

#if (!((eb_buf_zize_in < 257) && (eb_buf_zize_in & (eb_buf_zize_in - 1)) == 0))
#error "eb_buf_zize_in!! 256,128,64,32,16,8,4"
#endif


 char    _std_out_buffer[eb_buf_zize_out] __attribute__ ((aligned (4),used));
 char    _std_in_buffer[eb_buf_zize_in] __attribute__ ((aligned (4),used));

__attribute__ ((aligned (4),used))static struct _stdout
{
    const uint16_t              length;
    const volatile uint16_t     tail;
    volatile uint16_t           head;
    volatile uint16_t           mode;
    char                        *ptr;
}_eb_monitor_stdout ={eb_buf_zize_out,0,0,0,&_std_out_buffer[0]};

__attribute__ ((aligned (4),used))static struct _stdin
{
    const uint16_t              length;
    volatile uint16_t           tail;
    const volatile uint16_t     head;
    volatile uint16_t           mode;
    const volatile char         *ptr;
}_eb_monitor_stdin ={eb_buf_zize_in,0,0,0,&_std_in_buffer[0]};

static inline void delay( uint32_t volatile time_tmp){while ( --time_tmp );};


/// print text to OS
void M_print_OS  (char* text)//136
{
    if (*text == 0)return;
    if(_eb_monitor_stdout.mode == 2) return;
    while (_eb_monitor_stdout.mode != 0)os_pass(); /// busy there
    _eb_monitor_stdout.mode = 1;
    asm volatile ("dsb st":::"memory");
    int32_t temp_tim; uint32_t temp_h;
    temp_tim =  os_data.system_us + 2000;
    temp_tim =  1 - temp_tim;
    char  *out_txt;
    out_txt = &_eb_monitor_stdout.ptr[0];
    temp_h = _eb_monitor_stdout.head;
    temp_h = (temp_h + 1) & (eb_buf_zize_out - 1);
    while (*text != 0)
    {
        if (temp_h != _eb_monitor_stdout.tail)
        {
            out_txt[temp_h] = *(text++);
            asm volatile ("dsb st":::"memory");
            _eb_monitor_stdout.head = temp_h;
            temp_h = (temp_h + 1) & (eb_buf_zize_out - 1);
        }else if ((os_data.system_us + temp_tim) < 1) os_pass();
        else {_eb_monitor_stdout.mode = 2; return;}
    };
    _eb_monitor_stdout.mode = 0;
};

/// print text no os
void M_print (char* text)
{
    uint16_t temp_h; char *out_txt; char txt;
    out_txt = &_eb_monitor_stdout.ptr[0];
    temp_h = _eb_monitor_stdout.head;
    do{
        temp_h = (temp_h + 1) & (eb_buf_zize_out - 1);
        txt = *(text++);
        if(txt == 0) return;
        while (temp_h == _eb_monitor_stdout.tail);
        out_txt[temp_h] = txt;
        asm volatile ("dsb st":::"memory");
        _eb_monitor_stdout.head = temp_h;
    }while(1);
};

uint32_t M_balance(void)
{
    uint32_t temp_h, temp_t, out;
    temp_t = _eb_monitor_stdout.tail;
    temp_h = _eb_monitor_stdout.head;
    if ((temp_h < temp_t))out = (temp_t - temp_h) - 1;
    else out = (eb_buf_zize_out - 1) - (temp_h - temp_t);
    return out;
};

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
