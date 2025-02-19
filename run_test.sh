#!/bin/bash
if [ -d "build" ];then
  rm -rf build
fi

mkdir build && cd build
cmake ..
if [ $? -eq 0 ];then
  cmake --build .
fi

if [ $? -eq 0 ];then
  ./tiny_smart_pointer_unitest
fi