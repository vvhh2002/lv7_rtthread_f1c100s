/**
 * @file drv_fb.c
 * @author jiang xing
 * @brief   rt-thread 系统 f1c100s fb 驱动
 * @version 0.1
 * @date 2019-09-20
 *
 */

#include <rtthread.h>
#include <rthw.h>

#include "iomem.h"
#include "drv_clock.h"
#include "drv_gpio.h"
#include "drv_fb.h"

#include "reg-ccu.h"
#include "reg-defe.h"
#include "reg-debe.h"
#include "reg-tcon.h"

#ifdef LV_COLOR_DEPTH
#define LCD_PRE_PIXEL   LV_COLOR_DEPTH
#elif
#define LCD_PRE_PIXEL   24
#endif
#ifdef LV_HOR_RES
#define LCD_WIDTH       LV_HOR_RES
#elif
#define LCD_WIDTH       480
#endif
#ifdef LV_VER_RES
#define LCD_HEIGHT      LV_VER_RES
#elif
#define LCD_WIDTH       480
#endif
#define LCD_PGIO_PWR_EN  GPIO_PORT_E, GPIO_PIN_4
#define LCD_PGIO_BL_EN  GPIO_PORT_E, GPIO_PIN_5

// 需要2字节对齐，24位为3字节所以要4字节
#define LCD_BUFFER_SIZE (4 * LCD_WIDTH * LCD_HEIGHT)

/**
 * @brief 液晶驱动开关
 *
 */
#ifdef TINA_USING_LCD

/**
 * @brief 液晶屏时序
 *
 */
struct lcd_timing {
    rt_uint32_t pixel_clock_hz;
    rt_uint32_t h_front_porch;
    rt_uint32_t h_back_porch;
    rt_uint32_t h_sync_len;
    rt_uint32_t v_front_porch;
    rt_uint32_t v_back_porch;
    rt_uint32_t v_sync_len;
    rt_uint32_t h_sync_active;
    rt_uint32_t v_sync_active;
    rt_uint32_t den_active;
    rt_uint32_t clk_active;
};

/**
 * @brief lcd设备
 *
 */
struct lcd_f1c100s_device {
    struct rt_device              parent;
    struct rt_device_graphic_info lcd_info;
    struct lcd_timing             timing;
    rt_uint32_t                   virtdebe;
    rt_uint32_t                   virtdefe;
    rt_uint32_t                   virttcon;
    rt_uint32_t                   virtccu;
};

static rt_uint8_t f1c100s_framebuffer[LCD_BUFFER_SIZE];

/**
 * @brief 延时
 *
 * @param loops
 */
static inline void sdelay(int loops)
{
	__asm__ __volatile__ ("1:\n" "subs %0, %1, #1\n"
		"bne 1b":"=r" (loops):"0"(loops));
}

/**
 * @brief 延时
 *
 * @param loops
 */
static void dram_delay(int ms)
{
	sdelay(ms * 2 * 1000);
}

/**
 * @brief 复位总线时钟
 *
 * @param bus 总线
 */
static void reset_deassert(enum bus_gate bus)
{
    bus_software_reset_enalbe(bus);
    dram_delay(10);
    bus_software_reset_disalbe(bus);
}

/**
 * @brief 开启时钟总线门
 *
 * @param addr
 * @param shift
 * @param invert
 */
inline static void gate_clk_enable(rt_uint32_t addr, int shift, rt_bool_t invert)
{
    rt_uint32_t val = read32(addr);
    val &= ~(1 << shift);
    val |= (invert ? 0x00 : 0x01) << shift;
    write32(addr, val);
}

/**
 * @brief 关闭时钟总线门
 *
 * @param addr
 * @param shift
 * @param invert
 */
inline static void gate_clk_disable(rt_uint32_t addr, int shift, rt_bool_t invert)
{
    rt_uint32_t val = read32(addr);
    val &= ~(1 << shift);
    val |= (invert ? 0x01 : 0x00) << shift;
    write32(addr, val);
}

/**
 * @brief 获取视频时钟频率
 *
 * @return rt_uint32_t
 */
inline static rt_uint32_t get_video_pll_clk(void)
{
    rt_uint32_t reg;
    int n, m;

    reg = CCU->pll_video_ctrl;
    if (!(reg & (0x01 << 31)))
        return 0;

    if (reg & PLL_VIDEO_MODE_SEL)
    {
        //(24MHz*n)/m
        n = PLL_VIDEO_FACTOR_N(reg) + 1;
        m = PLL_VIDEO_PREDIV_M(reg) + 1;
        return (_24MHZ_ * n) / m;
    }

    if (reg & PLL_VIDEO_FRAC_CLK_OUT)
        return 270000000;
    else
        return 297000000;

    return 0;
}

/**
 * @brief 等待PLL时钟开启
 *
 * @param base
 * @return rt_err_t
 */
static rt_err_t f1c100s_clk_wait_pll_stable(rt_uint32_t base)
{
    rt_uint32_t rval  = 0;
    volatile int time = 0xfff;

    do {
        rval = read32(base);
        time--;
    } while (time && !(rval & (1 << 28)));

    return !time;
}

/**
 * @brief 启用视频时钟
 *
 * @param dev
 * @param enable
 */
inline static void f1c100s_clk_pll_video_set_enable(struct lcd_f1c100s_device *dev, rt_bool_t enable)
{
    rt_uint32_t val = read32(dev->virtccu + CCU_PLL_VIDEO_CTRL);
    if(enable) {
        val |= (1 << 31);
    } else {
        val &= ~(1 << 31);
    }
    write32(dev->virtccu + CCU_PLL_VIDEO_CTRL, val);

    if (enable) {
        f1c100s_clk_wait_pll_stable(dev->virtccu + CCU_PLL_VIDEO_CTRL);
    }
}

/**
 * @brief 设置CLK频率
 *
 * @param virt
 * @param width
 * @param shift
 * @param onebased
 * @param prate
 * @param rate
 */
static void clk_divider_set_rate(rt_uint32_t virt, rt_uint32_t width, rt_uint32_t shift, rt_bool_t onebased, rt_uint64_t prate, rt_uint64_t rate)
{
	rt_uint32_t mask = ((1 << (width)) - 1);
	rt_uint32_t div;
	rt_uint32_t val;

	if(rate == 0)
		rate = prate;

    rt_kprintf("clk_divider_set_rate %d %d %d", prate, rate, prate / rate);

	div = prate / rate;
	if(onebased)
		div--;
	if(div > mask)
		div = mask;

	val = read32(virt);
	val &= ~(mask << shift);
	val |= div << shift;
	write32(virt, val);
}

/**
 * @brief 替换寄存器位
 *
 * @param virt  地址
 * @param width 位宽度
 * @param shift 位偏移
 * @param parent_val 替换的位
 */
static void clk_mux_set_parent(rt_uint32_t virt, rt_uint32_t width, rt_uint32_t shift, rt_uint32_t parent_val)
{
	uint32_t val;
	val = read32(virt);
	val &= ~(((1 << width) - 1) << shift);
	val |= parent_val << shift;
	write32(virt, val);
}

/**
 * @brief 开启DEFE时钟
 *
 * @param dev 设备指针
 */
inline static void f1c100s_clk_defe_enable(struct lcd_f1c100s_device *dev)
{
	gate_clk_enable(dev->virtccu + CCU_DEFE_CLK, 31, RT_FALSE);
	gate_clk_enable(dev->virtccu + CCU_BUS_CLK_GATE1, 14, RT_FALSE);
}

/**
 * @brief 关闭DEFE时钟
 *
 * @param dev 设备指针
 */
inline static void f1c100s_clk_defe_disable(struct lcd_f1c100s_device *dev)
{
	gate_clk_disable(dev->virtccu + CCU_DEFE_CLK, 31, RT_FALSE);
	gate_clk_disable(dev->virtccu + CCU_BUS_CLK_GATE1, 14, RT_FALSE);
}

/**
 * @brief 初始化DEF时钟
 *
 * @param dev 设备指针
 */
static void f1c100s_clk_defe_init(struct lcd_f1c100s_device *dev)
{
    // 选择时钟源为 0b011
	clk_mux_set_parent(dev->virtccu + CCU_DEFE_CLK, 3, 24, 0);
	//clk_divider_set_rate(dev->virtccu + CCU_DEFE_CLK, 4, 0, RT_TRUE, 24000000, 198000000);
    clk_divider_set_rate(dev->virtccu + CCU_DEFE_CLK, 4, 0, RT_TRUE, 198000000, 198000000);
}

/**
 * @brief 开启DEBE时钟
 *
 * @param dev 设备指针
 */
inline static void f1c100s_clk_debe_enable(struct lcd_f1c100s_device *dev)
{
	gate_clk_enable(dev->virtccu + CCU_DEBE_CLK, 31, RT_FALSE);
	gate_clk_enable(dev->virtccu + CCU_BUS_CLK_GATE1, 12, RT_FALSE);
}

/**
 * @brief 关闭DEBE时钟
 *
 * @param dev 设备指针
 */
inline static void f1c100s_clk_debe_disable(struct lcd_f1c100s_device *dev)
{
	gate_clk_disable(dev->virtccu + CCU_DEBE_CLK, 31, RT_FALSE);
	gate_clk_disable(dev->virtccu + CCU_BUS_CLK_GATE1, 12, RT_FALSE);
}

/**
 * @brief 设置DEBE模式（DEBE控制显示尺寸和显示缓冲地址）
 *
 * @param dev 设备指针
 */
static inline void f1c100s_debe_set_mode(struct lcd_f1c100s_device *dev)
{
	struct f1c100s_debe_reg_t * debe = (struct f1c100s_debe_reg_t *)(dev->virtdebe);
	rt_uint32_t val;

	val = read32((rt_uint32_t)&debe->mode);
	val |= (1 << 0);
	write32((rt_uint32_t)&debe->mode, val);

	write32((rt_uint32_t)&debe->disp_size, (((dev->lcd_info.height) - 1) << 16) | (((dev->lcd_info.width) - 1) << 0));
	write32((rt_uint32_t)&debe->layer0_size, (((dev->lcd_info.height) - 1) << 16) | (((dev->lcd_info.width) - 1) << 0));
	write32((rt_uint32_t)&debe->layer0_stride, ((dev->lcd_info.width) << 5));
	write32((rt_uint32_t)&debe->layer0_addr_low32b, (uint32_t)(dev->lcd_info.framebuffer) << 3);
	write32((rt_uint32_t)&debe->layer0_addr_high4b, (uint32_t)(dev->lcd_info.framebuffer) >> 29);
	//write32((rt_uint32_t)&debe->layer0_attr1_ctrl, 0x09 << 8);
    write32((rt_uint32_t)&debe->layer0_attr1_ctrl, (0x09 << 8)|(1<<2));
	//write32((rt_uint32_t)&debe->layer0_attr1_ctrl, 0x0A << 8);
	val = read32((rt_uint32_t)&debe->mode);
	val |= (1 << 8);
	write32((rt_uint32_t)&debe->mode, val);

	val = read32((rt_uint32_t)&debe->reg_ctrl);
	val |= (1 << 0);
	write32((rt_uint32_t)&debe->reg_ctrl, val);

	val = read32((rt_uint32_t)&debe->mode);
	val |= (1 << 1);
	write32((rt_uint32_t)&debe->mode, val);
}

/**
 * @brief 初始化DEBE时钟
 *
 * @param dev
 */
static void f1c100s_clk_debe_init(struct lcd_f1c100s_device *dev)
{
	clk_mux_set_parent(dev->virtccu + CCU_DEBE_CLK, 3, 24, 0);
	// clk_divider_set_rate(dev->virtccu + CCU_DEBE_CLK, 4, 0, RT_TRUE, 24000000, 198000000);
    clk_divider_set_rate(dev->virtccu + CCU_DEBE_CLK, 4, 0, RT_TRUE, 198000000, 198000000);
}

/**
 * @brief 设置DEBE缓冲区地址
 *
 * @param dev
 * @param vram
 */
inline static void f1c100s_debe_set_address(struct lcd_f1c100s_device *dev, void *vram)
{
	struct f1c100s_debe_reg_t * debe = (struct f1c100s_debe_reg_t *)(dev->virtdebe);

	write32((rt_uint32_t)&debe->layer0_addr_low32b, (uint32_t)vram << 3);
	write32((rt_uint32_t)&debe->layer0_addr_high4b, (uint32_t)vram >> 29);
}

/**
 * @brief 开启TCON时钟
 *
 * @param dev 设备指针
 */
inline static void f1c100s_clk_tcon_enable(struct lcd_f1c100s_device *dev)
{
    gate_clk_enable(dev->virtccu + CCU_LCD_CLK, 31, RT_FALSE);
    gate_clk_enable(dev->virtccu + CCU_BUS_CLK_GATE1, 4, RT_FALSE);
}

/**
 * @brief 关闭TCON时钟
 *
 * @param dev 设备指针
 */
inline static void f1c100s_clk_tcon_disable(struct lcd_f1c100s_device *dev)
{
    gate_clk_disable(dev->virtccu + CCU_LCD_CLK, 31, RT_FALSE);
    gate_clk_disable(dev->virtccu + CCU_BUS_CLK_GATE1, 4, RT_FALSE);
}

/**
 * @brief 设置TCON模式（LCD的时序）
 *
 * @param dev 设备指针
 */
static inline void f1c100s_tcon_set_mode(struct lcd_f1c100s_device *dev)
{
	struct f1c100s_tcon_reg_t * tcon = (struct f1c100s_tcon_reg_t *)dev->virttcon;
	int bp, total;
	rt_uint32_t val;

	val = read32((rt_uint32_t)&tcon->ctrl);
	val &= ~(0x1 << 0);
	write32((rt_uint32_t)&tcon->ctrl, val);

	val = (dev->timing.v_front_porch + dev->timing.v_back_porch + dev->timing.v_sync_len);
	write32((rt_uint32_t)&tcon->tcon0_ctrl, (1 << 31) | ((val & 0x1f) << 4));
	// val = clk_get_rate(dev->clktcon) / dev->timing.pixel_clock_hz;
    // rt_kprintf("get_video_pll_clk:%d\n", get_video_pll_clk());
    val = get_video_pll_clk() / dev->timing.pixel_clock_hz;
	write32((rt_uint32_t)&tcon->tcon0_dclk, (0xf << 28) | (val << 0));
	write32((rt_uint32_t)&tcon->tcon0_timing_active, ((dev->lcd_info.width - 1) << 16) | ((dev->lcd_info.height - 1) << 0));

	bp = dev->timing.h_sync_len + dev->timing.h_back_porch;
	total = dev->lcd_info.width + dev->timing.h_front_porch + bp;
	write32((rt_uint32_t)&tcon->tcon0_timing_h, ((total - 1) << 16) | ((bp - 1) << 0));
	bp = dev->timing.v_sync_len + dev->timing.v_back_porch;
	total = dev->lcd_info.height + dev->timing.v_front_porch + bp;
	write32((rt_uint32_t)&tcon->tcon0_timing_v, ((total * 2) << 16) | ((bp - 1) << 0));
	write32((rt_uint32_t)&tcon->tcon0_timing_sync, ((dev->timing.h_sync_len - 1) << 16) | ((dev->timing.v_sync_len - 1) << 0));

	write32((rt_uint32_t)&tcon->tcon0_hv_intf, 0);
	write32((rt_uint32_t)&tcon->tcon0_cpu_intf, 0);

	if(dev->lcd_info.bits_per_pixel == 24 || dev->lcd_info.bits_per_pixel == 16 || dev->lcd_info.bits_per_pixel == 24)
	{
		write32((rt_uint32_t)&tcon->tcon0_frm_seed[0],  0x11111111);
		write32((rt_uint32_t)&tcon->tcon0_frm_seed[1],  0x11111111);
		write32((rt_uint32_t)&tcon->tcon0_frm_seed[2],  0x11111111);
		write32((rt_uint32_t)&tcon->tcon0_frm_seed[3],  0x11111111);
		write32((rt_uint32_t)&tcon->tcon0_frm_seed[4],  0x11111111);
		write32((rt_uint32_t)&tcon->tcon0_frm_seed[5],  0x11111111);

		write32((rt_uint32_t)&tcon->tcon0_frm_table[0], 0x01010000);
		write32((rt_uint32_t)&tcon->tcon0_frm_table[1], 0x15151111);
		write32((rt_uint32_t)&tcon->tcon0_frm_table[2], 0x57575555);
		write32((rt_uint32_t)&tcon->tcon0_frm_table[3], 0x7f7f7777);
        // 最高支持18位
		write32((rt_uint32_t)&tcon->tcon0_frm_ctrl, (dev->lcd_info.bits_per_pixel == 24 || dev->lcd_info.bits_per_pixel == 32) ? ((1 << 31) | (0 << 4)) : ((1 << 31) | (5 << 4)));
	}

	val = (1 << 28);
	if(!dev->timing.h_sync_active)
		val |= (1 << 25);
	if(!dev->timing.v_sync_active)
		val |= (1 << 24);
	if(!dev->timing.den_active)
		val |= (1 << 27);
	if(!dev->timing.clk_active)
		val |= (1 << 26);
	write32((rt_uint32_t)&tcon->tcon0_io_polarity, val);
	write32((rt_uint32_t)&tcon->tcon0_io_tristate, 0);
}

/**
 * @brief 开启TCON
 *
 * @param dev 设备指针
 */
inline static void f1c100s_tcon_enable(struct lcd_f1c100s_device *dev)
{
	struct f1c100s_tcon_reg_t * tcon = (struct f1c100s_tcon_reg_t *)dev->virttcon;
	uint32_t val;

	val = read32((rt_uint32_t)&tcon->ctrl);
	val |= (1 << 31);
	write32((rt_uint32_t)&tcon->ctrl, val);
}

/**
 * @brief 禁用TCON
 *
 * @param dev 设备指针
 */
static inline void f1c100s_tcon_disable(struct lcd_f1c100s_device *dev)
{
	struct f1c100s_tcon_reg_t * tcon = (struct f1c100s_tcon_reg_t *)dev->virttcon;
	rt_uint32_t val;

	write32((rt_uint32_t)&tcon->ctrl, 0);
	write32((rt_uint32_t)&tcon->int0, 0);

	val = read32((rt_uint32_t)&tcon->tcon0_dclk);
	val &= ~(0xf << 28);
	write32((rt_uint32_t)&tcon->tcon0_dclk, val);

	write32((rt_uint32_t)&tcon->tcon0_io_tristate, 0xffffffff);
	write32((rt_uint32_t)&tcon->tcon1_io_tristate, 0xffffffff);
}

/**
 * @brief 设置RGB屏的GPIO口
 *
 * @param port  A,B,C...口
 * @param pin   引脚开始编号
 * @param n     引脚个数
 * @param func  引脚功能
 * @param pull  引脚上拉
 * @param drv   引脚驱动
 */
static inline void fb_f1c100s_cfg_gpios(enum gpio_port port, enum gpio_pin pin, int n, rt_uint8_t func, enum gpio_pull pull, enum gpio_drv_level drv)
{
    int i;
	for(i = pin; i < n; i ++) {
        gpio_set_func(port, i, func);
        gpio_set_pull_mode(port, i, pull);
        gpio_set_drive_level(port, i, drv);
	}
}

/**
 * @brief 初始化f1c100s液晶设备
 *
 * @param dev 液晶设备指针
 */
static void fb_f1c100s_init(struct lcd_f1c100s_device *dev)
{
    f1c100s_clk_pll_video_set_enable(dev, RT_TRUE);
	f1c100s_clk_debe_init(dev);
	f1c100s_clk_defe_init(dev);

    fb_f1c100s_cfg_gpios(GPIO_PORT_D, GPIO_PIN_0, 22, IO_FUN_1, PULL_DISABLE, DRV_LEVEL_3);

    f1c100s_tcon_disable(dev);
    f1c100s_debe_set_mode(dev);
    f1c100s_tcon_set_mode(dev);
	f1c100s_tcon_enable(dev);
}

static rt_err_t drv_lcd_init(rt_device_t dev)
{
    rt_uint32_t i;

    struct lcd_f1c100s_device *lcd_dev = (struct lcd_f1c100s_device *)dev;
#ifdef LCD_PGIO_PWR_EN
    //屏的开关   PWR_EN   PE4
    gpio_set_func(LCD_PGIO_PWR_EN, IO_OUTPUT);
    gpio_direction_output(LCD_PGIO_PWR_EN, 0);
#endif
#ifdef LCD_PGIO_BL_EN
    //背光开关   LCD_EN   PE5
    gpio_set_func(LCD_PGIO_BL_EN, IO_OUTPUT);
    gpio_direction_output(LCD_PGIO_BL_EN, 0);
#endif
    //背光PWM   PE6
    gpio_set_func(GPIO_PORT_E, GPIO_PIN_6, IO_FUN_1);
    gpio_direction_output(GPIO_PORT_E, GPIO_PIN_6, 1);


    f1c100s_clk_defe_enable(lcd_dev);
    f1c100s_clk_debe_enable(lcd_dev);
    f1c100s_clk_tcon_enable(lcd_dev);

    reset_deassert(DEFE_GATING);
    reset_deassert(DEBE_GATING);
    reset_deassert(LCD_GATING);

    for (i = 0x0800;i < 0x1000;i += 4) {
        write32(lcd_dev->virtdebe + i, 0);
    }

    fb_f1c100s_init(lcd_dev);

    return RT_EOK;
}

static rt_err_t drv_lcd_open(rt_device_t dev, rt_uint16_t oflag)
{
#ifdef LCD_PGIO_PWR_EN
	 //屏的开关   PWR_EN   PE4
    gpio_direction_output(LCD_PGIO_PWR_EN, 1);
#endif
#ifdef LCD_PGIO_BL_EN
    //背光开关   LCD_EN   PE5
    gpio_direction_output(LCD_PGIO_BL_EN, 1);
#endif
    return RT_EOK;
}

static rt_err_t drv_lcd_close(rt_device_t dev)
{
#ifdef LCD_PGIO_PWR_EN
		 //屏的开关   PWR_EN   PE4
    gpio_direction_output(LCD_PGIO_PWR_EN, 0);
#endif
#ifdef LCD_PGIO_BL_EN
    //背光开关   LCD_EN   PE5
    gpio_direction_output(LCD_PGIO_BL_EN, 0);
    return RT_EOK;
#endif
}

static rt_size_t drv_lcd_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    return RT_EOK;
}

static rt_size_t drv_lcd_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    return RT_EOK;
}

static rt_err_t drv_lcd_control(rt_device_t dev, int cmd, void *args)
{
    struct lcd_f1c100s_device *lcd_dev = (struct lcd_f1c100s_device *)dev;

    switch (cmd) {
	case RTGRAPHIC_CTRL_RECT_UPDATE:
		break;
	case RTGRAPHIC_CTRL_POWERON:
	    drv_lcd_open(dev,0);
		break;
	case RTGRAPHIC_CTRL_POWEROFF:
        drv_lcd_close(dev);
		break;
	case RTGRAPHIC_CTRL_GET_INFO:
		rt_memcpy(args, &lcd_dev->lcd_info, sizeof(struct rt_device_graphic_info));
		break;
	case RTGRAPHIC_CTRL_SET_MODE:
		break;
	}

	return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS

static struct rt_device_ops drv_lcd_ops = {
    .init    = drv_lcd_init,
    .open    = drv_lcd_open,
    .close   = drv_lcd_close,
    .read    = drv_lcd_read,
    .write   = drv_lcd_write,
    .control = drv_lcd_control,
};

#endif

static struct lcd_f1c100s_device f1c100s_lcd_dev = {
    .parent = {
        .type        = RT_Device_Class_Graphic,
        .rx_indicate = RT_NULL,
        .tx_complete = RT_NULL,
        .user_data   = RT_NULL,
#ifdef RT_USING_DEVICE_OPS
        .ops         = &drv_lcd_ops,
#else
        .init        = drv_lcd_init,
        .open        = drv_lcd_open,
        .close       = drv_lcd_close,
        .read        = drv_lcd_read,
        .write       = drv_lcd_write,
        .control     = drv_lcd_control,
#endif
    },
    .lcd_info = {
        .bits_per_pixel = LCD_PRE_PIXEL,  //24
       // .pixel_format   = RTGRAPHIC_PIXEL_FORMAT_RGB666,
        .pixel_format   = RTGRAPHIC_PIXEL_FORMAT_RGB666,
        .width          = LCD_WIDTH,
        .height         = LCD_HEIGHT,
        .framebuffer    = f1c100s_framebuffer,
    },
   //800*480分辨率
    .timing = {
        .pixel_clock_hz = 33000000,

        .h_back_porch   = 87,
        .h_front_porch  = 40,
        .h_sync_len     = 1,

        .v_back_porch   = 31,
        .v_front_porch  = 13,
        .v_sync_len     = 1,

        .h_sync_active  = 0,
        .v_sync_active  = 0,

        .den_active     = 1,
        .clk_active     = 1,
    },
    
    /* 480x272 RGB液晶屏时序
    .timing = {
        .pixel_clock_hz = 9000000,

        .h_back_porch   = 40,
        .h_front_porch  = 3,
        .h_sync_len     = 2,

        .v_back_porch   = 12,
        .v_front_porch  = 2,
        .v_sync_len     = 2,

        .h_sync_active  = 0,
        .v_sync_active  = 0,

        .den_active     = 1,
        .clk_active     = 1,
    },
    */
    .virtdebe = F1C100S_DEBE_BASE,
    .virtdefe = F1C100S_DEFE_BASE,
    .virttcon = F1C100S_TCON_BASE,
    .virtccu  = F1C100S_CCU_BASE,
};

int rt_hw_lcd_init(void)
{
    rt_memset(f1c100s_lcd_dev.lcd_info.framebuffer, 0x00, LCD_BUFFER_SIZE);
    return rt_device_register((rt_device_t)&f1c100s_lcd_dev, "lcd", RT_DEVICE_FLAG_RDWR);
}

INIT_DEVICE_EXPORT(rt_hw_lcd_init);

#endif
