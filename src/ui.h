#ifndef UI_H
#define UI_H

#include "common.h"
#include "vector.h"

// StartFrame()
// PushLayout()
// PopLayout()
// UI_Button()
// UI_Input()
// EndFrame()

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
  UI_Text_Alignment_Centered,
  UI_Text_Alignment_Start_At,
  UI_Text_Alignment_End_At,
} ui_text_alignment;

typedef enum ui_widget_flags
{
  UI_WidgetFlag_Clickable = (1 << 0),
} ui_widget_flags;

typedef enum ui_size_kind
{
  UI_SizeKind_Null,
  UI_SizeKind_Pixels,
  UI_SizeKind_TextContent,
  UI_SizeKind_PercentOfParent,
  UI_SizeKind_ChildrenSum,
} ui_size_kind;

typedef struct ui_size
{
  ui_size_kind kind;
  f32          Value;
  f32          Strictness;
} ui_size;

enum ui_axis2
{
  Axis2_X,
  Axis2_Y,
  Axis2_Count
};

typedef struct ui_persistent_data
{
  f32    ActiveT;
  f32    HotT;
  string Input;
} ui_persistent_data;

typedef struct ui_widget ui_widget;
struct ui_widget
{
  u64             Key;
  ui_widget*      First;
  ui_widget*      Last;
  ui_widget*      Next;
  ui_widget*      Prev;
  ui_widget*      Parent;
  ui_widget_flags Flags;

  ui_size         SemanticSize[Axis2_Count];

  vec2f           ComputedRelPosition; // Position relative to parent
  vec2f           ComputedSize;        // Position computed in pixels

  string          Text;
  vec2f           Origin, Extents; // The on-screen rectangular coordinates taking Computed values above into account
};

typedef struct ui_comm
{
  bool Clicked;
  bool Hovered;
} ui_comm;

typedef struct ui_layout
{
  vec2f Origin, Extents;
} ui_layout;

typedef struct ui
{
  ui_layout Layouts[16];
  u32       LayoutCount;
} ui;

#endif
