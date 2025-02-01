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
float UI_PushPrefFontSize(float Size){return Size;}
void  UI_PopFontSize(){}
ui_box * UI_PopParent(){return 0;}
ui_box * UI_PushParent(){return 0;}

void UI_Init(void * Memory, u64 MemorySize, u32 MaxWidgetCount)
{
  arena Arena = {};
  Arena_Create(&Arena, Memory, MemorySize);

  // Allocate UI State
  UI = (ui_state*)Arena_Allocate(&Arena, sizeof(ui_state));

  // Allocate Widget Pool
  u64 WidgetSize  = sizeof(Widget);
  void * Memory   = Arena_Allocate(&Arena, WidgetSize * MaxWidgetCount);
  Pool_Create(&UI->WidgetPool, Memory, WidgetSize);

  s64 RemainingMemory = Arena->Size - Arena->Offset;
  Assert(RemainingMemory > 0);

  s64 BuildMemorySize = RemainingMemory / 2;
  Arena_Create(Ui->BuildArenas[0], (u8*)Arena->Memory + Arena->Offset, BuildMemorySize);
  Arena->Offset += BuildMemorySize;
  Arena_Create(Ui->BuildArenas[1], (u8*)Arena->Memory + Arena->Offset, BuildMemorySize);
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

ui_box * UI_BoxFromKey(u64 Key)
{
  ui_box * Head = UI->Persistent;
  while(Head)
  {
    if(Head->Key == Key)
    {
      break;
    }
    Head = Head->Next;
  }
  return Head;
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
  arena * NextArena    = &UI->BuildArenas[(++UI->BuildIndex)  % BUILD_ARENA_COUNT];
  Arena_Clear(NextArena);


  // Create root
  ui_box * Root   = UI_BoxMakeF(0, "Root");
  Root->PrefSize  = V2f(1.0f, 1.0f);
  Root->Rect.Min  = V2f(0, 0);
  Root->Rect.Max  = UI->WindowDim;
  Root->ChildLayoutAxis = Axis2_Y;
  UI_PushParent(Root);

  // Prune boxes
  ui_box * Persistent = UI->Persistent;
  while(Persistent)
  {
    ui_box * Next = Persistent->Next;
    if(Persistent->LastTouchedBuildIndex != UI->BuildIndex)
    {
      // Prune the box
      if(Next)
      {
        Next->Prev             = Persistent->Prev;
      }
      if(Persistent->Prev)
      {
        Persistent->Prev->Next = Next;
      }
      Pool_Free(&UI->WidgetPool, (u64)Persistent);
    }
    Persistent = Next;
  }

  UI->BuildIndex++;
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
  for(axis2 Axis = Axis2_X; Axis < Axis2_Count; Axis++)
  {
    UI_LayoutRoot(UI->Root, Axis);
  }

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
  string String = {};
  String_Build(&UI_Arena, &String, Text);

  ui_box * Box      = UI_BoxMake(UI_BoxFlag_DrawText, String);
  ui_signal Signal  = UI_BoxGetSignal(Box);

  return Signal;
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
