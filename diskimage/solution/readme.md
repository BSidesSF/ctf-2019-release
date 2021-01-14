pngtopam diskimage.png > disk.bin

dd if=disk.bin bs=1 skip=15 > raw.bin

fls raw.bin

icat -r raw.bin 75 > flag.ico

CTF{FAT12_FTW}

