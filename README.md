# Corid-QCT [![Build Status](https://travis-ci.com/ithron/CortidQCT.svg?token=sHAjhRB8BYK3KqcS7UsE&branch=master)](https://travis-ci.com/ithron/CortidQCT) [![Build status](https://ci.appveyor.com/api/projects/status/gkajxg953p5ny1gp/branch/master?svg=true)](https://ci.appveyor.com/project/ithron/cortidqct/branch/master)

A tool for automatic cortical shape identification for QCT scans.

This software is based on
"**An Analysis by Synthesis Approach for Automatic Vertebral Shape Identification in Clinical QCT**" by
*[Stefan Reinhold](https://orcid.org/0000-0003-3117-1569), [Timo Damm](https://orcid.org/0000-0002-5595-5205), Lukas Huber, [Reimer Andresen](https://orcid.org/0000-0002-1575-525X), Reinhard Barkmann, [Claus-C. Glüer](https://orcid.org/0000-0003-3539-8955) and [Reinhard Koch](https://orcid.org/0000-0003-4398-1569)*, accepted for publication in Springer LNCS, presented on "German Conference on Pattern Recognition 2018".

## Building
The build instructions below are for *NIX (Unix/Linux/macOS) systems only.
For Windows build, the configuration steps (1 to 3) should be the same, but the compilation step might be different, dependent on the compiler used.

### Requirements
- *cmake* version 3.9 or newer
- A modern C++ compiler with C++17 support is required. These compilers are known to work:
  - gcc: version 7.3 or later
  - clang: version 4.0 or later (with libc++ or stdlibc++-7.3 or later)
  - AppleClang: version 10.0 or later (Xcode 10, since macOS 10.4 Mojave)
  - ~~MSVC++: version 14.1 or later (Visual Studio 2017 version 15.0 or later)~~

### Configuration

*CortidQCT* uses the [Hunter](https://hunter.sh) package manager to manage its dependencies.
Therefore, it's not required to manually install any dependecies.
Hunter builds and installs all dependencies automatically in the ```${HOME}/.hunter``` directory.
This can be changed by setting the `HUNTER_ROOT` environment variable.

1. Clone the repository:
   ```bash
   git clone htts://github.com/ithron/CortidQCT.git
   ```
2. Create build directory
   ```bash
   cd CortidQCT
   mkdir Build
   cd build
   ```
3. Run `cmake`
  ```bash
  cmake -DCMAKE_BUILD_TYPE=Release ../
  ```
4. Run
  ```bash
  make
  ```
