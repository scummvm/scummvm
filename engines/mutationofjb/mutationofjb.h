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

#ifndef MUTATIONOFJB_MUTATIONOFJB_H
#define MUTATIONOFJB_MUTATIONOFJB_H

#include "engines/engine.h"
#include "mutationofjb/script.h"

namespace Common {
class Event;
}

namespace Graphics {
class Screen;
}

namespace MutationOfJB {

class Console;
class Game;

class MutationOfJBEngine : public Engine {
public:
	MutationOfJBEngine(OSystem *syst);
	~MutationOfJBEngine();

	virtual Common::Error run();
	Graphics::Screen *getScreen() const;
	Game &getGame();

private:
	bool loadGameData(bool partB);
	void setupCursor();
	void handleNormalScene(const Common::Event &event);
	void handleMapScene(const Common::Event &event);

	Console *_console;
	Graphics::Screen *_screen;
	Game *_game;
	ActionInfo::Action _currentAction;
	uint8 _mapObjectId;
};


}

#endif
