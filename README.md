# i80k

i80k is a small kernel targeting vintage Intel CPUs like the 8086 (for now). It runs on a custom homemade board.

_Note: the code is still under heavy development and must be used with caution._

## Build

The kernel uses [Bazel](https://bazel.build) and a IA16 version of GCC compiler from [tkchia/gcc-ia16](https://github.com/tkchia/gcc-ia16) project. The toolchain can be built using the Docker container provided or by pulling the binaries from the toolchain maintainer website.

### Build the toolchain

The GCC IA16 toolchain is expected to be located in `toolchains/ia16-elf` directory. The instructions below tell how to build it from scratch using the a Docker container.

Create the toolchain directory:

```
cd toolchains/
mdkir ia16-elf
```

Create the container image:

```
docker build -t build-gcc-ia16 --build-arg USER_ID=$(id -u) --build-arg GROUP_ID=$(id -g) .
```

Build the toolchain (that's where you take a cup of coffee):

```
docker run -v `pwd`/ia16-elf:/out build-gcc-ia16
```

Now the toolchain is available in `ia16-elf` directory, remove the docker image:

```
docker image rm build-gcc-ia16
```

### Build the kernel

To build the kernel, install Bazel first, then run:

```
bazel build --config=8088-rev2 //kernel:kernel.bin
```

the resulting ROM image is then available in `bazel-bin/kernel/kernel.bin`. The `--config=` flags helps selecting the right build configuration for the board you want to target. Valid configurations are `8088-rev2` or `8088-rev3`.

The custom Bazel toolchain declaration is located under `toolchains/`, see [Bazel Tutorial: Configure C++ Toolchains](https://bazel.build/tutorials/ccp-toolchain-config) for more details.

### Run tests

To build and run a test suite, run:

```
bazel test --cxxopt=-std=c++14 --test_output=all <target>
```

For instance:

```
bazel test --cxxopt=-std=c++14 --test_output=all //kernel/utils:utils_test
```

### Board configuration

The build is configured for a specific board using Bazel configurations, the parameters are located in `.bazelrc`. For now only one board is supported.

## ROM image

The build product is a ROM image ready to be flashed on a 32kB ROM, typically an EEPROM. The ROM is layered for an EEPROM physically starting at address `0xF8000` (`0xF000:0x8000`). The code will run in this address space while data (initialized and uninitialized) will be relocated in the first memory segment during the early boot phase.
