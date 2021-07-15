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

#include "ags/plugins/core/drawing_surface.h"
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/engine/ac/drawing_surface.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void DrawingSurface::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(DrawingSurface::Clear^1, DrawingSurface_Clear);
	SCRIPT_METHOD_EXT(DrawingSurface::CreateCopy^0, DrawingSurface_CreateCopy);
	SCRIPT_METHOD_EXT(DrawingSurface::DrawCircle^3, DrawingSurface_DrawCircle);
	SCRIPT_METHOD_EXT(DrawingSurface::DrawImage^6, DrawingSurface_DrawImage);
	SCRIPT_METHOD_EXT(DrawingSurface::DrawLine^5, DrawingSurface_DrawLine);
	SCRIPT_METHOD_EXT(DrawingSurface::DrawMessageWrapped^5, DrawingSurface_DrawMessageWrapped);
	SCRIPT_METHOD_EXT(DrawingSurface::DrawPixel^2, DrawingSurface_DrawPixel);
	SCRIPT_METHOD_EXT(DrawingSurface::DrawRectangle^4, DrawingSurface_DrawRectangle);
	SCRIPT_METHOD_EXT(DrawingSurface::DrawString^104, ScPl_DrawingSurface_DrawString);

	if (engine->version < kScriptAPI_v350)
		SCRIPT_METHOD_EXT(DrawingSurface::DrawStringWrapped^6, DrawingSurface_DrawStringWrapped_Old);
	else
		SCRIPT_METHOD_EXT(DrawingSurface::DrawStringWrapped^6, DrawingSurface_DrawStringWrapped);

	SCRIPT_METHOD_EXT(DrawingSurface::DrawSurface^2, DrawingSurface_DrawSurface);
	SCRIPT_METHOD_EXT(DrawingSurface::DrawTriangle^6, DrawingSurface_DrawTriangle);
	SCRIPT_METHOD_EXT(DrawingSurface::GetPixel^2, DrawingSurface_GetPixel);
	SCRIPT_METHOD_EXT(DrawingSurface::Release^0, DrawingSurface_Release);
	SCRIPT_METHOD_EXT(DrawingSurface::get_DrawingColor, DrawingSurface_GetDrawingColor);
	SCRIPT_METHOD_EXT(DrawingSurface::set_DrawingColor, DrawingSurface_SetDrawingColor);
	SCRIPT_METHOD_EXT(DrawingSurface::get_Height, DrawingSurface_GetHeight);
	SCRIPT_METHOD_EXT(DrawingSurface::get_UseHighResCoordinates, DrawingSurface_GetUseHighResCoordinates);
	SCRIPT_METHOD_EXT(DrawingSurface::set_UseHighResCoordinates, DrawingSurface_SetUseHighResCoordinates);
	SCRIPT_METHOD_EXT(DrawingSurface::get_Width, DrawingSurface_GetWidth);
}

void DrawingSurface::DrawingSurface_Clear(ScriptMethodParams &params) {
	PARAMS2(ScriptDrawingSurface *, sds, int, colour);
	AGS3::DrawingSurface_Clear(sds, colour);
}

void DrawingSurface::DrawingSurface_CreateCopy(ScriptMethodParams &params) {
	PARAMS1(ScriptDrawingSurface *, sds);
	params._result = AGS3::DrawingSurface_CreateCopy(sds);
}

void DrawingSurface::DrawingSurface_DrawCircle(ScriptMethodParams &params) {
	PARAMS4(ScriptDrawingSurface *, sds, int, x, int, y, int, radius);
	AGS3::DrawingSurface_DrawCircle(sds, x, y, radius);
}

void DrawingSurface::DrawingSurface_DrawImage(ScriptMethodParams &params) {
	PARAMS7(ScriptDrawingSurface *, sds, int, xx, int, yy, int, slot, int, trans, int, width, int, height);
	AGS3::DrawingSurface_DrawImage(sds, xx, yy, slot, trans, width, height);
}

void DrawingSurface::DrawingSurface_DrawLine(ScriptMethodParams &params) {
	PARAMS6(ScriptDrawingSurface *, sds, int, fromx, int, fromy, int, tox, int, toy, int, thickness);
	AGS3::DrawingSurface_DrawLine(sds, fromx, fromy, tox, toy, thickness);
}

void DrawingSurface::DrawingSurface_DrawMessageWrapped(ScriptMethodParams &params) {
	PARAMS6(ScriptDrawingSurface *, sds, int, xx, int, yy, int, wid, int, font, int, msgm);
	AGS3::DrawingSurface_DrawMessageWrapped(sds, xx, yy, wid, font, msgm);
}

void DrawingSurface::DrawingSurface_DrawPixel(ScriptMethodParams &params) {
	PARAMS3(ScriptDrawingSurface *, sds, int, x, int, y);
	AGS3::DrawingSurface_DrawPixel(sds, x, y);
}

void DrawingSurface::DrawingSurface_DrawRectangle(ScriptMethodParams &params) {
	PARAMS5(ScriptDrawingSurface *, sds, int, x1, int, y1, int, x2, int, y2);
	AGS3::DrawingSurface_DrawRectangle(sds, x1, y1, x2, y2);
}

void DrawingSurface::ScPl_DrawingSurface_DrawString(ScriptMethodParams &params) {
	PARAMS4(ScriptDrawingSurface *, sds, int, xx, int, yy, int, font);
	Common::String buf = params.format(4);

	AGS3::DrawingSurface_DrawString(sds, xx, yy, font, buf.c_str());
}

void DrawingSurface::DrawingSurface_DrawStringWrapped_Old(ScriptMethodParams &params) {
	PARAMS7(ScriptDrawingSurface *, sds, int, xx, int, yy, int, wid, int, font, int, alignment, const char *, msg);
	AGS3::DrawingSurface_DrawStringWrapped_Old(sds, xx, yy, wid, font, alignment, msg);
}

void DrawingSurface::DrawingSurface_DrawStringWrapped(ScriptMethodParams &params) {
	PARAMS7(ScriptDrawingSurface *, sds, int, xx, int, yy, int, wid, int, font, int, alignment, const char *, msg);
	AGS3::DrawingSurface_DrawStringWrapped(sds, xx, yy, wid, font, alignment, msg);
}

void DrawingSurface::DrawingSurface_DrawSurface(ScriptMethodParams &params) {
	PARAMS3(ScriptDrawingSurface *, target, ScriptDrawingSurface *, source, int, translev);
	AGS3::DrawingSurface_DrawSurface(target, source, translev);
}

void DrawingSurface::DrawingSurface_DrawTriangle(ScriptMethodParams &params) {
	PARAMS7(ScriptDrawingSurface *, sds, int, x1, int, y1, int, x2, int, y2, int, x3, int, y3);
	AGS3::DrawingSurface_DrawTriangle(sds, x1, y1, x2, y2, x3, y3);
}

void DrawingSurface::DrawingSurface_GetPixel(ScriptMethodParams &params) {
	PARAMS3(ScriptDrawingSurface *, sds, int, x, int, y);
	params._result = AGS3::DrawingSurface_GetPixel(sds, x, y);
}

void DrawingSurface::DrawingSurface_Release(ScriptMethodParams &params) {
	PARAMS1(ScriptDrawingSurface *, sds);
	AGS3::DrawingSurface_Release(sds);
}

void DrawingSurface::DrawingSurface_GetDrawingColor(ScriptMethodParams &params) {
	PARAMS1(ScriptDrawingSurface *, sds);
	params._result = AGS3::DrawingSurface_GetDrawingColor(sds);
}

void DrawingSurface::DrawingSurface_SetDrawingColor(ScriptMethodParams &params) {
	PARAMS2(ScriptDrawingSurface *, sds, int, newColour);
	AGS3::DrawingSurface_SetDrawingColor(sds, newColour);
}

void DrawingSurface::DrawingSurface_GetHeight(ScriptMethodParams &params) {
	PARAMS1(ScriptDrawingSurface *, sds);
	params._result = AGS3::DrawingSurface_GetHeight(sds);
}

void DrawingSurface::DrawingSurface_GetUseHighResCoordinates(ScriptMethodParams &params) {
	PARAMS1(ScriptDrawingSurface *, sds);
	params._result = AGS3::DrawingSurface_GetUseHighResCoordinates(sds);
}

void DrawingSurface::DrawingSurface_SetUseHighResCoordinates(ScriptMethodParams &params) {
	PARAMS2(ScriptDrawingSurface *, sds, int, highRes);
	AGS3::DrawingSurface_SetUseHighResCoordinates(sds, highRes);
}

void DrawingSurface::DrawingSurface_GetWidth(ScriptMethodParams &params) {
	PARAMS1(ScriptDrawingSurface *, sds);
	params._result = AGS3::DrawingSurface_GetWidth(sds);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
