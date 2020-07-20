#!/bin/sh

dd if=/dev/zero of=zero.bin bs=1024 count=8
sudo ../tools/sunxi-tools/sunxi-fel -p spiflash-write 0x00000000 zero.bin
rm -rf zero.bin
