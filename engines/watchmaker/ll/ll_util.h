/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WATCHMAKER_LL_UTIL_H
#define WATCHMAKER_LL_UTIL_H

#include "watchmaker/game.h"
#include "watchmaker/globvar.h"
#include "watchmaker/t3d.h"
#include "watchmaker/work_dirs.h"
#include "watchmaker/renderer.h"

namespace Watchmaker {

int32 LoadDDBitmap(WGame &game, const char *n, uint8 flags);
void LoadFont(struct SFont *f, const char *n);
void UpdateRoomVisibility(Init &init);
t3dMESH *LinkMeshToStr(Init &init, const Common::String &str);
int16 getRoomFromStr(Init &init, const Common::String &s);
void ChangeRoom(WGame &game, Common::String n, uint8 pos, int32 an);
bool SetBndLevel(Init &init, const char *roomname, int32 lev);
int32 GetBndLevel(char *roomname);
void GetDDBitmapExtends(struct SRect *r, struct SDDBitmap *b);
void DisplayDDBitmap(Renderer &, int32 tnum, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy);
void DisplayDDBitmap_NoFit(int32 tnum, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy);
void UpdateRoomInfo(Init &init);
bool CheckRect(Renderer &renderer, struct SRect p, int32 cmx, int32 cmy);
void DisplayD3DTriangle(Renderer &, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, uint8 r, uint8 g, uint8 b, uint8 al);
void DisplayD3DRect(Renderer &, int32 px, int32 py, int32 dx, int32 dy, uint8 r, uint8 g, uint8 b, uint8 al);
void DisplayDDText(Renderer &, char *text, struct SFont *f, uint8 color, int32 px, int32 py, int32 ox, int32 oy, int32 dx, int32 dy);
int32 CreateTooltipBitmap(Renderer &renderer, char *tooltip, struct SFont *f, uint8 color, uint8 r, uint8 g, uint8 b);
int32 WhatObj(WGame &game, int32 mx, int32 my, uint8 op);
void DebugVideo(Renderer &renderer, int32 px, int32 py, const char *format, ...);

} // End of namespace Watchmaker

#endif // WATCHMAKER_LL_UTIL_H
