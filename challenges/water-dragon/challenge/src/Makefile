all: WaterDragon.c
	gcc -O2 -fstack-protector-strong -D_FORTIFY_SOURCE=2 WaterDragon.c glob.c malloc-2.7.2.c -o WaterDragon

WaterDragon.c: WaterDragon.re
	re2c --tags WaterDragon.re  -o WaterDragon.c
