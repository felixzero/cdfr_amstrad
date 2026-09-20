PGM_NAME=cdfr

ASM=sdasz80
ASMFLAGS=
CC=sdcc
CCFLAGS=-mz80 --Werror
LDFLAGS=-mz80 --code-loc 0x8010 --data-loc 0x0100 -Wl-b_INITIALIZED=0x9120 -Wl-b_INIT=0x8000 -Wl-b_LOOKUP_TABLE=0x9000 --no-std-crt0
EMULATOR=/opt/AceDL/AceDL

ASM_OBJS= \
	build/crt0.s.rel \
	build/sprite_assets.s.rel \
	build/graphics.s.rel \
	build/inputs.s.rel

C_OBJS= \
	build/main.c.rel \
	build/sprites.c.rel \
	build/game_model.c.rel \
	build/model_view.c.rel

BACKGROUND_OBJ=build/background.scr

SPRITE_ASSETS= \
	artworks/block_1e.png \
	artworks/block_2e.png \
	artworks/block_3e.png \
	artworks/block_1s.png \
	artworks/block_2s.png \
	artworks/block_3s.png \
	artworks/robot_1n.png \
	artworks/robot_1s.png \
	artworks/robot_1w.png \
	artworks/robot_2e.png \
	artworks/robot_2n.png \
	artworks/robot_2s.png \
	artworks/robot_2w.png \
	artworks/robot_2e.png

all: dist/$(PGM_NAME).dsk dist/$(PGM_NAME).cdt

build/%.scr: artworks/%.png
	python tools/png_to_background_bin.py -o $@ $<

build/sprite_assets.s: $(SPRITE_ASSETS)
	python tools/generate_sprite_assets.py $^

build/sprite_assets.s.rel: build/sprite_assets.s
	$(ASM) $(ASMFLAGS) -o $@ $<

build/%.s.rel: src/%.s
	$(ASM) $(ASMFLAGS) -o $@ $<

build/%.c.rel: src/%.c
	$(CC) $(CCFLAGS) -c $< -o $@

build/$(PGM_NAME).ihx: $(ASM_OBJS) $(C_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

build/$(PGM_NAME).bin: build/$(PGM_NAME).ihx
	python tools/ihx_to_bin.py -o $@ $<
	@if [ `stat -c %s $@` -ge 9984 ]; then echo "Error: BIN file too large"; exit 1; fi

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
	@rm build/*

mrproper: clean
	@rm dist/*
