#ifndef LINUX_RENDERER_H
#define LINUX_RENDERER_H

#include "renderer_software.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <x86intrin.h>
#include "platform.h"



typedef struct linux_renderer_software
{
  platform_renderer Header;
  Display * display;
  Window window;
  GC Context;
  Pixmap Backbuffer;
  XImage * Image;
  u32 Width, Height;
  s32 Screen;
} linux_renderer_software;


#endif
