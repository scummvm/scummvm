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

#ifndef ULTIMA8_GAMES_CRUGAME_H
#define ULTIMA8_GAMES_CRUGAME_H

#include "ultima/ultima8/games/game.h"

namespace Ultima {
namespace Ultima8 {

class CruGame : public Game {
public:
	CruGame();
	~CruGame() override;

	//! load/init game's data files
	bool loadFiles() override;

	//! initialize new game
	bool startGame() override;

	//! start initial usecode
	bool startInitialUsecode(int saveSlot = -1) override;

	//! write game-specific savegame info (avatar stats, equipment, ...)
	void writeSaveInfo(Common::WriteStream *ws) override;

	ProcId playIntroMovie(bool fade) override;
	ProcId playIntroMovie2(bool fade);
	ProcId playEndgameMovie(bool fade) override;
	void playCredits() override;
	void playQuotes() override { }; // no quotes for Crusader
	void playDemoScreen() override;

	/** Play credits but without showing a menu at the end - just finish. */
	ProcId playCreditsNoMenu();

	void setSkipIntroMovie() {
		_skipIntroMovie = true;
	}

private:
	/** Whether this game should skip the intro movie on startup */
	bool _skipIntroMovie;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
