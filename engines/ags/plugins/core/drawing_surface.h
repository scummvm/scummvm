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

#ifndef AGS_PLUGINS_CORE_DRAWING_SURFACE_H
#define AGS_PLUGINS_CORE_DRAWING_SURFACE_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class DrawingSurface : public ScriptContainer {
public:
	void AGS_EngineStartup(IAGSEngine *engine);

	static void DrawingSurface_Clear(ScriptMethodParams &params);
	static void DrawingSurface_CreateCopy(ScriptMethodParams &params);
	static void DrawingSurface_DrawCircle(ScriptMethodParams &params);
	static void DrawingSurface_DrawImage(ScriptMethodParams &params);
	static void DrawingSurface_DrawLine(ScriptMethodParams &params);
	static void DrawingSurface_DrawMessageWrapped(ScriptMethodParams &params);
	static void DrawingSurface_DrawPixel(ScriptMethodParams &params);
	static void DrawingSurface_DrawRectangle(ScriptMethodParams &params);
	static void ScPl_DrawingSurface_DrawString(ScriptMethodParams &params);
	static void DrawingSurface_DrawStringWrapped_Old(ScriptMethodParams &params);
	static void DrawingSurface_DrawStringWrapped(ScriptMethodParams &params);
	static void DrawingSurface_DrawSurface(ScriptMethodParams &params);
	static void DrawingSurface_DrawTriangle(ScriptMethodParams &params);
	static void DrawingSurface_GetPixel(ScriptMethodParams &params);
	static void DrawingSurface_Release(ScriptMethodParams &params);
	static void DrawingSurface_GetDrawingColor(ScriptMethodParams &params);
	static void DrawingSurface_SetDrawingColor(ScriptMethodParams &params);
	static void DrawingSurface_GetHeight(ScriptMethodParams &params);
	static void DrawingSurface_GetUseHighResCoordinates(ScriptMethodParams &params);
	static void DrawingSurface_SetUseHighResCoordinates(ScriptMethodParams &params);
	static void DrawingSurface_GetWidth(ScriptMethodParams &params);	
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
