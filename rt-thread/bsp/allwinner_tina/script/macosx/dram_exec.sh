#!/bin/sh

~/f1c100s/sunxi-tools/sunxi-fel -p spl              ../../../f1c100s_spl/boot.bin
~/f1c100s/sunxi-tools/sunxi-fel -p write 0x80000000 rtthread.bin
~/f1c100s/sunxi-tools/sunxi-fel    exec  0x80000000
