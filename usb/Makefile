# SPDX-License-Identifier: GPL-2.0

KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
M ?= $(shell pwd)

KBUILD_OPTIONS	+= CONFIG_AOC_DRIVER=m

EXTRA_CFLAGS	+= -I$(KERNEL_SRC)/../private/google-modules/aoc_ipc
EXTRA_CFLAGS	+= -I$(KERNEL_SRC)/../private/google-modules/aoc
EXTRA_CFLAGS	+= -I$(KERNEL_SRC)/drivers/usb/host
EXTRA_SYMBOLS	+= $(OUT_DIR)/../private/google-modules/aoc/Module.symvers
EXTRA_SYMBOLS	+= $(OUT_DIR)/../private/google-modules/aoc/alsa/Module.symvers

modules modules_install clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) W=1 \
	$(KBUILD_OPTIONS) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" KBUILD_EXTRA_SYMBOLS="$(EXTRA_SYMBOLS)" $(@)
