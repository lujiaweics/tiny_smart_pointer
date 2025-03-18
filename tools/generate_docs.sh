#!/bin/bash
set -e

cd ../docs
doxygen Doxyfile

cd ../docs/build/html
python3 -m http.server 8000