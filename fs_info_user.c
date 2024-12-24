#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include "fs_info_ioctl.h"

void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <path> [--bytes]\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --bytes     Show sizes in bytes instead of gigabytes.\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    int fd, human_readable = 1;
    struct fs_info info;

    if (argc > 2 && strcmp(argv[2], "--bytes") == 0)
    {
        human_readable = 0;
    }

    fd = open("/dev/fs_info", O_RDONLY);
    if (fd < 0)
    {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    printf("%-30s %-15s %-15s %-15s %-15s %-15s %-15s\n",
           "Path", "Filesystem", "Total", "Used", "Free", "Use%", "1K-Blocks");

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--bytes") == 0)
            continue;

        strncpy(info.path, argv[i], sizeof(info.path) - 1);
        info.path[sizeof(info.path) - 1] = '\0';

        if (ioctl(fd, IOCTL_GET_FS_INFO, &info) < 0)
        {
            perror("Failed to get filesystem info");
            continue;
        }

        double used_percent = (double)info.used / info.total * 100.0;

        if (human_readable)
        {
            printf("%-30s %-15s %-15.2f %-15.2f %-15.2f %-15.2f %-15.2f\n",
                   info.path,
                   info.fs_name,
                   (double)info.total / (1024 * 1024 * 1024),
                   (double)info.used / (1024 * 1024 * 1024),
                   (double)info.free / (1024 * 1024 * 1024),
                   used_percent,
                   (double)info.total / 1024);
        }
        else
        {
            printf("%-30s %-15s %-15lu %-15lu %-15lu %-15.2f %-15lu\n",
                   info.path,
                   info.fs_name,
                   info.total,
                   info.used,
                   info.free,
                   used_percent,
                   info.total / 1024);
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}
