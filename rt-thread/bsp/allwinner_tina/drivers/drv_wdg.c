/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-08     36380       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#ifdef  RT_USING_WDT
#include <drivers/watchdog.h>
#include "iomem.h"
#define WDG_BASE_ADDR (0x01C20CA0)
enum {
    WDG_IRQ_EN      = 0x00,
    WDG_IRQ_STA = 0x04,
    WDG_CTRL        = 0x10,
    WDG_CFG     = 0x14,
    WDG_MODE        = 0x18,
};
struct wdt_driver
{
    unsigned long in_use;

    void* priv;
};



static int wdt_timeout = 0xa;
static const int wdt_timeout_map[] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x0, 0x7, 0x0, 0x8, 0x0, 0x9, 0x0, 0xa, 0x0, 0xb,
};
static rt_err_t watchdog_init(rt_watchdog_t *wdt)
{
    rt_kprintf("watchdog_init\n");
    struct wdt_driver *wdt_drv = wdt->parent.user_data;
    if (wdt_drv->in_use) return -RT_EBUSY;

    write32(WDG_BASE_ADDR + WDG_IRQ_EN, 0x0);
    write32(WDG_BASE_ADDR + WDG_IRQ_STA, 0x1);
    write32(WDG_BASE_ADDR+ WDG_CFG, 0x1);
    write32(WDG_BASE_ADDR+ WDG_MODE, 0x0);  //设为14秒必需喂狗
    write32(WDG_BASE_ADDR+ WDG_CTRL, (0xa57 << 1) | (1 << 0));

    return RT_EOK;
}

static rt_err_t watchdog_ctrl(rt_watchdog_t *wdt, int cmd, void *arg)
{
    rt_uint32_t val;
    rt_uint32_t timeout;

    switch (cmd)
    {
        case RT_DEVICE_CTRL_WDT_START:
            break;

        case RT_DEVICE_CTRL_WDT_STOP:
            write32(WDG_BASE_ADDR + WDG_MODE, 0x0);
              write32(WDG_BASE_ADDR + WDG_CTRL, (0xa57 << 1) | (1 << 0));
            break;
        case RT_DEVICE_CTRL_WDT_KEEPALIVE:
            val = read32(WDG_BASE_ADDR + WDG_MODE);
           val &= ~(0xf << 4);
             val |= (wdt_timeout << 4) | (0x1 << 0);
             write32(WDG_BASE_ADDR + WDG_MODE, val);
            write32(WDG_BASE_ADDR+ WDG_CTRL, (0xa57 << 1) | (1 << 0));
            break;

        case RT_DEVICE_CTRL_WDT_SET_TIMEOUT:
            timeout=*((rt_uint32_t *)(arg));

            if(timeout < 1)
                timeout = 1;
            if(timeout > 16)
                timeout = 16;
            if(wdt_timeout_map[timeout] == 0)
                        timeout++;

            wdt_timeout = wdt_timeout_map[timeout];
            val = read32(WDG_BASE_ADDR + WDG_MODE);
            val &= ~(0xf << 4);
            val |= (wdt_timeout << 4) | (0x1 << 0);
             write32(WDG_BASE_ADDR + WDG_MODE, val);
            write32(WDG_BASE_ADDR+ WDG_CTRL, (0xa57 << 1) | (1 << 0));
            break;

        case RT_DEVICE_CTRL_WDT_GET_TIMEOUT:
            arg = &wdt_timeout;
            break;

        case RT_DEVICE_CTRL_WDT_GET_TIMELEFT:
            arg = &wdt_timeout;
            break;

        default:
            return -RT_EIO;
    }

    return RT_EOK;
}

struct rt_watchdog_ops watchdog_ops =
{
    .init = &watchdog_init,
    .control = &watchdog_ctrl,
};

int rt_hw_wdt_init(void)
{

    rt_watchdog_t *wdt_dev;
    struct wdt_driver *wdt_drv;

   wdt_drv = (struct wdt_driver *)rt_malloc(sizeof(struct wdt_driver));
   rt_memset(wdt_drv, 0, sizeof(struct wdt_driver));

    wdt_dev = (rt_watchdog_t *)rt_malloc(sizeof(rt_watchdog_t));

    if (wdt_dev == RT_NULL)
    {
        rt_kprintf("ERROR: %s rt_watchdog_t malloc failed\n", __func__);
    }



    wdt_dev->ops = &watchdog_ops;

    rt_hw_watchdog_register(wdt_dev, "wdt", RT_DEVICE_OFLAG_RDWR, wdt_drv);
    watchdog_init(wdt_dev);
    rt_uint32_t timeout=13;
    watchdog_ctrl(wdt_dev,RT_DEVICE_CTRL_WDT_SET_TIMEOUT,&timeout);
    return 0;
}

INIT_BOARD_EXPORT(rt_hw_wdt_init);
#endif
