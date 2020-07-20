/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-04     36380       the first version
 */
#ifndef DRIVERS_DRV_TOUCH_H_
#define DRIVERS_DRV_TOUCH_H_
#include "rtthread.h"
#include "rtdevice.h"
#define TOUCH_EVENT_UP      (0x01)
#define TOUCH_EVENT_DOWN    (0x02)
#define TOUCH_EVENT_MOVE    (0x03)
#define TOUCH_EVENT_NONE    (0x80)
struct touch_message
{
    rt_uint16_t x;
    rt_uint16_t y;
    rt_uint8_t event;
};
typedef struct touch_message * touch_message_t;
typedef (*on_touch_message_handle)(touch_message_t);
void start_touch_listen(on_touch_message_handle call_back);

#endif /* DRIVERS_DRV_TOUCH_H_ */
