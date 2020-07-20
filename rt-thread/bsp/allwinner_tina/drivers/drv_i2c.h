#ifndef DRV_I2C_H__
#define DRV_I2C_H__

rt_size_t ingenic_i2c_xfer(struct rt_i2c_bus_device *bus,
                                         struct rt_i2c_msg msgs[],
                                         rt_uint32_t num);
#endif
