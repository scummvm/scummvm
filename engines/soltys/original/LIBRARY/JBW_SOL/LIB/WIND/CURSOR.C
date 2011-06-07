#include	<wind.h>







void CursorGoto (Wind *W, int x, int y)
{
  W->Cursor.X = x; W->Cursor.Y = y;
}