CC = gcc
CFLAGS = -O2 -Wall -Wextra -fPIC
LIBS = -loqs

all: build/libpqc_engine.so fuse

build/libpqc_engine.so: src/pqc_engine.c src/pqc_engine.h
mkdir -p build
$(CC) $(CFLAGS) -shared src/pqc_engine.c -o build/libpqc_engine.so $(LIBS)

fuse: build/libpqc_engine.so
cd src/quantumvault_fuse && cargo build

mount: all
mkdir -p /tmp/qvault_mount
cd src/quantumvault_fuse && LD_LIBRARY_PATH=../../build:/usr/local/lib cargo run -- /tmp/qvault_mount

unmount:
fusermount3 -u /tmp/qvault_mount

clean:
rm -rf build kyber_test
cd src/quantumvault_fuse && cargo clean
