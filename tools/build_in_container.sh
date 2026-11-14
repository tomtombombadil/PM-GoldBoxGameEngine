#!/bin/bash
# Builds goldbox.<arch> inside an Ubuntu 20.04 container (CI runs this; see
# .github/workflows/build.yml). 20.04 on purpose: its glibc (2.31) is old
# enough for every firmware PortMaster supports - a binary built against a
# newer one won't start on the older ones.
#   tools/build_in_container.sh aarch64|x86_64     (the repo mounted at /src)
set -euo pipefail
ARCH="$1"
export DEBIAN_FRONTEND=noninteractive
cd /src

if [ "$ARCH" = "aarch64" ]; then
  # Cross-compile: the arm64 SDL2 from Ubuntu's ports archive
  dpkg --add-architecture arm64
  sed -i 's/^deb /deb [arch=amd64] /' /etc/apt/sources.list
  for s in focal focal-updates focal-security; do
    echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports $s main universe" >> /etc/apt/sources.list
  done
  apt-get update -qq
  apt-get install -y -qq --no-install-recommends cmake make binutils g++-aarch64-linux-gnu libsdl2-dev:arm64 >/dev/null
  CMAKE_ARGS=(-DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64
              -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++
              -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER
              -DSDL2_DIR=/usr/lib/aarch64-linux-gnu/cmake/SDL2)
else
  apt-get update -qq
  apt-get install -y -qq --no-install-recommends cmake make g++ libsdl2-dev >/dev/null
  CMAKE_ARGS=()
fi

cmake -S . -B "build-$ARCH" -DCMAKE_BUILD_TYPE=Release "${CMAKE_ARGS[@]}"
cmake --build "build-$ARCH" -j"$(nproc)"
mkdir -p dist
cp "build-$ARCH/goldbox" "dist/goldbox.$ARCH"
# (the newest glibc symbol it needs - for port.json's min_glibc)
echo "newest glibc needed: $(objdump -T "dist/goldbox.$ARCH" 2>/dev/null | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1)" 
chown -R "$(stat -c %u:%g /src)" dist "build-$ARCH"
