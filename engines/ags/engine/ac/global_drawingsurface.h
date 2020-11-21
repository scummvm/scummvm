/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_ENGINE_AC_GLOBALDRAWINGSURFACE_H
#define AGS_ENGINE_AC_GLOBALDRAWINGSURFACE_H

namespace AGS3 {

void RawSaveScreen();
// RawRestoreScreen: copy backup bitmap back to screen; we
// deliberately don't free the Common::Bitmap *cos they can multiple restore
// and it gets freed on room exit anyway
void RawRestoreScreen();
// Restores the backup bitmap, but tints it to the specified level
void RawRestoreScreenTinted(int red, int green, int blue, int opacity);
void RawDrawFrameTransparent(int frame, int translev);
void RawClear(int clr);
void RawSetColor(int clr);
void RawSetColorRGB(int red, int grn, int blu);
void RawPrint(int xx, int yy, const char *text);
void RawPrintMessageWrapped(int xx, int yy, int wid, int font, int msgm);
void RawDrawImageCore(int xx, int yy, int slot, int alpha = 0xFF);
void RawDrawImage(int xx, int yy, int slot);
void RawDrawImageOffset(int xx, int yy, int slot);
void RawDrawImageTransparent(int xx, int yy, int slot, int opacity);
void RawDrawImageResized(int xx, int yy, int gotSlot, int width, int height);
void RawDrawLine(int fromx, int fromy, int tox, int toy);
void RawDrawCircle(int xx, int yy, int rad);
void RawDrawRectangle(int x1, int y1, int x2, int y2);
void RawDrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3);

} // namespace AGS3

#endif
