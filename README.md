#DB-Impl

This repository contains the solutions for the homeworks of the course [Database Systems on Modern CPU Architectures](http://www-db.in.tum.de/teaching/ss15/moderndbs/).

##Building

There a two types of builds: a debug build and a release build.
The release build is optimized for speed and does not contain debugging informations.
For the debug build all optimizations are turned off (`-O0`), debugging informations (`-g3`) are created and special debugging code is enabled (using `-DDEBUG`).

You can create a release build using `make`. For the debug build enter `make BUILD_TYPE=debug`.

The binaries will be created in the `bin/` folder.
Executables built with the debugging flags will have the `_debug` suffix.
