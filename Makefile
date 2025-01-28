CC := gcc
CFLAGS := -O0 -g 
LDFLAGS := -lm -lX11
VARIABLES := -DPLATFORM_LINUX=1

platform:
	$(CC) $(CFLAGS) $(VARIABLES) -o ./build/linux_platform ./src/linux_platform.c $(LDFLAGS)

game:
	$(CC) $(CFLAGS) $(VARIABLES) -c -fpic ./src/invaders.c -o ./build/invaders.o $(LDFLAGS) && gcc -shared ./build/invaders.o -o ./build/invaders.so.tmp && mv ./build/invaders.so.tmp ./build/invaders.so


