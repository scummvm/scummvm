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

	static void Clear(ScriptMethodParams &params);
	static void CreateCopy(ScriptMethodParams &params);
	static void DrawCircle(ScriptMethodParams &params);
	static void DrawImage(ScriptMethodParams &params);
	static void DrawLine(ScriptMethodParams &params);
	static void DrawMessageWrapped(ScriptMethodParams &params);
	static void DrawPixel(ScriptMethodParams &params);
	static void DrawRectangle(ScriptMethodParams &params);
	static void ScPl_DrawString(ScriptMethodParams &params);
	static void DrawStringWrapped_Old(ScriptMethodParams &params);
	static void DrawStringWrapped(ScriptMethodParams &params);
	static void DrawSurface(ScriptMethodParams &params);
	static void DrawTriangle(ScriptMethodParams &params);
	static void GetPixel(ScriptMethodParams &params);
	static void Release(ScriptMethodParams &params);
	static void GetDrawingColor(ScriptMethodParams &params);
	static void SetDrawingColor(ScriptMethodParams &params);
	static void GetHeight(ScriptMethodParams &params);
	static void GetUseHighResCoordinates(ScriptMethodParams &params);
	static void SetUseHighResCoordinates(ScriptMethodParams &params);
	static void GetWidth(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
