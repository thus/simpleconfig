# Simple config

Add configuration handling (argument parsing, config files, environment
variables, and so on) without having to add lots of boilerplate code to each
project.

## Features

Sconf supports the following:

* Basic node types like string, integer, float and boolean.
* Nested nodes using dictionaries and arrays.
* "Get" and "set" functions for the various types.
* Iterators to traverse through nodes in dictionaries and arrays.
* Config map to define command-line options, environment variables,
  default values, validation callback functions, etc.
* Configuration files in YAML format.
* Automatically generate usage strings (usually used with -h/--help).

## Requirements

libyaml is needed to build sconf.

## Build

Sconf is built like most other cmake projects:

```
mkdir build && cd build
cmake ..
make
make install
```

Both the tests and examples are built by default.

## Testing

The unit tests can be executed by running:

```
make test
make coverage  # to get a coverage report
```

## Example

See `examples/minimal.c` for an example showing off the basics of sconf.

## API

Check out the header file (`include/sconf.h`).
