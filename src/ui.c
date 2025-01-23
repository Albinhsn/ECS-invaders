#include "ui.h"

ui_state * UI;

#define UI_Arena (UI->BuildArenas[0])
#define BoxNull 0

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

string UI_GetStringFromKeyString(string String)
{
  bool HasDoubleHashtag = false;
  u32 DoubleHashtagIndex = 0;
  for(u32 CharIdx = 0, HashtagCount = 0; CharIdx < String.Length; CharIdx++)
  {
    HashtagCount += String.Buffer[CharIdx] == '#' ? 1 : -(s32)HashtagCount;
    if(HashtagCount == 2)
    {
      HasDoubleHashtag = true;
      DoubleHashtagIndex = CharIdx;
      break;
    }
  }
  string NewString = {};
  NewString.Buffer = String.Buffer + DoubleHashtagIndex;
  NewString.Length = String.Length - DoubleHashtagIndex;

  return NewString;
}

u64 UI_HashString(string String)
{
  u32 Hash = 2166136261u;
  for (u64 CharIdx = 0; CharIdx < String.Length; CharIdx++)
  {
    Hash ^= String.Buffer[CharIdx];
    Hash *= 16777619;
  }

  return Hash;
}

u64 UI_KeyFromString(string String)
{
  // Anything after a ## is hashed, but not displayed
  // If a ### occurs in the string, then only everyhing after it is hashed
  //    and only anything before it is displayed
  bool HasTripleHashtag = false;
  u32 TripleHashtagIndex = 0;
  for(u32 CharIdx = 0, HashtagCount = 0; CharIdx < String.Length; CharIdx++)
  {
    HashtagCount += String.Buffer[CharIdx] == '#' ? 1 : -(s32)HashtagCount;
    if(HashtagCount == 3)
    {
      HasTripleHashtag = true;
      TripleHashtagIndex = CharIdx;
      break;
    }
  }

  string NewString = {};
  NewString.Buffer = String.Buffer + TripleHashtagIndex;
  NewString.Length = String.Length - TripleHashtagIndex;
  u64 Key = UI_HashString(NewString);

  return Key;
}

ui_box * UI_BoxFromKey()
{
  return 0;
}

ui_box * UI_BoxMake(ui_box_flags Flags, string String)
{
  // Get a new box
  u64 Key = UI_KeyFromString(String);
  ui_box * Box = UI_BoxFromKey(Key);
  if(!Box)
  {
    Box = (ui_box*)Pool_Alloc(&UI->WidgetPool);
    Box->Key = Key;
  }

  ui_box * Parent = UI_TopParent();
  // Insert it into the tree
  // Also clear other links

  // Push the default values on to it
  Box->ChildCount = 0;
  Box->Flags = Flags;
  Box->ChildLayoutAxis = UI_TopChildLayoutAxis();

  if(Box->Flags & UI_BoxFlag_DrawText)
  {
    Box->String = String;
    Box->TextAlignment = UI_TopTextAlignment();
    Box->Font = UI_TopFont();
    Box->FontSize = UI_TopFontSize();
    Box->FontColor = UI_TopFontColor();
  }
  if(Box->Flags & (UI_BoxFlag_DrawBackground | UI_BoxFlag_DrawBorder))
  {

  }
  return Box;
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



  // Create root

  // Prune boxes

  // Check that stacks have been popped!
  // Push defaults

}

void UI_SolveIndependentSizes(ui_box * Box, axis2 Axis)
{
}

void UI_SolveUpwardDependentSizes(ui_box * Box, axis2 Axis)
{
}

void UI_SolveSizeViolations(ui_box * Box, axis2 Axis)
{

  // Here we also calculate the final rect for boxes
}

void UI_LayoutRoot(ui_box * Box, axis2 Axis)
{
  UI_SolveIndependentSizes(Box, Axis);
  UI_SolveUpwardDependentSizes(Box, Axis);
  UI_SolveSizeViolations(Box, Axis);
}

void UI_EndFrame()
{
  // Pop root

  // Calc the sizes for each axis

  // Push draw commands
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