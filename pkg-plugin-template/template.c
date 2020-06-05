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

/* Include other headers if needed */
#include <stdio.h>

/* Include pkg */
#include <pkg.h>

/* Define plugin name and configuration settings */
static const char PLUGIN_NAME[] = "template";
static const char CFG_TEXT[] = "text";
static const char CFG_COUNT[] = "count";
static const char CFG_FLAG[] = "flag";
static const char CFG_COLORS[] = "colors";

/* Maintain a reference to ourself */
static struct pkg_plugin *self;

/*
 * The plugin *must* provide an init function that is called by the library.
 *
 * The plugin's init function takes care of registering a hook in the library,
 * which is handled by the pkg_plugins_hook() function.
 *
 * Every plugin *must* provide a 'pkg_plugins_init_<plugin>' function, which is
 * called upon plugin loading for registering a hook in the library.
 *
 * The plugin's init function prototype should be in the following form:
 *
 * int pkg_plugins_init (void);
 *
 * No arguments are passed to the plugin's init function.
 *
 * Upon successful initialization of the plugin EPKG_OK (0) is returned and
 * upon failure EPKG_FATAL ( > 0 ) is returned to the caller.
 */
int
pkg_plugin_init(struct pkg_plugin *p)
{
	/* Keep a reference to our plugin object, so it can be used inside the callback functions */
	self = p;

	/*
	 * Declare the plugin's metadata
	 *
	 * This information is shown by 'pkg plugins'.
	 *
	 */

	pkg_plugin_set(p, PKG_PLUGIN_NAME, PLUGIN_NAME);
	pkg_plugin_set(p, PKG_PLUGIN_DESC, "Template plugin for pkgng");
	pkg_plugin_set(p, PKG_PLUGIN_VERSION, "1.0.0");

	/*
	 * Register configuration settings
	 *
	 * Declares
	 * - key (the name of the setting in the configuration file)
	 * - type (type of the setting, one of PKG_STRING, PKG_BOOL, PKG_INT, PKG_ARRAY)
	 * - default value (value if not set in the configuration file, provide empty string for no default)
	 *
	 */

	pkg_plugin_conf_add(p, PKG_STRING, CFG_TEXT, "");
	pkg_plugin_conf_add(p, PKG_INT, CFG_COUNT, "0");
	pkg_plugin_conf_add(p, PKG_BOOL, CFG_FLAG, "false");
	pkg_plugin_conf_add(p, PKG_ARRAY, CFG_COLORS, "");

	/* Parse the configuration file for above settings. Do not forget this! */
	pkg_plugin_parse(p);

	/*
	 * Register two functions for hooking into the library
	 *
	 * my_callback1() will be triggered directly before any install actions are taken, which is
	 * specified by the PKG_PLUGINS_HOOK_PRE_INSTALL hook.
	 *
	 * my_callback2() will be triggered directly after any install actions were taken, which is
	 * specified by the PKG_PLUGINS_HOOK_POST_INSTALL hook.
	 *
	 */

	/* printf(">>> Plugin '%s' is about to hook into pkgng.. yay! :)\n", pkg_plugin_get(p, PKG_PLUGIN_NAME)); */
	
	if (pkg_plugin_hook_register(p, PKG_PLUGIN_HOOK_PRE_INSTALL, &my_callback1) != EPKG_OK) {
		pkg_plugin_error(p, "failed to hook into the library");
		return (EPKG_FATAL);
	}
	
	if (pkg_plugin_hook_register(p, PKG_PLUGIN_HOOK_POST_INSTALL, &my_callback2) != EPKG_OK) {
		pkg_plugin_error(p, "failed to hook into the library");
		return (EPKG_FATAL);
	}
	
	return (EPKG_OK);
}

/*
 * Plugins may optionally provide a shutdown function.
 *
 * When a plugin provides a shutdown function, it is called
 * before a plugin is being unloaded. This is useful in cases
 * where a plugin needs to perform a cleanup for example, or
 * perform any post-actions like reporting for example.
 *
 * The plugin's shutdown function prototype should be in the following form:
 *
 * int pkg_plugins_shutdown (struct pkg_plugin *);
 *
 * Upon successful shutdown of the plugin EPKG_OK (0) is returned and
 * upon failure EPKG_FATAL ( > 0 ) is returned to the caller.
 */
int
pkg_plugin_shutdown(struct pkg_plugin *p __unused)
{
	/* printf(">>> Plugin '%s' is shutting down, enough working for today.. :)\n", pkg_plugin_get(p, PKG_PLUGIN_NAME)); */

	/*
	 * Perform any cleanup if needed, e.g.:
	 * 
	  if (tidy) {
	   	rc = perform_cleanup();
	   	if (rc != EPKG_OK)
	   		return (EPKG_FATAL);
	  }
	*/

	return (EPKG_OK);
}

/*
 * And now we need to define our workers,
 * the plugin functions that carry out the real work.
 *
 * A plugin callback function must satisfy the following function pointer signature:
 *
 * int(*pkg_plugin_callback)(void *data, struct pkgdb *db);
 *
 * It should return EPKG_OK (0) on success and EPKG_FATAL ( > 0 ) on failure.
 *
 * Plugin callbacks must also take care of proper casting of the (void *data) argument.
 *
 * Depending on where a plugin hooks into the library the data passed to the callback is
 * different.
 *
 * For example if a plugin hooks into PKG_PLUGINS_HOOK_PRE_INSTALL the (void *data) passed to the
 * called is (struct pkg_jobs *), so the plugin callback must cast it explicitely.
 *
 * If the callback needs to access configuration data, a reference to the plugin object
 * can be kept in the global scope of this compilation unit:
 *
 * static struct pkg_plugin *self;
 *
 */
int
my_callback1(void *data, struct pkgdb *db)
{
	const char *text = NULL;
	int64_t count = 0;
	bool flag = false;
	const pkg_object *colors = NULL;
	const pkg_object *color = NULL;
	const pkg_object *cfg = NULL;
	pkg_iter it = NULL;

	/* Get configuration object */
	cfg = pkg_plugin_conf(self);

	/* Get configuration data */
	flag = pkg_object_bool(pkg_object_find(cfg, CFG_FLAG));
	count = pkg_object_int(pkg_object_find(cfg, CFG_COUNT));
	text = pkg_object_string(pkg_object_find(cfg, CFG_TEXT));
	colors = pkg_object_find(cfg, CFG_COLORS);


	pkg_plugin_info(self, "Hey, I was just called by the library, lets see what we've got here..");

	pkg_plugin_info(self, "  Text: '%s'", text);
	pkg_plugin_info(self, "  Count: %i", count);
	pkg_plugin_info(self, "  Flag: %s", flag ? "true" : "false");
	while ((color = pkg_object_iterate(colors, &it)) != NULL) {
		pkg_plugin_info(self, "  Color: %s", pkg_object_string(color));
	}

	if (data == NULL)
		pkg_plugin_info(self, "Hmm.. no data for me today, guess I'll just go and grab a mohito then..");
	else
		pkg_plugin_info(self, "Got some data.. okay, okay.. I'll do something useful then..");

	return (EPKG_OK);
}

/*
 * Second callback function
 */
int
my_callback2(void *data, struct pkgdb *db)
{
	pkg_plugin_info(self, "Hey, I was just called again, lets see what its all about this time..");

	if (data == NULL)
		pkg_plugin_info(self, "Hmm.. no data, no work.. today is my lucky day!");
	else
		pkg_plugin_info(self, "Work never ends.. I'll do something useful again..");
	
	return (EPKG_OK);
}
