# pep440-cpp

## Overview

`pep440-cpp` is a C++ library that implements [PEP 440](https://peps.python.org/pep-0440/), the
Python version specification standard. It provides functionality to parse, compare, and manipulate
version strings according to PEP 440 rules, enabling robust version handling in C++ applications.
This library is designed to be lightweight, efficient, and easy to integrate into projects requiring
version management.

## Features

- **Version Parsing**: Parse version strings into structured components (epoch, release, pre-release,
post-release, dev, and local labels) as defined by PEP 440.
- **Version Comparison**: Compare version strings using operators like `==`, `!=`, `<`, `<=`, `>`,
and `>=`, with support for strict equality checks.
- **Range Matching**: Define and evaluate version ranges (e.g., `>=1.2.3`, `~=2.0`) to check if a
version satisfies specific constraints.
- **RangeSet Support**: Handle multiple version constraints combined with commas (e.g., `>=1.0,<2.0`).
- **Standard Compliance**: Fully adheres to PEP 440, including normalization of pre-release labels
and handling of optional version components.

## Installation

### Prerequisites

- C++17 or later
- CMake 3.10 or later
- A C++ compiler (e.g., GCC, Clang)

### Building

1. Clone the repository:

   ```bash
   git clone https://github.com/0xM4LL0C/pep440-cpp.git
   cd pep440-cpp
   ```

2. Create a build directory:

   ```bash
   mkdir .build && cd .build
   ```

3. Configure the project with CMake:

   ```bash
   cmake ..
   ```

4. Build the library:

   ```bash
   cmake --build .
   ```

5. (Optional) Build and run tests:
   ```bash
   cmake .. -DBUILD_TESTS=ON
   cmake --build .
   ctest # or ./tests
   ```

### CMake Integration

To use `pep440-cpp` in your CMake project, include it as a subdirectory or install it and link
against the `pep440` library:

```cmake
add_subdirectory(pep440-cpp)
target_link_libraries(your_target PRIVATE pep440)
```

Or, if installed:

```cmake
find_package(pep440 REQUIRED)
target_link_libraries(your_target PRIVATE pep440)
```

Alternatively, you can use [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) to fetch and integrate
`pep440-cpp` directly into your project:

```cmake
CPMAddPackage("gh:0xM4LL0C/pep440-cpp#0.1.0")
target_link_libraries(your_target PRIVATE pep440)
```

## Usage

### Parsing a Version

```cpp
#include <pep440/version.hpp>
#include <iostream>

int main() {
    try {
        auto v = pep440::Version::parse("1!2.3.4.rc1.post2.dev3+local");
        std::cout << v.to_string() << std::endl; // Outputs: 1!2.3.4rc1.post2.dev3+local
    } catch (const pep440::VersionParseError& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
```

### Comparing Versions

```cpp
#include <pep440/version.hpp>
#include <iostream>

int main() {
    auto v1 = pep440::Version::parse("1.0.0");
    auto v2 = pep440::Version::parse("1.0.1");
    std::cout << (v1 < v2) << std::endl; // Outputs: 1 (true)
    std::cout << v1.to_string() << std::endl; // Outputs: 1.0.0
}
```

### Checking Version Ranges

```cpp
#include <pep440/version.hpp>
#include <iostream>

int main() {
    auto range = pep440::Range::parse(">=1.0.0,<2.0.0");
    auto version = pep440::Version::parse("1.5.0");
    std::cout << range.matches(version) << std::endl; // Outputs: 1 (true)
}
```

### RangeSet Example

```cpp
#include <pep440/version.hpp>
#include <iostream>

int main() {
    auto range_set = pep440::RangeSet::parse(">=1.0.0,!=1.5.0");
    auto version = pep440::Version::parse("1.6.0");
    std::cout << range_set.matches(version) << std::endl; // Outputs: 1 (true)
}
```

## Testing

The library includes unit tests using [Catch2](https://github.com/catchorg/Catch2). To build and run
tests, enable the `BUILD_TESTS` option during CMake configuration:

```bash
cmake .. -DBUILD_TESTS=ON
cmake --build .
ctest
```

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Contributing

Contributions are welcome! Please submit pull requests or open issues on the
[GitHub repository](https://github.com/0xM4LL0C/pep440-cpp).

## Contact

For questions or feedback, please open an issue on the GitHub repository.
