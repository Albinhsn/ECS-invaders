#ifndef LINUX_PLATFORM_H
#define LINUX_PLATFORM_H

#include <X11/Xlib.h>
#include <alsa/asoundlib.h>
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

typedef struct linux_audio
{
  snd_pcm_t * Handle;
  game_audio GameAudio;
  snd_pcm_uframes_t BufferFrameCount;
  snd_pcm_uframes_t PeriodSize;
  u32 Channels;
  bool CanStart;
  pthread_mutex_t Mutex;
  pthread_cond_t Condition;
}linux_audio;

typedef struct linux_display_and_window
{
  Display * display;
  Window window;
  s32 Screen;
}linux_display_and_window;

typedef struct linux_game_code
{
  void * Library;
  game_update* GameUpdate;
  game_get_sound_samples * GameGetSoundSamples;
} linux_game_code;

typedef struct linux_render_code
{
  void * Library;
  renderer_begin_frame* BeginFrame;
  renderer_end_frame* EndFrame;
  renderer_create* Create;
  renderer_release* Release;
} linux_render_code;

#endif
