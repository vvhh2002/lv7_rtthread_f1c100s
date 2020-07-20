/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-02-08     Zhangyihong  the first version
 * 2018-04-03     XY           gt9xx for 1024 * 600
 * 2018-04-14     liu2guang    optimize int and rst to pin framework
 * 2017-08-08     XY           imxrt1052
 * 2018-10-29     XY
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_gpio.h"
#include "drv_touch.h"
#include "drv_i2c.h"

#ifdef RT_USING_TOUCH

#define TP_INT_PIN GPIO_PORT_E,GPIO_PIN_10
#define TP_RST_PIN GPIO_PORT_A,GPIO_PIN_0

#ifndef TP_INT_PIN
#error "Please config touch panel INT pin."
#endif
#ifndef TP_RST_PIN
#error "Please config touch panel RST pin."
#endif

#ifndef IIC_RETRY_NUM
#define IIC_RETRY_NUM 2
#endif

#define GT9xx_TS_ADDR               (0x14)

#define CFG_VERSION 0x4780
#define PRODUCT_ID  0x4081
#define State_ADD   0x4E81
#define COOR_ADD    0x4E81
#define CFG911_LEN  186

#define TOUCH_I2C_NAME  "i2c0"

#if 1
#define TPDEBUG     rt_kprintf
#define LOG         rt_kprintf
#else
#define TPDEBUG(...)
#define LOG(...)
#endif

struct rt_i2c_bus_device *i2c_bus;
static int gt911_read(struct rt_i2c_bus_device * dev, rt_uint16_t reg, rt_uint8_t * buf, int len)
{
    struct rt_i2c_msg msgs[2];
    int ret;

    msgs[0].addr = dev->addr;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].len = 2;
    msgs[0].buf = &reg;

    msgs[1].addr = dev->addr;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].len = len;
    msgs[1].buf = buf;
    //此处直接调用drv_i2c中的方法，请保证没有线程冲突
    //原始i2c_transfer(dev->i2c, msgs, 2) 如果在中断中调用，会因为锁问题停止
    ret=ingenic_i2c_xfer(dev, msgs, 2);//this must 2

    return ret;// < 0 ? ret : (ret != 2 ? -EIO : 0);
}

static rt_bool_t gt911_write(struct rt_i2c_bus_device * dev, rt_uint16_t reg, rt_uint8_t * buf, int len)
{
    struct rt_i2c_msg msg;
    rt_uint8_t mbuf[256];

    if(len > sizeof(mbuf) - 1)
        len = sizeof(mbuf) - 1;
    mbuf[0] = reg & 0xff;
    mbuf[1] = (reg >> 8) & 0xff;
    memcpy(&mbuf[2], buf, len);

    msg.addr = dev->addr;
    msg.flags = 0;
    msg.len = len + 2;
    msg.buf = &mbuf[0];
    //此处直接调用drv_i2c中的方法，请保证没有线程冲突
    //原始i2c_transfer(dev->i2c, &msg, 1) 如果在中断中调用，会因为锁问题停止
    if(ingenic_i2c_xfer(dev, &msg, 1) != 1)
        return RT_FALSE;
    return RT_TRUE;
}


void gt9xx_hw_reset(rt_uint8_t address)
{
      gpio_set_func(TP_RST_PIN,IO_OUTPUT);
      gpio_direction_output(TP_RST_PIN,0);
        rt_thread_mdelay(20);//T2>=10ms
        //HIGH: 0x14,
        gpio_set_func(TP_INT_PIN,IO_OUTPUT);
        gpio_direction_output(TP_INT_PIN,1);

        rt_thread_mdelay(1);//T3>=100us

        gpio_direction_output(TP_RST_PIN,1);
        rt_thread_mdelay(8);//T4>=5ms
      //  rt_pin_write(TP_INT_PIN, PIN_LOW);
      //  rt_thread_mdelay(50);//50ms将INT悬浮输入
        gpio_set_pull_mode(TP_INT_PIN,PULL_DISABLE);
     //   gpio_set_func(GPIO_PORT_E,GPIO_PIN_3,IO_INPUT);
        gpio_direction_input(TP_INT_PIN);
       // rt_pin_mode(TP_INT_PIN, PIN_MODE_INPUT);
}

static int goodix_i2c_test(struct rt_i2c_bus_device * dev)
{
    int ret;
    int retry=0;
    rt_uint8_t test;

    while (retry++ < 2)
    {
        ret = gt911_read(dev, CFG_VERSION, &test, 1);
        if(ret == 2)
        {
            LOG("i2c test CFG_VER:%c",test);
            return 1;
        }
        LOG("i2c test failed attempt %d: %d",retry,test);
        rt_thread_mdelay(20);
    }
    return 0;
}

static int goodix_read_version(struct rt_i2c_bus_device * dev)
{
    int error;
    rt_uint8_t buf[6];
    //char id_str[6];
    //u16_t gt_version;

    error = gt911_read(dev, PRODUCT_ID, buf, sizeof(buf));
    if (error !=2) {
        LOG("read id fail! error:%d",error);
        return error;
    }

    //memcpy(id_str, buf, 6);
    //id_str[4] = 0;

    //gt_version = get_unaligned_le16(&buf[4]);

    LOG("GT911 verson: %c%c%c%c_%02x%02x",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);

    return 0;
}

static int goodix_send_cfg(struct rt_i2c_bus_device  * dev)
{
    int i;
    rt_uint8_t check_sum = 0;
    rt_uint8_t ret;

    rt_uint8_t config0[CFG911_LEN]={//one point
            0x41,0x20,0x03,0xE0,0x01,0x01,0x3C,0x00,0x01,0x08,
            0x19,0x05,0x4A,0x3A,0x03,0x0F,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x18,0x1A,0x1E,0x14,0x89,0x2A,0x09,
            0x2F,0x2B,0x88,0x13,0x00,0x00,0x01,0xB8,0x03,0x2D,
            0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x10,0x12,
            0x14,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x02,0x04,0x06,0x08,0x0A,0x0F,0x10,
            0x12,0x16,0x18,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,
            0x24,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0xF2,0x01
            };
    /*u8_t config0[CFG911_LEN]={//tow point
            0x41,0x20,0x03,0xE0,0x01,0x02,0x3C,0x00,0x01,0x08,
            0x19,0x05,0x4A,0x3A,0x03,0x0F,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x18,0x1A,0x1E,0x14,0x89,0x2A,0x09,
            0x2F,0x2B,0x88,0x13,0x00,0x00,0x01,0xB8,0x03,0x2D,
            0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x10,0x12,
            0x14,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x02,0x04,0x06,0x08,0x0A,0x0F,0x10,
            0x12,0x16,0x18,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,
            0x24,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0xF1,0x01
            };*/
    for (i = 0; i < (CFG911_LEN-2); i++)
        check_sum += config0[i];
    check_sum = (~check_sum) + 1;
    if (check_sum != config0[CFG911_LEN-2]) {
        LOG("check_sum err:%02x",check_sum);
        return 0;
    }

    if (config0[CFG911_LEN - 1] != 1) {
        LOG("cfg err:last byte must be 0x01");
        return 0;
    }

    ret = gt911_write(dev, CFG_VERSION, config0, CFG911_LEN);
    if(ret)//no err
    {
        LOG("send cfg ok");
        return 0;
    }else
    {
        LOG("send cfg err");
        return 1;
    }
}

static void gt911_interrupt(void * data){
    struct rt_i2c_bus_device  * dev=i2c_bus;
     struct touch_message msg;
    on_touch_message_handle on_touch_callback=(on_touch_message_handle)data;
    rt_uint8_t cmd[2];
    rt_uint8_t buf[8] = {0};
    static rt_uint8_t s_tp_down = 0;
    int ret;


        gpio_irq_disable(GPIO_PORT_E,GPIO_PIN_3);
        ret = gt911_read(dev, COOR_ADD, buf, 8);
       if(ret !=2){
            LOG("int read coord err:%d",ret);
            return;
       }
       msg.x = ((rt_uint16_t)buf[3] << 8) | buf[2];
       msg.y = ((rt_uint16_t)buf[5] << 8) | buf[4];
       if((buf[0] & 0x01) == 0)
           {
               if(s_tp_down)
               {
                   s_tp_down = 0;
                   msg.event = TOUCH_EVENT_UP;
               }
               else
               {
                  msg.event = TOUCH_EVENT_NONE;
               }
           }
           else
           {


               if(s_tp_down)
               {
                   msg.event = TOUCH_EVENT_MOVE;
               }
               else
               {
                   msg.event = TOUCH_EVENT_DOWN;
                   s_tp_down = 1;
               }
           }
       if(on_touch_callback!=RT_NULL){
           on_touch_callback(&msg);
       }
      // LOG("Touch panel x=%d y=%d event=0x%02x",msg.x,msg.y,msg.event );
       rt_uint8_t temp=0;
       ret = gt911_write(dev, State_ADD, &temp, 1);
       if(!ret)
            LOG("clear status err:%d",ret);

        gpio_irq_enable(GPIO_PORT_E,GPIO_PIN_3);
}
static rt_bool_t gt9xx_probe()
{
    rt_uint8_t cmd[2];
    rt_uint8_t buffer[6] = {0};
  //  gpio_irq_enable(GPIO_PORT_E,GPIO_PIN_3);

    i2c_bus = rt_i2c_bus_device_find(TOUCH_I2C_NAME);
    RT_ASSERT(i2c_bus);

    if(rt_device_open(&i2c_bus->parent, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        TPDEBUG("[TP] %s I2C not open error!\n", TOUCH_I2C_NAME);
        return;
    }
    i2c_bus->addr=GT9xx_TS_ADDR;
    gt9xx_hw_reset(GT9xx_TS_ADDR);
    //gt9xx_soft_reset(i2c_bus);
    rt_thread_delay(RT_TICK_PER_SECOND / 5);

    if (!goodix_i2c_test(i2c_bus) )
    {
        TPDEBUG("[TP] %s goodix_i2c_test!  error\n", __func__);
        return RT_FALSE;
    }

   if(goodix_read_version(i2c_bus))
   {
        TPDEBUG("[TP] %s goodix_read_version error!\n", __func__);
        return RT_FALSE;
    }
   rt_thread_mdelay(50);//maybe need 50ms before send config
   if(goodix_send_cfg(i2c_bus)){

   }

    return RT_FALSE;
}
void start_touch_listen(on_touch_message_handle call_back){
    gpio_set_func(TP_INT_PIN,IO_FUN_5);
    gpio_set_irq_type(TP_INT_PIN,NEGATIVE);
    gpio_set_irq_callback(TP_INT_PIN,gt911_interrupt,call_back);
    gpio_irq_enable(TP_INT_PIN);
}
static int gt9xx_driver_register(void)
{
    gt9xx_probe();
    return RT_EOK;
}
INIT_DEVICE_EXPORT(gt9xx_driver_register);

#endif
