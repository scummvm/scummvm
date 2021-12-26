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

#ifndef AGS_ENGINE_AC_GLOBAL_OVERLAY_H
#define AGS_ENGINE_AC_GLOBAL_OVERLAY_H

namespace AGS3 {

void RemoveOverlay(int ovrid);
int  CreateGraphicOverlay(int xx, int yy, int slott, int trans);
int  CreateTextOverlayCore(int xx, int yy, int wii, int fontid, int text_color, const char *text, int disp_type, int allowShrink);
int  CreateTextOverlay(int xx, int yy, int wii, int fontid, int clr, const char *text, int disp_type);
void SetTextOverlay(int ovrid, int xx, int yy, int wii, int fontid, int text_color, const char *text);
void MoveOverlay(int ovrid, int newx, int newy);
int  IsOverlayValid(int ovrid);

} // namespace AGS3

#endif
