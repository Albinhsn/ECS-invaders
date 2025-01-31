PLATFORM := PLATFORM_WEB
RENDERER := RENDERER_SOFTWARE

ifeq ($(RENDERER), RENDERER_SOFTWARE)
	RENDERER_LOCATION := ./src/emscripten_renderer_software.c
	RENDERER_WASM := ./build/emcc_renderer_software.wasm
else
	RENDERER_LOCATION := ./src/emscripten_renderer_gl.c
	RENDERER_WASM := ./build/emcc_renderer_gl.wasm
endif

web:
	emcc $(RENDERER_LOCATION) -D$(PLATFORM) -o $(RENDERER_WASM) -g -O0 -sSHARED_MEMORY=1 -sSIDE_MODULE=2 && emcc ./src/invaders.c -o ./build/invaders.wasm -sSHARED_MEMORY=1 -D$(PLATFORM) -g -O0 -sSIDE_MODULE=2 && emcc ./src/emscripten_platform.c -g -sSHARED_MEMORY=1  -sMAIN_MODULE=2 -O0 -sUSE_WEBGL2=1 -sFULL_ES2=1 -o ./build/emscripten_platform.html -D$(RENDERER)=1 -D$(PLATFORM)=1 --shell-file ./src/shell.html --preload-file ./assets --preload-file ./build/invaders.wasm --preload-file $(RENDERER_WASM)
rw:
	emrun ./build/emscripten_platform.html

