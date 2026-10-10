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

#ifndef ULTIMA3_VIEWS_INTERACTIONS_DUNGEON_SPECIAL_H
#define ULTIMA3_VIEWS_INTERACTIONS_DUNGEON_SPECIAL_H

#include "ultima/ultima3/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

/**
 * What happens when the party step onto one of the special squares of a
 * dungeon: a vision, a fountain, a wind, a trap, a hot rod or gremlins
 */
class DungeonSpecial : public Interaction {
private:
	byte _kind;
	bool _finished = false;
	bool _waiting = false;
	PlayerChooser _chooser;

	/**
	 * Shows one of the pictures that take the place of the dungeon view
	 */
	void showScene(const char *filename);
	void hideScene();

	void askWhoDrinks();
	void drink(int slot);
	void touchRod(int slot);
	void trap();
	void gremlins();

public:
	DungeonSpecial(byte kind);
	~DungeonSpecial() override;

	bool isFinished() const override {
		return _finished;
	}

	bool keypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
