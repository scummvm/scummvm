/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGS_SPRITE_VIDEO_H
#define AGS_PLUGINS_AGS_SPRITE_VIDEO_H

#include "ags/plugins/ags_plugin.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteVideo {

class AGSSpriteVideo : public PluginBase {
	SCRIPT_HASH(AGSSpriteVideo)
protected:
	void SetLoopsPerSecond(ScriptMethodParams &params);
	void OpenVideo(ScriptMethodParams &params);
	void OpenSprite(ScriptMethodParams &params);
	void OpenSpriteFile(ScriptMethodParams &params);

	void get_scaling(ScriptMethodParams &params);
	void set_scaling(ScriptMethodParams &params);
	void get_relativeTo(ScriptMethodParams &params);
	void set_relativeTo(ScriptMethodParams &params);
	void get_isLooping(ScriptMethodParams &params);
	void set_isLooping(ScriptMethodParams &params);
	void SetAnchor(ScriptMethodParams &params);
	void Autoplay(ScriptMethodParams &params);
	void IsAutoplaying(ScriptMethodParams &params);
	void StopAutoplay(ScriptMethodParams &params);


public:
	AGSSpriteVideo() : PluginBase() {}
	virtual ~AGSSpriteVideo() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *engine) override;
};


} // namespace AGSSpriteVideo
} // namespace Plugins
} // namespace AGS3

#endif
