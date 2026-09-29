obj-m += crosstime.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
 
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	
install:
	insmod crosstime.ko
	
test:
	cp crosstime.ko /tmp/
	-sudo rmmod crosstime
	sudo insmod /tmp/crosstime.ko
	sudo dmesg
