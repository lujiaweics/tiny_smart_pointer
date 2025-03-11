#!/bin/bash
cd unitest
rm -rf genhtml.info
rm unitest-*

g++ main.cpp test_sharedpointer.cpp test_weakpointer.cpp -lgtest -lpthread -fprofile-arcs -ftest-coverage -o unitest --coverage -fno-inline -fno-inline-small-functions -fno-default-inline
if [ $? -eq 0 ];then
  ./unitest
  if [ $? -eq 0 ];then
    lcov -c -d ../ -o test_gcov.info
    genhtml -o genhtml.info test_gcov.info
  fi
fi