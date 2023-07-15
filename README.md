# HDF5 DataSet C99 Library

Abstractions are provided to cover common dataset operations.

See the [tests/write.c](tests/write.c) source code, which writes strings, scalar primitives and a chunked multi-dimensional integer array.


## Building

Uses `meson` and `ninja`, typically acquired via `pip`.

```
$ meson setup build && cd build
build $ ninja test && ninja install
```