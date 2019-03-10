slowfire: main.c
	gcc -fno-stack-protector -z execstack -no-pie -g -o $@ $^
	mkdir -p ../../distfiles
	cp $@ ../../distfiles/

clean:
	rm -f slowfire
