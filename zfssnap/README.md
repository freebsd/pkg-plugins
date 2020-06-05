## General Information

The *zfssnap* plugin is a plugin meant to be used for creating snapshots
on a system with ZFS prior any install/deinstall actions are taken.

*zfssnap* is useful in a way that if something breaks in case your installation or
deinstallation of package(s) fails you will be able to rollback to a previously known and working
state of your system.

## How to build the plugin?

In order to build the plugin enter into the plugin's directory and run make(1), e.g.:

	$ cd /path/to/zfssnap
	$ make
	
Once the plugin is built you can install it using the following command:

	$ make install 
	
The plugin will be installed as a shared library in ${PREFIX}/lib/pkg/zfssnap.so

## Configuring the plugin

In order to configure the plugin simply copy the *zfssnap.conf* file to the pkgng plugins directory,
which by default is set to */usr/local/etc/pkg*, unless you've specified it elsewhere by 
using the *PKG\_PLUGINS\_DIR* option.

	$ cp /path/to/zfssnap/zfssnap.conf /usr/local/etc/pkg/
	
Next, open */usr/local/etc/pkg/zfssnap.conf* and configure any ZFS related options.
	
## Testing the plugin

To test the plugin, first check that it is recognized and
loaded by pkgng by executing the `pkg plugins` command:

	$ pkg plugins
	NAME       DESC                                VERSION
	zfssnap    ZFS snapshot plugin                 2.0.0

If the plugin shows up correctly then you are good to go! :)

Once you start installing/deinstall package(s) zfssnap will create a snapshot for you! 

