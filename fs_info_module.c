#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/statfs.h>
#include <linux/namei.h>
#include "fs_info_ioctl.h"

#define DEVICE_NAME "fs_info"

static dev_t dev;
static struct cdev fs_info_cdev;
static struct class *fs_info_class;

static int get_fs_info(struct fs_info *info)
{
    struct kstatfs stat;
    struct path user_path;

    if (kern_path(info->path, LOOKUP_FOLLOW, &user_path))
    {
        pr_err("fs_info: Failed to resolve path for %s\n", info->path);
        return -ENOENT;
    }

    if (vfs_statfs(&user_path, &stat))
    {
        pr_err("fs_info: Failed to get filesystem stats for %s\n", info->path);
        return -EIO;
    }

    switch (stat.f_type)
    {
    case EXT4_SUPER_MAGIC:
        strncpy(info->fs_name, "ext4", sizeof(info->fs_name) - 1);
        break;
    case TMPFS_MAGIC:
        strncpy(info->fs_name, "tmpfs", sizeof(info->fs_name) - 1);
        break;
    case NFS_SUPER_MAGIC:
        strncpy(info->fs_name, "nfs", sizeof(info->fs_name) - 1);
        break;
    default:
        strncpy(info->fs_name, "unknown", sizeof(info->fs_name) - 1);
        break;
    }

    info->fs_name[sizeof(info->fs_name) - 1] = '\0';
    info->total = stat.f_blocks * stat.f_bsize;
    info->free = stat.f_bfree * stat.f_bsize;
    info->used = info->total - info->free;

    return 0;
}

static long fs_info_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct fs_info user_info;
    struct fs_info kernel_info;

    switch (cmd)
    {
    case IOCTL_GET_FS_INFO:
        if (copy_from_user(&user_info, (struct fs_info __user *)arg, sizeof(user_info)))
        {
            return -EFAULT;
        }

        strncpy(kernel_info.path, user_info.path, sizeof(kernel_info.path));
        kernel_info.path[sizeof(kernel_info.path) - 1] = '\0';

        if (get_fs_info(&kernel_info))
        {
            return -EIO;
        }

        if (copy_to_user((struct fs_info __user *)arg, &kernel_info, sizeof(kernel_info)))
        {
            return -EFAULT;
        }

        break;

    default:
        return -EINVAL;
    }

    return 0;
}

static const struct file_operations fs_info_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = fs_info_ioctl,
};

static int __init fs_info_init(void)
{
    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME))
    {
        pr_err("fs_info: Failed to allocate char device region\n");
        return -ENODEV;
    }

    cdev_init(&fs_info_cdev, &fs_info_fops);

    if (cdev_add(&fs_info_cdev, dev, 1))
    {
        pr_err("fs_info: Failed to add char device\n");
        unregister_chrdev_region(dev, 1);
        return -ENODEV;
    }

    fs_info_class = class_create(DEVICE_NAME);
    if (IS_ERR(fs_info_class))
    {
        pr_err("fs_info: Failed to create device class\n");
        cdev_del(&fs_info_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(fs_info_class);
    }

    if (!device_create(fs_info_class, NULL, dev, NULL, DEVICE_NAME))
    {
        pr_err("fs_info: Failed to create device\n");
        class_destroy(fs_info_class);
        cdev_del(&fs_info_cdev);
        unregister_chrdev_region(dev, 1);
        return -ENODEV;
    }

    pr_info("fs_info: Module loaded\n");
    return 0;
}

static void __exit fs_info_exit(void)
{
    device_destroy(fs_info_class, dev);
    class_destroy(fs_info_class);
    cdev_del(&fs_info_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("fs_info: Module unloaded\n");
}

module_init(fs_info_init);
module_exit(fs_info_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daria");
MODULE_DESCRIPTION("Filesystem Info via IOCTL");
