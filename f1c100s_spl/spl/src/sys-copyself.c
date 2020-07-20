/*
 * sys-copyself.c
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

#define SIZE_START	(0x00010000)

extern void return_to_fel(void);
extern void sys_uart_putc(char c);
extern void sys_spi_flash_init(void);
extern void sys_spi_flash_exit(void);
extern int  sys_spi_flash_read(int addr, void * buf, int count);

enum {
	BOOT_DEVICE_FEL	= 0,
	BOOT_DEVICE_SPI	= 1
};

struct flash_head {
	uint32_t magic[4];
};

static int get_boot_device(void)
{
	uint32_t * t = (void *)0x00000058;

	if(t[0] == 0x1)
		return BOOT_DEVICE_FEL;
	return BOOT_DEVICE_SPI;
}

void print_string(const char *s)
{
	while(*s != 0) {
		sys_uart_putc(*s);
		if (*s == '\n') {
			sys_uart_putc('\r');
		}
		s ++;
	}
}

void print_number(uint32_t num)
{
	char str[16];
	int  i, n;

	n = 0;
	while (num > 0) {
		str[n ++] = num % 10;
		num = num / 10;
	}

	if (n == 0) {
	sys_uart_putc('0');
	} else {
		for (i = n;i > 0;i --) {
			sys_uart_putc(str[i - 1] + 0x30);
		}
	}
	sys_uart_putc('\n');
	sys_uart_putc('\r');
}

void print_hex(uint32_t num)
{
	char str[16];
	int  i, n;

	sys_uart_putc('0');
	sys_uart_putc('x');

	n = 0;
	while (num > 0) {
		str[n ++] = num & 0xf;
		num = num >> 4;
	}

	if (n == 0) {
		sys_uart_putc('0');
	} else {
		for (i = n;i > 0;i --) {
			if (str[i - 1] > 9) {
				sys_uart_putc(str[i - 1] + ('A' - 0x0a));
			} else {
				sys_uart_putc(str[i - 1] + 0x30);
			}
		}
	}
	sys_uart_putc('\n');
	sys_uart_putc('\r');
}

void sys_copyself(void)
{
	int d = get_boot_device();
	void * mem;

	void (*jamp)(void);

	struct flash_head *h;

	mem  = (void*)0x80000000;
	h    = mem;
	jamp = mem;

	if(d == BOOT_DEVICE_FEL) {
		print_string("Boot to FEL mode\n");
		return_to_fel();
	} else if(d == BOOT_DEVICE_SPI) {
		print_string("Boot to SPI mode\n");
        //先读4个int(4*4=16)字节，最后一个字节是下一可执行文件的位置
		sys_spi_flash_init();
		sys_spi_flash_read(SIZE_START, mem, 16);
		sys_spi_flash_exit();

		if (h->magic[1] == 0xaa55aa55) {
			print_string("Copy Flash offset 64 to RAM 0x8000000 size:");
			print_number(h->magic[3]);
			sys_spi_flash_init();
			sys_spi_flash_read(SIZE_START, mem, h->magic[3]);
			sys_spi_flash_exit();
			print_string("Copy Flash Ok!\n");
		} else {
			print_string("Magic error\n");
			while (1);
		}
	}

	print_string("Goto 0x80000000 ...\n");
	jamp();
}
