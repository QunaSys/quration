# pyqret

## About pyqret

pyqret is a library for developing quantum circuits and compiling them into instruction sequences for concrete quantum computers, such as fault-tolerant quantum computers.
Because pyqret is a Python wrapper for qret, which is implemented in C++, refer to the qret documentation for internal implementation details.

### Features

* Concise description of quantum circuits
* Verification of quantum circuits with a simulator
* Compilation for fault-tolerant quantum computers

### Requirements

Linux, macOS (ARM64), and Windows 11 are supported.

* Linux
  * Python 3.10, 3.11, 3.12, 3.13, 3.14
* macOS (ARM64)
  * Python 3.10, 3.11, 3.12, 3.13, 3.14
* Windows 11
  * Python 3.10, 3.11, 3.12, 3.13, 3.14

## Installation

### Quick install

Install the `.whl` file that matches your environment (OS and Python version) with `pip`.
Replace `<version>` with the pyqret version to install, for example `0.7.2`.

#### Python 3.12 and later (3.12, 3.13, 3.14)

For Python 3.12 and later, use the common Stable ABI (abi3) compatible file.

```sh
# Common to Python 3.12, 3.13, and 3.14
pip install pyqret-<version>-cp312-abi3-manylinux_2_27_x86_64.manylinux_2_28_x86_64.whl
```

#### Python 3.10, 3.11

Use the dedicated file for each Python version.

```sh
# For Python 3.10
pip install pyqret-<version>-cp310-cp310-manylinux_2_27_x86_64.manylinux_2_28_x86_64.whl
```

### Install Python library from source

#### Requirements

* CMake >= 3.18
* C++ compiler (clang++ or VisualStudio)
  * clang++ >= 16.0.0 (checked in Linux)
  * Microsoft VisualStudio C++ 2022
  * g++ is not checked.
* C++ library: see `vcpkg.json`
* Python
  * Linux: 3.10, 3.11, 3.12, 3.13, 3.14
  * macOS(ARM64): 3.10, 3.11, 3.12, 3.13, 3.14
  * Windows: 3.10, 3.11, 3.12, 3.13, 3.14
* Python library: see `Pipfile`

#### How to install

##### Linux/macOS(ARM64)

Run the following command in the directory that contains `pyproject.toml` (the parent directory of this file).

```sh
SKBUILD_CMAKE_DEFINE="QRET_BUILD_PYTHON=ON" pip install .
```

To install the algorithm module as well, run the following command.

```sh
SKBUILD_CMAKE_DEFINE="QRET_BUILD_PYTHON=ON;QRET_PYTHON_WITH_ALGORITHM=ON" pip install .[algorithm]
```

If you are building the environment with Vcpkg, run the following command.

```sh
SKBUILD_CMAKE_DEFINE="CMAKE_TOOLCHAIN_FILE=<VCPKG_ROOT>/scripts/buildsystems/vcpkg.cmake;QRET_BUILD_PYTHON=ON" pip install .
```

##### Windows

Run the following commands in the directory that contains `pyproject.toml` (the parent directory of this file).

```cmd
set SKBUILD_CMAKE_DEFINE=QRET_BUILD_PYTHON=ON
pip install .
```

##### Install options

* `tests` : use when running tests.
* `docs` : use when building documentation.
* `algorithm` : use when installing with the algorithm module included.
* `dev` : use when developing pyqret.
