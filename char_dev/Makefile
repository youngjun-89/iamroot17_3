obj-m := iamroot.o
iamroot-objs := main.o

modules:
	$(MAKE) -C /lib/modules/`uname -r`/build M=$(PWD) modules 
	gcc -o user user.c
clean:
		rm -rf *.o~ core .cepend .*.cmd *.ko *.mod.c .tmp_versions
		rm -rf user user.o
