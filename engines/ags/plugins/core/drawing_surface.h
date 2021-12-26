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

#ifndef AGS_PLUGINS_CORE_DRAWING_SURFACE_H
#define AGS_PLUGINS_CORE_DRAWING_SURFACE_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class DrawingSurface : public ScriptContainer {
	BUILT_IN_HASH(DrawingSurface)
public:
	virtual ~DrawingSurface() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void Clear(ScriptMethodParams &params);
	void CreateCopy(ScriptMethodParams &params);
	void DrawCircle(ScriptMethodParams &params);
	void DrawImage(ScriptMethodParams &params);
	void DrawLine(ScriptMethodParams &params);
	void DrawMessageWrapped(ScriptMethodParams &params);
	void DrawPixel(ScriptMethodParams &params);
	void DrawRectangle(ScriptMethodParams &params);
	void ScPl_DrawString(ScriptMethodParams &params);
	void DrawStringWrapped_Old(ScriptMethodParams &params);
	void DrawStringWrapped(ScriptMethodParams &params);
	void DrawSurface(ScriptMethodParams &params);
	void DrawTriangle(ScriptMethodParams &params);
	void GetPixel(ScriptMethodParams &params);
	void Release(ScriptMethodParams &params);
	void GetDrawingColor(ScriptMethodParams &params);
	void SetDrawingColor(ScriptMethodParams &params);
	void GetHeight(ScriptMethodParams &params);
	void GetUseHighResCoordinates(ScriptMethodParams &params);
	void SetUseHighResCoordinates(ScriptMethodParams &params);
	void GetWidth(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
