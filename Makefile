PGM_NAME=cdfr

ASM=sdasz80
CC=sdcc
CCFLAGS=-mz80
LDFLAGS=-mz80 --code-loc 0x8100 --no-std-crt0
EMULATOR=/opt/AceDL/AceDL

ASM_OBJS= \
	build/crt0.s.rel \
	build/putchar.s.rel

C_OBJS= \
	build/main.c.rel

BACKGROUND_OBJ=build/background.scr

ARTWORKS_OBJS= \
	build/background.scr

all: build/$(PGM_NAME).dsk build/$(PGM_NAME).cdt

build/%.scr: artworks/%.png
	python tools/png_to_background_bin.py -o $@ $<

build/%.s.rel: src/%.s
	$(ASM) -o $@ $<

build/%.c.rel: src/%.c
	$(CC) $(CCFLAGS) -c $< -o $@

build/$(PGM_NAME).ihx: $(ASM_OBJS) $(C_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

build/$(PGM_NAME).bin: build/$(PGM_NAME).ihx
	python tools/ihx_to_bin.py -o $@ $<

build/$(PGM_NAME).dsk: build/$(PGM_NAME).bin $(BACKGROUND_OBJ)
	python tools/bin_to_dsk.py --background-image $(BACKGROUND_OBJ) --basic-loader loaders/disk.bas -o $@ $<

build/$(PGM_NAME).cdt: build/$(PGM_NAME).bin $(BACKGROUND_OBJ)
	tools/2cdt -s 0 -n loaders/tape.bas -t 0 -r CDFR.BAS -F 22 $@
	tools/2cdt -s 0 $(BACKGROUND_OBJ) -t 0 -r BACKGND.BIN $@
	tools/2cdt -s 0 $< -t 0 -r CDFR.BIN $@

play: build/$(PGM_NAME).dsk
	$(EMULATOR) $<

playk7: build/$(PGM_NAME).cdt
	$(EMULATOR) $<

clean:
	rm build/*
