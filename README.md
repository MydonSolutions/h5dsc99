# HDF5 DataSet C99 Library

Abstractions are provided to cover common dataset operations.

- See the [tests/write.c](tests/write.c) source code, which writes strings, scalar primitives, attributes and a chunked multi-dimensional integer array.
- See the [tests/reads.c](tests/reads.c) source code, which reads strings, scalar primitives, attributes and a chunked multi-dimensional integer array.


## Building

Uses `meson` and `ninja`, typically acquired via `pip`.

```
$ meson setup build && cd build
build $ ninja test && ninja install
```

## Development
```bash
$ docker compose up --build -d`
$ docker attach h5dsc99`
/work# cd h5dsc99_build/ && ninja test
```
