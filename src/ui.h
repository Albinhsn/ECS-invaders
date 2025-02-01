#ifndef UI_H
#define UI_H

#include "common.h"
#include "vector.h"
#include "math.h"

#define UI_FillHeight()
#define UI_FillWidth()
#define UI_BuildArena UI->BuildArenas[UI->BuildIndex]


#define UI_ChildLayoutAxis(Axis) DeferLoop(UI_PushChildLayoutAxis(Axis), UI_PopChildLayoutAxis())
#define UI_Padding(Padding) DeferLoop(UI_PushPadding(Padding), UI_PopPadding())
#define UI_PrefWidth(Width) DeferLoop(UI_PushPrefWidth(Width), UI_PopPrefWidth())
#define UI_PrefHeight(Height) DeferLoop(UI_PushPrefHeight(Height), UI_PopPrefHeight())
#define UI_FontSize(Size) DeferLoop(UI_PushPrefFontSize(Size), UI_PopFontSize())
#define UI_FontColor(Color)DeferLoop(UI_PushFontColor(Size), UI_PopFontColor())

#define UI_TopParent() 0
#define UI_TopChildLayoutAxis() 0
#define UI_TopTextAlignment() 0
#define UI_TopFont() 0
#define UI_TopFontSize() 0
#define UI_TopFontColor() (vec4f){}

/*
GOALS
* When using the API to build a ui it should:
  * Have simple to use function calls to create a node, this includes
    * Creating "components" like button etc
    * Creating a new subspace that have some properties
      * This includes defining how it's children are supposed to be:
        * Which direction to grow
        * Sized
        * Take up the space
        * Padding/Spacing between them
        * What happens if/when we extend the bounds?
      * As well as for yourself
        * The percentage space of parent you occupy or something else?
    * It should in general feel like creating a </div>
  * It's fine if the flags and widget creation are per project basis
    * This because writing say a "general" native app vs a game has different constraints
    * This does mean that "cores core" should remains throughout projects
* Assumption is made that features such as animating hover is the same across all components?
  * Some can be carried over (such as hover/click etc) but can also just be rewritten per project
*/


typedef struct msdf_font
{
  texture Texture;
  u16     Columns, Rows;
  u16     WidthPerCell;
  u16     HeightPerCell;
  u16     GlyphCount;

} msdf_font;

typedef enum ui_text_alignment
{
  UI_TextAlignment_Centered,
  UI_TextAlignment_StartAt,
  UI_TextAlignment_EndAt,
} ui_text_alignment;


typedef enum ui_box_flags
{
  // Interactions
  UI_BoxFlag_Clickable = (1 << 0),
  UI_BoxFlag_Scroll   =  (1 << 1),
  // Layout
  UI_BoxFlag_FixedWidth=  (1 << 2),
  UI_BoxFlag_FixedHeight=  (1 << 3),
  UI_BoxFlag_AllowOverflow=  (1 << 4), // Use this for highscore just  because?

  // Appearance / Animation
  UI_BoxFlag_HoverAnimation = (1 << 5),
  UI_BoxFlag_DrawText = (1 << 6),
  UI_BoxFlag_DrawBackground= (1 << 7),
  UI_BoxFlag_DrawBorder=  (1 << 8),
}ui_box_flags;

typedef enum axis2
{
  Axis2_X,
  Axis2_Y,
  Axis2_Count
}axis2;

typedef enum ui_size_kind
{
  UI_SizeKind_Null,
  UI_SizeKind_Pixels,
  UI_SizeKind_TextContent,
  UI_SizeKind_PercentOfParent,
  UI_SizeKind_ChildrenSum,
} ui_size_kind;

typedef struct ui_box ui_box;
struct ui_box
{
  // Links
  ui_box * First;
  ui_box * Last;
  ui_box * Next;
  ui_box * Prev;
  ui_box * Parent;
  u64      ChildCount;


  // Per build equipment
  u64 Key;
  ui_box_flags Flags;
  string String;

  vec2f FixedPosition;
  vec2f FixedSize;
  vec2f PrefSize;
  axis2 ChildLayoutAxis;
  // Hoist this to something about text
  msdf_font * Font;
  f32 FontSize;
  vec4f FontColor;
  ui_text_alignment TextAlignment;
  // Hoist this to something about rect
  f32 BorderThickness;
  vec4f BackgroundColor;
  vec4f BorderColor;

  // Per build artifacts
  ui_size_kind SizeKind;
  rect2        Rect; // This defines the space it wants to operate in
  f32          CalcSize[Axis2_Count];

  // Persistent data
  u64 FirstTouchedBuildIndex;
  u64 LastTouchedBuildIndex;
  f32 HotT;
};



typedef enum ui_signal_flags
{
  UI_SignalFlag_LeftPressed =  (1 << 0),
  UI_SignalFlag_LeftClicked =  (1 << 1),
  UI_SignalFlag_Hovered     =  (1 << 2)
}ui_signal_flags;

typedef struct ui_signal
{
  ui_box * Box;
  ui_signal_flags Flags;
}ui_signal;

typedef struct ui_child_layout_node ui_child_layout_node;
struct ui_child_layout_node {ui_child_layout_node * Next; ui_box * Box;};

typedef struct ui_text_alignment_node ui_text_alignment_node;
struct ui_text_alignment_node {ui_text_alignment_node * Next; ui_box * Box;};

typedef struct ui_text_size_node ui_text_size_node;
struct ui_text_size_node {ui_text_size_node * Next; ui_box * Box;};

typedef struct ui_text_color_node ui_text_color_node;
struct ui_text_color_node {ui_text_color_node * Next; ui_box * Box;};

typedef struct ui_border_color_node ui_border_color_node;
struct ui_border_color_node {ui_border_color_node * Next; ui_box * Box;};

typedef struct ui_rect_node ui_rect_node;
struct ui_rect_node {ui_rect_node * Next; ui_box * Box;};

typedef struct os_event os_event;
#define BUILD_ARENA_COUNT 2
typedef struct ui_state
{

  pool_allocator WidgetPool;
  arena BuildArenas[BUILD_ARENA_COUNT];
  u64   BuildIndex;

  // Build output
  ui_box * Root;

  // build parameters
  os_event * Events;
  u32        EventCount;
  vec2f      MousePosition;
  f32        DeltaTime;
  vec2f      WindowDim;

  // Build stacks
  ui_child_layout_node *   ChildLayoutHead;
  ui_text_alignment_node * TextAlignmentHead;
  ui_text_size_node *      TextSizeHead;
  ui_text_color_node    *  TextColorHead;
  ui_border_color_node   * BorderColorHead;

  // Persistent
  ui_box * Persistent;

}ui_state;


#endif
