CC := gcc
CFLAGS := -O2 -g 
LDFLAGS := -lm -lX11 -lasound
VARIABLES := -DPLATFORM_LINUX=1 -DRENDERER_SOFTWARE=1

platform:
	$(CC) $(CFLAGS) $(VARIABLES) -o ../build/linux_platform linux_platform.c $(LDFLAGS)

renderer:
	$(CC) $(CFLAGS) $(VARIABLES) -c -fpic linux_renderer_software.c -o ../build/renderer.o $(LDFLAGS) && gcc -shared ../build/renderer.o -o ../build/linux_renderer_software.so.tmp $(LDFLAGS) && mv ../build/linux_renderer_software.so.tmp ../build/linux_renderer_software.so

game:
	$(CC) $(CFLAGS) $(VARIABLES) -c -fpic invaders.c -o ../build/invaders.o $(LDFLAGS) && gcc -shared ../build/invaders.o -o ../build/invaders_temp.so $(LDFLAGS) && mv ../build/invaders_temp.so ../build/invaders.so

run:
	../build/linux_platform

web:
	emcc ../src/emscripten_platform.c -sMAIN_MODULE=1 -O0 -pthread -s USE_WEBGL2=1 -s FULL_ES2=1 -s WASM=1 -o ../build/emscripten_platform.html -DPLATFORM_WEB=1 --shell-file ../src/shell.html --preload-file ../assets --preload-file ../build && emcc ../src/invaders.c -O3 -o ../build/invaders.wasm -sSIDE_MODULE=2 -DPLATFORM_WEB=1 -s EXPORT_ALL=1

rw:
	emrun ../build/emscripten_platform.html

f:
	make renderer && make platform && make game && make run

