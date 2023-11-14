# Third party code

This directory contains third party code used in the project.

## libart

libart is an Adaptive Radix Tree (ART) implementation, that is used to
implement the dictionary type in sconf. The reason that this is used
instead of a hash map is because it (potentially) uses less memory when
only a few entries are added to the dictionary.

Project URL: https://github.com/armon/libart
