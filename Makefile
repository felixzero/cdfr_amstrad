PGM_NAME=cdfr

CODE_LOC=0100
INIT_LOC=8000
DATA_LOC=A700
INIT_CODE_LOC=8000
TRUE_INITIALIZED_DATA_LOC=8020
BACKGROUND_LOC=4000
ASM_LOADER_LOC=A600
FB_BUFFER_HEAP_SIZE=0x1200

MAX_CODE_SIZE=11520
MAX_INITIALIZED_SIZE=9952

ASM=sdasz80
ASMFLAGS=
CC=sdcc
CCFLAGS=-mz80 -Ibuild/ -DFB_BUFFER_HEAP_SIZE=$(FB_BUFFER_HEAP_SIZE)
LDFLAGS=-mz80 --code-loc 0x$(CODE_LOC) --data-loc 0x$(DATA_LOC) \
	-Wl-b_INITIALIZED=0x$(TRUE_INITIALIZED_DATA_LOC) -Wl-b_INIT=0x$(INIT_CODE_LOC) --no-std-crt0
EMULATOR=/opt/AceDL/AceDL

ASM_OBJS= \
	build/crt0.s.rel \
	build/sprite_assets.s.rel \
	build/graphics.s.rel \
	build/inputs.s.rel \
	build/rect.s.rel \
	build/model_data.s.rel \
	build/font.s.rel

C_OBJS= \
	build/main.c.rel \
	build/sprites.c.rel \
	build/model.c.rel \
	build/view.c.rel \
	build/controller.c.rel \
	build/ui.c.rel \
	build/print.c.rel \
	build/score.c.rel

BACKGROUND_OBJ=build/background.scr

SPRITE_ASSETS= \
	artworks/block_1e.png \
	artworks/block_2e.png \
	artworks/block_3e.png \
	artworks/block_3e_built.png \
	artworks/block_1s.png \
	artworks/block_2s.png \
	artworks/block_3s.png \
	artworks/block_3s_built.png \
	artworks/tower.png \
	artworks/robot_1n.png \
	artworks/robot_1s.png \
	artworks/robot_1w.png \
	artworks/robot_1e.png \
	artworks/robot_2n.png \
	artworks/robot_2s.png \
	artworks/robot_2w.png \
	artworks/robot_2e.png \
	artworks/digit_0.png \
	artworks/digit_1.png \
	artworks/digit_2.png \
	artworks/digit_3.png \
	artworks/digit_4.png \
	artworks/digit_5.png \
	artworks/digit_6.png \
	artworks/digit_7.png \
	artworks/digit_8.png \
	artworks/digit_9.png \
	artworks/word_build.png \
	artworks/word_mine.png \
	artworks/word_ready.png \
	artworks/question_ready.png \
	artworks/question_will_start.png \
	artworks/question_finished.png \
	artworks/logo.png

all: dist/$(PGM_NAME).dsk dist/$(PGM_NAME).cdt

build/%.scr: artworks/%.png
	python tools/png_to_background_bin.py -o $@ $<

build/sprite_assets.s: $(SPRITE_ASSETS)
	python tools/generate_sprite_assets.py $^

build/sprite_assets.s.rel: build/sprite_assets.s
	$(ASM) $(ASMFLAGS) -o $@ $<

build/%.s.rel: src/%.s
	$(ASM) $(ASMFLAGS) -o $@ $<

build/%.c.rel: src/%.c build/sprite_assets.s.rel
	$(CC) $(CCFLAGS) -c $< -o $@

build/$(PGM_NAME).ihx: $(ASM_OBJS) $(C_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

build/code.bin: build/$(PGM_NAME).ihx
	python tools/ihx_to_bin.py -c $(CODE_LOC) -d $(INIT_LOC) -o $@ $<
	@if [ `stat -c %s build/initialized.bin` -ge $(MAX_INITIALIZED_SIZE) ]; then echo "Error: init BIN file too large"; exit 1; fi
	@if [ `stat -c %s build/code.bin` -ge $(MAX_CODE_SIZE) ]; then echo "Error: code BIN file too large"; exit 1; fi

build/loader.ihx: loaders/loader.s
	$(ASM) $(ASMFLAGS) -o build/loader.s.rel loaders/loader.s
	$(CC) -mz80 --code-loc 0x$(ASM_LOADER_LOC) \
		-Wl-g_bg_dest_addr=0x$(BACKGROUND_LOC) -Wl-g_code_dest_addr=0x$(CODE_LOC) -Wl-g_init_dest_addr=0x$(INIT_LOC) \
		--no-std-crt0 build/loader.s.rel -o $@

build/loader.bin: build/loader.ihx
	python tools/ihx_to_bin.py --code-loc $(ASM_LOADER_LOC) -o $@ $<

build/loader.bas: loaders/loader.bas
	sed -e 's/%addr/$(ASM_LOADER_LOC)/' $< > $@

dist/$(PGM_NAME).dsk: build/code.bin $(BACKGROUND_OBJ) build/loader.bas build/loader.bin
	python tools/bin_to_dsk.py -c $(CODE_LOC) -b $(BACKGROUND_LOC) -d $(INIT_LOC) \
		--code build/code.bin --background build/background.scr --initialized build/initialized.bin \
		--basic-loader build/loader.bas -o $@

dist/$(PGM_NAME).cdt: build/code.bin $(BACKGROUND_OBJ) build/loader.bas
	tools/2cdt -s 0 -n build/loader.bas -t 0 -r CDFR.BAS -F 22 $@
	tools/2cdt -s 0 $(BACKGROUND_OBJ) -t 0 -r BACKGND.BIN $@
	tools/2cdt -s 0 build/code.bin -t 0 -r CODE.BIN $@
	tools/2cdt -s 0 build/initialized.bin -t 0 -r INIT.BIN $@

play: build/code.bin $(BACKGROUND_OBJ)
	$(EMULATOR) -enable_webapi -web_port 6128 &
	sleep 2
	python tools/load_to_emulator.py -c $(CODE_LOC) -b $(BACKGROUND_LOC) -d $(INIT_LOC) \
		 --code build/code.bin --background build/background.scr --initialized build/initialized.bin

playdisk: dist/$(PGM_NAME).dsk
	$(EMULATOR) $<

playk7: dist/$(PGM_NAME).cdt
	$(EMULATOR) $<

clean:
	@rm build/*

mrproper: clean
	@rm dist/*
