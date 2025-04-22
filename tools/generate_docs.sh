#!/bin/bash
set -e

PROJECT_ROOT=$(dirname "$(realpath "$0")")/..
DOCS_DIR="$PROJECT_ROOT/docs"
HTML_DIR="$DOCS_DIR/build/html"

echo "Entering docs directory..."
cd "$DOCS_DIR" || { echo "Failed to enter docs directory."; exit 1; }

echo "Generating Doxygen documentation..."
doxygen Doxyfile

echo "Entering HTML directory..."
cd "$HTML_DIR" || { echo "Failed to enter HTML directory."; exit 1; }

echo "Starting HTTP server to view documentation..."
python3 -m http.server 8000