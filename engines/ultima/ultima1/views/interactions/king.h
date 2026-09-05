
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

#ifndef ULTIMA1_VIEWS_INTERACTIONS_KING_H
#define ULTIMA1_VIEWS_INTERACTIONS_KING_H

#include "ultima/ultima1/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

using namespace Ultima::Shared::Messages;

/**
 * Handler for the Transact command's king interaction within castles.
 * Only usable when standing on the throne tile and the guards aren't
 * hostile
 */
class King : public Interaction {
	enum Mode {
		KING_PROMPT, KING_PENCE
	};
private:
	Mode _mode = KING_PROMPT;
	Common::String _amount;

	void finish();
	void redrawStats();

	/**
	 * If the player has just completed this castle's quest, gives the
	 * reward and prints the completion message. Returns true if a reward
	 * was given, in which case the pence/service prompt is skipped
	 */
	bool checkQuestReward();

	/**
	 * Starts a new quest for this castle - either to kill a specific
	 * dungeon monster, or to find a specific landmark - depending on
	 * whether the castle's index is even or odd
	 */
	void giveQuest();

	/**
	 * Handles the pence amount entry, digit by digit
	 */
	void penceKeypress(const KeypressMessage &msg);

public:
	King();
	~King() override {
	}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
