# Build against the exact TWRP kernel ABI tree, never a stock Android tree.
# Example (on the Ubuntu build host):
#   make KDIR=/tmp/marlin-twrp-module-build \
#        KSRC=/tmp/marlin-twrp-kernel \
#        ARCH=arm64 CROSS_COMPILE=/tmp/android-gcc49-aarch64/bin/aarch64-linux-android-
obj-m += marlin_diag.o

all:
	$(MAKE) -C $(KSRC) O=$(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KSRC) O=$(KDIR) M=$(CURDIR) clean
