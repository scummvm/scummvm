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

#include "ags/plugins/core/drawing_surface.h"
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/engine/ac/drawing_surface.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void DrawingSurface::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(DrawingSurface::Clear^1, DrawingSurface::Clear);
	SCRIPT_METHOD(DrawingSurface::CreateCopy^0, DrawingSurface::CreateCopy);
	SCRIPT_METHOD(DrawingSurface::DrawCircle^3, DrawingSurface::DrawCircle);
	SCRIPT_METHOD(DrawingSurface::DrawImage^6, DrawingSurface::DrawImage);
	SCRIPT_METHOD(DrawingSurface::DrawLine^5, DrawingSurface::DrawLine);
	SCRIPT_METHOD(DrawingSurface::DrawMessageWrapped^5, DrawingSurface::DrawMessageWrapped);
	SCRIPT_METHOD(DrawingSurface::DrawPixel^2, DrawingSurface::DrawPixel);
	SCRIPT_METHOD(DrawingSurface::DrawRectangle^4, DrawingSurface::DrawRectangle);
	SCRIPT_METHOD(DrawingSurface::DrawString^104, DrawingSurface::ScPl_DrawString);

	if (engine->version < kScriptAPI_v350)
		SCRIPT_METHOD(DrawingSurface::DrawStringWrapped^6, DrawingSurface::DrawStringWrapped_Old);
	else
		SCRIPT_METHOD(DrawingSurface::DrawStringWrapped^6, DrawingSurface::DrawStringWrapped);

	SCRIPT_METHOD(DrawingSurface::DrawSurface^2, DrawingSurface::DrawSurface);
	SCRIPT_METHOD(DrawingSurface::DrawTriangle^6, DrawingSurface::DrawTriangle);
	SCRIPT_METHOD(DrawingSurface::GetPixel^2, DrawingSurface::GetPixel);
	SCRIPT_METHOD(DrawingSurface::Release^0, DrawingSurface::Release);
	SCRIPT_METHOD(DrawingSurface::get_DrawingColor, DrawingSurface::GetDrawingColor);
	SCRIPT_METHOD(DrawingSurface::set_DrawingColor, DrawingSurface::SetDrawingColor);
	SCRIPT_METHOD(DrawingSurface::get_Height, DrawingSurface::GetHeight);
	SCRIPT_METHOD(DrawingSurface::get_UseHighResCoordinates, DrawingSurface::GetUseHighResCoordinates);
	SCRIPT_METHOD(DrawingSurface::set_UseHighResCoordinates, DrawingSurface::SetUseHighResCoordinates);
	SCRIPT_METHOD(DrawingSurface::get_Width, DrawingSurface::GetWidth);
}

void DrawingSurface::Clear(ScriptMethodParams &params) {
	PARAMS2(ScriptDrawingSurface *, sds, int, colour);
	AGS3::DrawingSurface_Clear(sds, colour);
}

void DrawingSurface::CreateCopy(ScriptMethodParams &params) {
	PARAMS1(ScriptDrawingSurface *, sds);
	params._result = AGS3::DrawingSurface_CreateCopy(sds);
}

void DrawingSurface::DrawCircle(ScriptMethodParams &params) {
	PARAMS4(ScriptDrawingSurface *, sds, int, x, int, y, int, radius);
	AGS3::DrawingSurface_DrawCircle(sds, x, y, radius);
}

void DrawingSurface::DrawImage(ScriptMethodParams &params) {
	PARAMS7(ScriptDrawingSurface *, sds, int, xx, int, yy, int, slot, int, trans, int, width, int, height);
	AGS3::DrawingSurface_DrawImage6(sds, xx, yy, slot, trans, width, height);
}

void DrawingSurface::DrawLine(ScriptMethodParams &params) {
	PARAMS6(ScriptDrawingSurface *, sds, int, fromx, int, fromy, int, tox, int, toy, int, thickness);
	AGS3::DrawingSurface_DrawLine(sds, fromx, fromy, tox, toy, thickness);
}

void DrawingSurface::DrawMessageWrapped(ScriptMethodParams &params) {
	PARAMS6(ScriptDrawingSurface *, sds, int, xx, int, yy, int, wid, int, font, int, msgm);
	AGS3::DrawingSurface_DrawMessageWrapped(sds, xx, yy, wid, font, msgm);
}

void DrawingSurface::DrawPixel(ScriptMethodParams &params) {
	PARAMS3(ScriptDrawingSurface *, sds, int, x, int, y);
	AGS3::DrawingSurface_DrawPixel(sds, x, y);
}

void DrawingSurface::DrawRectangle(ScriptMethodParams &params) {
	PARAMS5(ScriptDrawingSurface *, sds, int, x1, int, y1, int, x2, int, y2);
	AGS3::DrawingSurface_DrawRectangle(sds, x1, y1, x2, y2);
}

void DrawingSurface::ScPl_DrawString(ScriptMethodParams &params) {
	PARAMS4(ScriptDrawingSurface *, sds, int, xx, int, yy, int, font);
	Common::String buf = params.format(4);

	AGS3::DrawingSurface_DrawString(sds, xx, yy, font, buf.c_str());
}

void DrawingSurface::DrawStringWrapped_Old(ScriptMethodParams &params) {
	PARAMS7(ScriptDrawingSurface *, sds, int, xx, int, yy, int, wid, int, font, int, alignment, const char *, msg);
	AGS3::DrawingSurface_DrawStringWrapped_Old(sds, xx, yy, wid, font, alignment, msg);
}

void DrawingSurface::DrawStringWrapped(ScriptMethodParams &params) {
	PARAMS7(ScriptDrawingSurface *, sds, int, xx, int, yy, int, wid, int, font, int, alignment, const char *, msg);
	AGS3::DrawingSurface_DrawStringWrapped(sds, xx, yy, wid, font, alignment, msg);
}

void DrawingSurface::DrawSurface(ScriptMethodParams &params) {
	PARAMS3(ScriptDrawingSurface *, target, ScriptDrawingSurface *, source, int, translev);
	AGS3::DrawingSurface_DrawSurface2(target, source, translev);
}

void DrawingSurface::DrawTriangle(ScriptMethodParams &params) {
	PARAMS7(ScriptDrawingSurface *, sds, int, x1, int, y1, int, x2, int, y2, int, x3, int, y3);
	AGS3::DrawingSurface_DrawTriangle(sds, x1, y1, x2, y2, x3, y3);
}

void DrawingSurface::GetPixel(ScriptMethodParams &params) {
	PARAMS3(ScriptDrawingSurface *, sds, int, x, int, y);
	params._result = AGS3::DrawingSurface_GetPixel(sds, x, y);
}

void DrawingSurface::Release(ScriptMethodParams &params) {
	PARAMS1(ScriptDrawingSurface *, sds);
	AGS3::DrawingSurface_Release(sds);
}

void DrawingSurface::GetDrawingColor(ScriptMethodParams &params) {
	PARAMS1(ScriptDrawingSurface *, sds);
	params._result = AGS3::DrawingSurface_GetDrawingColor(sds);
}

void DrawingSurface::SetDrawingColor(ScriptMethodParams &params) {
	PARAMS2(ScriptDrawingSurface *, sds, int, newColour);
	AGS3::DrawingSurface_SetDrawingColor(sds, newColour);
}

void DrawingSurface::GetHeight(ScriptMethodParams &params) {
	PARAMS1(ScriptDrawingSurface *, sds);
	params._result = AGS3::DrawingSurface_GetHeight(sds);
}

void DrawingSurface::GetUseHighResCoordinates(ScriptMethodParams &params) {
	PARAMS1(ScriptDrawingSurface *, sds);
	params._result = AGS3::DrawingSurface_GetUseHighResCoordinates(sds);
}

void DrawingSurface::SetUseHighResCoordinates(ScriptMethodParams &params) {
	PARAMS2(ScriptDrawingSurface *, sds, int, highRes);
	AGS3::DrawingSurface_SetUseHighResCoordinates(sds, highRes);
}

void DrawingSurface::GetWidth(ScriptMethodParams &params) {
	PARAMS1(ScriptDrawingSurface *, sds);
	params._result = AGS3::DrawingSurface_GetWidth(sds);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
