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

#ifndef AGS_PLUGINS_AGS_PARALLAX_AGS_PARALLAX_H
#define AGS_PLUGINS_AGS_PARALLAX_AGS_PARALLAX_H

#include "ags/plugins/plugin_base.h"
#include "ags/plugins/serializer.h"

namespace AGS3 {
namespace Plugins {
namespace AGSParallax {

struct Sprite {
	int32 x = 0;
	int32 y = 0;
	int slot = -1;
	int speed = 0;

	void syncGame(Serializer &s);
};

#define MAX_SPEED 1000
#define MAX_SPRITES 100

/**
 * This is not the AGS Parallax plugin by Scorpiorus
 * but a workalike plugin created for the AGS engine ports.
 */
class AGSParallax : public PluginBase {
	SCRIPT_HASH(AGSParallax)
private:
	int32 _screenWidth = 320;
	int32 _screenHeight = 200;
	int32 _screenColorDepth = 32;

	bool _enabled = false;
	Sprite _sprites[MAX_SPRITES];

private:
	void pxDrawSprite(ScriptMethodParams &params);
	void pxDeleteSprite(ScriptMethodParams &params);

	void syncGame(Serializer &s);
	void Draw(bool foreground);
	void clear();

public:
	AGSParallax() : PluginBase() {}
	virtual ~AGSParallax() {}

	const char *AGS_GetPluginName() override;
	void AGS_EngineStartup(IAGSEngine *lpEngine) override;
	int64 AGS_EngineOnEvent(int event, NumberPtr data) override;
};

} // namespace AGSParallax
} // namespace Plugins
} // namespace AGS3

#endif
