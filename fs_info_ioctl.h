#ifndef FS_INFO_IOCTL_H
#define FS_INFO_IOCTL_H

#include <linux/ioctl.h>
#include <linux/limits.h>

#define FS_INFO_MAGIC 'f'

#define IOCTL_GET_FS_INFO _IOR(FS_INFO_MAGIC, 1, struct fs_info)

struct fs_info
{
    char fs_name[32];
    unsigned long total;
    unsigned long free;
    unsigned long used;
    char path[PATH_MAX];
};

#endif // FS_INFO_IOCTL_H
