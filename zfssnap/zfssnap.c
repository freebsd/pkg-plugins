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
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libutil.h>
#include <unistd.h>
#include <spawn.h>

#include <pkg.h>

#define PLUGIN_NAME "zfssnap"
extern char **environ;

enum {
	ZFS_FS=1,
	ZFS_PREFIX,
	ZFS_RECURSIVE,
};

struct pkg_plugin *self;

static int plugins_zfssnap_callback(void *data, struct pkgdb *db);

static int plugins_zfssnap_fd = -1;

int
pkg_plugin_init(struct pkg_plugin *p)
{
	self = p;
	pkg_plugin_set(p, PKG_PLUGIN_NAME, PLUGIN_NAME);
	pkg_plugin_set(p, PKG_PLUGIN_DESC, "ZFS snapshot plugin");
	pkg_plugin_set(p, PKG_PLUGIN_VERSION, "1.0.0");

	pkg_plugin_conf_add_list(p, ZFS_FS, "ZFS_FS");
	pkg_plugin_conf_add_string(p, ZFS_PREFIX, "ZFS_PREFIX", "pkgsnap");
	pkg_plugin_conf_add_bool(p, ZFS_RECURSIVE, "ZFS_RECURSIVE", false);

	pkg_plugin_parse(p);

	if (pkg_plugin_hook_register(p, PKG_PLUGIN_HOOK_PRE_INSTALL, &plugins_zfssnap_callback) != EPKG_OK) {
		pkg_plugin_error(self, "failed to hook into the library");
		return (EPKG_FATAL);
	}

	if (pkg_plugin_hook_register(p, PKG_PLUGIN_HOOK_PRE_DEINSTALL, &plugins_zfssnap_callback) != EPKG_OK) {
		pkg_plugin_error(self, "failed to hook into the library");
		return (EPKG_FATAL);
	}

	return (EPKG_OK);
}

int
pkg_plugin_shutdown(struct pkg_plugin *p __unused)
{
	close(plugins_zfssnap_fd);

	return (EPKG_OK);
}

static int
plugins_zfssnap_callback(void *data, struct pkgdb *db)
{
	char snap[MAXPATHLEN];
	struct tm *tm = NULL;
	struct pkg_config_value *zfs_fs;
	bool zfs_recursive = false;
	const char *zfs_prefix = NULL;
	time_t t = 0;
	int error, pstat;
	pid_t pid;
	char *argv[] = {
		"zfs",
		"snapshot",
		NULL,
		NULL,
		NULL,
	};

	t = time(NULL);
	tm = localtime(&t);
	
	/* we don't care about data and db, so nothing to assert() here */
	/* assert(db != NULL); */ 
	/* assert(data != NULL); */

	pkg_plugin_conf_bool(self, ZFS_RECURSIVE, &zfs_recursive);
	pkg_plugin_conf_string(self, ZFS_PREFIX, &zfs_prefix);

	while (pkg_plugin_conf_list(self, ZFS_FS, &zfs_fs) == EPKG_OK) {
		snprintf(snap, sizeof(snap), "%s@%s-%d-%d-%d_%d.%d.%d",
		    pkg_config_value(zfs_fs), zfs_prefix,
		    1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
		    tm->tm_hour, tm->tm_min, tm->tm_sec);

		if (zfs_recursive) {
			argv[2] = "-r";
			argv[3] = snap;
		} else {
			argv[2] = snap;
			argv[3] = NULL;
		}
		pkg_plugin_info(self, "Creating ZFS snapshot: %s", snap);
		if ((error = posix_spawn(&pid, "/sbin/zfs", NULL, NULL,
		    __DECONST(char **, argv), environ)) != 0) {
			errno = error;
			pkg_plugin_errno(self, "Failed to snapshot", snap);
			return (EPKG_FATAL);
		}
		while (waitpid(pid, &pstat, 0) == -1) {
			if (errno != EINTR)
				return (EPKG_FATAL);
		}

		if (WEXITSTATUS(pstat) != 0) {
			pkg_plugin_error(self, "Failed to snapshot %s", snap);
			return (EPKG_FATAL);
		}
	}

	return (EPKG_OK);
}

