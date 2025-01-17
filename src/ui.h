#ifndef UI_H
#define UI_H

#include "common.h"
#include "math.h"
#include "vector.h"

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

typedef enum ui_widget_flags
{
  UI_WidgetFlag_Clickable      = (1 << 0),
  UI_WidgetFlag_AnimateHover   = (1 << 1),
  UI_WidgetFlag_AnimateClick   = (1 << 1),
  UI_WidgetFlag_Input          = (1 << 2),
  UI_WidgetFlag_DrawBackground = (1 << 3),
  UI_WidgetFlag_DrawText       = (1 << 4)
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

typedef enum ui_axis2
{
  Axis2_X,
  Axis2_Y,
  Axis2_Count
} ui_axis2;

typedef struct ui_widget ui_widget;
struct ui_widget
{
  // links
  u64        Key;
  ui_widget* First;
  ui_widget* Last;
  ui_widget* Next;
  ui_widget* Prev;
  ui_widget* Parent;

  // per build data
  // On Creation data
  string            String;   // This is fine to be temporary, or not, this might be persitent?
  f32               FontSize; // Described in units 0-100?
  msdf_font*        Font;
  ui_axis2          ChildLayoutAxis;
  ui_widget_flags   Flags;
  ui_text_alignment TextAlignment;
  float             Padding;
  rect2             FixedRect; // The initial position of the widget

  // Dependent on Parent
  vec2f ComputedRelPosition; // Position relative to parent

  // Final position
  rect2 FinalRect;      // The on-screen rectangular coordinates taking Computed values above into account
  f32   FontSizeScreen; // FontSize on the screen

  // persistent data
  // This includes String for input
  u64 LastFrameActive;
  f32 ActiveT;
};

typedef struct ui_comm
{
  bool Released;
  bool Pressed;
  bool Hovered;
} ui_comm;

typedef struct ui_widget_pool
{
  ui_widget* Head;
} ui_widget_pool;

typedef struct input_event input_event;

// Have a stack of each type of thing you want to push
// TextAlign
// ChildLayoutAxis
// Font
// FontSize

typedef struct ui_state
{
  arena          BuildArena;
  arena          PrevFrameArena;

  u64            Frame;
  ui_widget*     TopOfHierarchy; // This is the top of the hierarchy
  ui_widget*     PersistentData;
  ui_widget*     Parent; // Parents

  vec2f          MousePosition;
  input_event*   Input;
  u32            InputCount;

  pool_allocator WidgetPool;

  ui_widget*     WidgetSizeHead;
  ui_widget*     TextAlignHead;
  ui_widget*     ChildLayoutAxisHead;
  ui_widget*     FontHead;
  ui_widget*     FontSizeHead;
} ui_state;

#endif
