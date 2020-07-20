.\script\windows\sunxi-fel.exe -p spl              .\script\windows\boot.bin
.\script\windows\sunxi-fel.exe -p write 0x80000000 rtthread.bin
.\script\windows\sunxi-fel.exe    exec  0x80000000