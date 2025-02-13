
#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H


#include "platform.h"
#include <audioclient.h>
#include <combaseapi.h>
#include <initguid.h>
#include <mmdeviceapi.h>

const CLSID CLSID_MMDeviceEnumerator = {
    0xbcde0395, 0xe52f, 0x467c, {0x8e, 0x3d, 0xc4, 0x57, 0x92, 0x91, 0x69, 0x2e}
};
const IID   IID_IMMDeviceEnumerator   = {
    //MIDL_INTERFACE("A95664D2-9614-4F35-A746-DE8DB63617E6")
    0xa95664d2, 0x9614, 0x4f35, {0xa7, 0x46, 0xde, 0x8d, 0xb6, 0x36, 0x17, 0xe6}
};
const IID   IID_IAudioClient = {
    //MIDL_INTERFACE("1CB9AD4C-DBFA-4c32-B178-C2F568A703B2")
    0x1cb9ad4c, 0xdbfa, 0x4c32, {0xb1, 0x78, 0xc2, 0xf5, 0x68, 0xa7, 0x03, 0xb2}
};
const IID   IID_IAudioRenderClient    = {
    //MIDL_INTERFACE("F294ACFC-3146-4483-A7BF-ADDCA7C260E2")
    0xf294acfc, 0x3146, 0x4483, {0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2}
};
const IID   IID_IAudioSessionControl  = {
    //MIDL_INTERFACE("F4B1A599-7266-4319-A8CA-E70ACB11E8CD")
    0xf4b1a599, 0x7266, 0x4319, {0xa8, 0xca, 0xe7, 0x0a, 0xcb, 0x11, 0xe8, 0xcd}
};
const IID   IID_IAudioSessionEvents   = {
    //MIDL_INTERFACE("24918ACC-64B3-37C1-8CA9-74A66E9957A8")
    0x24918acc, 0x64b3, 0x37c1, {0x8c, 0xa9, 0x74, 0xa6, 0x6e, 0x99, 0x57, 0xa8}
};
const IID IID_IMMEndpoint = {
    //MIDL_INTERFACE("1BE09788-6894-4089-8586-9A2A6C265AC5")
    0x1be09788, 0x6894, 0x4089, {0x85, 0x86, 0x9a, 0x2a, 0x6c, 0x26, 0x5a, 0xc5}
};
const IID IID_IAudioClockAdjustment = {
    //MIDL_INTERFACE("f6e4c0a0-46d9-4fb8-be21-57a3ef2b626c")
    0xf6e4c0a0, 0x46d9, 0x4fb8, {0xbe, 0x21, 0x57, 0xa3, 0xef, 0x2b, 0x62, 0x6c}
};
const IID IID_IAudioCaptureClient = {
    //MIDL_INTERFACE("C8ADBD64-E71E-48a0-A4DE-185C395CD317")
    0xc8adbd64, 0xe71e, 0x48a0, {0xa4, 0xde, 0x18, 0x5c, 0x39, 0x5c, 0xd3, 0x17}
};
const IID IID_ISimpleAudioVolume = {
    //MIDL_INTERFACE("87ce5498-68d6-44e5-9215-6da47ef883d8")
    0x87ce5498, 0x68d6, 0x44e5,{ 0x92, 0x15, 0x6d, 0xa4, 0x7e, 0xf8, 0x83, 0xd8 }
};


typedef struct win32_game_code{
  game_update * GameUpdate;
  game_get_sound_samples * GameGetSoundSamples;
  void * Library;
}win32_game_code;

typedef struct win32_render_code
{
  renderer_begin_frame * BeginFrame;
  renderer_end_frame * EndFrame;
  renderer_create * Create;
  renderer_release * Release;
  void * Library;
} win32_render_code;

typedef struct win32_thread{
    HANDLE Handle;
    DWORD Id;
}win32_thread;

typedef struct win32_audio
{
  IMMDeviceEnumerator* Enumerator;
  IMMDevice*           Device;
  IAudioClient*        AudioClient;
  IAudioRenderClient*          RenderClient;
  WAVEFORMATEXTENSIBLE*        WaveFormat;
  u32                          BufferFrameCount;
  HANDLE                       RefillEvent;
  game_audio                   GameAudio;
  bool CanStartThread;
} win32_audio;

#endif
