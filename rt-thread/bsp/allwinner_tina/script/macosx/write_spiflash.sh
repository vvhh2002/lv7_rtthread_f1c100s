#!/bin/sh

~/f1c100s/sunxi-tools/sunxi-fel-p spiflash-write 0x00000000 ../../../f1c100s_spl/boot.bin
~/f1c100s/sunxi-tools/sunxi-fel -p spiflash-write 0x00010000 rtthread.bin
