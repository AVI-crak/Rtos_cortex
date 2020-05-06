/**
 @file    RtoS_cortex_m7.S
 @author  AVI-crak
 @version V-45%
 @date    28-декабря-2016
 @brief   Аxis sRtoS, Cortex-M7 ARM GCC EmBitz

 license
 Million helicopters and one cent.

 форум для обсуждения
 http://forum.ixbt.com/topic.cgi?id=48:11735

 репозиторий
 https://bitbucket.org/AVI-crak/rtos-cortex-m3-gcc
*/

/// сборка 6752.124.540
/// сборка 6704.124.540
/// сборка 6728.124.540
/// сборка 6704.124.540
/// сборка 6500.124.540
/// сборка 6496.124.540
/// сборка 7432.184.300 - ser_new_ram


#include "RtoS_.h"
#include "sPrint.h"

#define  __SYSHCLK     216000000
#define  SERVISE_SIZE  500
/// _estack - upper bound of memory
///	Add to startup_XXX.s
/// .equ   _irqsize, 600 // Interrupt Stack Size
/// .equ   _main_stask,         ( _estack -   ((_irqsize + 240) & 0xFFFFFFF8))
/// Replace below _estack on the _main_stask

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
    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x0B           \n\t" //#0B __os_res_ask
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
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

int32_t os_tim_ms(int32_t * timer_name, int32_t timer_ms)
{
    int32_t tim = os_data.system_us;
    int32_t tim2 = tim + *timer_name;
	if ( tim2 > timer_ms)
	{
	    /// таймер убежал далеко вперёд
		*timer_name = 1 - (tim + timer_ms - (tim2 % timer_ms));
    }else if (tim2 > 0)
    {
        /// значение таймера достаточно для выполнения условия
        *timer_name = 1 - (timer_ms + tim - tim2);
    }else if ((timer_ms + tim2) < 0)
    {
        /// timer_name установлен на время превышающее timer_mc
		*timer_name = 1 - (tim + timer_ms);
    }else tim2 = 0;
    return tim2;
};





/*

/// sTask_wake (& глобальный флаг) разбудить задачу
void sTask_wake(volatile uint32_t* task_global_flag)
{
    register volatile uint32_t   *__task_global_flag     asm  ("r0") = task_global_flag;
    asm volatile    ("push      {r3}        \n\t"
                    "ldr        r3, [r0]    \n\t"
                    "cmp        r3, #0      \n\t"
                    "itt        ne          \n\t"
                    "movne      r3, 0x9     \n\t" //__sTask_wake //9;
                    "svcne      0x0         \n\t"
                    "pop        {r3}        \n\t"
                    :: "r" (__task_global_flag):);
}


/// sTask_wait (& глобальный флаг) остановить задачу в ожидание пинка
void sTask_wait(volatile uint32_t* task_global_flag)
{
    register volatile uint32_t   *__task_global_flag     asm  ("r0") = task_global_flag;
    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x8            \n\t" //__sTask_wait //8;
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
                    :: "r" (__task_global_flag):"memory");
}
*/




/// Allocate memory (size)
void *os_malloc(int32_t d_size)
{
    os_data.activ->task_mode = new_ram;
    os_data.activ->swap_data = (d_size + 7) >> 2;
    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x04            \n\t" //#4 __to_service
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
                    :::"memory");
    void* out; out = (void*) os_data.activ->swap_data;
return out;
}

/// sDelay_mc (ms)
void os_Delay_ms(uint32_t delay_mc)
{
    os_data.activ->swap_data = delay_mc;
    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x02			\n\t" //#2 __delay_new
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
                    ::: "memory");
}








struct _os_wbasic
{
    uint32_t* w_activ;                     ///#00- Список активных
    uint32_t* w_delay;                     ///#04- Список спящих
    uint32_t* w_hold;                      ///#08- Список замороженных
    uint32_t* w_service;                   ///#12- Список обслуживания
	uint32_t* w_task_switch;                ///#16- Новый/старый таск
    uint32_t* w_nvic_stack;              ///#20- Адрес стека прерываний, Task maximum NVIC used stack
    uint32_t  w_nvic_size_use;               ///#24- Размер стека прерываний,
    uint32_t* w_main_stask;			///#28- Адрес стека main
    uint32_t* w_use_task_stop;        ///#32- Последний адрес стека, Stack last address
    uint32_t  w_tick_1ms;				///#36- Таймер активности задачи 100%, Task activity timer 100%
    int32_t   w_system_us;      ///#40- Системное время, System time counter
    uint32_t* w_malloc0_start;           ///#44- Первый адрес malloc0, First malloc0 address
    uint32_t* w_malloc0_stop;          ///#48- Последний адрес malloc0, Last malloc0 address
    uint32_t* w_malloc1_start;           ///#52- Первый адрес malloc1, First malloc1 address
    uint32_t* w_malloc1_stop;          ///#56- Последний адрес malloc1, Last malloc1 address
};



/// Replica isr_vector
struct _my_isr_vector
{
  uint32_t  _main_stask;
  void*  Reset_Handler;
  void*  NMI_Handler;
  void*  HardFault_Handler;
  void*  MemManage_Handler;
  void*  BusFault_Handler;
  void*  UsageFault_Handler;
  uint32_t*  _irq_stack;
  uint32_t   _irqsize;
  uint32_t*  _free_ram;
  uint32_t*  _free_exram;
 // uint32_t  _nc;
//  void*  SVC_Handler;
};

//uintptr_t
struct  task_argument
{
	uint32_t*	_func;
	uint32_t	_size;
	uint32_t	_time_rate;
	char* 		_func_name;
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
    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x04            \n\t" //#4 __to_service
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
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

    SCB->CPACR = 0x0F << 20; /// FPU settings
    SysTick->LOAD = (__SYSHCLK / 1000) - 6;
    SysTick->VAL = (__SYSHCLK / 1000) - 6;
    SysTick->CTRL =	SysTick_CTRL_CLKSOURCE_Msk|
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
    os_EnableIRQ(TIM6_DAC_IRQn, 15);
	__memory();

    RNG->CR = RNG_CR_RNGEN;
};

void  __attribute__ ((weak)) TIM6_DAC_IRQHandler(void)
{
    TIM6->SR = 0;
    asm volatile    ("mov   r3, #1		\n\t" //#1 __sleep_tasks
                    "svc    0x0         \n\t"
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
    }else
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
    };
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
volatile struct  task* r_task;

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
    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x04           \n\t" //#4 __to_service
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
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

    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x08           \n\t" //#8 __delete_task
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
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

    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x07            \n\t" //#7 __task_new
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
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
    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x04           \n\t" //#4 __to_service
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
                    :::"memory");
}


void ser_os_free(void)
{
    uint8_t* del_ram;
    del_ram = (uint8_t*)os_data.service->swap_data;
    *del_ram = 0;
    int32_t offset = 0;
    int32_t offsetl = 0;
    uint32_t* ram_have;
    ram_have = (uint32_t*) os_data.malloc0_start;
    int32_t end_ram = 0;
    uint32_t name;
    do
    {
        do
        {
            ram_have += offset;
            name = *ram_have;
            offset = name & 0x00FFFFFF;
            name >>= 24;
        }while (name != 0);
        do
        {
            end_ram = *(ram_have + offset);
            offsetl = end_ram & 0x00FFFFFF;
            name = end_ram >> 24;
            if ( name == 0 )
                {
                    offset += offsetl;
                    *ram_have = offset;
                }else break;
        }while (end_ram != 0);
    }while( name != 0 );
    *ram_have = 0;
    os_data.malloc0_stop = ram_have;
    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x03           \n\t" //#3 __ret_service
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
                    :::"memory");
};


void ser_os_free_all(uint8_t nomer)
{
    int32_t offset = 0;
    int32_t offsetl = 0;
    uint32_t* ram_have;
    ram_have = (uint32_t*) os_data.malloc0_start;
    int32_t end_ram = 0;
    uint32_t name;
    do
    {
        do
        {
            ram_have += offset;
            name = *ram_have;
            offset = name & 0x00FFFFFF;
            name >>= 24;
            if (name == nomer)
            {
                *(ram_have + offset) = offset;
                name = 0;
            };
        }while (name != 0);
        do
        {
            end_ram = *(ram_have + offset);
            offsetl = end_ram & 0x00FFFFFF;
            name = end_ram >> 24;
            if (( name == 0 )||( name == nomer ))
                {
                    offset += offsetl;
                    *(ram_have + offset) = offset;
                }else break;
        }while (end_ram != 0);
    }while( end_ram != 0 );
    *ram_have = 0;
    os_data.malloc0_stop = ram_have;
};


 void ser_new_ram (void)
 {
    int32_t new_ram_size, new_head;
    new_ram_size = os_data.service->swap_data;
    new_head = new_ram_size + 1;
    int32_t offset = 0;
    int32_t offsetl;
    uint32_t* ram_have;
    uint32_t* ram_head;
    ram_have = (uint32_t*) os_data.malloc0_start;
    uint32_t new_ram;
    uint32_t name;
    do
    {
        do
        {
            ram_have += offset;
            name = *ram_have;
            offset = name & 0x00FFFFFF;
            name >>= 24;
        }while (name != 0);
        do
        {
            name = *(ram_have + offset);
            offsetl = name & 0x00FFFFFF;
            if (( name >> 24) == 0 )
                {
                    offset += offsetl;
                    *ram_have = offset;
                }else offsetl = 0;
        }while (offsetl != 0);
        if (offset >= new_head)
            {
                *(ram_have + new_ram_size) = offset - new_ram_size;
                break;
            }else if (offset == new_ram_size)
            {
                break;
            }else if(offset == 0)
            {
                ram_head = ram_have; ram_head += new_ram_size;
                if (os_data.use_task_stop < ram_head)
                    {
                        os_data.service->swap_data = 0;
                        __BKPT();
                        return;
                    }else
                    {
                        *(ram_have + new_ram_size) = 0;
                        os_data.malloc0_stop = ram_head;
                        break;
                    };
            };
    }while(1);
    *ram_have = new_ram_size + (os_data.service->task_nomer << 24);
    new_ram = (uint32_t) ram_have + 4;
    os_data.service->swap_data = new_ram;
    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x03           \n\t" //#3 __ret_service
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
                    :::"memory");
 }







/// Включить прерывание, преоритет (от 14 до 0)
void  os_EnableIRQ(IRQn_Type IRQn, uint8_t priority)
{
    register uint8_t    __IRQn          asm  ("r0") = IRQn;
    register uint8_t    __priority      asm  ("r1") = priority;
    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x09			\n\t" //#9 __EnableIRQ
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
                    :: "r" (__IRQn), "r" (__priority):"memory");
}



/// Отключить прерывание - после запуска ос
void os_DisableIRQ(IRQn_Type IRQn)
{
    register uint8_t    __IRQn          asm  ("r0") = IRQn;
    asm volatile    ("push   {r3}               \n\t"
                    "mov     r3, 0x0A			\n\t" //#10 __DisableIRQ;
                    "svc    0x0                 \n\t"
                    "pop    {r3}                \n\t"
                    :: "r" (__IRQn):"memory");
}



uint32_t os_Ranlom( uint32_t range)
{
    while(!RNG->SR) delay(10);
    uint32_t tmp_x =  RNG->DR;
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
            while(!RNG->SR)delay(10);
            tmp_x = RNG->DR;
            tmp_m = (uint64_t) tmp_x * range;
            tmp_l = (uint32_t) tmp_m;
        };
    };
    tmp_x = tmp_m >> 32;
    return tmp_x;
};









 /*
 /// 6472
    union
    {
       uint32_t all32[8];
       uint8_t all8[32];
    }n;
    step = 0;
    while (step < 8)  n.all32[step++] = (uint32_t) 0-1;
    n.all32[0] ^= 1;
    do{
        n.all8[task_r->task_nomer >> 5] = __BIC(n.all8[task_r->task_nomer >> 3], (uint32_t) 1 << (task_r->task_nomer & 7));
        task_r = task_r->task_form_head;
    }while ((uint32_t)task_r != 0);
    uint8_t nomer_new; step = 0;
    while(__CLZ(__RBIT(n.all32[step])) == 32) step++;
    nomer_new = __CLZ(__RBIT(n.all32[step])) + (step << 5);





   // 6464
    uint32_t  task_new, step, n_l, n_h;
    task_r = (struct  task*)os_data.main_task;
    uint32_t b_nomer[8];
    step = 0;
    while (step < 8)  b_nomer[step++] = (uint32_t) 0-1;
    b_nomer[0] ^= 1;
    do{
        n_h = task_r->task_nomer; n_l = n_h & 31; n_h >> 5;
        b_nomer[n_h] = __BIC(b_nomer[n_h], (uint32_t) 1 << n_l);
        task_r = task_r->task_form_head;
    }while ((uint32_t)task_r != 0);
    uint8_t nomer_new; step = 0;
    while(__CLZ(__RBIT(b_nomer[step])) == 32) step++;
    nomer_new = __CLZ(__RBIT(b_nomer[step])) + (step << 5);
    */




	/*
	uint32_t* tmp = (uint32_t*)_irq_tmp - 16;
	for (uint32_t i = 0; i < 26; i++) tmp[i] = 0;

	os_data.activ->task_head = (struct  task*) isr_v->_main_stask;
	os_data.activ->task_tail = (struct  task*) isr_v->_main_stask;
	os_data.activ->top_stack = (uint32_t) _irq_tmp - 64; // -m
	os_data.activ->task_names = (char*)service_txt;
	os_data.activ->stack_area_size = (SERVISE_SIZE + 7) & 0xFFFFFFF8;
	os_data.activ->task_nomer = 2;
	os_data.activ->activ_time = (__SYSHCLK / 1000) - 6;



	_os_wb->w_activ = (uint32_t*)isr_v->_main_stask;
	os_data.activ->task_head = (struct  task*) _irq_tmp;
	os_data.activ->task_tail = (struct  task*) _irq_tmp;
	os_data.activ->task_form_head = (struct  task*) _irq_tmp;
	os_data.activ->task_names = (char*) main_txt;
	os_data.activ->stack_area_size = main_sizel;
	os_data.activ->stack_area_used = 0;
	os_data.activ->activ_time = (__SYSHCLK / 1000) - 6;
	os_data.activ->task_nomer = 1;
	os_data.activ->link_data = 0;
	os_data.activ->link_head = 0;
	os_data.activ->link_tail = 0;
	os_data.activ->link_task_nomer = 0;
	os_data.activ->task_mode = 0;

    volatile struct _os_new_stack* _os_new_r;
    _os_new_r = (struct _os_new_stack*)_irq_tmp - 1;
    _os_new_r->lr = 0x1000000;
    _os_new_r->pc = (void*)service ;
    _os_new_r->psr = 0x1000000;

    */

/*
struct _os_new_stack
{
    uint32_t    r4;
    uint32_t    r5;
    uint32_t    r6;
    uint32_t    r7;
    uint32_t    r8;
    uint32_t    r9;
    uint32_t    r10;
    uint32_t    r11;
    uint32_t    r0;
    uint32_t    r1;
    uint32_t    r2;
    uint32_t    r3;
    uint32_t    r12;
    uint32_t    lr;
    void*       pc;
    uint32_t    psr;

};

struct _os_rwr
{
    uint32_t    r4; // 0
    uint32_t    r5; // 1
    uint32_t    r6; // 2
    uint32_t    r7; // 3
    uint32_t    r8; // 4
    uint32_t    r9; // 5
    uint32_t    r10; // 6
    uint32_t    r11; // 7
    uint32_t    r0; // 8
    uint32_t    r1; // 9
    uint32_t    r2; // 10
    uint32_t    r3; // 11
    uint32_t    r12; // 12
    uint32_t    lr; // 13
    uint32_t       pc; // 14
    uint32_t    psr; // 15
    uint32_t    task_head; //  16
    uint32_t    task_tail; // 17
    uint32_t    task_form_head;	// 18
    uint32_t    top_stack;  // 19
    uint32_t    swap_data;  // 20
    uint32_t    task_names; // 21
    uint32_t    stack_area_size__used;  // 22
    uint32_t    activ_time_task_nomer;  // 23
    uint32_t    link_data;  // 24
    uint32_t    link___mode;  // 25
    //26

};
*/

