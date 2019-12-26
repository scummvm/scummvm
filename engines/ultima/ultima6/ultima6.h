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

#ifndef ULTIMA6_ULTIMA6_H
#define ULTIMA6_ULTIMA6_H

#include "ultima/detection.h"
#include "ultima/shared/std/string.h"
#include "ultima/ultima6/conf/configuration.h"
#include "common/archive.h"
#include "common/random.h"
#include "engines/engine.h"

namespace Ultima {
namespace Ultima6 {

class Screen;
class Script;
class Game;

class Ultima6Engine : public Engine {
private:
	const Ultima::UltimaGameDescription *_gameDescription;
	Configuration *_config;
	Screen *_screen;
	Script *_script;
	Game *_game;
	Common::RandomSource _randomSource;
	Common::Archive* _dataArchive;
private:
	bool initialize();

	bool loadData();
	void initConfig();
	const char *getConfigPathWin32();
	void SharedDefaultConfigValues();
	void assignGameConfigValues(uint8 game_type);
	bool checkGameDir(uint8 game_type);
	bool checkDataDir();

	bool playIntro();
public:
	Ultima6Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~Ultima6Engine();

	void GUIError(const Common::String& msg);

	/**
	 * Play the game
	 */
	virtual Common::Error run() override;

	/**
	 * Get a random number
	 */
	uint getRandomNumber(uint maxVal) { return _randomSource.getRandomNumber(maxVal); }
};

extern Ultima6Engine *g_engine;

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
