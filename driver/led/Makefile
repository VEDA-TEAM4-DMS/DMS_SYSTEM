obj-m := dms_led.o

ifdef ARCH
	KDIR := /work/linux
else
	KDIR := /lib/modules/$(shell uname -r)/build
endif

.PHONY: all led led-dev led-app led-dts clean disp help

all: led

led: led-dev led-app led-dts disp

led-dev:
	@if [ -f ./dms_led.c ] ; then \
		$(MAKE) -C $(KDIR) M=$(PWD) modules ;\
	else \
		echo "No dms_led.c found, skipping module build"; \
	fi

led-app:
	@if [ -f ./dms_led_test.c ] ; then \
		gcc -o app dms_led_test.c ;\
	else \
		echo "No dms_led_test.c found, skipping app build"; \
	fi

led-dts:
	@echo "Building device tree overlay..."
	@if [ -f ./dms_led.dts ] ; then \
		dtc -@ -I dts -O dtb -o dms_led.dtbo dms_led.dts ;\
	else \
		echo "No dms_led.dts found, skipping dtbo build"; \
	fi

disp:
	@echo "=============================="
	@if [ -f ./dms_led.ko ] ; then \
		echo "insmod dms_led.ko"; \
	else \
		echo "No dms_led.ko found"; \
	fi
	@if [ -f ./dms_led.dtbo ] ; then \
		echo "dtoverlay dms_led.dtbo"; \
	else \
		echo "No dms_led.dtbo found"; \
	fi
	@if [ -f ./app ] ; then \
		echo "./app"; \
	else \
		echo "No app binary found"; \
	fi
	@echo "=============================="

clean:
	@$(MAKE) -C $(KDIR) M=$(PWD) clean
	@rm -f app
	@echo ".ko and app cleaned"

help:
	@echo "Usage:"
	@echo "  make            # Build all (module, app, dts, disp)"
	@echo "  make led-dev    # Build only kernel module"
	@echo "  make led-app    # Build only user app"
	@echo "  make led-dts    # Build only device tree overlay"
	@echo "  make clean      # Clean all build files"