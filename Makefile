obj-m += keylogger.o

KDIR  := /lib/modules/$(shell uname -r)/source
PWD   := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

