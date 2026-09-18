PGM_NAME=cdfr

ASM=sdasz80
CC=sdcc
CCFLAGS=-mz80
LDFLAGS=-mz80 --code-loc 0x8000 --data-loc 0xA000 --no-std-crt0
EMULATOR=/opt/AceDL/AceDL

ASM_OBJS= \
	build/crt0.s.rel \
	build/sprite_assets.s.rel \
	build/putchar.s.rel \
	build/graphics.s.rel

C_OBJS= \
	build/main.c.rel

BACKGROUND_OBJ=build/background.scr

SPRITE_ASSETS= \
	artworks/robot.png \
	artworks/element_jeu_bidon.png

all: dist/$(PGM_NAME).dsk dist/$(PGM_NAME).cdt

build/%.scr: artworks/%.png
	python tools/png_to_background_bin.py -o $@ $<

build/sprite_assets.s: $(SPRITE_ASSETS)
	python tools/generate_sprite_assets.py $^

build/sprite_assets.s.rel: build/sprite_assets.s
	$(ASM) -o $@ $<

build/%.s.rel: src/%.s
	$(ASM) -o $@ $<

build/%.c.rel: src/%.c
	$(CC) $(CCFLAGS) -c $< -o $@

build/$(PGM_NAME).ihx: $(ASM_OBJS) $(C_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

build/$(PGM_NAME).bin: build/$(PGM_NAME).ihx
	python tools/ihx_to_bin.py -o $@ $<

dist/$(PGM_NAME).dsk: build/$(PGM_NAME).bin $(BACKGROUND_OBJ)
	python tools/bin_to_dsk.py --background-image $(BACKGROUND_OBJ) --basic-loader loaders/disk.bas -o $@ $<

dist/$(PGM_NAME).cdt: build/$(PGM_NAME).bin $(BACKGROUND_OBJ)
	tools/2cdt -s 0 -n loaders/tape.bas -t 0 -r CDFR.BAS -F 22 $@
	tools/2cdt -s 0 $(BACKGROUND_OBJ) -t 0 -r BACKGND.BIN $@
	tools/2cdt -s 0 $< -t 0 -r CDFR.BIN $@

play: build/$(PGM_NAME).bin $(BACKGROUND_OBJ)
	$(EMULATOR) -enable_webapi -web_port 6128 &
	sleep 2
	python tools/load_to_emulator.py --code build/cdfr.bin --background build/background.scr

playdisk: dist/$(PGM_NAME).dsk
	$(EMULATOR) $<

playk7: dist/$(PGM_NAME).cdt
	$(EMULATOR) $<

clean:
	rm build/*

mrproper: clean
	rm dist/*
