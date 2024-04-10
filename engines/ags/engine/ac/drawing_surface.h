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

#ifndef AGS_ENGINE_AC_DRAWING_SURFACE_H
#define AGS_ENGINE_AC_DRAWING_SURFACE_H

#include "ags/engine/ac/dynobj/script_drawing_surface.h"

namespace AGS3 {

void    DrawingSurface_Release(ScriptDrawingSurface *sds);
// convert actual co-ordinate back to what the script is expecting
ScriptDrawingSurface *DrawingSurface_CreateCopy(ScriptDrawingSurface *sds);
void    DrawingSurface_DrawSurface(ScriptDrawingSurface *target, ScriptDrawingSurface *source, int trans,
								   int dst_x, int dst_y, int dst_width, int dst_height,
								   int src_x, int src_y, int src_width, int src_height);
void    DrawingSurface_DrawSurface2(ScriptDrawingSurface *target, ScriptDrawingSurface *source, int trans);
void    DrawingSurface_DrawImage6(ScriptDrawingSurface *sds, int xx, int yy, int slot, int trans, int width, int height);
void    DrawingSurface_SetDrawingColor(ScriptDrawingSurface *sds, int newColour);
int     DrawingSurface_GetDrawingColor(ScriptDrawingSurface *sds);
void    DrawingSurface_SetUseHighResCoordinates(ScriptDrawingSurface *sds, int highRes);
int     DrawingSurface_GetUseHighResCoordinates(ScriptDrawingSurface *sds);
int     DrawingSurface_GetHeight(ScriptDrawingSurface *sds);
int     DrawingSurface_GetWidth(ScriptDrawingSurface *sds);
void    DrawingSurface_Clear(ScriptDrawingSurface *sds, int colour);
void    DrawingSurface_DrawCircle(ScriptDrawingSurface *sds, int x, int y, int radius);
void    DrawingSurface_DrawRectangle(ScriptDrawingSurface *sds, int x1, int y1, int x2, int y2);
void    DrawingSurface_DrawTriangle(ScriptDrawingSurface *sds, int x1, int y1, int x2, int y2, int x3, int y3);
void    DrawingSurface_DrawString(ScriptDrawingSurface *sds, int xx, int yy, int font, const char *text);
void    DrawingSurface_DrawStringWrapped(ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int alignment, const char *msg);
void    DrawingSurface_DrawStringWrapped_Old(ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int alignment, const char *msg);
void    DrawingSurface_DrawMessageWrapped(ScriptDrawingSurface *sds, int xx, int yy, int wid, int font, int msgm);
void    DrawingSurface_DrawLine(ScriptDrawingSurface *sds, int fromx, int fromy, int tox, int toy, int thickness);
void    DrawingSurface_DrawPixel(ScriptDrawingSurface *sds, int x, int y);
int     DrawingSurface_GetPixel(ScriptDrawingSurface *sds, int x, int y);

} // namespace AGS3

#endif
