
libdolramdisk
=============

*by carstene1ns 2012, 2014*

Compressed Ramdisk included in the .dol executable. 
Speed up access to data files and avoid SD card file clutter.

Provides the libogc standard devoptab for easy file access.

Installation
------------

To build all just enter:

    make

in the main directory. This will create the library for the
Wii and GameCube platform and ramdisk creator.

To build only the library run the target specific command:

    make library

You will also need the `mkdolramdisk` packer program:

    make packer

To install all in the libogc/devkitPPC folders enter:

    make install

To install just the parts:

    make install-library
    make install-packer

Usage
-----

TODO

LICENSE
-------

MIT, see LICENSE.md for details.

CREDITS
-------

* Maintainer: carstene1ns
* Original Idea (libdolfs): Greg Kennedy 2009

The interface is modeled after libfat:
* libfat: Michael "Chishm" Chisholm et al. 2006+

It uses libogc and DevkitPPC
* libogc: Michael "shagkur" Wiedenbauer et al. 2004+
* DevkitPPC: Dave "WinterMute" Murphy et al. 2004+
