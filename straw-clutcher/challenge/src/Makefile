
all:
	re2c --tags -o StrawClutcher.c StrawClutcher.re
	gcc -fPIC -Os -D_FORTIFY_SOURCE=2 -fstack-protector-strong -pie --pie -o StrawClutcher StrawClutcher.c
