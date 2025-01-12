#ifndef UI_H
#define UI_H

#include "common.h"
#include "image.h"

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

struct ui_widget
{
  u32 reserved;
};

struct ui_comm
{
  u32 reserved;
};

#endif
