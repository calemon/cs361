CXX=riscv64-unknown-linux-gnu-g++
CXXFLAGS=-Wall -O0 -g -I. -L. -Llib/target/debug -std=c++11 -mabi=ilp32 -march=rv32imac -nostartfiles -nostdinc -ffreestanding -nostdlib -Tvirt.lds
OUT=labx
SOURCES=$(wildcard *.cpp)
LIBDEP=$(wildcard lib*.a) $(wildcard lib/target/debug/lib*.a)
LIBS=-llabx
QEMU=qemu-system-riscv32
HDD=fl.blk
QARGS=-machine virt -drive if=none,format=raw,file=$(HDD),id=flhdd -device virtio-blk-device,scsi=off,drive=flhdd -device virtio-rng-device,rng=rng0 -object rng-random,filename=/dev/random,id=rng0 -m 128M -nographic -serial mon:stdio

all: $(OUT)

$(OUT): $(SOURCES) Makefile $(LIBDEP) vio.h
	$(CXX) $(CXXFLAGS) -o $(OUT) $(SOURCES) $(LIBS)

run: $(OUT) $(HDD)
	$(QEMU) $(QARGS) -kernel $(OUT)

$(HDD):
	dd if=/dev/urandom of=$(HDD) bs=1M count=16

.PHONY: clean
clean:
	rm -f $(OUT)
