/*
 * Copyright (c) 2020 Markus Stoff
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

/* Include other headers if needed */
#include <stdio.h>

/* Include pkg */
#include <pkg.h>

/* Define plugin name and configuration settings */
static const char PLUGIN_NAME[] = "checkcfg";
static const char PLUGIN_VERSION[] = "1.0.0";

/*
 * Upon successful initialization of the plugin EPKG_OK (0) is returned and
 * upon failure EPKG_FATAL ( > 0 ) is returned to the caller.
 */
int
pkg_plugin_init(struct pkg_plugin *p)
{
	const pkg_object *cfg = NULL;
	bool hasConfig = false;
	char message[80];

	/* Get configuration object */
	cfg = pkg_plugin_conf(p);

	/* Confirm access to configuration options */
	hasConfig = (pkg_object_type(cfg) == PKG_OBJECT);
	snprintf(message, sizeof(message), "Configuration available: %s", hasConfig ? "YES" : "NO");

	/* Register plugin */
	pkg_plugin_set(p, PKG_PLUGIN_NAME, PLUGIN_NAME);
	pkg_plugin_set(p, PKG_PLUGIN_VERSION, PLUGIN_VERSION);
	pkg_plugin_set(p, PKG_PLUGIN_DESC, message);

	return (EPKG_OK);
}

