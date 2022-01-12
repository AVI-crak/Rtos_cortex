/**
 @file    monitor.c
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


#include <stdint.h>
#include "monitor.h"

#include "RtoS_.h"
/// os_pass() - Command for OS, task forced switching
/// os_data.system_us - System time, step 1ms

#if (!((eb_buf_zize_out < 32769UL) && (eb_buf_zize_out & (eb_buf_zize_out - 1)) == 0))
#error "eb_buf_zize_out!! 256,128,64,32,16,8,4"
#endif

#if (!((eb_buf_zize_in < 32769UL) && (eb_buf_zize_in & (eb_buf_zize_in - 1)) == 0))
#error "eb_buf_zize_in!! 256,128,64,32,16,8,4"
#endif


 char    _std_out_buffer[eb_buf_zize_out] __attribute__ ((aligned (4),visibility("hidden")));
 char    _std_in_buffer[eb_buf_zize_in] __attribute__ ((aligned (4),visibility("hidden")));
 char    m_scan_buf[eb_buf_zize_in]__attribute__ ((aligned (4),visibility("hidden")));

__attribute__ ((aligned (4),visibility("hidden")))static struct _stdout
{
    uint16_t    length;
    uint16_t    tail;
    uint16_t    head;
    int16_t     mode;
    char*       ptr;
}_eb_monitor_stdout ={eb_buf_zize_out,0,0,0,&_std_out_buffer[0]};

__attribute__ ((aligned (4),visibility("hidden")))static struct _stdin
{
    uint16_t    length;
    uint16_t    tail;
    uint16_t    head;
    int16_t     mode;
    char*       ptr;
}_eb_monitor_stdin ={eb_buf_zize_in,0,0,0,&_std_in_buffer[0]};

/// print text to OS
__attribute__ ((optimize("-Os")))void M_print_OS  ( char* text)//120
{
    int32_t temp_tim, tmp; uint16_t temp_h;
    if (*text == 0)return;
    while(1)
    {
        asm volatile ("":::"memory");
        temp_tim = _eb_monitor_stdout.mode;
        if(temp_tim < 0) return;
        if (temp_tim == 0) break;
        if(temp_tim > 0) os_pass();
    };
    _eb_monitor_stdout.mode = 1;
    asm volatile ("dsb st":::"memory");
    temp_tim =  os_data.system_us;
    temp_tim += eb_buf_zize_out * 2 + 256;
    temp_tim =  1 - temp_tim;
    temp_h = _eb_monitor_stdout.head;
    while(1)
    {
        temp_h = (temp_h +1) & (eb_buf_zize_out - 1);
        while (1)
        {
            asm volatile ("":::"memory");
            if (temp_h != _eb_monitor_stdout.tail)break;
            tmp = os_data.system_us; tmp += temp_tim;
            if (tmp < 0) os_pass();
            else {_eb_monitor_stdout.mode = -1; return;}
        };
        if (*(text++) == 0) break;
        _eb_monitor_stdout.ptr[temp_h] = *(text - 1);
        asm volatile ("dsb st":::"memory");
        _eb_monitor_stdout.head = temp_h;
    };
    _eb_monitor_stdout.mode = 0;
};

/// print text no os
__attribute__ ((optimize("-Os")))void M_print (char* text)//40
{
    uint32_t temp_h,temp_t; char *out_txt; char txt;
    out_txt = &_eb_monitor_stdout.ptr[0];
    temp_h = _eb_monitor_stdout.head;
    do{
        temp_h = (temp_h + 1) & (eb_buf_zize_out - 1);
        txt = *(text++);
        if(txt == 0) return;
        do{
            asm volatile ("":::"memory");
            temp_t = _eb_monitor_stdout.tail;
        }while ( temp_t == temp_h);
        out_txt[temp_h] = txt;
        asm volatile ("dsb st":::"memory");
        _eb_monitor_stdout.head = temp_h;
    }while(1);
};

__attribute__ ((optimize("-Os")))void M_print_ch (char letter)//40
{
    uint32_t temp_h, temp_t;
    temp_h =  _eb_monitor_stdout.head + 1;
    temp_h &= eb_buf_zize_out - 1;
    do{
        asm volatile ("":::"memory");
        temp_t = _eb_monitor_stdout.tail;
    }while ( temp_t == temp_h);
    _eb_monitor_stdout.ptr[temp_h] = letter;
    asm volatile ("dsb st":::"memory");
    _eb_monitor_stdout.head = temp_h;
};

__attribute__ ((optimize("-Os")))uint32_t M_balance(void)
{
    uint32_t temp_h, temp_t, out;
    temp_t = _eb_monitor_stdout.tail;
    temp_h = _eb_monitor_stdout.head;
    if ((temp_h < temp_t))out = (temp_t - temp_h) - 1;
    else out = (eb_buf_zize_out - 1) - (temp_h - temp_t);
    return out;
};

__attribute__ ((optimize("-Os")))char *M_scan(void)//48
{
    uint32_t tail = _eb_monitor_stdin.tail;
    uint32_t head = _eb_monitor_stdin.head;
    char* out_txt = &m_scan_buf[0];
    while (1){
        if (tail == head) break;
        tail = (tail + 1) & (eb_buf_zize_in - 1);
        *(out_txt++) = _eb_monitor_stdin.ptr[tail];
    };
    _eb_monitor_stdin.tail = tail;
    *out_txt = tail - head;
    return m_scan_buf;
};
