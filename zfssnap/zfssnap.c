/*
 * Copyright (c) 2012 Marin Atanasov Nikolov <dnaeon@gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/param.h>

#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libutil.h>
#include <unistd.h>

#include <pkg.h>

#define PLUGIN_NAME "zfssnap"

enum {
	ZFS_FS=1,
	ZFS_PREFIX,
	ZFS_ARGS,
};

struct pkg_plugin *self;

static int plugins_zfssnap_callback(void *data, struct pkgdb *db);

static int plugins_zfssnap_fd = -1;

int
init(struct pkg_plugin *p)
{
	self = p;
	pkg_plugin_set(p, PKG_PLUGIN_NAME, PLUGIN_NAME);
	pkg_plugin_set(p, PKG_PLUGIN_DESC, "ZFS snapshot plugin");
	pkg_plugin_set(p, PKG_PLUGIN_VERSION, "1.0.0");

	pkg_plugin_conf_add_string(p, ZFS_FS, "ZFS_FS", NULL);
	pkg_plugin_conf_add_string(p, ZFS_PREFIX, "ZFS_PREFIX", NULL);
	pkg_plugin_conf_add_string(p, ZFS_ARGS, "ZFS_ARGS", NULL);

	pkg_plugin_parse(p);

	if (pkg_plugin_hook_register(p, PKG_PLUGIN_HOOK_PRE_INSTALL, &plugins_zfssnap_callback) != EPKG_OK) {
		fprintf(stderr, ">>> Plugin '%s' failed to hook into the library\n", PLUGIN_NAME);
		return (EPKG_FATAL);
	}

	if (pkg_plugin_hook_register(p, PKG_PLUGIN_HOOK_PRE_DEINSTALL, &plugins_zfssnap_callback) != EPKG_OK) {
		fprintf(stderr, ">>> Plugin '%s' failed to hook into the library\n", PLUGIN_NAME);
		return (EPKG_FATAL);
	}

	return (EPKG_OK);
}

int
shutdown(struct pkg_plugin *p __unused)
{
	close(plugins_zfssnap_fd);
	
	return (EPKG_OK);
}

static int
plugins_zfssnap_callback(void *data, struct pkgdb *db)
{
	char cmd_buf[MAXPATHLEN + 1];
	struct tm *tm = NULL;
	const char *zfs_fs = NULL;
	const char *zfs_args = NULL;
	const char *zfs_prefix = NULL;
	time_t t = 0;

	t = time(NULL);
	tm = localtime(&t);
	
	/* we don't care about data and db, so nothing to assert() here */
	/* assert(db != NULL); */ 
	/* assert(data != NULL); */

	pkg_plugin_conf_string(self, ZFS_FS, &zfs_fs);
	pkg_plugin_conf_string(self, ZFS_ARGS, &zfs_args);
	pkg_plugin_conf_string(self, ZFS_PREFIX, &zfs_prefix);

	if ((zfs_fs == NULL) || (zfs_prefix == NULL)) {
		fprintf(stderr, ">>> Configuration options missing, plugin '%s' will not be loaded\n",
			       PLUGIN_NAME);
		return (EPKG_FATAL);
	}

	snprintf(cmd_buf, sizeof(cmd_buf), "%s %s %s@%s-%d-%d-%d_%d.%d.%d",
		 "/sbin/zfs snapshot", zfs_args,
		 zfs_fs, zfs_prefix,
		 1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
		 tm->tm_hour, tm->tm_min, tm->tm_sec);

	printf(">>> Creating ZFS snapshot\n");
	system(cmd_buf);
	
	return (EPKG_OK);
}

