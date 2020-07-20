/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author         Notes
 * 2020-04-16     bigmagic       first version
 */

#include <rthw.h>
#include <rtthread.h>

#include "board.h"
#include "drv_uart.h"

#include "cp15.h"
#include "mmu.h"

struct mem_desc platform_mem_desc[] = {
    {0x0, 0x6400000, 0x0, NORMAL_MEM},
    {0xFE000000, 0xFE400000, 0xFE000000, DEVICE_MEM},//uart gpio
    {0xFF800000, 0xFFA00000, 0xFF800000, DEVICE_MEM} //gic
};

const rt_uint32_t platform_mem_desc_size = sizeof(platform_mem_desc)/sizeof(platform_mem_desc[0]);

void rt_hw_timer_isr(int vector, void *parameter)
{
    ARM_TIMER_IRQCLR = 0;
    rt_tick_increase();
}

void rt_hw_timer_init(void)
{
    rt_hw_interrupt_install(ARM_TIMER_IRQ, rt_hw_timer_isr, RT_NULL, "tick");
    rt_hw_interrupt_umask(ARM_TIMER_IRQ);
    /* timer_clock = apb_clock/(pre_divider + 1) */
    ARM_TIMER_PREDIV = (250 - 1);

    ARM_TIMER_RELOAD = 0;
    ARM_TIMER_LOAD   = 0;
    ARM_TIMER_IRQCLR = 0;
    ARM_TIMER_CTRL   = 0;

    ARM_TIMER_RELOAD = 10000;
    ARM_TIMER_LOAD   = 10000;

    /* 23-bit counter, enable interrupt, enable timer */
    ARM_TIMER_CTRL   = (1 << 1) | (1 << 5) | (1 << 7);
}

void idle_wfi(void)
{
    asm volatile ("wfi");
}

/**
 *  Initialize the Hardware related stuffs. Called from rtthread_startup() 
 *  after interrupt disabled.
 */
void rt_hw_board_init(void)
{
    /* initialize hardware interrupt */
    rt_hw_interrupt_init();
    /* initialize uart */
    rt_hw_uart_init();      // driver/drv_uart.c
#ifdef RT_USING_CONSOLE
    /* set console device */
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif /* RT_USING_CONSOLE */

#ifdef RT_USING_HEAP
    /* initialize memory system */
    rt_kprintf("heap: 0x%08x - 0x%08x\n", RT_HW_HEAP_BEGIN, RT_HW_HEAP_END);
    rt_system_heap_init(RT_HW_HEAP_BEGIN, RT_HW_HEAP_END);
#endif
        /* initialize timer for os tick */
    rt_hw_timer_init();
    rt_thread_idle_sethook(idle_wfi);

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}
