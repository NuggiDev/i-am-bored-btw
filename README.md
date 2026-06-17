# I am bored btw

> "I am bored btw" but it's a Turing-complete programming language.
>
> **Forked from [overmighty/i-use-arch-btw](https://github.com/overmighty/i-use-arch-btw)**

## Introduction

I am bored btw is an esoteric programming language based on [Brainfuck](
https://en.wikipedia.org/wiki/Brainfuck) in which the commands are the following
keywords:

`i`, `am`, `bored`, `btw`, `by`, `the`, `way`, `sleep`.

See the [language specification](./docs/language_specification.md) for more
information.

This repository contains a [C/C++ library implementing I am bored btw](./lib)
and a dependent [command-line interpreter](./cmd).

## Getting Started

### Prerequisites

- [CMake](https://cmake.org/) >= 3.23
- a C99 and C++17 compiler toolchain supported by CMake and providing POSIX
  [`unistd.h`](https://en.wikipedia.org/wiki/Unistd.h), `mmap()`, `MAP_ANON`,
  and defining `__x86_64__` when targeting x86-64

### Building

    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON ..
    $ cmake --build .

### Installation

    # cmake --install .

### Usage

#### Command-line interpreter

    $ i-am-bored-btw <source file>

Try some of the [example I am bored btw programs](./examples) as source files.

For details:

    $ i-am-bored-btw -h

#### C/C++ library

For documentation of the public API, see the [public headers](
./lib/include/iabb).

For example usage, see the [command-line interpreter](./cmd) and [example
libiabb programs](./examples/libiabb).

## License

This software is licensed under the [GNU General Public License, version 3](
./LICENSE.md).

This is a derivative work of [overmighty/i-use-arch-btw](https://github.com/overmighty/i-use-arch-btw).
