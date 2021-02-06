/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#ifndef AGS_PLUGINS_AGS_PARALLAX_AGS_PARALLAX_H
#define AGS_PLUGINS_AGS_PARALLAX_AGS_PARALLAX_H

#include "ags/plugins/dll.h"
#include "ags/plugins/serializer.h"

namespace AGS3 {
namespace Plugins {
namespace AGSParallax {

struct Sprite {
	int x = 0;
	int y = 0;
	int slot = -1;
	int speed = 0;

	void SyncGame(Serializer &s);
};

#define MAX_SPEED 1000
#define MAX_SPRITES 100

/**
 * This is not the AGS Parallax plugin by Scorpiorus
 * but a workalike plugin created for the AGS engine ports.
 */
class AGSParallax : public DLL {
private:
	static IAGSEngine *_engine;
	static int _screenWidth;
	static int _screenHeight;
	static int _screenColorDepth;

	static bool _enabled;
	static Sprite _sprites[MAX_SPRITES];
private:
	static const char *AGS_GetPluginName();
	static void AGS_EngineStartup(IAGSEngine *lpEngine);
	static int AGS_EngineOnEvent(int event, int data);

	static void pxDrawSprite(int id, int x, int y, int slot, int speed);
	static void pxDeleteSprite(int id);

private:
	static void SyncGame(Serializer &s);
	static void Draw(bool foreground);
	static void clear();

public:
	AGSParallax();
};

} // namespace AGSParallax
} // namespace Plugins
} // namespace AGS3

#endif
