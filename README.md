# PhoDispl

A responsive and minimalistic image viewer for linux.



# Overview

## Why Does PhoDispl Exist?

While there are many excellent image viewers for linux out there,
they usually assume that loading an image is an easy and fast task
(which is of course the general case).
However, when frequently dealing with images which due to their format or size
take multiple seconds to decode, this can become quite inconvenient.
PhoDispl aims at providing a reasonable image viewing experience across multiple orders
of magnitude of image decoding durations.


## Features
* Cache for fast switching to next / previous images
* GPU based rendering
* Loading progress feedback for large image files
* Animated gif/webp/avif/jxl images
* Auto reloading if image file changed


## Non-Features
* Image editing
* Image organization / Album creation
* Cloud connection



# Getting Started

## Dependencies

To build PhoDispl you will need:
* GCC C++ 12 or newer
* meson
* libepoxy
* freetype
* fontconfig

At least one window backend:
* Native wayland support:
    - wayland development libraries
    - xkbcommon
* GLFW (for X support)

As well as the following libraries which will be pulled as meson subprojects and compiled
if they are not available:
* [logcerr](https://github.com/wolmibo/logcerr)
* [iconfigp](https://github.com/wolmibo/iconfigp)
* [pixglot](https://github.com/wolmibo/pixglot)

To install all dependencies on Fedora run:
```sh
sudo dnf install gcc-c++ meson libepoxy-devel fmt-devel fontconfig-devel \
wayland-devel wayland-protocols-devel freetype-devel libxkbcommon-devel glfw-devel \
libpng-devel libjpeg-turbo-devel giflib-devel libwebp-devel \
libavif-devel libjxl-devel openexr-devel
```


## Build Instructions

To install PhoDispl run the following commands in the project's root directory:
```sh
meson setup -Dbuildtype=release build
meson compile -C build
sudo meson install -C build
```


## Updating the Source
When updating the source code, remember to also update all meson subprojects:
```sh
git pull
meson subprojects update
```



# Further Resources
* [How to configure PhoDispl](doc/config.ini)
* [Keybindings](doc/keybindings.md)
