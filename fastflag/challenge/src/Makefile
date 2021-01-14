fastflag: stringops.o fastflag.o
	gcc -Wall -o $@ $^
	strip fastflag

%.o: %.c
	gcc -Wall -c -o $@ $^

stringops.o: stringops.S
	as -o $@ $^

stringops_test: stringops.o stringops_test.o
	gcc -Wall -o $@ $^
	./stringops_test
