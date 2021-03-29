/*
 @file    RtoS_cortex_m7.S
 @author  AVI_crak
 @version V-0.14
 @date    1-january-2020
 @brief   RTOS

 license
 Million helicopters and one cent.

 discussion forum
 http://forum.ixbt.com/topic.cgi?id=48:11735

 repository
 https://github.com/AVI-crak/Rtos_cortex

*/
 .syntax unified
 .cpu cortex-m7
 .arch armv7e-m
 .thumb_func


 .equ   __Test_psp, 0               // checking for stack overflow errors
 .equ   __SysTick_CTRL, 0xE000E010  // #0xE010 SysTick->CTRL, #0xE014 SysTick->LOAD, #0xE018 SysTick->VAL


 .align 2
 .section  .text.SVC_Handler
 .globl     SVC_Handler
 .type    SVC_Handler, %function

SVC_Handler:
            ldr     r12, = os_data
            tbh     [pc, r3, lsl #1]
SVC_Table:
    .hword    ((__switch - SVC_Table)/2)            // 0
    .hword    ((__sleep_tasks - SVC_Table)/2)       // 1
    .hword    ((__delay_new - SVC_Table)/2)         // 2
    .hword    ((__ret_service - SVC_Table)/2)       // 3
    .hword    ((__to_service - SVC_Table)/2)        // 4
    .hword    ((__wake - SVC_Table)/2)              // 5
    .hword    ((__freeze - SVC_Table)/2)            // 6
    .hword    ((__task_new - SVC_Table)/2)          // 7
    .hword    ((__delete_task - SVC_Table)/2)       // 8
    .hword    ((__EnableIRQ - SVC_Table)/2)         // 9
    .hword    ((__DisableIRQ - SVC_Table)/2)        // A
    .hword    ((__os_res_ask - SVC_Table)/2)        // B


__delete_task:    // task_status = delete_task, swap_data - адрес структуры
            ldr        r1, [r12, #12]            // task* service
            ldr        r0, [r1, #4]
            ldr        r2, [r1]
            subs       r3, r0, r1
            iteee      eq
            streq      r3, [r12, #12]
            strne      r2, [r12, #12]
            strne      r2, [r0]
            strne      r0, [r2, #4]
            bx         lr


__to_service:    // task_status уже содержит код операции, swap_data - данные или адрес структуры
            ldr        r0, [r12]                // active task
            mov        r3, #12                  // list nomer service
            b          __switch_rxx


__freeze:
            ldr        r0, [r12]                // активная задача
            mov        r3, #8                   // list nomer hold
            b          __switch_rxx


__delay_new:    //r#16 swap_data = Delay_mc +
            ldr        r3, [r12, #40]           // system_us
            ldr        r0, [r12]                // active task
            mov        r2, #2                   // task_status task_delay
            strb       r2, [r0, #39]
            ldr        r1, [r0, #16]            // Delay_mc +
            add        r1, r1, r3
            str        r1, [r0, #16]            // (swap_data) saved the sleep point
            mov        r3, #4                   // list nomer delay
            b          __switch_rxx


__switch:    // переключатель задач
            ldr        r0, [r12]
            ldr        r3, [r0]
            cmp        r0, r3
            it         eq
            bxeq       lr



__switch_rxx:    //switch: r0 save, r3<17 r0 move to inactive
 .ifdef  __Test_psp
            ldrh       r1, [r0, #26]            // stack area used
            mrs        r2, psp
            sub        r2, r0, r2
            add        r2, r2, #240             // текущий актуальный размер стека( 200 + 40)
            cmp        r2, r1
            ittt       hi
            strhhi     r2, [r0, #26]            // new stack area used
            ldrhhi     r1, [r0, #24]            // stack area size
            cmphi      r2, r1
            bls        __switch_sawe            // no overflow
            ldrb       r1, [r0, #39]
            add        r1, r1, #1
            strb       r1, [r0, #39]            // + _error
            ldr        r1, [r0, #4]
            ldr        r3, [r0]                 // no free space - delete task
            str        r1, [r3, #4]
            str        r3,    [r1]
            b          __switch_read
 .endif
__switch_sawe:    //switch: r0 save, r3 read
            mrs        r1, psp
            stmdb      r1!, {r4-r11}            // save registers
            tst        lr, #0x04
            itte       eq
            mrseq      r2, msp
            ldreq      r2, [r2, #20]
            movne      r2, lr
            tst        r2, #0x10                // used math?
            itt        eq
            vstmdbeq   r1!, {s16-s31}           // save math registers
            addeq      r1, r1, #1
            str        r1, [r0, #12]            // saved task r0 stack address
            cmp        r3, #17
            bhi        __switch_read            // r3>17 there the task address
            mov        r4, r3                   //
            ldr        r3, [r0]                 // r3 new active
            ldr        r1,    [r0, #4]
            ldr        r2, [r12, r4]
            str        r1, [r3, #4]
            str        r3, [r1]                 // removed R0 from active
            cmp        r2,    #0
            itttt      eq                       // inactive list empty
            streq      r0, [r12, r4]
            streq      r0, [r0]
            streq      r0, [r0, #4]
            beq        __switch_read
            ldr        r1, [r2, #4]
            str        r2, [r0]
            str        r0, [r2, #4]
            str        r0, [r1]
            str        r1, [r0, #4]


__switch_read:    // rx3 - new task - data loading
            str        r3, [r12]                // new active
            ldr        r12, =__SysTick_CTRL
            ldr        r0, [r3, #12]            // read the stack address
            ldr        r3, [r3, #28]            // activ time
            mov        r1, #0
            tst        r0, #0x01                // return recovery for stream
            itte       ne
            subne      r0, r0, #1
            vldmiane   r0!, {s16-s31}
            moveq      r1, #0x10
            ldmia      r0!, {r4-r11}            // read stored
            bfc        r3, #24, #8
            tst        lr, #0x04                // recovery state override
            msr        psp, r0                  // restore top of task stack
            ittt       ne
            bicne      lr, #0x10
            orrne      lr, lr, r1
            bne        __switch_r1
            mrs        r0, msp                  // restore return for interrupt
            ldr        r2, [r0, #20]
            bic        r2, #0x10
            orr        r2, r2, r1
            str        r2, [r0, #20]
__switch_r1:
            str        r3, [r12, #4]            // #0xE000E014 SysTick->LOAD
            mov        r1, #7
            str        r1, [r12, #8]            // #0xE000E018 SysTick->VAL
            dsb        0xF
            isb        0xF
            bx         lr


__task_new:
            push       {r4, lr}
            ldr        r0, [r12, #16]            // адрес новой задачи
            mov        r4, #16                   // ссылка на task* task_switch
            bl         __to_activ                // __to_activ
            ldr        r0, [r12, #12]
            mov        r4, #12                   // ссылка на task* service
            bl         __to_activ                // __to_activ
            pop        {r4, pc}


__ret_service:
            push       {r4, lr}
            ldr        r0, [r12, #12]
            mov        r4, #12                   // ссылка на task* service
            bl         __to_activ                // __to_activ
            pop        {r4, pc}


__wake: // r0(uint8_t* global_task_nomer) разбудить
            mov        r3, r0
            ldr        r0, [r12, #8]            // Адрес задачь ожидающих пинка
            cmp        r0, #0
            it         eq
            bxeq       lr                       // а там пусто...
            push       {r4, r5, r6, lr}
            mov        r4, #8                   // ссылка на task* hold
            ldr        r2, [r0, #4]             // task_tail
            ldrb       r6, [r3]                 // читаем номер задачи
__wake_next:
            ldrb       r5, [r0, #31]            //#31 task_nomer
            ldr        r1, [r0]
            subs       r5, r5, r6
            itt        eq
            strbeq     r5, [r3]                 // обнулить флаг
            bleq       __to_activ               // __to_activ
            cmp        r0, r2
            it         eq
            popeq      {r4, r5, r6, pc}
            mov        r0, r1
            b           __wake_next


__sleep_tasks:    // обслуживание задержки
            ldr        r3, [r12, #40]           //#40 system_us
            ldr        r0, [r12, #4]            //#4 task* delay
            add        r3, r3, #1
            str        r3, [r12, #40]           //#40 system_us++
            cmp        r0, #0
            it         eq
            bxeq       lr
            push       {r4, r5, lr}
            mov        r4, #4                   // ссылка на task* delay
            ldr        r2, [r0, #4]             // task_tail
__sleep_next:
            ldr        r5, [r0, #16]            //#16 swap_data (delay)
            ldr        r1, [r0]
            subs       r5, r5, r3
            itt        eq
            streq      r5, [r0, #16]            // == 0
            bleq       __to_activ
            cmp        r0, r2
            mov        r0, r1
            bne        __sleep_next
            pop        {r4, r5, pc}


__to_activ:        // r0 - задача на обмен, r4 - адрес структуры (4-8-12-16)
            push       {r1, r2, lr}
            ldr        r1, [r0]
            subs       r2, r1, r0
            itt        eq
            streq      r2, [r12, r4]            // закрыли
            beq        __to_activ1
            ldr        r2, [r0, #4]
            str        r1, [r2]                 // хвост на голову
            str        r2, [r1, #4]             // голова на хвост
            str        r1, [r12, r4]            // новая голова
__to_activ1:
            ldr        r2, [r12]                // активная задача в потоке
            ldr        r1, [r2, #4]             // хвост активной задачи
            str        r2,    [r0]
            str        r1, [r0, #4]
            str        r0, [r2, #4]
            str        r0, [r1]                 // хвост на новую
            mov        r2, #0
            strb       r2, [r0, #39]            //  task_activ =0,
            pop        {r1, r2, pc}



__EnableIRQ:    // ("r0") = IRQn, ("r1") = priority; ++
            movw       r12, #0xE70C
            movt       r12, #0xE000
            and        r0, #0xFF                //IRQn безопасность
            and        r1, #0x0F                //Priority безопасность
            lsl        r1, r1, #4               //сдвиг на границу
            mov        r3, r0
            and        r3, #0x0F
            sub        r3, r3, #4
            cmp        r0, #0xF1
            ittee      cs
            addwcs     r2, r12, #0x60C          //SCB->SHPx 0xE000ED18
            strbcs     r1, [r2, r3]
            subwcc     r2, r12, #0x30c          //NVIC->IPx 0xE000E400
            strbcc     r1, [r2, r0]
            and        r1, r0, #0x1F
            mov        r3, #0x1
            lsls       r1, r3, r1
            lsrs       r0, r0, #5
            lsls       r0, r0, #2
            subw       r2, r12, #0x60C          //NVIC_ISERx 0xE000E100
            str        r1, [r2, r0]
            bx         lr


__DisableIRQ: // ("r0") = IRQn; ++
//NVIC->ICER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F)); /* disable interrupt */
            mov        r3, #0x1F
            and        r1, r0, r3
            mov        r3, #0x1
            lsls       r2, r3, r1
            lsrs       r1, r0, #5
            lsls       r1, r1, #2
            movw       r3, #0xE180
            movt       r3, #0xE000
            str        r2, [r3, r1]
            bx         lr

__os_res_ask:
            ldr        r0, [r12]
            ldr        r1, [r0, #16]
            ldr        r2, [r1]
            cmp        r2, #0
            itt        eq
            streq      r0, [r1]
            streq      r2, [r0, #16]
            bx         lr


 .size SVC_Handler, . - SVC_Handler


 .align 2
 .globl     SysTick_Handler
 .type      SysTick_Handler, %function
 .section  .text.SysTick_Handler
SysTick_Handler:
            mov        r3, #0x00
            svc        0x0
            bx         lr
 .size SysTick_Handler, . - SysTick_Handler



