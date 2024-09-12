
#!/bin/bash

set -e

echo "running lvec tests"

mkdir -p build
gcc src/test_lvec64.c -o build/test
./build/test
echo "✅ lvec64 tests pass "

mkdir -p build
gcc src/test_lvec.c -o build/test
./build/test
echo "✅ lvec tests pass "



