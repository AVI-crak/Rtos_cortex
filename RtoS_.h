/**
 @file+   RtoS_cortex_m7.S
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

#include <stdint.h>  // типы данных
/// _estack  Общий адрес стека до модификаций
/// _irqsize  Размер стека прерываний
///_ebss  Последний используемый адрес памяти (.data + .bss)





#ifdef _RtoS_
 extern "C" {
#endif /* _RtoS_ */

#include "stm32f7xx.h"




struct _os_basic
{
    struct task* activ;                     ///#00- Список активных
    struct task* delay;                     ///#04- Список спящих
    struct task* hold;                      ///#08- Список замороженных
    struct task* service;          ///#12- Список обслуживания
	struct task* task_switch;				///#16- Новый/старый таск
    const uint32_t nvic_stack;              ///#20- Адрес стека прерываний, Task maximum NVIC used stack
    const uint16_t nvic_size;               ///#24- Размер стека прерываний,
    volatile uint16_t use_nvic_size;        ///#26- Использованный стек прерываний
    struct task* main_task;			///#28- Адрес стека main
    uint32_t* use_task_stop;       ///#32- Последний адрес стека, Stack last address
    const uint32_t tick_1ms;				///#36- Таймер активности задачи 100%, Task activity timer 100%
    const volatile int32_t system_us;       ///#40- Системное время, System time counter
    const uint32_t* malloc0_start;           ///#44- Первый адрес malloc0, First malloc0 address
    volatile uint32_t* malloc0_stop;          ///#48- Последний адрес malloc0, Last malloc0 address
    const uint32_t* malloc1_start;           ///#52- Первый адрес malloc1, First malloc1 address
    volatile uint32_t* malloc1_stop;          ///#56- Последний адрес malloc1, Last malloc1 address
}os_data;



typedef enum
{
    task_activ =0,
    activ_error,
    task_delay,
    delay_error,
    task_hold,
    hold_error,
    serv,
    serv_error,
    new_task,
    new_task_error,
    delete_task,
    delete_task_error,
    new_ram,
    new_ram_error,
    delete_ram,
    delete_ram_error,
}task_status_TypeDef;

struct  task   /// новое 40b
{
    struct task*            task_head;          /// 0x00, #00  *32b,- Адрес новой задачи,
    struct task*            task_tail;          /// 0x04, #04, *32b,- Адрес старой задачи,
    struct task*            task_form_head;		/// 0x08, #08, *32b,- Задачи в общем стеке,
    uint32_t       top_stack;          /// 0x0C, #12, 32b,- Cтек задачи, Stack pointer
    uint32_t       swap_data;          /// 0x10, #16, 32b,- Внешние данные,
    char*    		task_names;         /// 0x14, #20, 32b,- Имя задачи, Task name
    uint16_t       stack_area_size;    /// 0x18, #24, 16b,- Размер стека, Task stack size
    uint16_t       stack_area_used;    /// 0x1A, #26, 16b,- Рабочий стек, Task maximum used stack
    struct
        {
            uint32_t   activ_time:24;      /// 0x1C, #28, 24b,- Таймер активности задачи, Task activity timer
            uint32_t	task_nomer:8;       /// 0x1F, #31, 8b,- Номер таска, Task unique ID
        };
    uint8_t*       link_data;          /// 0x20, #32, *32b,- Буфер обмена 256b, Ring clipboard
    uint8_t        link_head;          /// 0x24, #36, 8b,- Голова, Head
    uint8_t        link_tail;          /// 0x25, #37, 8b,- Хвост, Tail
    uint8_t        link_task_nomer;    /// 0x26, #38, 8b,- Номер звонка, Call number
    task_status_TypeDef     task_mode;          /// 0x27, #39, 8b,- Режим таска, Task mode
};





/// Запрос ресурса, бесконечный цикл - пока не освободится
/// Resource request, endless loop while resource not released
uint32_t os_resource_ask (uint32_t *name_resource);


/// Освободить ресурс
/// Release resourse
uint32_t os_resource_free (uint32_t *name_resource);


/// Функция отложенного события по циклическому таймеру
/// пример
/// static int32_t alarm_mc2;
/// if (sTask_alarm_mc(&alarm_mc1,1000)) { действие каждую новую секунду }
int32_t os_alarm_ms(int32_t * timer_name, int32_t timer_ms);


/// Функция фиксированного времени сна по циклическому таймеру
/// пример
/// static int32_t alarm_mc1; os_tim_mc(&alarm_mc1,1000);
/// активное состояние 0-999мс, сон 999-1мс, цикл 1с
int32_t os_tim_ms(int32_t * timer_name, int32_t timer_mc);



/// Выделить память (размер)
void *os_malloc(int32_t size );



/// Удалить память (адрес)
void os_free (void* malloc_adres);


/// sTask_wake (& глобальный флаг) разбудить задачу
//void sTask_wake(volatile uint32_t* task_global_flag);



/// sTask_wait (& глобальный флаг) остановить задачу в ожидание пинка
//void sTask_wait(volatile uint32_t* task_global_flag);




/// sDelay_mc (в миллисекундах)
void os_Delay_ms(uint32_t delay_mc);



//void __attribute__ ((weak))___sRandom(void);


/// Старт ОS
/// частота ядра в гц, размер стека майна,
///       > размер стека прерываний, стартовое время задачи в микросекундах
void os_Run(const uint16_t main_size );



/// Барьер оптимизатора GCC
static inline void __memory(void){asm volatile ("nop" : : : "memory");}

/// Измерение рабочего размера стека прерываний
//void __attribute__ ((weak)) sHandler_zize(void);


/// Включить прерывание, преоритет (от 14 до 0)
void  os_EnableIRQ(IRQn_Type IRQn, uint8_t priority);




/// Отключить прерывание - после запуска ос
void os_DisableIRQ(IRQn_Type IRQn);





/// Новая задача - после запуска ос
uint32_t os_Task_new (void (*taskS_func),uint16_t task_size,uint8_t task_time_rate,char* const task_func_name);




/// Уступить время - только внутри работающей задачи
__attribute__( ( always_inline ) ) static inline void os_pass (void)
{
    asm volatile    ("push   {r3}               \n\t"
                     "mov   r3, #0		\n\t" // __switch
                    "svc    0x0         \n\t"
                    "pop    {r3}                \n\t"
                    :::"memory");
}

/// Убить задачу - только внутри работающей задачи
void os_Task_del(void);



uint32_t os_Ranlom(uint32_t range);






/// Bit operation, 1 in the mask resets the data
__attribute__( ( always_inline ) ) static inline uint32_t __BIC (uint32_t data, uint32_t mask)
{
  uint32_t result;
  asm volatile ("bic %0, %1, %2" : "=l" (result) : "l" (data), "l" (mask) );
  return(result);
}





/*
Обратите внимание, что для ARM вы можете указать тип прерывания, которое нужно обработать, добавив необязательный параметр к атрибуту прерывания, например:

          void f () __attribute__ ((interrupt ("IRQ")));

Допустимыми значениями для этого параметра являются: IRQ, FIQ, SWI, ABORT и UNDEF.
 */




#ifdef _RtoS_
}
#endif /* _RtoS_ */

#define _RtoS_






