#!/bin/bash
set -x

git submodule update --init --force

# Install required dependencies for libical

if [[ "$OSTYPE" == "linux-gnu" ]]; then
  sudo apt-get install cmake perl
else
  which -s perl || brew install perl
  which -s cmake || brew install cmake
fi

# Build libical
rm -rf libical/build
mkdir libical/build
cd libical/build
cmake ..
make
sudo make install

if [[ "$OSTYPE" == "linux-gnu" ]]; then
  sudo ldconfig
fi
