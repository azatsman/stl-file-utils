# Utilities for STL files

This is a small set of simple tools to manipulate and get info about [STL files](https://en.wikipedia.org/wiki/STL_(file_format)) used in 3D pringing.
All tools use command-line interface and at this point have only been tested on a Linux system.

## Description

Here are the tools in alphabetical order:

 - `stl-bin-to-text` -- converts binary STL file to text version; this is mostly useful for getting a readble version.
 
 - `stl-check` -- performs limited checking of the file consistency; in particular, the tool makes sure that each edge has exactly two adjoint and consistently oriented faces 
 
 - `stl-compute-normals` -- creates a copy of the input file with recomputed normals; the old normals are ignored; the computed normals point outwards if the original STL file is correctly oriented
 
 - `stl-scale-xyz` --  creates a scaled copy of the input file; the scale factors can be different for each axis.

 - `stl-stats` -- dumps general information about the file , including the volume and the coordinate range, to the standard output
 
 - `stl-text-to-bin` -- converts the input, which is expected to be a text STL file, to a binary STL file.

## Build

The source code of some of the tools depends on __`boost::program_options`__ include files and the library. On a Debian-based Linux system the corresponding package is called `libboost-program-options-dev`.

Currently only `Makefile` is provided in the hope that it works on most Linux systems.
