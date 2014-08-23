# Makefile for libdolramdisk

all: library packer test

.PHONY: all clean library packer test install \
		install-library install-packer

clean:
	@$(MAKE) --no-print-directory -C library clean
	@$(MAKE) --no-print-directory -C packer clean
	@$(MAKE) --no-print-directory -C test clean

library:
	@echo Compiling library...
	@$(MAKE) --no-print-directory -C library

packer:
	@echo Compiling mkdolramdisk...
	@$(MAKE) --no-print-directory -C packer

test:
	@echo Compiling test program...
	@$(MAKE) --no-print-directory -C test

install: all install-library install-packer

install-library:
	@echo not implemented yet!

install-packer:
	@echo not implemented yet!
