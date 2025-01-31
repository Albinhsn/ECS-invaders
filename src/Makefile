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

f:
	make renderer && make platform && make game && make run

