#DB-Impl

[![Build Status](https://travis-ci.org/vogelsgesang/dbimpl.svg?branch=master)](https://travis-ci.org/vogelsgesang/dbimpl)

This repository contains the solutions for the homeworks of the course [Database Systems on Modern CPU Architectures](http://www-db.in.tum.de/teaching/ss15/moderndbs/).

##Building

There a two types of builds: a debug build and a release build.
The release build is optimized for speed and does not contain debugging informations.
For the debug build all optimizations are turned off (`-O0`), debugging informations (`-g3`) are created and special debugging code is enabled (using `-DDEBUG`).

You can create a release build using `make`. For the debug build enter `make BUILD_TYPE=debug`.

The binaries will be created in the `bin/` folder.
Executables built with the debugging flags will have the `_debug` suffix.

##Testing

There are two types of test cases:

* GoogleTest tests: use these tests for unit and integration tests. All files with a name which matches `*Test.cpp` in the directory `tests/` and its subdirectories
  are automatically compiled and added to the test suite. The compiled testsuite will be named `bin/runTests` respectively `bin/runTests_debug`.
* scripted tests: these tests are intended for testing the commands provided to bash scripts. Test scripts must be placed in the `tests/` directory or one of its subdirectories
  and their name must match `*Test.sh`.

`make test` executes both types of tests.
All test cases are automatically executed by the continous integration service [TravisCI](https://travis-ci.org/vogelsgesang/dbimpl) as soon as new commits are pushed.
Nevertheless, always test your commits before pushing them!

##Unused code

There is some code which I wrote but which is not used currently. You can find it in the `unused` directory.

##External sorting

For the first assignment an external sort algorithm had to be implemented. We chose to implement a merge sort algorithm.
The binary is called `bin/externalSort`.

###Useful shell commands

`od` can be used in order to inspect the contents of a file.

```
od -A n -t x2 -v <fileName> #create a hex dump
od -A n -t u8 -v <fileName> #print the file as uint_64t
od -A n -t u8 -w8 -v <fileName> #print the file as uint_64t, one uint64_t per line
```

You can combine `od` with `sort` in order to sort the values in a file:

```
od -A n -t u8 -w8 -v <fileName> | sort -n
```

##Buffer manager

A thread-safe buffer manager can be found in the `buffer` directory.
It uses the 2Q strategy for page replacement.

##Schema

Schema definitions can be stored in the database.
There are three different tools to deal with the schemas:

* `bin/parseSchema <inputFile>`: parses the schema from `<inputFile>` and prints it to the console.
* `bin/loadSchema <inputFile>`: parses the schema from `<inputFile>` and stores it within the database. All previously loaded schema information will be lost.
* `bin/showSchema`: shows the schema currently stored in the database.

Only a subset of the valid SQL schema definitions are accepted by `parseSchema` and `loadSchema`. One accepted schema definition can be found in `exampleSchema.sql`.

##Slotted pages

An implementation of slotted pages can be found in `slottedPages`.

##B+-Tree

A template implementation of a B+-Tree can be found in `bTree`.
It uses the existing implemtation of the buffer manager to store its nodes.
Concurrent access is provided by lock coupling.

A simple CLI is available as `bin/btreeVisualizer`. The file `btreeVisualizer.input.txt` contains some example commands.

#Code generation

A code generator for arithmetic expressions can be found in the `codegen` folder.

It provides classes for forming an operator tree, generating LLVM code out of this tree and executing it.

It also contains a `MetaExpression` class which provides a more convenient interface for building
the operator trees by using C++ operator overloading.
