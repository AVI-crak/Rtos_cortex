/**
 @file    monitor.c
 @author  AVI-crak
 @version V-51%
 @date    25-января-2017
 @brief   Аxis sRtoS, Cortex-M7 ARM GCC EmBitz

 license
 Million helicopters and one cent.

 форум для обсуждения
 http://forum.ixbt.com/topic.cgi?id=48:11735

 репозиторий
 https://bitbucket.org/AVI-crak/rtos-cortex-m3-gcc
*/


#include <stdint.h>
#include "stm32f7xx.h"
#include "monitor.h"
#include "RtoS_.h"
#include "sPrint.h"


 __attribute__((packed, aligned(4))) char     _std_out_buffer[(eb_buf_zize_out & 0xFFFFFFFC)];
 __attribute__((packed, aligned(4))) char     _std_in_buffer[(eb_buf_zize_in & 0xFFFFFFFC)];

char     m_mk_buf[64];
char     m_mk_nol = 0;


static struct _stdout
{
    const uint16_t              length;
    const volatile uint16_t     tail;
    volatile uint16_t           head;
    volatile uint16_t           mode;
    volatile char               *ptr;
}_eb_monitor_stdout ={(eb_buf_zize_out & 0xFFFFFFFC),0,0,0,&_std_out_buffer[0]};

static struct _stdin
{
    const uint16_t              length;
    volatile uint16_t           tail;
    const volatile uint16_t     head;
    volatile uint16_t           mode;
    const volatile char         *ptr;
}_eb_monitor_stdin ={(eb_buf_zize_in & 0xFFFFFFFC),0,0,0,&_std_in_buffer[0]};

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
    uint32_t temp_h, temp_t, temp_hl;
    int32_t temp_tim;
    temp_tim = -2000;
    temp_tim -= os_data.system_us;
    temp_hl = _eb_monitor_stdout.head;
    temp_h = temp_hl + 1;
    temp_t = _eb_monitor_stdout.tail;
    in_txt = text;
    do
    {
        do{ temp_t = _eb_monitor_stdout.tail; delay(50);}
            while (temp_t != _eb_monitor_stdout.tail);
        if (temp_h == (eb_buf_zize_out & 0xFFFFFFFC)) temp_h = 0;
        if (temp_h == temp_t)
        {
            _eb_monitor_stdout.head = temp_hl;
            if ((os_data.system_us + temp_tim) > 1)
            {
                status = 2; return;
            }else os_pass(); /// busy there
        }else
        {
            _eb_monitor_stdout.ptr[temp_h] = *in_txt++;
            _eb_monitor_stdout.head = temp_h; temp_hl = temp_h;
            temp_h++; __DSB();
        };
    }while (*in_txt != '\0');
    _eb_monitor_stdout.head = temp_hl;
    status = 0;
};



uint32_t monitor_balance(void)
{
    uint32_t temp_h, temp_t, out;
    do{ temp_t = _eb_monitor_stdout.tail; delay(50);}
        while (temp_t != _eb_monitor_stdout.tail);
    temp_h = _eb_monitor_stdout.head;
    if (temp_h < temp_t) out = temp_t - temp_h - 1;
        else out = temp_t + (eb_buf_zize_out & 0xFFFFFFFC) - 1 - temp_h;
    return out;
}

char *monitor_scan (void)
{
    uint32_t temp_x = 0;
    uint32_t tail = _eb_monitor_stdin.tail;
    while (tail != _eb_monitor_stdin.head)
        {
            tail = (tail + 1) & ((eb_buf_zize_in & 0xFFFFFFFC) -1);
            m_ms_buf[temp_x++] = _eb_monitor_stdin.ptr[tail];
            _eb_monitor_stdin.tail = tail;
        };
        return m_ms_buf;


};

static const char* task_status_txt[]={"activ","stack_error","delay","delay_error","hold",
    "hold_error","serv","serv_error","new_task","new_task_error","delete_task","delete_task_error",
    "new_ram","new_ram_error","delete_ram","delete_ram_error",};
char buf_log[70];

void uint_char (char* txt, uint32_t date);
void uint_char (char* txt, uint32_t date)
{
    uint32_t tmp; tmp = date;
    char* t; t = txt;
    do { *t-- = tmp %10 + '0'; tmp /= 10; } while (tmp != 0);
};

void monitor (void)
{
uint32_t    temp, tmp, next_task ;
volatile struct  task* r_task;
int32_t timeer;

    while(1)
    {
        printo("\f");
//                            15 N17/21  6  28   6  35   40  +20
        printo("Task_name       ;M  ID; stack;   use;    %; mode\n");
        next_task = ( uint32_t)os_data.main_task;
        while( next_task != 0 )
            {
                r_task = ( struct task*)next_task;
                timeer = os_data.system_us;
                tmp = 0; do {buf_log[tmp++] = ' ';} while (tmp < 70); tmp = 0; __ISB();
                while (( tmp < 15) && ( r_task->task_names[tmp] != 0)) { buf_log[tmp] = r_task->task_names[tmp]; tmp++; };
                buf_log[16] = ';';
                if ( (uint32_t)r_task->top_stack & 1 )  buf_log[17] = 'M'; else buf_log[17] = 'n';
                uint_char(&buf_log[21], r_task->task_nomer);
                buf_log[22] = ';';
                uint_char(&buf_log[28], r_task->stack_area_size);
                buf_log[29] = ';';  buf_log[36] = ';';
                uint_char(&buf_log[35], r_task->stack_area_used);
                temp = r_task->activ_time * 100; temp /= os_data.tick_1ms;
                buf_log[41] = '%'; buf_log[42] = ';';
                uint_char(&buf_log[40], temp);
                tmp = 44; temp = r_task->task_mode; uint32_t temp2 = 0;
                while (  task_status_txt[temp][temp2] != 0 ) { buf_log[tmp++] = task_status_txt[temp][temp2++];};

                buf_log[tmp++] = '\n';//buf_log[tmp++] = '\r';
                buf_log[tmp] = 0;
                __DSB();
                next_task = (uint32_t) r_task->task_form_head;
                __DSB();__ISB();
                monitor_print (buf_log); //monitor_print ("/n");

                timeer = os_data.system_us - timeer;
              //  printo(timeer, "=");
            };
        os_Delay_ms(300);


    };
}

/*
// считаем чтобы влезло
void monitor_print (char* text)
{
    if ( (text[0] == '\0') || (_eb_monitor_stdout.mode == 2) ) return;
    while (_eb_monitor_stdout.mode == 1)os_pass();
    __DSB(); __DMB();
    _eb_monitor_stdout.mode = 1;
    char* in_txt;
    uint32_t temp_h, temp_l,temp_i, temp_t;
    int32_t temp_tim;
    temp_tim = 0 -(os_data.system_us +1000);
    temp_h = _eb_monitor_stdout.head;
    in_txt = text;
    temp_i = 0 ; while (in_txt[temp_i++] != 0);
    do
    {
        temp_t = _eb_monitor_stdout.tail;
        if ( temp_h >= temp_t ) temp_l = ((eb_buf_zize_out & 0xFFFFFFFC) - temp_h - 2) + temp_t;
        if ( temp_h < temp_t ) temp_l = temp_t - temp_h - 1;
        if ((os_data.system_us + temp_tim) > 1)
            { _eb_monitor_stdout.mode = 2; return;};
        if (temp_l < temp_i) os_pass();
    }while (temp_l < temp_i);

    do
    {
        if (++temp_h == (eb_buf_zize_out & 0xFFFFFFFC)) temp_h = 0; else;
        _eb_monitor_stdout.ptr[temp_h] = *in_txt++;
    }while (*in_txt != '\0');
    _eb_monitor_stdout.head = temp_h;
    _eb_monitor_stdout.mode = 0;
};
*/
