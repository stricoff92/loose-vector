
#!/bin/bash

set -e

echo "running vec_t tests"

mkdir -p build
gcc src/test.c -o build/test
./build/test

echo "✅ vec_t tests pass "


