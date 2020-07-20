#!/bin/sh

sudo ../../../tools/sunxi-tools/sunxi-fel -p spiflash-write 0x00000000 ../../../f1c100s_spl/boot.bin
sudo ../../../tools/sunxi-tools/sunxi-fel -p spiflash-write 0x00010000 rtthread.bin
