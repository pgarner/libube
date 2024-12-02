#
# Copyright 2013 by Philip N. Garner
#
# See the file COPYING for the licence associated with this software.
#
# Author(s):
#   Phil Garner, July 2013
#
cmake_minimum_required(VERSION 3.10)
project(libube VERSION 1.0.3)

# Package information
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(KISSFFT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/kissfft")

# Make cache variables for install destinations
include(GNUInstallDirs)
enable_testing()

# External deps
include(KissFFT)

add_subdirectory(lube)
add_subdirectory(test)
