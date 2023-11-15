# Simple config

Add configuration handling (argument parsing, config files, environment
variables, and so on) without having to add lots of boilerplate code to each
project.

## Features

Simpleconfig supports the following:

* Basic node types like string, integer, float and boolean.
* Nested nodes using dictionaries and arrays.
* "Get" and "set" functions for the various types.
* Iterators to traverse through nodes in dictionaries and arrays.
* Config map to define command-line options, environment variables,
  default values, validation callback functions, etc.
* Configuration files in YAML format.
* Automatically generate usage strings (usually used with -h/--help).

## Requirements

libyaml is needed to build simpleconfig.

## Build

Sconf is built like most other cmake projects:

```
mkdir build && cd build
cmake ..
make
make install
```

## Testing

The unit tests can be executed by running:

```
make test
make coverage  # to get a coverage report
```

## Fuzzing with AFL++

Support for fuzzing with AFL++ is added to the project.

After installing AFL++, fuzzers for sconf could be built in the following
way:

```
AFL_HARDEN=1 CC=afl-clang-fast cmake .. -DSCONF_ENABLE_COVERAGE=ON -DSCONF_ENABLE_ASAN=ON -DSCONF_BUILD_FUZZERS=ON
make
```

It is wise to enable ASAN (as above) when fuzzing. Also remember to create
a directory with input files (corpus) for the fuzzer.

The fuzzers could then be executed with afl-fuzz like this:

```
ASAN_OPTIONS=verbosity=3,abort_on_error=1 afl-fuzz -m none -i in/ -o out/ fuzz/fuzz_sconf_yaml_read
```

## Example

See `examples/minimal.c` for an example showing off the basics of simpleconfig.

## API

Check out the header file (`include/sconf.h`).
