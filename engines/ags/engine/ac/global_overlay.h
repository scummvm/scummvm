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
#ifndef __AGS_EE_AC__GLOBALOVERLAY_H
#define __AGS_EE_AC__GLOBALOVERLAY_H

void RemoveOverlay(int ovrid);
int  CreateGraphicOverlay(int xx, int yy, int slott, int trans);
int  CreateTextOverlayCore(int xx, int yy, int wii, int fontid, int text_color, const char *text, int disp_type, int allowShrink);
int  CreateTextOverlay(int xx, int yy, int wii, int fontid, int clr, const char* text, int disp_type);
void SetTextOverlay(int ovrid, int xx, int yy, int wii, int fontid, int text_color, const char *text);
void MoveOverlay(int ovrid, int newx, int newy);
int  IsOverlayValid(int ovrid);

#endif // __AGS_EE_AC__GLOBALOVERLAY_H
