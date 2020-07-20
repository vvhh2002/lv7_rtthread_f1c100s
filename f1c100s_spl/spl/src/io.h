#ifndef __IO_H__
#define __IO_H__

#include <stdint.h>

#define NULL    ((void*)0)

static inline uint8_t read8(uint32_t addr)
{
	return( *((volatile uint8_t *)(addr)) );
}

static inline uint32_t read32(uint32_t addr)
{
	return( *((volatile uint32_t *)(addr)) );
}

static inline void write8(uint32_t addr, uint8_t value)
{
	*((volatile uint8_t *)(addr)) = value;
}

static inline void write32(uint32_t addr, uint32_t value)
{
	*((volatile uint32_t *)(addr)) = value;
}

#endif
