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

#ifndef __Nuvie_h__
#define __Nuvie_h__

#include <string>
#include "Configuration.h"

namespace Ultima {

class Screen;
class Script;
class Game;

class Nuvie {
	Configuration *config;
	Screen *screen;
	Script *script;
	Game *game;

public:

	Nuvie();
	~Nuvie();


	bool init(int argc, char **argv);
	bool play();

protected:

	bool initConfig();
	const char *getConfigPathWin32();
	void SharedDefaultConfigValues();
	void set_safe_video_settings();
	bool initDefaultConfigWin32();
	bool initDefaultConfigMacOSX(const char *home_env);
	bool initDefaultConfigUnix(const char *home_env);
	bool loadConfigFile(std::string filename, bool readOnly = NUVIE_CONF_READONLY);
	void assignGameConfigValues(uint8 game_type);
	bool checkGameDir(uint8 game_type);
	bool checkDataDir();

	bool playIntro();
};

} // End of namespace Ultima8

#endif /* __Nuvie_h__ */

