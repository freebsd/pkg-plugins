## General Information

The *pkg-plugin-template* plugin is a plugin meant to be used as a template
when developing a new plugin for pkgng.

It contains some examples and hints how a plugin should be structured and built. To start a new
project, copy the plugin directory to a new location:

	$ cp -r /path/to/pkg-plugins-template /path/to/new/project

## How to build the plugin?

In order to build the plugin enter into the plugin's directory and run make(1), e.g.:

	$ cd /path/to/pkg-plugins-template
	$ make
	
Once the plugin is built you can install it using the following command:

	$ make install 
	
The plugin will be installed as a shared library in ${PREFIX}/lib/pkg/template.so (or wherever
the *PKG\_PLUGINS\_DIR* option points to).

## Configuring the plugin

In order to configure the plugin simply copy the *template.conf* file to the pkgng plugins directory,
which by default is set to */usr/local/etc/pkg*, unless you've specified it elsewhere by
using the *PLUGINS\_CONF\_DIR* option.

	$ cp /path/to/pkg-plugins-template/template.conf /usr/local/etc/pkg/
	
## Enabling the plugin

To use the plugin, tell pkgng about it by adding it to the *PLUGINS* option in ${PREFIX}/etc/pkg.conf.
Also make sure *PKG\_ENABLE\_PLUGINS* is set to true:

	PKG_ENABLE_PLUGINS = true;
	PLUGINS [
		"foo",
		"template",
	]

## Testing the plugin

To test the plugin, first check that it is recongnized and
loaded by pkgng by executing the `pkg plugins` command:

	$ pkg plugins
	NAME       DESC                                VERSION
	foo        Foo plugin for pkg                  1.1.0
	template   Template plugin for pkgng           1.0.0

If the plugin shows up correctly then you are good to go! :)

