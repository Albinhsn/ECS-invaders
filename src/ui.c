#include "ui.h"

ui_state * UI;

#define UI_Arena (UI->BuildArenas[0])

axis2 UI_PushChildLayoutAxis(axis2 Axis){return Axis;}
void  UI_PopChildLayoutAxis(){}
float UI_PushPadding(float Padding){return Padding;}
void  UI_PopPadding(){}
float UI_PushPrefWidth(float Width){return Width;}
void  UI_PopPrefWidth(){}
float UI_PushPrefHeight(float Height){return Height;}
void  UI_PopPrefHeight(){}
ui_box * UI_PushParent(){return 0;}
ui_box * UI_PopParent(){return 0;}

void UI_Init(void * Memory, u64 MemorySize, u32 MaxWidgetCount)
{
  arena Arena = {};
  Arena_Create(&Arena, Memory, MemorySize);

  // Allocate UI State
  UI = (ui_state*)Arena_Allocate(&Arena, sizeof(ui_state));

  // Allocate Widget Pool
  // Allocate build arenas

}

u64 UI_KeyFromString(string String)
{
  // ToDo Keep in mind this have to check for hash!
  return 0;
}

ui_box * UI_BoxMake(ui_box_flags Flags, string String)
{
  return 0;
}

ui_box * UI_BoxMakeF(ui_box_flags Flags, const char * fmt, ...)
{
  return 0;
}

void UI_BeginFrame(os_event * Events, u32 EventCount, f32 DeltaTime)
{
  UI->DeltaTime     = DeltaTime;
  UI->Events        = Events;
  UI->EventCount   = EventCount;
  // Reset and swap arenas

  // Check that stacks have been popped!
}

void UI_EndFrame()
{
}


ui_signal UI_BoxGetSignal(ui_box * Box)
{
  return (ui_signal){};
}

ui_signal UI_Spacer(float Space)
{
  return (ui_signal){};
}

ui_signal UI_Text(const char * Text)
{
  return (ui_signal){};
}

ui_signal UI_Button(const char * Text)
{
  string String = {};
  String_Build(&UI_Arena, &String, Text);

  ui_box * Box = UI_BoxMake(UI_BoxFlag_Clickable     |
                               UI_BoxFlag_DrawText      |
                               UI_BoxFlag_DrawBorder    |
                               UI_BoxFlag_DrawBackground|
                               UI_BoxFlag_HoverAnimation,
                               String
      );
  ui_signal Signal = UI_BoxGetSignal(Box);

  return Signal;
}
// ToDo implement!!!
ui_signal UI_ButtonF(const char * Text, ...)
{
  string String = {};
  String_Build(&UI_Arena, &String, Text);

  ui_box * Box = UI_BoxMake(UI_BoxFlag_Clickable     |
                               UI_BoxFlag_DrawText      |
                               UI_BoxFlag_DrawBorder    |
                               UI_BoxFlag_DrawBackground|
                               UI_BoxFlag_HoverAnimation,
                               String
      );
  ui_signal Signal = UI_BoxGetSignal(Box);

  return Signal;
}


void UI_PushRect(rect2 Rect)
{
}
void UI_PopRect()
{
}