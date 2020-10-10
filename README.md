Небольшая по объёму ос с вытеснением по времени + приоритеты на 3 уровня. 
Задачи могут использовать математику, при этом обработка стека зависит от включенной/выключенной математики. Так как задач с математикой обычно намного меньше простых - вход в прерывания пользователя выполняются намного быстрее. Количество задач ограничено размером памяти мк и числом 253.

1) Активные задачи **struct task<<*>> activ** выполняются последовательно и постоянно. 
Время работы каждой отдельной задачи можно задать разным (в процентах 1-100), что является аналогом приоритета выполнения.
Важной функцией этой Ос является команда **os_pass()**, которая предназначена для добровольного переключения в момент когда задаче нечего делать.

2) Задачи которые выполняются с нормированным временным интервалом, либо через внешнее событие. Ос использует программное прерывание **SVC_Handler** - которое невозможно вытеснить в нормальном режиме работы. Таким образом достигается эксклюзивное выполнение кода без блокировок прерываний. Важно заметить - что обработка задач происходит в моменты внешних событий практически моментально, без длительных задержек. Точно так-же как и любое другое прерывание в этой системе.
**os_Delay_ms(мс)** стандартная задержка выполнения кода в мс. Обработка **struct task<<*>> delay** происходит каждые 1мс по системному таймеру TIM6 (можно изменить), которое определяет общее время **system_us**.
Задачу можно заморозить **os_freeze(<<*>> global_flag)**, и разбудить из этого состояния **os_wake(<<*>> global_flag)** (из задачи или прерывания). Время нахождения задачи в **struct task<<*>> hold** не ограничено, а для доступа необходимо знать "временный пароль". В этом случае задача должна сначала сообщить это "временный пароль" тому событию или задаче - которая будет её будить.

3) Функции что требуют много процессорного времени - выполняются в системной задаче **service()**, которая обслуживает **struct task<<*>> service** и печать состояния Ос на внешний терминал.
Это выделение памяти **os_malloc**, удаление памяти **os_free**, запуск новой задачи **os_Task_new**, удаление задачи **os_Task_del**, а так-же сбор данных о системе.

Запуск Ос однократный из **main()** функцией **os_Run()**. До запуска необходимо по возможности настроить всю периферию, для этих целей существует  **SystemInit()**.
Для того чтобы не бороться с ветряными мельницами, стек **main()** был принудительно смещён вниз на размер стека прерываний **_irqsize**, относительно **_estack** - который задаётся в таблице прерываний. Для файла startup_stm32fxxxx.s необходимо добавить и изменить строки:
 <<.>>equ   _irqsize, 600
 <<.>>equ   _main_stask,         ( _estack -   ((_irqsize + 200) <<&>> 0xFFFFFFF8))
 <<.>>equ   _irq_stack,          _estack
 
    g_pfnVectors:
 
  <<.>>word  **_main_stask**
  <<.>>word  Reset_Handler
 
  <<.>>word  NMI_Handler
  <<.>>word  HardFault_Handler
  <<.>>word  MemManage_Handler
  <<.>>word  BusFault_Handler
  <<.>>word  UsageFault_Handler
  <<.>>word  **_irq_stack**               /* добавить              */
  <<.>>word  **_irqsize**                   /* добавить              */
  <<.>>word  **_ebss**                      /* добавить              */
  <<.>>word  0
  <<.>>word  SVC_Handler
  <<.>>word  DebugMon_Handler
  <<.>>word  0
  <<.>>word  PendSV_Handler
  <<.>>word  SysTick_Handler
  
  Если g_pfnVectors не имеет пробелов в местах модификаций - то придётся менять часть алгоритма последующей обработки. Это не так сложно как кажется.

Простые CMSIS функции включения и выключения прерываний в условиях работы Ос запрещены, режим запрещает доступ к этим регистрам. 
Установка через **os_EnableIRQ(IRQn_Type IRQn, uint8_t priority)**, при этом приоритет от 1 до 14. Да, в CMSIS использует значения от 0 до 255, но физически регистр может вместить в себя всего 4 бита, да и то не всегда.
Удаление **os_DisableIRQ(IRQn_Type IRQn)**. 

Для эксклюзивного доступа к аппаратным физическим интерфейсам, или области памяти - используются функции запроса ресурса **os_resource_ask**, и освобождение ресурса **os_resource_free**.

Генератор случайных цифр имеет аппаратное решение для M4 и M7 **os_Ranlom(range)** с проверкой на слепые зоны.

репозиторий <https://github.com/AVI-crak/Rtos_cortex>
Средство контроля версий - черепаха <https://tortoisegit.org/]https://tortoisegit.org/>.