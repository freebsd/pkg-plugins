/*
 * Copyright (c) 2012 Baptiste Daroussin <bapt@FreeBSD.org>
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
#include <sys/sbuf.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#define _WITH_DPRINTF
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <errno.h>

#include <pkg.h>

enum {
	EVENT_FIFO_PATH=1,
	EVENT_SOCK_PATH,
	EVENT_IGNORE_ERROR,
	EVENT_FIFO_BLOCK
};

struct pkg_plugin *self;
static int fd[2];
struct sbuf *msg = NULL;

static int
print_event(void *data, struct pkgdb *db __unused)
{
	struct pkg *pkg = NULL;
	struct pkg_dep *dep = NULL;
	const char *message;
	const char *name, *version, *newversion;
	const char *filenmae;
	struct pkg_event *ev = (struct pkg_event *)data;

	if (msg == NULL)
		msg = sbuf_new_auto();
	else
		sbuf_clear(msg);

	switch(ev->type) {
	case PKG_EVENT_ERRNO:
		sbuf_printf(msg, "{ \"type\": \"ERROR\", "
		    "\"data\": {\"msg\": \"%s(%s)\"}}",
		    ev->e_errno.func, ev->e_errno.arg);
		break;
	case PKG_EVENT_ERROR:
		sbuf_printf(msg, "{ \"type\": \"ERROR\", "
		    "\"data\": {\"msg\": \"%s\"}}",
		    ev->e_pkg_error.msg);
		break;
	case PKG_EVENT_DEVELOPER_MODE:
		sbuf_printf(msg, "{ \"type\": \"ERROR\", "
		    "\"msg\": \"DEVELOPPER_MODE: %s\"}",
		    ev->e_pkg_error.msg);
		break;
	case PKG_EVENT_FETCHING:
		sbuf_printf(msg, "{ \"type\": \"INFO_FETCH\", "
		    "\"data\": { "
		    "\"url\": \"%s\", "
		    "\"fetched\": %" PRId64 ", "
		    "\"total\": %" PRId64
		    "}}",
		    ev->e_fetching.url,
		    ev->e_fetching.done,
		    ev->e_fetching.total
		    );
		break;
	case PKG_EVENT_INSTALL_BEGIN:
		pkg_get(ev->e_install_begin.pkg, PKG_NAME, &name,
		    PKG_VERSION, &version);

		sbuf_printf(msg, "{ \"type\": \"INFO_INSTALL_BEGIN\", "
		    "\"data\": { "
		    "\"pkgname\": \"%s\", "
		    "\"pkgversion\": \"%s\""
		    "}}",
		    name,
		    version
		    );
		break;
	case PKG_EVENT_INSTALL_FINISHED:
		pkg_get(ev->e_install_finished.pkg,
		    PKG_MESSAGE, &message,
		    PKG_NAME, &name,
		    PKG_VERSION, &version);

		sbuf_printf(msg, "{ \"type\": \"INFO_INSTALL_FINISHED\", "
		    "\"data\": { "
		    "\"pkgname\": \"%s\", "
		    "\"pkgversion\": \"%s\", "
		    "\"message\": \"%s\""
		    "}}",
		    name,
		    version,
		    message
		    );
		break;
	case PKG_EVENT_INTEGRITYCHECK_BEGIN:
		sbuf_printf(msg, "{ \"type\": \"INFO_INTEGRITYCHECK_BEGIN\", "
		    "\"data\": {"
		    "}}");
		break;
	case PKG_EVENT_INTEGRITYCHECK_FINISHED:
		sbuf_printf(msg, "{ \"type\": \"INFO_INTEGRITYCHECK_FINISHED\", "
		    "\"data\": {"
		    "}}");
		break;
	case PKG_EVENT_DEINSTALL_BEGIN:
		pkg_get(ev->e_deinstall_begin.pkg,
		    PKG_NAME, &name,
		    PKG_VERSION, &version);

		sbuf_printf(msg, "{ \"type\": \"INFO_DEINSTALL_BEGIN\", "
		    "\"data\": { "
		    "\"pkgname\": \"%s\", "
		    "\"pkgversion\": \"%s\""
		    "}}",
		    name,
		    version
		    );
		break;
	case PKG_EVENT_DEINSTALL_FINISHED:
		pkg_get(ev->e_deinstall_finished.pkg,
		    PKG_NAME, &name,
		    PKG_VERSION, &version);

		sbuf_printf(msg, "{ \"type\": \"INFO_DEINSTALL_FINISHED\", "
		    "\"data\": { "
		    "\"pkgname\": \"%s\", "
		    "\"pkgversion\": \"%s\""
		    "}}",
		    name,
		    version
		    );
		break;
	case PKG_EVENT_UPGRADE_BEGIN:
		pkg_get(ev->e_upgrade_begin.pkg,
		    PKG_NAME, &name,
		    PKG_VERSION, &version,
		    PKG_NEWVERSION, &newversion);

		sbuf_printf(msg, "{ \"type\": \"INFO_UPGRADE_BEGIN\", "
		    "\"data\": { "
		    "\"pkgname\": \"%s\", "
		    "\"pkgversion\": \"%s\" ,"
		    "\"pkgnewversion\": \"%s\""
		    "}}",
		    name,
		    version,
		    newversion
		    );
		break;
	case PKG_EVENT_UPGRADE_FINISHED:
		pkg_get(ev->e_upgrade_finished.pkg,
		    PKG_NAME, &name,
		    PKG_VERSION, &version,
		    PKG_NEWVERSION, &newversion);

		sbuf_printf(msg, "{ \"type\": \"INFO_UPGRADE_FINISHED\", "
		    "\"data\": { "
		    "\"pkgname\": \"%s\", "
		    "\"pkgversion\": \"%s\" ,"
		    "\"pkgnewversion\": \"%s\""
		    "}}",
		    name,
		    version,
		    newversion
		    );
		break;
	case PKG_EVENT_LOCKED:
		pkg_get(ev->e_locked.pkg,
		    PKG_NAME, &name,
		    PKG_VERSION, &version);
		sbuf_printf(msg, "{ \"type\": \"ERROR_LOCKED\", "
		    "\"data\": { "
		    "\"pkgname\": \"%s\", "
		    "\"pkgversion\": \"%s\""
		    "}}",
		    name,
		    version
		    );
		break;
	case PKG_EVENT_REQUIRED:
		pkg_get(ev->e_required.pkg,
		    PKG_NAME, &name,
		    PKG_VERSION, &version);
		sbuf_printf(msg, "{ \"type\": \"ERROR_REQUIRED\", "
		    "\"data\": { "
		    "\"pkgname\": \"%s\", "
		    "\"pkgversion\": \"%s\", "
		    "\"force\": %s, "
		    "\"required_by\": [",
		    name,
		    version,
		    ev->e_required.force == 1 ? "true": "false");
		while (pkg_rdeps(pkg, &dep) == EPKG_OK)
			sbuf_printf(msg, "{ \"pkgname\": \"%s\", "
			    "\"pkgversion\": \"%s\" }, ",
			    pkg_dep_name(dep),
			    pkg_dep_version(dep));
		sbuf_setpos(msg, sbuf_len(msg) - 2);
		sbuf_cat(msg, "]}}");
		break;
	case PKG_EVENT_ALREADY_INSTALLED:
		pkg_get(ev->e_already_installed.pkg,
		    PKG_NAME, &name,
		    PKG_VERSION, &version);
		sbuf_printf(msg, "{ \"type\": \"ERROR_ALREADY_INSTALLED\", "
		    "\"data\": { "
		    "\"pkgname\": \"%s\", "
		    "\"pkgversion\": \"%s\""
		    "}}",
		    name,
		    version);
		break;
	case PKG_EVENT_MISSING_DEP:
		sbuf_printf(msg, "{ \"type\": \"ERROR_MISSING_DEP\", "
		    "\"data\": { "
		    "\"depname\": \"%s\", "
		    "\"depversion\": \"%s\""
		    "}}" ,
		    pkg_dep_name(ev->e_missing_dep.dep),
		    pkg_dep_version(ev->e_missing_dep.dep));
		break;
	case PKG_EVENT_NOREMOTEDB:
		sbuf_printf(msg, "{ \"type\": \"ERROR_NOREMOTEDB\", "
		    "\"data\": { "
		    "\"url\": \"%s\" "
		    "}}" ,
		    ev->e_remotedb.repo);
		break;
	case PKG_EVENT_NOLOCALDB:
		sbuf_printf(msg, "{ \"type\": \"ERROR_NOLOCALDB\", "
		    "\"data\": {} ");
		break;
	case PKG_EVENT_NEWPKGVERSION:
		sbuf_printf(msg, "{ \"type\": \"INFO_NEWPKGVERSION\", "
		    "\"data\": {} ");
		break;
	case PKG_EVENT_FILE_MISMATCH:
		pkg_get(ev->e_file_mismatch.pkg,
		    PKG_NAME, &name,
		    PKG_VERSION, &version);
		sbuf_printf(msg, "{ \"type\": \"ERROR_FILE_MISMATCH\", "
		    "\"data\": { "
		    "\"pkgname\": \"%s\", "
		    "\"pkgversion\": \"%s\", "
		    "\"path\": \"%s\""
		    "}}",
		    name,
		    version,
		    pkg_file_path(ev->e_file_mismatch.file));
		break;
	case PKG_EVENT_PLUGIN_ERRNO:
		sbuf_printf(msg, "{ \"type\": \"ERROR_PLUGIN\", "
		    "\"data\": {"
		    "\"plugin\": \"%s\", "
		    "\"msg\": \"%s(%s)\""
		    "}}",
		    pkg_plugin_get(ev->e_plugin_errno.plugin, PKG_PLUGIN_NAME),
		    ev->e_plugin_errno.func, ev->e_plugin_errno.arg);
		break;
	case PKG_EVENT_PLUGIN_ERROR:
		sbuf_printf(msg, "{ \"type\": \"ERROR_PLUGIN\", "
		    "\"data\": {"
		    "\"plugin\": \"%s\", "
		    "\"msg\": \"%s\""
		    "}}",
		    pkg_plugin_get(ev->e_plugin_error.plugin, PKG_PLUGIN_NAME),
		    ev->e_plugin_error.msg);
		break;
	case PKG_EVENT_PLUGIN_INFO:
		sbuf_printf(msg, "{ \"type\": \"INFO_PLUGIN\", "
		    "\"data\": {"
		    "\"plugin\": \"%s\", "
		    "\"msg\": \"%s\""
		    "}}",
		    pkg_plugin_get(ev->e_plugin_info.plugin, PKG_PLUGIN_NAME),
		    ev->e_plugin_info.msg);
		break;
	default:
		break;
	}
	sbuf_finish(msg);
	if (fd[0] != -1)
		dprintf(fd[0], "%s\n", sbuf_data(msg));
	if (fd[1] != -1)
		dprintf(fd[1], "%s\n", sbuf_data(msg));

	return (EPKG_OK);
}

static int
connect_socket(const char *path)
{
	struct sockaddr_un sock;

	if ((fd[1] = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		pkg_plugin_errno(self, "socket", NULL);
		return (EPKG_FATAL);
	}

	memset(&sock, 0, sizeof(struct sockaddr_un));
	sock.sun_family = AF_UNIX;
	if (strlcpy(sock.sun_path, path, sizeof(sock.sun_path)) >=
	    sizeof(sock.sun_path)) {
		pkg_plugin_error(self, "Socket path too long: %s", path);
		close(fd[1]);
		return (EPKG_FATAL);
	}

	if (connect(fd[1], (struct sockaddr *)&sock, SUN_LEN(&sock)) == -1) {
		pkg_plugin_errno(self, "connect", sock.sun_path);
		return (EPKG_FATAL);
	}

	return (EPKG_OK);
}

int
pkg_plugin_init(struct pkg_plugin *p)
{
	const char *pathpipe;
	const char *pathsock;
	bool ignore_error;
	bool block;
	int flag = O_WRONLY;

	self = p;
	fd[0] = -1;
	fd[1] = -1;

	pkg_plugin_set(p, PKG_PLUGIN_NAME, "pipeevent");
	pkg_plugin_set(p, PKG_PLUGIN_DESC, "send events through pipes");
	pkg_plugin_set(p, PKG_PLUGIN_VERSION, "1.0.0");

	pkg_plugin_conf_add_string(p, EVENT_FIFO_PATH, "EVENT_FIFO_PATH", NULL);
	pkg_plugin_conf_add_string(p, EVENT_SOCK_PATH, "EVENT_SOCK_PATH", NULL);
	pkg_plugin_conf_add_bool(p, EVENT_FIFO_BLOCK, "EVENT_FIFO_BLOCK", false);
	pkg_plugin_conf_add_bool(p, EVENT_IGNORE_ERROR, "EVENT_IGNORE_ERROR", false);

	pkg_plugin_parse(p);

	pkg_plugin_conf_string(self, EVENT_FIFO_PATH, &pathpipe);
	pkg_plugin_conf_string(self, EVENT_SOCK_PATH, &pathsock);
	pkg_plugin_conf_bool(self, EVENT_IGNORE_ERROR, &ignore_error);
	pkg_plugin_conf_bool(self, EVENT_FIFO_BLOCK, &block);

	if (pathpipe != NULL) {
		if (!block)
			flag |= O_NONBLOCK;
		if ((fd[0] = open(pathpipe, flag)) == -1) {
			pkg_plugin_errno(self, "open", pathpipe);
			if (!ignore_error)
				return (EPKG_FATAL);
		}
	}

	if (pathsock != NULL) {
		if (connect_socket(pathsock) == EPKG_FATAL && !ignore_error) {
			if (fd[0] != -1)
				close(fd[0]);
			return (EPKG_FATAL);
		}
	}

	if (pkg_plugin_hook_register(p, PKG_PLUGIN_HOOK_EVENT, &print_event) != EPKG_OK) {
		pkg_plugin_error(self, "failed to hook into the library");
		return (EPKG_FATAL);
	}

	return (EPKG_OK);
}

int
pkg_plugin_shutdown(struct pkg_plugin *p __unused)
{
	bool create;
	const char *path;

	if (fd[0] != -1)
		close(fd[0]);
	if (fd[1] != -1)
		close(fd[1]);

	return (EPKG_OK);
}
