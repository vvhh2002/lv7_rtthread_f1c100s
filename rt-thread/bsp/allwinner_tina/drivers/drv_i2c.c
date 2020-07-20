#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>

#include "iomem.h"
#include "reg-ccu.h"
#include "board.h"
#include "drv_clock.h"
#include "drv_gpio.h"
#include "drv_i2c.h"
#include "interrupt.h"
#ifdef RT_USING_I2C

#ifdef RT_I2C_DEBUG
#define I2C_DBG(...)     rt_kprintf("[I2C]"),rt_kprintf(__VA_ARGS__)
#else
#define I2C_DBG(...)
#endif

#define I2C0_BASE_ADDR (0x01C27000)
#define I2C0_CCU_BUS_SOFT_RST_BASE (F1C100S_CCU_BASE | CCU_BUS_SOFT_RST3)
#define I2C0_CCU_BUS_SOFT_RST_BIT (1 << (80-64))
enum {
    I2C_ADDR            = 0x000,
    I2C_XADDR           = 0x004,
    I2C_DATA            = 0x008,
    I2C_CNTR            = 0x00c,
    I2C_STAT            = 0x010,
    I2C_CCR             = 0x014,
    I2C_SRST            = 0x018,
    I2C_EFR             = 0x01c,
    I2C_LCR             = 0x020,
};

enum {
    I2C_STAT_BUS_ERROR  = 0x00,
    I2C_STAT_TX_START   = 0x08,
    I2C_STAT_TX_RSTART  = 0x10,
    I2C_STAT_TX_AW_ACK  = 0x18,
    I2C_STAT_TX_AW_NAK  = 0x20,
    I2C_STAT_TXD_ACK    = 0x28,
    I2C_STAT_TXD_NAK    = 0x30,
    I2C_STAT_LOST_ARB   = 0x38,
    I2C_STAT_TX_AR_ACK  = 0x40,
    I2C_STAT_TX_AR_NAK  = 0x48,
    I2C_STAT_RXD_ACK    = 0x50,
    I2C_STAT_RXD_NAK    = 0x58,
    I2C_STAT_IDLE       = 0xf8,
};

#define TIMEOUT             0xff

struct ingenic_i2c_bus
{
    struct rt_i2c_bus_device parent;

    rt_uint32_t hwaddr;  // I2C0_BASE_ADDR
     rt_uint32_t irqno;  //  TWI0_INTERRUPT
} ;
static struct ingenic_i2c_bus ingenic_i2c0;

static const struct rt_i2c_bus_device_ops i2c_ops =
{
    ingenic_i2c_xfer,
    RT_NULL,
    RT_NULL
};


static struct ingenic_i2c_bus ingenic_i2c0;

static inline unsigned short i2c_readl(struct ingenic_i2c_bus *i2c,
        unsigned short offset)
{
    return readl(i2c->hwaddr + offset);
}

static inline void i2c_writel(struct ingenic_i2c_bus *i2c, unsigned short offset,
        unsigned short value)
{
    writel(value, i2c->hwaddr + offset);
}


static int f1c100s_i2c_wait_status(struct ingenic_i2c_bus *i2c)
{
    int timeout = TIMEOUT;
    while (!(i2c_readl(i2c, I2C_CNTR) & (1 << 3)) && (--timeout > 0))
                 rt_thread_delay(1);
    if (timeout){
        timeout= i2c_readl(i2c,I2C_STAT);
       // I2C_DBG("f1c100s_i2c_wait_status return 0x%02x\n",timeout);
        return timeout;
    }
    return I2C_STAT_BUS_ERROR;
}

static int f1c100s_i2c_start(struct ingenic_i2c_bus *i2c)
{
    int timeout = TIMEOUT;
   // I2C_DBG("f1c100s_i2c_start timeout=%d\n",timeout);
    i2c_writel(i2c,I2C_CNTR, (1 << 7) | (1 << 6) | (1 << 5) | (1 << 2));

    while (!(i2c_readl(i2c, I2C_CNTR) & (1 << 5)) && (--timeout > 0))
                     rt_thread_delay(1);
   // I2C_DBG("f1c100s_i2c_start timeout=%d\n",timeout);
   return f1c100s_i2c_wait_status(i2c);
}

static int f1c100s_i2c_stop(struct ingenic_i2c_bus *i2c)
{
    int timeout=TIMEOUT;
   // rt_uint32_t val;

  //  val = i2c_readl(i2c,I2C_CNTR);
  //  val |= (1 << 4) | (1 << 3);
  //  i2c_writel(i2c,I2C_CNTR, val);
    i2c_writel(i2c, I2C_CNTR, (i2c_readl(i2c,I2C_CNTR) | (1 << 4)) & ~((1 << 7) | (1 << 3)));
    while (!(i2c_readl(i2c, I2C_CNTR) & (1 << 4)) && (--timeout > 0))
                       rt_thread_delay(1);
    return (timeout>0)?0:-1;
}

static int f1c100s_i2c_send_data(struct ingenic_i2c_bus *i2c,rt_uint8_t dat)
{
    i2c_writel(i2c,I2C_DATA, dat);
    i2c_writel(i2c,I2C_CNTR, i2c_readl(i2c, I2C_CNTR) & (~(1 << 3)));
    return f1c100s_i2c_wait_status(i2c);
}

static int f1c100s_i2c_read(struct ingenic_i2c_bus *i2c,struct rt_i2c_msg * msg)
{

    rt_uint8_t * p = msg->buf;
        int len = msg->len;

        if(f1c100s_i2c_send_data(i2c, (rt_uint8_t)(msg->addr << 1 | 1)) != I2C_STAT_TX_AR_ACK)
        {
            I2C_DBG("f1c100s_i2c_read !=I2C_STAT_TX_AR_ACK\n");
            return -1;
        }


        while(len > 0)
        {
            switch(f1c100s_i2c_wait_status(i2c))
            {
                case I2C_STAT_TX_AR_ACK:
                   // I2C_DBG("f1c100s_i2c_read switch I2C_STAT_TX_AR_ACK\n");
                    if(len == 1)
                        i2c_writel(i2c, I2C_CNTR, i2c_readl(i2c,I2C_CNTR) & ~((1 << 3) | (1 << 2)));
                    else
                        i2c_writel(i2c,I2C_CNTR, i2c_readl(i2c,I2C_CNTR) & ~(1 << 3));
                    break;
                case I2C_STAT_RXD_ACK:
                  //  I2C_DBG("f1c100s_i2c_read switch I2C_STAT_RXD_ACK\n");
                    *p++ = i2c_readl(i2c, I2C_DATA);
                    len--;
                    if(len == 1)
                        i2c_writel(i2c, I2C_CNTR, i2c_readl(i2c,I2C_CNTR) & ~((1 << 3) | (1 << 2)));
                    else
                        i2c_writel(i2c, I2C_CNTR, i2c_readl(i2c, I2C_CNTR) & ~(1 << 3));
                    break;
                case I2C_STAT_RXD_NAK:
                   // I2C_DBG("f1c100s_i2c_read switch I2C_STAT_RXD_NAK\n");
                    *p++ = i2c_readl(i2c, I2C_DATA);
                    len--;
                    break;
                default:
                    I2C_DBG("f1c100s_i2c_read switch unknow\n");
                    return -1;
            }
        }
        return 0;
}

static int f1c100s_i2c_write(struct ingenic_i2c_bus *i2c,struct rt_i2c_msg * msg)
{
    rt_uint8_t * p = msg->buf;
    int len = msg->len;

    if(f1c100s_i2c_send_data(i2c, (rt_uint8_t)(msg->addr << 1)) != I2C_STAT_TX_AW_ACK)
    {
        I2C_DBG("f1c100s_i2c_write !I2C_STAT_TX_AW_ACK\n");
        return -1;
    }

    while(len > 0)
    {
        if(f1c100s_i2c_send_data(i2c, *p++) != I2C_STAT_TXD_ACK){
            I2C_DBG("f1c100s_i2c_write %d !I2C_STAT_TXD_ACK \n",len);
            return -1;
        }
        len--;
    }
    return 0;
}

rt_size_t ingenic_i2c_xfer(struct rt_i2c_bus_device *bus,
                                         struct rt_i2c_msg msgs[],
                                         rt_uint32_t num){

        struct rt_i2c_msg * pmsg = msgs;
        struct ingenic_i2c_bus *i2c=( struct ingenic_i2c_bus *)bus;//RT_NULL;
        int i, res;

        if(!msgs || num <= 0)
            return 0;
        if(f1c100s_i2c_start(i2c) != I2C_STAT_TX_START)
        {
            I2C_DBG("f1c100s_i2c_start !=I2C_STAT_TX_START\n");
            return 0;
        }


        for(i = 0; i < num; i++, pmsg++)
        {
            if(i != 0)
            {
                if(f1c100s_i2c_start(i2c) != I2C_STAT_TX_RSTART)
                {
                    I2C_DBG("mis[%d]f1c100s_i2c_start !=I2C_STAT_TX_START\n",i);
                    break;
                }


            }
            if(pmsg->flags & RT_I2C_RD)
                res = f1c100s_i2c_read(i2c, pmsg);
            else
                res = f1c100s_i2c_write(i2c, pmsg);
            if(res < 0)
                break;
        }
        f1c100s_i2c_stop(i2c);
      //  I2C_DBG("ingenic_i2c_xfer return %d\n",i);
        return i;
}


static int ingenic_i2c_set_speed(struct ingenic_i2c_bus *i2c, int rate){
    rt_uint64_t pclk = apb_get_clk();
    rt_int64_t freq, delta, best = 0x7fffffffffffffffLL;
    int tm = 5, tn = 0;
    int m, n;

    for(n = 0; n <= 7; n++)
    {
        for(m = 0; m <= 15; m++)
        {
            freq = pclk / (10 * (m + 1) * (1 << n));
            delta = rate - freq;
            if(delta >= 0 && delta < best)
            {
                tm = m;
                tn = n;
                best = delta;
            }
            if(best == 0)
                break;
        }
    }
    I2C_DBG("ingenic_i2c_set_speed clk=%d %d  %d\n",pclk,tm,tn);
    i2c_writel(i2c,I2C_CCR, ((tm & 0xf) << 3) | ((tn & 0x7) << 0));
    return 0;
}


int rt_hw_i2c_init(void)
{
    struct ingenic_i2c_bus *i2c;
    struct rt_i2c_bus_device *i2c_bus;
    i2c = &ingenic_i2c0;
    rt_memset((void *)i2c, 0, sizeof(struct ingenic_i2c_bus));
    i2c->hwaddr = I2C0_BASE_ADDR;
    i2c->irqno = TWI0_INTERRUPT;

    bus_gate_clk_enalbe(TWI0_GATING);
    //0x01c202d0

//    rt_int64_t  val = read32(I2C0_CCU_BUS_SOFT_RST_BASE);
//    val |= I2C0_CCU_BUS_SOFT_RST_BIT;
    write32(I2C0_CCU_BUS_SOFT_RST_BASE, read32(I2C0_CCU_BUS_SOFT_RST_BASE)|I2C0_CCU_BUS_SOFT_RST_BIT);

  //  bus_software_reset_enalbe(TWI0_GATING);
    /* Set PE11 PE12 in func2 (I2C0) */
    gpio_set_func(GPIO_PORT_E, GPIO_PIN_11, IO_FUN_2);
    gpio_set_func(GPIO_PORT_E, GPIO_PIN_12, IO_FUN_2);

    gpio_set_pull_mode(GPIO_PORT_E, GPIO_PIN_11,PULL_UP);
    gpio_set_pull_mode(GPIO_PORT_E, GPIO_PIN_12, PULL_UP);



//    gpio_set_func(GPIO_PORT_E, GPIO_PIN_3,IO_FUN_5);
//   // gpio_direction_output(GPIO_PORT_E, GPIO_PIN_3, 0);
//    gpio_set_irq_type(GPIO_PORT_E, GPIO_PIN_3, POSITIVE);
//    gpio_set_irq_callback(GPIO_PORT_E, GPIO_PIN_3, gpio_e_3_irq, i2c);
//    gpio_irq_enable(GPIO_PORT_E, GPIO_PIN_3);


  //  bus_software_reset_enalbe(TWI0_GATING);
  //  bus_software_reset_disalbe(TWI0_GATING);

    i2c_bus = &i2c->parent;
    i2c_bus->ops = &i2c_ops;

    i2c_writel(i2c, I2C_SRST, 1 << 0);
    i2c_writel(i2c, I2C_SRST, 0 << 0);

    ingenic_i2c_set_speed(i2c, 400 * 1000);
    i2c_writel(i2c, I2C_ADDR, 0);
    i2c_writel(i2c, I2C_XADDR, 0);
    i2c_writel(i2c, I2C_CNTR, (1 << 6) | (1 << 4));


    rt_i2c_bus_device_register(i2c_bus, "i2c0");

  //  rt_hw_interrupt_install(TWI0_INTERRUPT, i2c_irq_handler, &i2c, "i2c0");
  //   rt_hw_interrupt_umask(TWI0_INTERRUPT);
      return 0;
}


INIT_DEVICE_EXPORT(rt_hw_i2c_init);
#endif


