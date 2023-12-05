
<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://github.com/gyrovorbis/libevmu">
    <img src="https://vmu.elysianshadows.com/libevmu_icon.gif" width="200" height="200" alt="libEVMU">
  </a>

  <h3 align="center">libElysianVMU</h3>

  <p align="center">
    C17-Based library for emulating the Sega Dreamcast's Visual Memory Unit
    <br />
    <a href="http://vmu.elysianshadows.com"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    Accurate
    ·
    Full-Featured
    ·
    Cross-Platform
  </p>
</div>

# Overview #
libElysianVMU (libEVMU) is a standalone emulator core of the Sega Dreamcast's 8-bit Visual Memory Unit (VMU), written in C17. It is the core powering the ElysianVMU emulator for Windows, MacOS, and Linux, which has been separated from any platform-specific back-end or UI front-end. Several years of meticulous research and reverse engineering have gone into the creation of this core, which has now been open-sourced in hopes of benefitting the Dreamcast community at-large. 

# Goals # 
The primary goal of libEVMU is to provide a one-stop, all-encompassing framework exposing everything the VMU has to offer in a standard C codebase that can be ported to any platform or wrapped to any language, allowing for everyone to use it in their projects and to benefit from a common codebase. At a high-level, this goal encompasses:
- Fully and accurately emulating the VMU as a standalone gaming device
- Providing tooling and APIs around its filesystem and all relevant file formats
- Meticulously documenting every aspect of the codebase to expose everything that was previously undiscovered
- Being performant enough to be ported to a Sega Saturn, N64, or toaster oven
- Offering a high-level entity/component-based API that is intuitive and easy to work with
- Allowing for modeling exotic, customized, nonstandard VMUs with extended volumes or capabilities
- Providing low-level hooks for supporting debuggers and high-level tooling
- Rigorously unit testing all aspects of the codebase to ensure correctness

# Features #
- Battery
    - Emulated low-voltage signal
- BIOS Support
    - Official (US and Japanese)
        - Skip directly to GAME
        - Modify date/time
    - Emulated Software back-end
- Buzzer
    - Creates and manages PCM buffers for audio back-end
- File System 
    - Import + export files
    - Formatting
    - Defragmentation
    - Unlock/lock extra flash blocks
    - Diagnostics and validation
    - Changing volume icons or color
    - Loading custom ICONDATA 
    - Modifying copy protection bits
    - Loading a GAME file from only a VMS and no VMI
    - Recalculating checksums
    - Extracting and texturing icons
    - Extracting and texturing eyecatches
    - Supported File Formats
        - .VMI
        - .VMS
        - .DCI
        - .DCM
        - .flash
        - .bin
- Gamepad
    - Polling or event-driven input back-ends
    - Supports turbo buttons, slow motion, and fast-forward
- LCD Screen
    - Emulated pixel ghosting and grayscale effects
    - Extra options for bilinear filtering, color inversion, etc
    - Provides a simple virtual framebuffer abstraction for renderer back-end
    - Provides asynchronous screen refresh callbacks, only when contents change

# Platforms #
libEVMU is being actively tested in CI on the following targets:
- Windows 
- MacOS 
- Linux 
- Sega Dreamcast
- Sony PSVita
- WebAssembly
- iOS
- Android

# Compilers #
libEVMU is being built in CI with the following compilers:
- GCC
- Clang
- MinGW-w64
- Emscripten

NOTE: Microsoft Visual Studio support is currently a work in progress!

# Building #
Building is done with standard CMake. You should be able to open CMakeLists.txt directly as a project file in most IDEs such as XCode, Qt Creator, CLion, etc if you wish to build from a UI.

First, ensure submodules are installed with:
```
git submodule update --init --recursive
```

To build the project and its unit tests from the command-line, you can do the following:
```
mkdir build
cd build
cmake -DEVMU_ENABLE_TESTS=ON ..
cmake --build . 
```

# Credits #
Author
- Falco Girgis

Contributors
- Colton Pawielski

Collaborators 
- Andrew Apperley
- Patrick Kowalik
- Walter Tetzner
- jvsTSX 
- Tulio Goncalves
- Kresna Susila 
- Sam Hellawell

Special Thanks
- Marcus Comstedt
- Shirobon
- Deunan Knute
- Dmitry Grinberg
- RinMaru
- UltimaNumber
- Joseph-Eugene Winzer