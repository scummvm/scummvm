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

#ifndef AGS_PLUGINS_CORE_DYNAMIC_SPRITE_H
#define AGS_PLUGINS_CORE_DYNAMIC_SPRITE_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

class DynamicSprite : public ScriptContainer {
	BUILT_IN_HASH(DynamicSprite)
public:
	virtual ~DynamicSprite() {}
	void AGS_EngineStartup(IAGSEngine *engine) override;

	void ChangeCanvasSize(ScriptMethodParams &params);
	void CopyTransparencyMask(ScriptMethodParams &params);
	void Crop(ScriptMethodParams &params);
	void Delete(ScriptMethodParams &params);
	void Flip(ScriptMethodParams &params);
	void GetDrawingSurface(ScriptMethodParams &params);
	void Resize(ScriptMethodParams &params);
	void Rotate(ScriptMethodParams &params);
	void SaveToFile(ScriptMethodParams &params);
	void Tint(ScriptMethodParams &params);
	void GetColorDepth(ScriptMethodParams &params);
	void GetGraphic(ScriptMethodParams &params);
	void GetHeight(ScriptMethodParams &params);
	void GetWidth(ScriptMethodParams &params);
	void Create(ScriptMethodParams &params);
	void CreateFromBackground(ScriptMethodParams &params);
	void CreateFromDrawingSurface(ScriptMethodParams &params);
	void CreateFromExistingSprite_Old(ScriptMethodParams &params);
	void CreateFromExistingSprite(ScriptMethodParams &params);
	void CreateFromFile(ScriptMethodParams &params);
	void CreateFromSaveGame(ScriptMethodParams &params);
	void CreateFromScreenShot(ScriptMethodParams &params);
};

} // namespace Core
} // namespace Plugins
} // namespace AGS3

#endif
