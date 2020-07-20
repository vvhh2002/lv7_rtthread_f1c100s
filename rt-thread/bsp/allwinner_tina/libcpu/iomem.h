#ifndef __IOMEM_H__
#define __IOMEM_H__

#include <stdint.h>

#define XBOOT_SUPPORT   1

#if XBOOT_SUPPORT

static inline uint32_t read32(uint32_t addr)
{
	return( *((volatile uint32_t *)(addr)) );
}

static inline void write32(uint32_t addr, uint32_t value)
{
	*((volatile uint32_t *)(addr)) = value;
}

#endif

static inline uint32_t readl(uint32_t addr)
{
	return( *((volatile uint32_t *)(addr)) );
}

static inline void writel(uint32_t value, uint32_t addr)
{
	*((volatile uint32_t *)(addr)) = value;
}

#endif
