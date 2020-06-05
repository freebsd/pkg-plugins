## General Information

The *checkcfg* plugin checks if plugins have access to their configuration data.

## How to build the plugin?

In order to build the plugin enter into the plugin's directory and run make(1), e.g.:

	$ cd /path/to/checkcfg
	$ make
	
## How to check if plugins have acces to their configuration data

If the plugin is displayed when pkg(8) is invoked with the `plugins` command, the
status line for the plugin will indicate if plugins can access their configuration data.
From within the plugin's build directory, run:

	$ pkg -C pkg.conf plugins
	NAME       DESC                                          VERSION   
	checkcfg   Configuration available: YES                  1.0.0     

For convenience, the same result can be achieved by invoking the `run` target via make(1), e.g.:

	$ make run
	NAME       DESC                                          VERSION   
	checkcfg   Configuration available: YES                  1.0.0     

If the description field of *checkcfg* says `YES`, access is possible. If it says `NO`,
plugins cannot access their configuration data.

The easiest way to check is by invoking the `test` target via make(1), e.g.:

	$ make test

If make(1) returns without errors, the current pkg(8) tool allows plugins to access
their configuration data.

To test a specific pkg(8) binary, set `PKG` when invoking the `run` and `test` targets:

	$ make PKG=/path/to/pkg run
	$ make PKG=/path/to/pkg test

