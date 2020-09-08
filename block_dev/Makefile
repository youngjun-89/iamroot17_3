obj-m := yj_ramdisk.o
yj_ramdisk-objs := ramdisk.o

modules:
	$(MAKE) -C /lib/modules/`uname -r`/build M=$(PWD) modules 
	#gcc -o user user.c
clean:
		rm -rf *.o core .cepend .*.cmd *.ko *.mod.c .tmp_versions *.symvers *.order *.mod*
		#rm -rf user user.o
