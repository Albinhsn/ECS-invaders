#include "ui.h"

ui_state * UI;

#define UI_Arena (UI->BuildArenas[0])
#define BoxIsNull(Box) ((Box) == 0)

#define UI_PushValue(value, type)\
	ui_box * Box = (ui_box*)Arena_Allocate(&UI_Arena, sizeof(ui_box));\
	Box->##type## = value;\
	if(BoxIsNull(UI->##type##Head))\
	{\
		UI->##type##Head = Box;\
	}else\
	{\
		UI->##type##Head->Prev = Box;\
		Box->Next = UI->##type##Head;\
		UI->##type##Head = Box;\
	}\
	return value;\

#define UI_PopValue(type)\
	ui_box * Out = UI->##type##Head;\
	UI->##type##Head = Out->Next;\
	if(Out->Next){\
		UI->##type##Head->Prev =0;\
	}\

// ToDo implement these
axis2 UI_PushChildLayoutAxis(axis2 Axis){ UI_PushValue(Axis, ChildLayoutAxis) }
void  UI_PopChildLayoutAxis(){UI_PopValue(ChildLayoutAxis)}
float UI_PushPadding(float Value){ UI_PushValue(Value, Padding) }
void  UI_PopPadding(){UI_PopValue(Padding)}
vec2f UI_PushPrefSize(vec2f Size){UI_PushValue(Size, PrefSize)}
void  UI_PopPrefSize(){UI_PopValue(PrefSize)}
float UI_PushPrefFontSize(float Size){UI_PushValue(Size, FontSize)}
void  UI_PopFontSize(){UI_PopValue(FontSize)}
ui_box * UI_PushParent(ui_box * P){UI_PushValue(P, Parent)}
void UI_PopParent(){UI_PopValue(Parent)}

msdf_font* UI_PushFont(msdf_font* F_){UI_PushValue(F_, Font)}
void UI_PopFont(){UI_PopValue(Font)}

void UI_Init(arena * Arena, pushbuffer * Pushbuffer, u32 MaxWidgetCount)
{

  // Allocate UI State
  UI = (ui_state*)Arena_Allocate(Arena, sizeof(ui_state));
	UI->Pushbuffer = Pushbuffer;

  // Allocate Widget Pool
  u64 WidgetSize  = sizeof(ui_box);
  void * UIMemory   = Arena_Allocate(Arena, WidgetSize * MaxWidgetCount);
  Pool_Create(&UI->WidgetPool, UIMemory, WidgetSize, MaxWidgetCount);


	u64 BuildMemorySize = Megabyte(1);
	void * BuildMemory = Arena_Allocate(Arena, BuildMemorySize);
  Arena_Create(&UI->BuildArenas[0], BuildMemory, BuildMemorySize);
	BuildMemory = Arena_Allocate(Arena, BuildMemorySize);
  Arena_Create(&UI->BuildArenas[1], BuildMemory, BuildMemorySize);
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
	if(Parent == 0)
	{
		UI->Root = Parent;
	}else
	{
		if(Parent->First == 0)
		{
			Parent->First = Parent->Last = Box;
			Box->Next = Box->Prev = 0;
		}else
		{
			Box->Prev = Parent->Last;
			Parent->Last->Next = Box;
		}
	}
	

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
		Box->BackgroundColor 	= UI_TopBackgroundColor();
		Box->BorderColor 			= UI_TopBorderColor();
		Box->BorderThickness  = UI_TopBorderThickness();
  }

  return Box;
}

ui_box * UI_BoxMakeF(ui_box_flags Flags, const char * String, ...)
{
	string FormattedString = {};
	va_list Args;
	va_start(Args,String); 
	String_Format(&UI_BuildArena, &FormattedString, String, Args);

  return UI_BoxMake(Flags, FormattedString);
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
	UI->Root = Root;

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

  UI_PushParent(Root);
	UI_PushFont(UI->Font);
	UI_PushPadding(0);
	UI_PushPrefSize(V2f(1.0f, 1.0f));
	UI_PushPrefFontSize(40);
	UI_PushChildLayoutAxis(Axis2_Y);
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

void UI_RenderBox(ui_box * Box)
{
	pushbuffer * Pushbuffer = UI->Pushbuffer;
	while(Box)
	{
		if(Box->Flags & UI_BoxFlag_DrawText)
		{
			Pushbuffer_PushText(Pushbuffer, &Box->String, Box->Font,
												  Box->TextAlignment, Box->FixedPosition,
											 		(u32)Box->FontSize, Color_vec4fToU32(Box->FontColor));
		}

		ui_box * Child = Box->First;
		while(Child != Box->Last)
		{
			UI_RenderBox(Child);
			Child = Child->Next;
		}
		Box = Box->Next;
	}
}

void UI_EndFrame()
{

  // Calc the sizes for each axis
  for(axis2 Axis = Axis2_X; Axis < Axis2_Count; Axis++)
  {
    UI_LayoutRoot(UI->Root, Axis);
  }

  // Push draw commands
  ui_box * Box = UI->Root;
	UI_RenderBox(Box);


	UI->Root = 0;

	// ToDo Pop defaults
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
