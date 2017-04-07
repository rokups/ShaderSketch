#
# Copyright (c) 2016 Rokas Kupstys
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
cmake_minimum_required(VERSION 2.8.12.1)
project(ShaderSketch)

# Disable various Atomic features. 'CACHE BOOL "" FORCE' is required for options declared with cmake `option()` call.
set(ATOMIC_CPLUSPLUS_EXAMPLES 0 CACHE BOOL "" FORCE)
set(ATOMIC_WEBVIEW 0 CACHE BOOL "" FORCE)
set(ATOMIC_JAVASCRIPT 0 CACHE BOOL "" FORCE)
set(ATOMIC_DOTNET 0 CACHE BOOL "" FORCE)
set(ATOMIC_EDITOR 0 CACHE BOOL "" FORCE)
# Add game engine to the project.
add_subdirectory(dep/AtomicGameEngine)
# Define our own project
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(SOURCE_FILES main.cpp)
add_executable(ShaderSketch ${SOURCE_FILES})
# Link engine to our project. It pulls in engine defines and include directories automatically.
target_link_libraries (ShaderSketch Atomic)
