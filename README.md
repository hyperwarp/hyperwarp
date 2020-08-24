# Hyperwarp Metadata

## Dependencies

This build requires a C compiler like GCC, [CMake](https://cmake.org), [FoundationDB](https://github.com/apple/foundationdb), [protobuf-c](https://github.com/protobuf-c/protobuf-c) to be installed on your machine.

## Build

First, after cloning this repository run CMake to generate the build files:

```bash
$ cmake .
-- Configuring done
-- Generating done
-- Build files have been written to: /home/ubuntu/hyperwarp-metadata
$
```

Now you can run `make` to build the source code:

```bash
$ make
[ 33%] Built target hyperwarp-metadata-model
[ 66%] Built target hyperwarp-metadata-fdb
[100%] Built target hyperwarp-metadata
$
```

The build will output the binary files into the `bin` folder in a structure like this:

```
bin/
    metadata-foundationdb/
        libhyperwarp-metadata-fdb.so
    metadata-model/
        libhyperwarp-metadata-model.so
    samples/
        physical-disk-sample
```

## Run the samples using the development container

1. Pull the development container image for Linux amd64 from [Docker Hub](https://hub.docker.com/r/hyperwarp/build-metadata) and run it, either manually or as a VSCode devcontainer.
2. Build the code as described [above](README.md#build).
3. Start a local FoundationDB server by running `sudo service foundationdb start` inside the container.
4. Run the samples, e.g. `./bin/samples/physical-disk-sample`

```bash
$ ./bin/samples/physical-disk-sample
Wrote PhysicalDisk to FDB
  Key = 1234567890
  Sector Count = 266144
  Sector Size = 4096
Read PhysicalDisk from FDB
  Key = 1234567890
  Sector Count = 266144
  Sector Size = 4096
Tada!
$
```