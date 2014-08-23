# Makefile for mkdolramdisk

all: mkdolramdisk

mkdolramdisk: mkdolramdisk.c 
	@gcc -std=c99 -D_BSD_SOURCE -Wall -O2 -fomit-frame-pointer mkdolramdisk.c -o mkdolramdisk

mkdolramdisk-debug: mkdolramdisk.c 
	@gcc -std=c99 -D_BSD_SOURCE -Wall -Wextra -Wpedantic -g -O0 mkdolramdisk.c -o mkdolramdisk-debug

clean:
	@echo Cleaning mkdolramdisk...
	@rm -f mkdolramdisk mkdolramdisk-debug
