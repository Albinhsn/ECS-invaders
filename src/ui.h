#ifndef UI_H
#define UI_H

#include "common.h"

typedef struct msdf_font
{
  texture Texture;
  u16    Columns, Rows;
  u16    WidthPerCell;
  u16    HeightPerCell;
  u16    GlyphCount;

} msdf_font;

typedef enum ui_text_alignment
{
  UI_Text_Alignment_Centered,
  UI_Text_Alignment_Start_At,
  UI_Text_Alignment_End_At,
} ui_text_alignment;

enum ui_widget_flags
{
  UI_WidgetFlag_Clickable = (1 << 0),
};

enum UI_SizeKind
{
  UI_SizeKind_Null,
  UI_SizeKind_Pixels,
  UI_SizeKind_TextContent,
  UI_SizeKind_PercentOfParent,
  UI_SizeKind_ChildrenSum,
};

typedef struct UI_Size
{
  UI_SizeKind kind;
  f32 Value;
  f32 Strictness;
} UI_Size;

enum UI_Axis2
{
  Axis2_X,
  Axis2_Y,
  Axis2_Count
};

bool UI_Button(const char * Text);
void UI_PushParent(ui_widget * Widget);
void UI_PopParent(ui_widget * Widget);

typedef struct ui_persistent_data
{
  f32    active_t;
  f32    hot_t;
  string Input;
} ui_persistent_data;

typedef struct ui_widget
{
  u64         Key;
  ui_widget * First;
  ui_widget * Last;
  ui_widget * Next;
  ui_widget * Prev;
  ui_widget * Parent;
  ui_widget_flags Flags;

  ui_size SemanticSize[Axis2_Count];

  vec2f ComputedRelPosition; // Position relative to parent
  vec2f ComputedSize;        // Position computed in pixels


  vec2f Origin, Extents;     // THe on-screen rectangular coordinates taking Computed values above into account
} ui_widget;

typedef struct ui_comm
{
  bool Clicked;
  bool Hovered;
} ui_comm;

#endif
