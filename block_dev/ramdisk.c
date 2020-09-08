#include <linux/string.h>
#include <linux/slab.h>
#include <asm/atomic.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/pagemap.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>
#include <linux/buffer_head.h>
#include <linux/backing-dev.h>
#include <linux/blkpg.h>
#include <linux/writeback.h>
#include <linux/uaccess.h>

/* RAM Size granuality MB */
static int ram_size = 16;
module_param(ram_size, int, 0);

#define DEVICE_NAME		"yjramdisk"
#define BLK_SIZE		512

static int g_drv_major = 0;
static int g_drv_minor = 1;

struct request_queue *g_drv_q;
struct gendisk *g_drv_disk;
static void *g_data_area;

static int drv_open(struct block_device *bdev, fmode_t mode)
{
	printk("%s\n", __func__);
	return 0;
}

static void drv_release(struct gendisk *disk, fmode_t mode)
{
	printk("%s\n", __func__);
}

static int drv_ioctl (struct block_device *bdev, fmode_t mode,
                 unsigned int cmd, unsigned long arg)
{
	printk("%s\n", __func__);
	return 0;
}

static struct block_device_operations g_drv_fops = {
	.owner = THIS_MODULE,
	.open = drv_open,
	.release = drv_release,
	.ioctl = drv_ioctl,
};

/**
 * re-write based chpter 8 226 ~ 230 source
 * 1) update kernel API
 * 2) some bug fix.
 */
static blk_qc_t make_request(struct request_queue *q, struct bio *bio)
{
	int rw;
	struct bio_vec bvec;
	sector_t sector;
	struct bvec_iter iter;
	int err = -EIO;
	void *mem;
	char *data;
	int ops;

	sector = bio->bi_iter.bi_sector;
	if (bio_end_sector(bio) > get_capacity(bio->bi_disk))
			goto out;

	ops = bio_op(bio);
	if (ops == REQ_OP_DISCARD) {
		err = 0;
		goto out;
	}

	if (!op_is_write(ops)) {
			rw = READ;
	} else {
			rw = WRITE;
	}

	bio_for_each_segment(bvec, bio, iter) {
		unsigned int len = bvec.bv_len;
		data = g_data_area + (sector * BLK_SIZE);
		mem = kmap_atomic(bvec.bv_page) + bvec.bv_offset;
		if (rw == READ) {
			memcpy(mem, data, len);
			flush_dcache_page(bvec.bv_page);
		} else {
			flush_dcache_page(bvec.bv_page);
			memcpy(data, mem, len);
		}
		kunmap_atomic(mem);
		data += len;
		sector += len >> 9;
		err = 0;
	}

out:
	bio_endio(bio);
	return BLK_QC_T_NONE;
}

__init int ramdisk_init(void)
{
	int ret = 0;

	g_drv_major = register_blkdev(0, DEVICE_NAME);
    if (g_drv_major < 0) {
		printk("register blkdev fail\n");
		ret = -EIO;
		goto err1;
	}
	
	printk("major number = %d\n", g_drv_major);
	
	g_data_area = vmalloc(ram_size * 1024 * 1024);
	if (!g_data_area) {
		ret = -ENOMEM;
		goto err2;
	}

	g_drv_disk = alloc_disk(g_drv_minor);
	if (!g_drv_disk) {
		ret = -EFAULT;
		goto err3;
	}

	g_drv_q = blk_alloc_queue(GFP_KERNEL);
	if (!g_drv_q) {
		ret = -EFAULT;
		goto err4;
	}

	blk_queue_make_request(g_drv_q, &make_request);
	blk_queue_max_hw_sectors(g_drv_q, BLK_SIZE);
	g_drv_disk->major= g_drv_major;
	g_drv_disk->first_minor= g_drv_minor;
	g_drv_disk->fops = &g_drv_fops;
	g_drv_disk->queue = g_drv_q;

	sprintf(g_drv_disk->disk_name, DEVICE_NAME);
	set_capacity(g_drv_disk, ram_size* 1024 * 1024/ BLK_SIZE);
	add_disk(g_drv_disk);
	return 0;

err4:
	del_gendisk(g_drv_disk);
	put_disk(g_drv_disk);
err3:
	vfree(g_data_area);
err2:
	unregister_blkdev(g_drv_major, DEVICE_NAME);
err1:
	return ret;
}

__exit void ramdisk_exit(void)
{
	del_gendisk(g_drv_disk);
	put_disk(g_drv_disk);
	vfree(g_data_area);
	blk_cleanup_queue(g_drv_q);
	unregister_blkdev(g_drv_major, DEVICE_NAME);
	return;
}
		
module_init(ramdisk_init)
module_exit(ramdisk_exit)
MODULE_AUTHOR("youngjun.park");
MODULE_LICENSE("GPL");
