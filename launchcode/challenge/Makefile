all: realpatch.bin calc-original.exe
	ruby ./do_patch.rb

realpatch.bin: realpatch.asm
	nasm -o realpatch.bin realpatch.asm
