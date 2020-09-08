insmod ./yj_ramdisk.ko
mkfs.ext4 /dev/yjramdisk
mkdir ./ext4
mount -t ext4 -o loop /dev/yjramdisk ./ext4
