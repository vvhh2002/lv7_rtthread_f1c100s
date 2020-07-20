/*
 * sys-uart.c
 *
 * Copyright(c) 2007-2019 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <io.h>

void sys_uart_putc(char c)
{
	uint32_t addr = 0x01c25000;

	while((read32(addr + 0x7c) & (0x1 << 1)) == 0);
	write32(addr + 0x00, c);
}

void sys_uart_init(void)
{
	uint32_t addr;
	uint32_t val;

	/* Config GPIOE1 and GPIOE0 to txd0 and rxd0 */
	addr = 0x01c20890 + 0x00;
	val = read32(addr);
	val &= ~(0xf << ((1 & 0x7) << 2));
	val |= ((0x5 & 0x7) << ((1 & 0x7) << 2));
	write32(addr, val);

	val = read32(addr);
	val &= ~(0xf << ((0 & 0x7) << 2));
	val |= ((0x5 & 0x7) << ((0 & 0x7) << 2));
	write32(addr, val);

	/* Open the clock gate for uart0 */
	addr = 0x01c20068;
	val = read32(addr);
	val |= 1 << 20;
	write32(addr, val);

	/* Deassert uart0 reset */
	addr = 0x01c202d0;
	val = read32(addr);
	val |= 1 << 20;
	write32(addr, val);

	/* Config uart0 to 115200-8-1-0 */
	addr = 0x01c25000;
	write32(addr + 0x04, 0x0);
	write32(addr + 0x08, 0xf7);
	write32(addr + 0x10, 0x0);
	val = read32(addr + 0x0c);
	val |= (1 << 7);
	write32(addr + 0x0c, val);
	val = 54;
	write32(addr + 0x00, val & 0xff);
	write32(addr + 0x04, (val >> 8) & 0xff);
	val = read32(addr + 0x0c);
	val &= ~(1 << 7);
	write32(addr + 0x0c, val);
	val = read32(addr + 0x0c);
	val &= ~0x1f;
	val |= (0x3 << 0) | (0 << 2) | (0x0 << 3);
	write32(addr + 0x0c, val);

	sys_uart_putc('I');
	sys_uart_putc('n');
	sys_uart_putc('i');
	sys_uart_putc('t');
	sys_uart_putc(' ');
	sys_uart_putc('O');
	sys_uart_putc('K');
	sys_uart_putc('\r');
	sys_uart_putc('\n');
}
