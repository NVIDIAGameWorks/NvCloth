NvCloth 1.0
===========

Introduction
------------

NvCloth is a library that provides low level access to a cloth solver designed for realtime interactive applications.

Features:
* Fast and robust cloth simulation suitable for games
* Collision detection and response suitable for animated characters
* Low level interface with little overhead and easy integration

Documentation
-------------

See ./NvCloth/ReleaseNotes.txt for changes and platform support.
See ./NvCloth/docs/documentation/NvCloth.html for the users guide.
See ./NvCloth/docs/doxy/index.html for the api documentation.

Compiling
---------
For windows:
Download and install the following dependencies:
* CMake 3.7 https://cmake.org/download/
* Visual studio 12 or later (2013 or later)
* Windows 8.1 sdk https://developer.microsoft.com/en-us/windows/downloads/windows-8-1-sdk
* CUDA sdk https://developer.nvidia.com/cuda-downloads (version 8 or later)

edit ./NvCloth/scripts/locate_cmake.bat to point to the cmake executable
edit ./NvCloth/scripts/locate_cuda.bat to point to the CUDA installation folder (that contains the bin, include and lib folder)
edit ./NvCloth/scripts/locate_win8sdk.bat to point to the windows SDK (C:\Program Files (x86)\Windows Kits\8.1 by default)

Run ./CmakeGenerateAll.bat to generate the visual studio solution files in ./compiler/vcXXwinXX-cmake/
Run ./samples/CmakeGenerateProjects.bat to generate the visual studio solution files in ./samples/compiler/vcXXwinXX-cmake/

For osx:
Download and install cmake 3.4 and add the cmake binary to your path.
Install xcode.
Run ./NvCloth/GenerateProjectsOsx.sh to generate xcode projects in .//NvCloth/compiler/osxXX-cmake/

For linux:
Download and install cmake 3.4.
Installing cmake on Ubuntu 16.04.1 LTS can be done using the following commands:
```
sudo apt-get install software-properties-common
sudo add-apt-repository ppa:george-edison55/cmake-3.x
sudo apt-get update
sudo apt-get install cmake
```
Run ./NvCloth/GenerateProjectsLinux.sh to generate make files in .//NvCloth/compiler/linux64-XXXXX-cmake/
