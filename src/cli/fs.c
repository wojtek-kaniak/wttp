// Needed for `unshare`
#define _GNU_SOURCE

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>

#include "fs.h"

void fs_chroot(const char* path)
{
	// Obtain root, then chroot into `path`

	int old_uid = getuid();

	if (unshare(CLONE_NEWUSER) != 0)
		perror("failed to create a user namespace");

	int fd = open("/proc/self/uid_map", O_WRONLY);
	if (fd == -1)
		perror("failed to open a UID map");

	char UID_MAPPING[64];
	snprintf(UID_MAPPING, sizeof(UID_MAPPING), "0 %d 1", old_uid);
	size_t mapping_len = strlen(UID_MAPPING);

	if (write(fd, UID_MAPPING, mapping_len) != mapping_len)
		perror("failed to write to the UID map");

	if (close(fd) != 0)
		perror("failed to close the UID map");

	chroot(path);
}
