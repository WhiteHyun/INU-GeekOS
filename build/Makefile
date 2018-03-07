UNAMEALL=$(shell uname -a)
MACHTYPE?=$(shell uname -m)
OSTYPE?=$(shell uname -s | tr A-Z a-z)

QEMU_NO_KQEMU?=$(shell qemu-system-i386 --help | grep -e -no-kqemu | awk '{print $$1}') 
RANDOM_PORT=1234

HAS_GDB_OPT=$(shell qemu-system-i386 --help | grep -e -gdb)
ifneq "$(findstring -gdb,$(HAS_GDB_OPT))" ""
  GDB_OPT = -gdb tcp::$(RANDOM_PORT)
else
  GDB_OPT = -s -p $(RANDOM_PORT)
endif

OSTYPE ?= linux
ifeq ($(MACHTYPE),x86_64)
	ifeq ($(wildcard Makefile.$(OSTYPE).$(MACHTYPE)),Makefile.linux.x86_64)
	    include Makefile.$(OSTYPE).$(MACHTYPE)
	else
	    include Makefile.$(OSTYPE)
	endif
else 
	include Makefile.$(OSTYPE)
endif
