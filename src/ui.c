#include "ui.h"
#include "platform.h"

ui_state* UI;

u64       UI_KeyFromString(string String)
{
  u64 Key = 2166136261u;
  for (u32 CharIdx = 0; CharIdx < String.Length; CharIdx++)
  {
    Key ^= (u8)String.Buffer[CharIdx];
    Key *= 16777619;
  }
  return Key;
}

void UI_Init(ui_state* UI_, void* Memory, u32 MemorySize, u32 WidgetCount)
{

  u32 WidgetSize = sizeof(ui_widget);
  UI             = UI_;
  // Allocate widget pool
  Pool_Create(&UI->WidgetPool, Memory, WidgetSize);
  u32 WidgetOffset          = UI->WidgetPool.ChunkSize * WidgetCount;

  u64 ArenaSize             = (MemorySize - WidgetOffset) / 2;
  UI->BuildArena.Memory     = (u8*)Memory + WidgetOffset;
  UI->BuildArena.Size       = ArenaSize;
  UI->BuildArena.Offset     = 0;
  UI->PrevFrameArena.Memory = (u8*)Memory + WidgetOffset + ArenaSize;
  UI->PrevFrameArena.Size   = ArenaSize;
  UI->PrevFrameArena.Offset = 0;
  UI->Frame                 = 0;
}

void UI_SetPersitent(ui_widget* Data)
{
  if (UI->PersistentData != 0)
  {
    UI->PersistentData->Next = Data;
  }
  Data->Prev         = UI->PersistentData;
  UI->PersistentData = Data;
}

ui_widget* UI_GetPersitent(u64 Key)
{
  ui_widget* Out  = 0;
  ui_widget* Head = UI->PersistentData;
  while (Head)
  {
    if (Head->Key == Key)
    {
      Head->Next->Prev = Head->Prev;
      Head->Prev->Next = Head->Next;
      Out              = Head;
      break;
    }

    Head = Head->Next;
  }
  return Out;
}

ui_widget* UI_WidgetCreate()
{
  ui_widget* Widget = (ui_widget*)Pool_Alloc(&UI->WidgetPool);
  return Widget;
}

void UI_WidgetFree(ui_widget* Widget)
{
}

bool UI_Hovering(rect2 Rect)
{
  return Rect_PointInside(Rect, UI->MousePosition);
}

bool UI_Pressing()
{
  for (u32 InputEventIndex = 0; InputEventIndex < UI->InputCount; InputEventIndex++)
  {
    input_event* Event = UI->Input + InputEventIndex;
    if (Event->Key == MOUSE_BUTTON_LEFT && Event->IsDown == true)
    {
      return true;
    }
  }
  return false;
}

bool UI_Clicked()
{
  for (u32 InputEventIndex = 0; InputEventIndex < UI->InputCount; InputEventIndex++)
  {
    input_event* Event = UI->Input + InputEventIndex;
    if (Event->Key == MOUSE_BUTTON_LEFT && Event->WasDown == true)
    {
      return true;
    }
  }
  return false;
}

void UI_StartFrame(game_input* Input)
{
  // Get Mouse position
  UI->Input         = Input->Input;
  UI->InputCount    = Input->InputCount;
  UI->MousePosition = Input->MousePosition;

  UI->Frame++;

  // Make sure every stack is empty
  // Create a default first parent for the screen
  Assert(UI->WidgetSizeHead);
  Assert(UI->TextAlignHead);
  Assert(UI->ChildLayoutAxisHead);
  Assert(UI->FontHead);
  Assert(UI->FontSizeHead);
}

void UI_EndFrame(pushbuffer* Pushbuffer)
{
  // Create final layout data

  // Render using the new layout data
  //  At the same time we can free the widget at the end of its use
  //  Since everything has been calculated

  // Check cache for evictions i.e don't copy?
  //  iterate over persitent data
  //    check active frame vs frame
  //    if active < frame
  //      evict

  // Reset and swap arenas
}

void UI_PushParent(ui_widget* Widget)
{

  UI->Parent->Next = Widget;
  Widget->Prev     = UI->Parent;
  UI->Parent       = Widget;
}

void UI_PopParent()
{
  ui_widget* Out  = UI->Parent;
  Out->Prev->Next = 0;
  UI->Parent      = Out->Prev;

  Pool_Free(&UI->WidgetPool, (u64)Out);
}

ui_widget* UI_GetParent()
{
  return UI->Parent;
}

ui_widget* UI_BuildWidgetFromKey(ui_widget_flags Flags, u64 Key)
{
  return 0;
}

ui_widget* UI_BuildWidgetFromString(ui_widget_flags Flags, string String)
{
  return 0;
}

ui_comm UI_GetCommFromWidget(ui_widget* Widget)
{
  ui_comm Comm          = {};
  rect2   FinalPosition = Widget->FinalRect;
  Comm.Hovered          = UI_Hovering(FinalPosition);
  Comm.Released         = Comm.Hovered && UI_Clicked();
  Comm.Pressed          = Comm.Hovered && UI_Pressing();
  return Comm;
}

ui_comm UI_Button(const char* Text)
{
  ui_comm Comm   = {};

  string  String = {};
  String.Length  = String_Length((u8*)Text);
  String.Buffer  = (u8*)Arena_Allocate(&UI->BuildArena, String.Length * sizeof(u8));
  Memcpy(String.Buffer, (u8*)Text, String.Length);

  u64        Key    = UI_KeyFromString(String);
  ui_widget* Widget = UI_GetPersitent(Key);
  if (Widget == 0)
  {
    // Use the previous frames data  (if it exists)
    ui_widget* Widget = UI_WidgetCreate(); // Change this to different call!
    Widget->Key       = Key;
    Widget->String    = String;
    Widget->Font      = UI->FontHead->Font;
    Widget->Flags     = UI_WidgetFlag_Clickable | UI_WidgetFlag_AnimateClick | UI_WidgetFlag_AnimateHover | UI_WidgetFlag_DrawBackground;
    Widget->Parent    = UI->Parent;
    Widget->FontSize  = UI->FontSizeHead->FontSize;
    Widget->FixedRect = UI->WidgetSizeHead->FixedRect;

    // Check whether we intersected this frame with
    Comm = UI_GetCommFromWidget(Widget);
  }

  // Add new time etc to animations

  // Add this to the hierarchy

  ui_widget* Parent = UI_GetParent();
  if (Parent->Last != 0)
  {
    Parent->Last->Next = Widget;
    Widget->Prev       = Parent->Last;
  }
  Parent->Last   = Widget;
  Widget->Parent = Parent;

  return Comm;
}

ui_comm UI_InputField()
{
  ui_comm Comm = {};

  // Get (if it exist) the persistent data for this
  //
  // Allocate a new string in the next arena
  // Copy it over
  // Look at events to figure out if we add anything to the string
  return Comm;
}
