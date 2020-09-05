#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include "ioctl.h"

#define CLASS_NAME "iamrootclass"
#define DEVICE_NAME "iamroot"
/* Global variable */
static struct class *g_iamroot_class;
static struct cdev *g_iamroot_cdev;
static struct device *g_iamroot_device;
static dev_t g_iamroot_dev;

static long iamroot_ioctl (struct file *filp, unsigned int cmd, unsigned long arg);

/* We don't need read, write, open, release... if you need implement it */
static struct file_operations g_iamroot_fops = {
	.owner = THIS_MODULE,
	//.read = iamroot_read,
	//.write = iamroot_write,
	//.open = iamroot_open,
	//.release = iamroot_release,
	.unlocked_ioctl = iamroot_ioctl,
};

/* 8.1 problem. implement it */
long iamroot_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err = 0;
	printk("ioctl called\n");
	/* IOCTL paramter verify */
	if (_IOC_DIR(cmd) & _IOC_READ) {
			err = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
	} else if (_IOC_DIR(cmd) & _IOC_WRITE)
			err = !access_ok((void __user *)arg, _IOC_SIZE(cmd));
	
	if (err)
			return -EFAULT;

	/* IOCTL cmd */
	switch (_IOC_NR(cmd)) {
	case IOCTL_PID:
		__put_user(current->pid, (int __user *)arg);
		break;
	//case:
	default:
			printk("unknown cmd\n");
			break;
	};

	return err;
}

__init int iamroot_init(void)
{
	int ret = 0;

	if (alloc_chrdev_region(&g_iamroot_dev, 0, 1, DEVICE_NAME) < 0) {
		printk("%s fail\n", __func__);
		ret = -EBUSY;
		goto err1;
	}

	g_iamroot_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(g_iamroot_class)) {
		ret = PTR_ERR(g_iamroot_class);
		goto err2;
	}

	g_iamroot_device = device_create(g_iamroot_class, NULL, g_iamroot_dev, NULL, DEVICE_NAME);
	if (IS_ERR(g_iamroot_device)) {
		ret = -ENOMEM;
		goto err3;
	}

	g_iamroot_cdev = cdev_alloc();
	if (!g_iamroot_cdev) {
			ret = -ENOMEM;
			goto err4;
	}

	g_iamroot_cdev->owner = THIS_MODULE;
	g_iamroot_cdev->ops = &g_iamroot_fops;

	g_iamroot_cdev->owner = THIS_MODULE;
	if (cdev_add(g_iamroot_cdev, g_iamroot_dev, 1) < 0) {
		ret = -EBUSY;
		goto err5;
	}

	printk("module init success :)\n");
	return ret;

err5:
	cdev_del(g_iamroot_cdev);	
err4:
	device_destroy(g_iamroot_class, g_iamroot_dev);
err3:
	class_destroy(g_iamroot_class);
err2:
	unregister_chrdev_region(g_iamroot_dev, 1);
err1:
	return ret;
}

__exit void iamroot_exit(void)
{
	device_destroy(g_iamroot_class, g_iamroot_dev);
	cdev_del(g_iamroot_cdev);
	class_destroy(g_iamroot_class);
	unregister_chrdev_region(g_iamroot_dev, 1);
	printk("module exit success :)\n");
}

module_init(iamroot_init)
module_exit(iamroot_exit)
MODULE_AUTHOR("youngjun.park");
MODULE_LICENSE("GPL");
