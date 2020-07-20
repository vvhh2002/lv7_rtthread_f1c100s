/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date             Author          Notes
 * 2020-05-22       Sherman         first version
 */

#include <stdint.h>
#include <string.h>
#include "board.h"
#include "drv_spi.h"

#define DBG_LEVEL   DBG_LOG
#include <rtdbg.h>
#define LOG_TAG                "drv.spi"

#ifdef BSP_USING_SPI

static struct nrfx_drv_spi_config spi_config[] =
{
#ifdef BSP_USING_SPI0
    NRFX_SPI0_CONFIG,
#endif

#ifdef BSP_USING_SPI1
    NRFX_SPI1_CONFIG,
#endif

#ifdef BSP_USING_SPI2
    NRFX_SPI2_CONFIG,
#endif

};

static struct nrfx_drv_spi spi_bus_obj[sizeof(spi_config) / sizeof(spi_config[0])];

//Configure SPI bus pins using the menuconfig
static struct nrfx_drv_spi_pin_config bsp_spi_pin[] = 
{
#ifdef BSP_USING_SPI0
    {
        .sck_pin = BSP_SPI0_SCK_PIN,
        .mosi_pin = BSP_SPI0_MOSI_PIN,
        .miso_pin = BSP_SPI0_MISO_PIN,
        .ss_pin = BSP_SPI0_SS_PIN
    },
#endif

#ifdef BSP_USING_SPI1
    {
        .sck_pin = BSP_SPI1_SCK_PIN,
        .mosi_pin = BSP_SPI1_MOSI_PIN,
        .miso_pin = BSP_SPI1_MISO_PIN,
        .ss_pin = BSP_SPI1_SS_PIN
    },
#endif

#ifdef BSP_USING_SPI2
    {
        .sck_pin = BSP_SPI2_SCK_PIN,
        .mosi_pin = BSP_SPI2_MOSI_PIN,
        .miso_pin = BSP_SPI2_MISO_PIN,
        .ss_pin = BSP_SPI2_SS_PIN
    },
#endif
};


static rt_uint8_t spi_index_find(struct rt_spi_bus *spi_bus)
{
    for (int i = 0; i < sizeof(spi_config) / sizeof(spi_config[0]); i++)
    {
        if(spi_bus == &spi_bus_obj[i].spi_bus)
            return i;
    }
    return 0xFF;
}

/**
 * spi event handler function
 */
static void spi0_handler(const nrfx_spi_evt_t *p_event, void *p_context)
{
    LOG_I("\nspi0_handler");
}

static void spi1_handler(const nrfx_spi_evt_t *p_event, void *p_context)
{
    LOG_I("\nspi1_handler");
}

static void spi2_handler(const nrfx_spi_evt_t *p_event, void *p_context)
{
    LOG_I("\nspi2_handler");
}
nrfx_spi_evt_handler_t spi_handler[] = {spi0_handler, spi1_handler, spi2_handler};

/**
  * @brief  This function config spi bus
  * @param  device    
  * @param  configuration
  * @retval RT_EOK / RT_ERROR
  */
static rt_err_t spi_configure(struct rt_spi_device *device,
                              struct rt_spi_configuration *configuration)
{
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
    RT_ASSERT(device->bus->parent.user_data != RT_NULL);
    RT_ASSERT(configuration != RT_NULL);

    rt_uint8_t index = spi_index_find(device->bus);
    RT_ASSERT(index != 0xFF);

    nrfx_spi_t spi = spi_bus_obj[index].spi;
    nrfx_spi_config_t config = NRFX_SPI_DEFAULT_CONFIG(bsp_spi_pin[index].sck_pin, 
        bsp_spi_pin[index].mosi_pin, bsp_spi_pin[index].miso_pin, bsp_spi_pin[index].ss_pin);

    /* spi config ss pin */
    if(device->user_data != RT_NULL)
    {
        config.ss_pin = (rt_uint8_t)device->user_data;
    }
    /* spi config bit order */
    if(configuration->mode & RT_SPI_MSB)
    {
        config.bit_order = NRF_SPI_BIT_ORDER_MSB_FIRST;
    }
    else
    {
        config.bit_order = NRF_SPI_BIT_ORDER_LSB_FIRST;
    }
    /* spi mode config */
    switch (configuration->mode & RT_SPI_MODE_3 )
    {
    case RT_SPI_MODE_0/* RT_SPI_CPOL:0 , RT_SPI_CPHA:0 */:
        config.mode = NRF_SPI_MODE_0;
        break;
    case RT_SPI_MODE_1/* RT_SPI_CPOL:0 , RT_SPI_CPHA:1 */:
        config.mode = NRF_SPI_MODE_1;
        break;
    case RT_SPI_MODE_2/* RT_SPI_CPOL:1 , RT_SPI_CPHA:0 */:
        config.mode = NRF_SPI_MODE_2;
        break;
    case RT_SPI_MODE_3/* RT_SPI_CPOL:1 , RT_SPI_CPHA:1 */:
        config.mode = NRF_SPI_MODE_3;
        break;
    default:
        LOG_E("spi_configure mode error %x\n",configuration->mode);
        return RT_ERROR;
    }
    /* spi frequency config */
    switch (configuration->max_hz / 1000)
    {
    case 125:
        config.frequency = NRF_SPI_FREQ_125K;
        break;
    case 250:
        config.frequency = NRF_SPI_FREQ_250K;
        break;
    case 500:
        config.frequency = NRF_SPI_FREQ_500K;
        break;
    case 1000:
        config.frequency = NRF_SPI_FREQ_1M;
        break;
    case 2000:
        config.frequency = NRF_SPI_FREQ_2M;
        break;
    case 4000:
        config.frequency = NRF_SPI_FREQ_4M;
        break;
    case 8000:
        config.frequency = NRF_SPI_FREQ_8M;
        break;
    default:
        LOG_E("spi_configure rate error %d\n",configuration->max_hz);
        break;
    }

    rt_memcpy((void*)&spi_bus_obj[index].spi_config, (void*)&config, sizeof(nrfx_spi_config_t));
    nrfx_spi_evt_handler_t handler = RT_NULL;    //spi send callback handler ,default NULL
    void * context = RT_NULL;
    nrfx_err_t nrf_ret = nrfx_spi_init(&spi, &config, handler, context);
    if(NRFX_SUCCESS == nrf_ret)
        return RT_EOK;

    return RT_ERROR;
}

static rt_uint32_t spixfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
    RT_ASSERT(device->bus->parent.user_data != RT_NULL);

    rt_uint8_t index = spi_index_find(device->bus);
    RT_ASSERT(index != 0xFF);

    nrfx_spi_t * p_instance =  &spi_bus_obj[index].spi;
    nrfx_spi_xfer_desc_t p_xfer_desc;
    if(message->send_buf == RT_NULL)
    {
        p_xfer_desc.p_rx_buffer = message->recv_buf;
        p_xfer_desc.rx_length = message->length;

        p_xfer_desc.p_tx_buffer = RT_NULL;
        p_xfer_desc.tx_length = 0;
    }
    else
    {
        p_xfer_desc.p_tx_buffer = message->send_buf;
        p_xfer_desc.tx_length = message->length ;

        p_xfer_desc.p_rx_buffer = RT_NULL;
        p_xfer_desc.rx_length = 0;
    }

    nrfx_err_t nrf_ret = nrfx_spi_xfer(p_instance, &p_xfer_desc, 0);
    if( NRFX_SUCCESS == nrf_ret)
        return message->length; 
    else
        return 0;
}

/* spi bus callback function  */
static const struct rt_spi_ops nrfx_spi_ops =
{
    .configure = spi_configure,
    .xfer = spixfer,
};

/*spi bus init*/
static int rt_hw_spi_bus_init(void)
{
    rt_err_t result = RT_ERROR;
    for (int i = 0; i < sizeof(spi_config) / sizeof(spi_config[0]); i++)
    {
        spi_bus_obj[i].spi = spi_config[i].spi;
        spi_bus_obj[i].spi_bus.parent.user_data = &spi_config[i];   //SPI INSTANCE
        result = rt_spi_bus_register(&spi_bus_obj[i].spi_bus, spi_config[i].bus_name, &nrfx_spi_ops);
        RT_ASSERT(result == RT_EOK);
    }
    return result;
}

int rt_hw_spi_init(void)
{
    return rt_hw_spi_bus_init();
}
INIT_BOARD_EXPORT(rt_hw_spi_init);

/**
  * Attach the spi device to SPI bus, this function must be used after initialization.
  */
rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, rt_uint32_t ss_pin)
{
    RT_ASSERT(bus_name != RT_NULL);
    RT_ASSERT(device_name != RT_NULL);

    rt_err_t result;
    struct rt_spi_device *spi_device;
    /* attach the device to spi bus*/
    spi_device = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
    RT_ASSERT(spi_device != RT_NULL);
    /* initialize the cs pin */
    spi_device->user_data = (void*)ss_pin;
    result = rt_spi_bus_attach_device(spi_device, device_name, bus_name, RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("%s attach to %s faild, %d", device_name, bus_name, result);
        result = RT_ERROR;
    }
    RT_ASSERT(result == RT_EOK);
    return result;
}

#endif /*BSP_USING_SPI*/
