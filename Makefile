PGM_NAME=cdfr

ASM=sdasz80
CC=sdcc
CCFLAGS=-mz80
LDFLAGS=-mz80 --code-loc 0x4100 --no-std-crt0
EMULATOR=/opt/AceDL/AceDL

ASM_OBJS= \
	build/crt0.s.rel \
	build/putchar.s.rel

C_OBJS= \
	build/main.c.rel

all: build/$(PGM_NAME).dsk build/$(PGM_NAME).cdt

build/%.s.rel: src/%.s
	$(ASM) -o $@ $<

build/%.c.rel: src/%.c
	$(CC) $(CCFLAGS) -c $< -o $@

build/$(PGM_NAME).ihx: $(ASM_OBJS) $(C_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

build/$(PGM_NAME).bin: build/$(PGM_NAME).ihx
	python tools/ihx_to_bin.py -o $@ $<

build/$(PGM_NAME).dsk: build/$(PGM_NAME).bin
	python tools/bin_to_dsk.py -o $@ $<

build/$(PGM_NAME).cdt: build/$(PGM_NAME).bin
	tools/2cdt -s 0 -n tools/loader.bas -t 0 -r CDFR.BAS $@
	tools/2cdt -s 0 $< -t 0 -r CDFR.BIN $@

play: build/$(PGM_NAME).dsk
	$(EMULATOR) $<

clean:
	rm build/*
