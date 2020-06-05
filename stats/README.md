## General Information

The *stats* plugin is used for displaying package stats during:

* pre-install
* post-install
* pre-deinstall
* post-deinstall

## How to build the plugin?

In order to build the plugin enter into the plugin's directory and run make(1), e.g.:

	$ cd /path/to/stats
	$ make
	
Once the plugin is built you can install it using the following command:

	$ make install 
	
The plugin will be installed as a shared library in ${PREFIX}/lib/pkg/stats.so

## Testing the plugin

To test the plugin, first check that it is recongnized and
loaded by pkgng by executing the `pkg plugins` command:

	$ pkg plugins
	NAME       DESC                                VERSION
	stats      Plugin for displaying package stats 1.1.0

If the plugin shows up correctly then you are good to go!

Now go ahead and install/deintall a package and see it in action! :)

