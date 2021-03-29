/**
 @file    RtoS_.c
 @author  AVI-crak
 @version V-50%
 @date    6-мая-2020
 @brief   Аxis sRtoS, Cortex ARM GCC EmBitz

 license
 Million helicopters and one cent.

 discussion forum
 http://forum.ixbt.com/topic.cgi?id=48:11735

 repository
 https://github.com/AVI-crak/Rtos_cortex
*/



#include "RtoS_.h"
#include "sPrint.h"

#define  __SYSHCLK     216000000   /// stm32f7
//#define  __SYSHCLK     74000000   /// stm32f1
#define  SERVISE_SIZE  500
#pragma GCC push_options
#pragma GCC optimize ("Os")


/// _estack - upper bound of memory
/// Add to startup_XXX.s
/// .equ   _irqsize, 600 // Interrupt Stack Size
/// .equ   _main_stask,         ( _estack -   ((_irqsize + 240) & 0xFFFFFFF8))
/// Replace below _estack on the _main_stask

struct _os_basic os_data;




const char main_txt[] = "Main";
const char service_txt[] = "Service";
__attribute__( ( always_inline ) ) static inline void delay( uint32_t volatile time_tmp){do {time_tmp--;} while ( time_tmp );};
void service (void);
void ser_new_task (void);
void ser_new_ram (void);
void ser_os_free(void);
void ser_os_free_all(uint8_t nomer);
uint32_t ser_del_task(uint32_t next);
void ser_val_txt (char* txt, uint32_t date);
uint32_t ser_status_log (uint32_t next_task );


static const char* task_status_txt[]={"activ","stack_error","delay","delay_error","hold",
    "hold_error","serv","serv_error","new_task","new_task_error","delete_task","delete_task_error",
    "new_ram","new_ram_error","delete_ram","delete_ram_error",};
char ser_buf_log[70];



/// Запрос ресурса, бесконечный цикл - пока не освободится
/// Resource request, endless loop while resource not released
uint32_t os_resource_ask (uint32_t *name_resource)
{
    uint32_t tmp;
    tmp = (uint32_t) os_data.activ;
    if ( *name_resource != 0 ) return *name_resource;
    os_data.activ->swap_data = (uint32_t) name_resource;
    asm volatile ("push    {r3}         \n\t"
                  "mov     r3, 0x0B     \n\t" //#0B __os_res_ask
                  "svc     0x0          \n\t"
                  "pop     {r3}         \n\t"
                  :::"memory");
    tmp = os_data.activ->swap_data;
    return tmp;
};

uint32_t os_resource_free (uint32_t *name_resource)
{
    uint32_t tmp;
    tmp = (uint32_t) os_data.activ;
    if ( *name_resource == tmp ) *name_resource = 0;
    tmp = *name_resource;
    return tmp;
};


int32_t os_alarm_ms(int32_t * timer_name, int32_t timer_ms)
{
    int32_t tim = os_data.system_us;
    int32_t tim2 = tim + *timer_name;
    if ((tim2 >= 1) || ((tim2 + timer_ms) < 0))
    {
        *timer_name = 1 - (timer_ms + tim);
    }else tim2 = 0;
    return tim2;
};

int32_t os_tim_ms(int32_t* timer_name, int32_t timer_ms)
{
    int32_t tim = os_data.system_us;
    int32_t tim2 = tim + *timer_name;
    if ( tim2 > timer_ms) /// таймер убежал далеко вперёд
    {
        *timer_name = 1 - (tim + timer_ms - (tim2 % timer_ms));
    }else if (tim2 > 0)   /// значение таймера достаточно для выполнения условия
    {
        *timer_name = 1 - (timer_ms + tim - tim2);
    }else if ((timer_ms + tim2) < 0)  /// timer_name установлен на время превышающее timer_mc
    {
        *timer_name = 1 - (tim + timer_ms);
    }else tim2 = 0;
    return tim2;
};







/// Разбудить задачу
void os_wake(volatile uint8_t* global_task_nomer)
{
    register volatile uint8_t   *__global_nomer     asm  ("r0") = global_task_nomer;
    asm volatile ("push    {r3}         \n\t"
                  "ldrb    r3, [r0]     \n\t"
                  "cmp     r3, #0       \n\t"
                  "itt     ne           \n\t"
                  "movne   r3, #0x5     \n\t" //#5 __wake
                  "svcne   0x0          \n\t"
                  "pop     {r3}         \n\t"
                  :: "r" (__global_nomer):);
}


/// Остановить задачу в ожидание пинка
void os_freeze(volatile uint8_t* global_task_nomer)
{
    os_data.activ->swap_data = (uint32_t) global_task_nomer;
    os_data.activ->task_mode = task_hold;
    *global_task_nomer = os_data.activ->task_nomer;
    asm volatile ("push   {r3}          \n\t"
                  "mov    r3, #0x6      \n\t" //#6 __freeze
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                  :::"memory");
}





/// Allocate memory (size)
void *os_malloc(int32_t d_size)
{
    os_data.activ->task_mode = new_ram;
    os_data.activ->swap_data = (d_size + 7) >> 2;
    asm volatile ("push   {r3}          \n\t"
                  "mov    r3, 0x04      \n\t" //#4 __to_service
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                  :::"memory");
    void* out; out = (void*) os_data.activ->swap_data;
return out;
}

/// os_Delay_ms (ms)
void os_Delay_ms(uint32_t delay_mc)
{
    os_data.activ->swap_data = delay_mc;
    asm volatile ("push   {r3}          \n\t"
                  "mov    r3, 0x02      \n\t" //#2 __delay_new
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                  ::: "memory");
}








struct _os_wbasic
{
    uint32_t*  w_activ;                     ///#00- Список активных
    uint32_t*  w_delay;                     ///#04- Список спящих
    uint32_t*  w_hold;                      ///#08- Список замороженных
    uint32_t*  w_service;                   ///#12- Список обслуживания
    uint32_t*  w_task_switch;               ///#16- Новый/старый таск
    uint32_t*  w_nvic_stack;                ///#20- Адрес стека прерываний, Task maximum NVIC used stack
    uint32_t   w_nvic_size_use;             ///#24- Размер стека прерываний,
    uint32_t*  w_main_stask;                ///#28- Адрес стека main
    uint32_t*  w_use_task_stop;             ///#32- Последний адрес стека, Stack last address
    uint32_t   w_tick_1ms;                  ///#36- Таймер активности задачи 100%, Task activity timer 100%
    int32_t    w_system_us;                 ///#40- Системное время, System time counter
    uint32_t*  w_malloc0_start;             ///#44- Первый адрес malloc0, First malloc0 address
    uint32_t*  w_malloc0_stop;              ///#48- Последний адрес malloc0, Last malloc0 address
    uint32_t*  w_malloc1_start;             ///#52- Первый адрес malloc1, First malloc1 address
    uint32_t*  w_malloc1_stop;              ///#56- Последний адрес malloc1, Last malloc1 address
};



/// Replica isr_vector
struct _my_isr_vector
{
    uint32_t   _main_stask;
    void*      Reset_Handler;
    void*      NMI_Handler;
    void*      HardFault_Handler;
    void*      MemManage_Handler;
    void*      BusFault_Handler;
    void*      UsageFault_Handler;
    uint32_t*  _irq_stack;
    uint32_t   _irqsize;
    uint32_t*  _free_ram;
    uint32_t*  _free_exram;
//  uint32_t   _nc;
//  void*      SVC_Handler;
};

//uintptr_t
struct  task_argument
{
    uint32_t*  _func;
    uint32_t   _size;
    uint32_t   _time_rate;
    char*      _func_name;
};

/// Новая задача - после запуска ос
///  функция , размер стека , процент времени 1-100 , имя
uint32_t os_Task_new (void (*taskS_func),uint16_t task_size,uint8_t task_time_rate,char* const task_func_name)
{
    volatile struct  task_argument  data;
    data._func = taskS_func;
    data._size = (task_size + 7) & 0xFFFFFFF8;
    data._time_rate = (os_data.tick_1ms * task_time_rate ) / 100;
    data._func_name = task_func_name;
    os_data.activ->task_mode = new_task;
    os_data.activ->swap_data = (uint32_t) &data;
    asm volatile ("push   {r3}          \n\t"
                  "mov     r3, 0x04     \n\t" //#4 __to_service
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                  :::"memory");
    uint32_t ret; ret = os_data.activ->swap_data;
    return ret;
};


/// Старт ОS
/// частота ядра в гц, размер стека майна,
///       > размер стека прерываний, стартовое время задачи в микросекундах
void os_Run(const uint16_t main_size )
{

    struct _os_wbasic* _os_wb;
    _os_wb = (struct _os_wbasic*) &os_data;
    uint32_t main_sizel;
    main_sizel = (main_size + 7) & 0x0000FFF8;
    uint32_t _irq_tmp;
    struct _my_isr_vector* isr_v;
    isr_v = (struct _my_isr_vector*)SCB->VTOR;
    _irq_tmp = (isr_v->_main_stask - (main_sizel + sizeof(struct task))) & 0xFFFFFFF8;
    _os_wb->w_activ = (uint32_t*) isr_v->_main_stask;
    _os_wb->w_main_stask = (uint32_t*) isr_v->_main_stask;
    _os_wb->w_use_task_stop = (uint32_t*) (_irq_tmp - ((SERVISE_SIZE + 7) & 0x0000FFF8) - 4);
    _os_wb->w_nvic_stack = isr_v->_irq_stack;
    _os_wb->w_nvic_size_use = isr_v->_irqsize;
    _os_wb->w_malloc0_start = isr_v->_free_ram;
    _os_wb->w_malloc0_stop = isr_v->_free_ram;
    *_os_wb->w_malloc0_stop = 0;
    _os_wb->w_malloc1_start = isr_v->_free_exram;
    _os_wb->w_malloc1_stop = isr_v->_free_exram;
    _os_wb->w_system_us = 0;
    _os_wb->w_delay = 0;
    _os_wb->w_hold = 0;
    _os_wb->w_service = 0;
    _os_wb->w_task_switch = 0;
    _os_wb->w_tick_1ms = (__SYSHCLK / 1000) - 6;

    uint32_t* rwserv;
    rwserv = (uint32_t*) _irq_tmp - 16;
    for (uint32_t i = 0; i < 26; i++) {*rwserv++ = 0;}; rwserv -=3;
    uint32_t* rwmain;
    rwmain = (uint32_t*) isr_v->_main_stask;
    for (uint32_t i = 0; i < 10; i++) {*rwmain++ = 0;}; rwmain -=3;
    *rwserv-- = ((__SYSHCLK / 1000) - 6) | (2 << 24);
    *rwmain-- = ((__SYSHCLK / 1000) - 6) | (1 << 24);
    *rwserv-- = (SERVISE_SIZE + 7) & 0x0000FFF8;
    *rwmain-- = main_sizel;
    *rwserv-- = (uint32_t) &service_txt[0];
    *rwmain-- = (uint32_t) &main_txt[0];
    *--rwserv = (uint32_t) _irq_tmp - 64;
    rwmain -= 2; rwserv -= 2;
    *rwmain-- = _irq_tmp;
    *rwmain-- = _irq_tmp;
    *rwmain = _irq_tmp;
    *rwserv-- = (uint32_t) isr_v->_main_stask;
    *rwserv-- = (uint32_t) isr_v->_main_stask;
    *rwserv-- = 0x1000000;
    *rwserv-- = (uint32_t)((void*)service);
    *rwserv = 0x1000000;

#if defined(__STM32F4xx_H) ||  defined(__STM32F7xx_H )
    SCB->CPACR = 0x0F << 20; /// FPU settings
#endif
    SysTick->LOAD = (__SYSHCLK / 1000) - 6;
    SysTick->VAL = (__SYSHCLK / 1000) - 6;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk|
                    SysTick_CTRL_TICKINT_Msk|
                    SysTick_CTRL_ENABLE_Msk;
    os_EnableIRQ(SysTick_IRQn, 15);

    __ISB();
    __set_PSP(__get_MSP());
    __set_MSP((uint32_t)isr_v->_irq_stack );
    __ISB();
    __set_CONTROL(CONTROL_SPSEL_Msk| CONTROL_nPRIV_Msk);
    __ISB();

/// таймер system_us
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = 1;
    TIM6->CR1 |= TIM_CR1_ARPE;
    TIM6->ARR = (__SYSHCLK / 4) / 1000; // APB1= /4
    TIM6->DIER = TIM_DIER_UIE;
    TIM6->CR1 |= TIM_CR1_CEN;
#if defined(__STM32F7xx_H )
    os_EnableIRQ(TIM6_DAC_IRQn, 15); // stm32f7
#elif defined(__STM32F1XX_H)
    os_EnableIRQ(TIM6_IRQn, 15); // stm32f1
#endif
    __memory();

#if defined(__STM32F4xx_H) ||  defined(__STM32F7xx_H )
    RNG->CR = RNG_CR_RNGEN; // stm32f7
#endif

#if defined(__STM32F1XX_H)
 os_data.ranlom[0] = *(os_data.malloc0_start + 4);
 os_data.ranlom[1] = *(os_data.malloc0_start + 8);
 os_data.ranlom[2] = *(os_data.malloc0_start + 12);
 if ((os_data.ranlom[0]==0)|(os_data.ranlom[1]==0)|(os_data.ranlom[2]==0)){
 os_data.ranlom[0] = 5468651;
 os_data.ranlom[0] = 7454164;
 os_data.ranlom[0] = 1587975;};

#endif

};

#if defined(__STM32F7xx_H )
void  __attribute__ ((weak)) TIM6_DAC_IRQHandler(void) // stm32f7
#elif defined(__STM32F1XX_H)
void  __attribute__ ((weak)) TIM6_IRQHandler(void) // stm32f1
#endif
{
    TIM6->SR = 0;
    asm volatile ("mov    r3, #1        \n\t" //#1 __sleep_tasks
                  "svc    0x0           \n\t"
                  ::: "r3", "memory");
};




void service (void)
 {
    task_status_TypeDef state;
    uint32_t next_t;
    next_t = 0;
    int32_t time_log;
    time_log = -200;
    time_log -= os_data.system_us;
    while (1)
 {
    if ((uint32_t)os_data.service != 0)
    {
        state = os_data.service->task_mode;
        switch (state)
        {
            case new_ram    : ser_new_ram(); break;
            case delete_ram : ser_os_free(); break;
            case new_task   : ser_new_task(); break;
            case delete_task: next_t = ser_del_task(next_t); break;
            default: break;
        };
    }

#if (__Test_psp == 1)
    else
    {
        if (monitor_balance() > 70)
        {
            if (next_t != 0 )
            {
                next_t = ser_status_log(next_t);
                time_log = -2000;
                time_log -= os_data.system_us;
            }else if ((os_data.system_us + time_log) > 1)
            {
                printo("\f");
                printo("Task_name       ;M  ID; stack;   use;    %; mode\n");
                next_t = ( uint32_t)os_data.main_task;
            };
        };
    }
#endif
     ;
    os_pass();
  };
};


void ser_val_txt (char* txt, uint32_t date)
{
    uint32_t tmp; tmp = date;
    char* t; t = txt;
    do { *t-- = tmp %10 + '0'; tmp /= 10; } while (tmp != 0);
};


uint32_t ser_status_log (uint32_t next_task )
{
uint32_t    temp, tmp;
struct  task* r_task;

    r_task = ( struct task*)next_task;
    tmp = 0; do {ser_buf_log[tmp++] = ' ';} while (tmp < 70); tmp = 0; __ISB();
    while (( tmp < 15) && ( r_task->task_names[tmp] != 0)) { ser_buf_log[tmp] = r_task->task_names[tmp]; tmp++; };
    ser_buf_log[16] = ';';
    if ( (uint32_t)r_task->top_stack & 1 )  ser_buf_log[17] = 'M'; else ser_buf_log[17] = 'n';
    ser_val_txt(&ser_buf_log[21], r_task->task_nomer);
    ser_buf_log[22] = ';';
    ser_val_txt(&ser_buf_log[28], r_task->stack_area_size);
    ser_buf_log[29] = ';';  ser_buf_log[36] = ';';
    ser_val_txt(&ser_buf_log[35], r_task->stack_area_used);
    temp = r_task->activ_time * 100; temp /= os_data.tick_1ms;
    ser_buf_log[41] = '%'; ser_buf_log[42] = ';';
    ser_val_txt(&ser_buf_log[40], temp);
    tmp = 44; temp = r_task->task_mode; uint32_t temp2 = 0;
    while (  task_status_txt[temp][temp2] != 0 ) { ser_buf_log[tmp++] = task_status_txt[temp][temp2++];};
    ser_buf_log[tmp++] = '\n'; ser_buf_log[tmp] = 0;
    __DSB();
    printo (ser_buf_log); //monitor_print ("/n");
    __DSB();__ISB();
    next_task = (uint32_t) r_task->task_form_head;
    return next_task;
};

void os_Task_del(void)
{
    os_data.activ->task_mode = delete_task;
    asm volatile ("push   {r3}          \n\t"
                  "mov    r3, 0x04      \n\t" //#4 __to_service
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                  :::"memory");
};


uint32_t ser_del_task(uint32_t next)
{
    struct  task* task_l;
    struct  task* task_d;
    task_l = os_data.main_task;
    task_d = os_data.service;
    ser_os_free_all(os_data.service->task_nomer);
    while (task_l->task_form_head != task_d) task_l = task_l->task_form_head;
    if (next == (uint32_t)task_d) next = (uint32_t)task_d->task_form_head;
    task_l->task_form_head = task_d->task_form_head;

    asm volatile ("push   {r3}          \n\t"
                  "mov    r3, 0x08      \n\t" //#8 __delete_task
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                  :::"memory");
    return next;
};


void ser_new_task (void)
 {
    struct  task_argument* tmp_arg;
    tmp_arg =  (struct  task_argument*)os_data.service->swap_data;
    struct  task* task_r;
    uint32_t  task_new, step, n_l, n_h;
    task_r = (struct  task*)os_data.main_task;
    uint32_t b_nomer[8]; b_nomer[0] = (uint32_t) 0-2;
    for (uint32_t i = 1; i < 8; i++) {b_nomer[i] = (uint32_t) 0-1;};
    task_r = (struct  task*)os_data.main_task;
    struct  task* task_wt; task_new = 0;
    step = sizeof(struct task)*2 + tmp_arg->_size;
    do{
        if (task_new == 0)
        {
            if (((((uint32_t)task_r->task_form_head + step + task_r->stack_area_size ) <= (uint32_t)task_r)))
            {
                 task_new = (uint32_t)task_r - (task_r->stack_area_size + sizeof(struct task));
                 task_wt = task_r;
            };
        };
        n_h = task_r->task_nomer >> 5; n_l = task_r->task_nomer & 31;
        b_nomer[n_h] = __BIC(b_nomer[n_h], (uint32_t) 1 << n_l);
        task_r = task_r->task_form_head;
    }while ((uint32_t)task_r != 0);
    if ((uint32_t) task_wt->task_form_head == 0) os_data.use_task_stop = (uint32_t*) ( task_new - tmp_arg->_size - 4);
    uint8_t nomer_new; step = (uint32_t) 0-1;
    do{ nomer_new = __CLZ(__RBIT(b_nomer[++step]));} while( nomer_new == 32);
    nomer_new += (step << 5);
    uint32_t* rwnew;
    uint32_t* rw0;
    rwnew = (uint32_t*) task_new - 17; rw0 = rwnew + 28;
    do{*++rwnew = 0;} while (rwnew < rw0); rwnew -= 15;
    *++rwnew = (uint32_t) os_Task_del; // lr
    *++rwnew = (uint32_t) tmp_arg->_func; // pc
    *++rwnew = 0x1000000; // psr
    *++rwnew = task_new; // task_head
    *++rwnew = task_new; //task_tail
    *++rwnew = (uint32_t) task_wt->task_form_head; //task_form_head
    task_wt->task_form_head = (struct  task*) task_new;
    os_data.task_switch = (struct  task*) task_new;
    *++rwnew = task_new - 64; rwnew += 1; // top_stack
    *++rwnew = (uint32_t) tmp_arg->_func_name; //task_names
    *++rwnew = tmp_arg->_size; // stack_area_size
    *++rwnew = tmp_arg->_time_rate | (nomer_new << 24); //activ_time

    asm volatile ("push   {r3}          \n\t"
                  "mov    r3, 0x07      \n\t" //#7 __task_new
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                  :::"memory");
};



//uintptr_t
struct ram_malloc
{
    uint32_t offset:24;
    uint32_t name:8;
};

/// Удалить память (адрес)
void os_free (void* d_adres)
{
    uint8_t* adres; adres = (uint8_t*) d_adres - 1;
    if (os_data.activ->task_nomer != *adres ) return;
    os_data.activ->task_mode = delete_ram;
    os_data.activ->swap_data = (uint32_t) adres;
    asm volatile ("push   {r3}          \n\t"
                  "mov    r3, 0x04      \n\t" //#4 __to_service
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                  :::"memory");
};


void ser_os_free(void)
{
    uint8_t* del_ram;
    del_ram = (uint8_t*)os_data.service->swap_data;
    *del_ram = 0;
    asm volatile ("push   {r3}          \n\t"
                  "mov    r3, 0x03      \n\t" //#3 __ret_service
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                  :::"memory");

};


void ser_os_free_all(uint8_t nomer)
{
    uint32_t names;
    uint32_t* ram_have;
    ram_have = (uint32_t*) os_data.malloc0_start;
    uint32_t name = 0XFF;
    int32_t offset = 0;
    do
    {
        if (!((name == nomer) | (name == 0)))
        {
            ram_have += offset;
            names = *ram_have;
            offset = names & 0x00FFFFFF;
            name = names >> 24;
        }else
        {
            *ram_have = offset;
            names = *(ram_have + offset);
            offset += names & 0x00FFFFFF;
            name = names >> 24;
        };
    }while(names != 0);
    *ram_have = 0;
    os_data.malloc0_stop = ram_have;
};


 void ser_new_ram (void)
 {
    int32_t new_ram_size;
    new_ram_size = os_data.service->swap_data;
    int32_t offset = 0;
    uint32_t* ram_have;
    ram_have = (uint32_t*) os_data.malloc0_start;
    uint32_t name = 0XFF;
    uint32_t names;

    do
    {
        if (name != 0)
        {
            ram_have += offset;
            names = *ram_have;
            offset = names & 0x00FFFFFF;
            name = names >> 24;
            if (names == 0) break;
        }else
        {
            names = *(ram_have + offset);
            offset += names & 0x00FFFFFF;
            name = names >> 24;
            if (names == 0) break;
        };
    }while((name != 0)|(offset < new_ram_size));
    if (os_data.use_task_stop < (ram_have + new_ram_size) )
      {
          os_data.service->task_mode--;
          return;
      };
    if (os_data.malloc0_stop < (ram_have + new_ram_size) )
      {
          os_data.malloc0_stop = ram_have + new_ram_size;
      };
    *ram_have = new_ram_size + (os_data.service->task_nomer << 24);
    if (names == 0) { *(ram_have + new_ram_size) = 0;}
    else if (offset > new_ram_size) { *(ram_have + new_ram_size) = offset - new_ram_size;};
    os_data.service->swap_data = (uint32_t) ram_have + 4;
    asm volatile ("push   {r3}          \n\t"
                  "mov    r3, 0x03      \n\t" //#3 __ret_service
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                   :::"memory");
     return;
 };







/// Включить прерывание, преоритет (от 14 до 0)
void  os_EnableIRQ(IRQn_Type IRQn, uint8_t priority)
{
    register uint8_t    __IRQn          asm  ("r0") = IRQn;
    register uint8_t    __priority      asm  ("r1") = priority;
    asm volatile ("push   {r3}          \n\t"
                  "mov    r3, 0x09      \n\t" //#9 __EnableIRQ
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                  :: "r" (__IRQn), "r" (__priority):"memory");
};



/// Отключить прерывание - после запуска ос
void os_DisableIRQ(IRQn_Type IRQn)
{
    register uint8_t    __IRQn          asm  ("r0") = IRQn;
    asm volatile ("push   {r3}          \n\t"
                  "mov    r3, 0x0A      \n\t" //#10 __DisableIRQ;
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                  :: "r" (__IRQn):"memory");
};

uint32_t soft_ranndom (void)
{
    asm volatile ("push   {r3}          \n\t"
                  "mov    r3, 0x0C      \n\t" //#C __ranndom
                  "svc    0x0           \n\t"
                  "pop    {r3}          \n\t"
                  :::"memory");
    volatile uint32_t out = os_data.activ->swap_data;
    return out;
};



uint32_t os_Random( uint32_t range)
{
    uint32_t tmp_x;
#if defined(__STM32F4xx_H) ||  defined(__STM32F7xx_H )
    while(!RNG->SR) delay(10);  // stm32f4-stm32f7
    tmp_x =  RNG->DR;  // stm32f4-stm32f7
#else
    tmp_x = soft_ranndom();
#endif
    uint64_t tmp_m = (uint64_t) tmp_x * range;
    uint32_t tmp_l = (uint32_t) tmp_m;
    uint32_t tmp_t;
    if (tmp_l < range)
    {
        tmp_t = -range;
        if (tmp_t >= range)
            {
                tmp_t -= range;
                if (tmp_t >= range) tmp_t %= range;
            };
        while (tmp_l < tmp_t)
        {
#if defined(__STM32F4xx_H) ||  defined(__STM32F7xx_H )
            while(!RNG->SR)delay(10); // stm32f4-stm32f7
            tmp_x = RNG->DR;          // stm32f4-stm32f7
#else
            tmp_x = soft_ranndom();
#endif
            tmp_m = (uint64_t) tmp_x * range;
            tmp_l = (uint32_t) tmp_m;
        };
    };
    tmp_x = tmp_m >> 32;
    return tmp_x;
};


#pragma GCC pop_options

