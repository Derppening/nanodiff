# nanodiff

An extremely small portable diff checker.

## Why Another Diff Program?

The primary goal of this project is to provide a simple diff checker with the following characteristics:

- Portable: Supports all of Windows, Mac OS and Linux.
- No Dependencies: Only requires a C++ compiler to compile.
- Easy to Distribute: Only `nanodiff.cpp` and `Makefile` needs to be bundled for distribution.
- Simple: Outputs the first mismatch for easy identification of what went wrong.

This diff program is designed such that it can be easily distributed as part of a course assignment, so that
students can easily check their program output without the need for external commands.

## Getting Started

### Prerequisites

- **Compiler:** Requires a C++23-compatible compiler (e.g., GCC 13+, Clang 16+, MSVC 2022+).
- **Build System:** You can use either `make` or `cmake` to build the project.

### Building with Make

```sh
make        # Builds the release version
make debug  # Builds with debug info and sanitizers
make clean  # Removes the built executable
```

The executable will be `./nanodiff`, relative to the root of this directory.

### Building with CMake

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The executable will be `./build/nanodiff`, relative to the root of this directory.

### Running nanodiff

After building, run the program as follows:

```sh
./nanodiff <expected_file> <actual_file>
```

More options will be implemented in the future.

## Distribution

The simplest way to distribute this project is to include the entire project tree into your assignment.
You may then build `nanodiff` as part of the compilation/test process of your assignment.

For instance, for `make` users add the following directive as one of the recipes of your Makefile:

```make
diff:
		$(MAKE) -C nanodiff build
```

Automated zipping of necessary sources may be implemented in the future.

## Testing

### Prerequisites

- **Google Test:** The test suite uses Google Test. Make sure it is installed and available to CMake.
- **CMake:** Testing is only supported via CMake.

### Running Tests

```sh
ctest --build-and-test . build --build-target nanodiff-test --build-options -DCMAKE_BUILD_TYPE=Debug --test-command ctest --test-dir .
```

Test cases are located in the `test/resources` directory.

## Versioning

This project follows [Semantic Versioning](https://semver.org/).

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
