//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
//
//
//=============================================================================
#ifndef __AGS_EE_AC__WALKABLEAREA_H
#define __AGS_EE_AC__WALKABLEAREA_H

void  redo_walkable_areas();
int   get_walkable_area_pixel(int x, int y);
int   get_area_scaling (int onarea, int xx, int yy);
void  scale_sprite_size(int sppic, int zoom_level, int *newwidth, int *newheight);
void  remove_walkable_areas_from_temp(int fromx, int cwidth, int starty, int endy);
int   is_point_in_rect(int x, int y, int left, int top, int right, int bottom);
Common::Bitmap *prepare_walkable_areas (int sourceChar);
int   get_walkable_area_at_location(int xx, int yy);
int   get_walkable_area_at_character (int charnum);

#endif // __AGS_EE_AC__WALKABLEAREA_H
