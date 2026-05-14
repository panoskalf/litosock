# LitoSock

*A minimal cross-platform socket wrapper*
> Following [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) but in C++, in any OS

## What it is

RAII wrappers for BSD sockets that work on both Linux and Windows.
Meant to be read alongside Beej's guide, not as a replacement.

## Status

`Work in progress` - being built chapter by chapter through the guide.

## Build

From project root folder:
1. `cmake --preset linux` for linux
   `cmake --preset windows` fow windows with MinGW
2. `cmake --build build`
