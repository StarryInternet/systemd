/*
 * udev.c
 *
 * Userspace devfs
 *
 * Copyright (C) 2003,2004 Greg Kroah-Hartman <greg@kroah.com>
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License as published by the
 *	Free Software Foundation version 2 of the License.
 * 
 *	This program is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	General Public License for more details.
 * 
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>

#include "libsysfs/sysfs/libsysfs.h"
#include "udev.h"
#include "udev_version.h"
#include "logging.h"
#include "namedev.h"

/* global variables */
char **main_argv;
char **main_envp;

#ifdef LOG
unsigned char logname[LOGNAME_SIZE];
void log_message (int level, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	if (format[strlen(format)-1] != '\n')
		printf("\n");
}
#endif

static char *subsystem_blacklist[] = {
	"net",
	"scsi_host",
	"scsi_device",
	"usb_host",
	"pci_bus",
	"pcmcia_socket",
	""
};

static int udev_hotplug(void)
{
	char *devpath;
	char *subsystem;
	int retval = -EINVAL;
	int i;

	devpath = main_argv[1];
	if (!devpath) {
		dbg("no devpath?");
		goto exit;
	}
	dbg("looking at '%s'", devpath);

	/* we only care about class devices and block stuff */
	if (!strstr(devpath, "class") &&
	    !strstr(devpath, "block")) {
		dbg("not a block or class device");
		goto exit;
	}

	/* skip blacklisted subsystems */
	subsystem = main_argv[1];
	i = 0;
	while (subsystem_blacklist[i][0] != '\0') {
		if (strcmp(subsystem, subsystem_blacklist[i]) == 0) {
			dbg("don't care about '%s' devices", subsystem);
			goto exit;
		}
		i++;
	}

	/* initialize our configuration */
	udev_init_config();

	/* initialize the naming deamon */
	namedev_init();

	/* simulate node creation with fake flag */
	retval = udev_add_device(devpath, subsystem, 1);

exit:
	if (retval > 0)
		retval = 0;

	return -retval;
}

int main(int argc, char *argv[], char *envp[])
{
	main_argv = argv;
	main_envp = envp;

	dbg("version %s", UDEV_VERSION);

	return udev_hotplug();
}


