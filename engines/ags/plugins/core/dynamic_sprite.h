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

#ifndef AGS_PLUGINS_CORE_DYNAMIC_SPRITE_H
#define AGS_PLUGINS_CORE_DYNAMIC_SPRITE_H

#include "ags/plugins/plugin_base.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class DynamicSprite : public ScriptContainer {
public:
	static void AGS_EngineStartup(IAGSEngine *engine);

	static void ChangeCanvasSize(ScriptMethodParams &params);
	static void CopyTransparencyMask(ScriptMethodParams &params);
	static void Crop(ScriptMethodParams &params);
	static void Delete(ScriptMethodParams &params);
	static void Flip(ScriptMethodParams &params);
	static void GetDrawingSurface(ScriptMethodParams &params);
	static void Resize(ScriptMethodParams &params);
	static void Rotate(ScriptMethodParams &params);
	static void SaveToFile(ScriptMethodParams &params);
	static void Tint(ScriptMethodParams &params);
	static void GetColorDepth(ScriptMethodParams &params);
	static void GetGraphic(ScriptMethodParams &params);
	static void GetHeight(ScriptMethodParams &params);
	static void GetWidth(ScriptMethodParams &params);
	static void Create(ScriptMethodParams &params);
	static void CreateFromBackground(ScriptMethodParams &params);
	static void CreateFromDrawingSurface(ScriptMethodParams &params);
	static void CreateFromExistingSprite_Old(ScriptMethodParams &params);
	static void CreateFromExistingSprite(ScriptMethodParams &params);
	static void CreateFromFile(ScriptMethodParams &params);
	static void CreateFromSaveGame(ScriptMethodParams &params);
	static void CreateFromScreenShot(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
