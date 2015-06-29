DTS := $(wildcard *.dts)

DTBOs := $(patsubst %.dts,%-00A0.dtbo,$(DTS))

%-00A0.dtbo: %.dts
	dtc -O dtb -o $@ -b 0 -@ $<

all: $(DTBOs)

install:
	install -m 0644 $(DTBOs) /lib/firmware

.PHONY clean:
clean:
	rm -f  $(DTBOs)
