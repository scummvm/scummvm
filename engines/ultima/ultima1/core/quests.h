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

#ifndef ULTIMA_ULTIMA1_CORE_QUESTS_H
#define ULTIMA_ULTIMA1_CORE_QUESTS_H

#include "common/array.h"
#include "common/serializer.h"

namespace Ultima {
namespace Ultima1 {

#define FLAGS_COUNT 9

class Ultima1Game;

/**
 * Quest entry
 */
class QuestFlag {
	enum FlagState { UNSTARTED = 0, IN_PROGRESS = -1, COMPLETED = 1 };
private:
	Ultima1Game *_game;
	FlagState _state;
public:
	/**
	 * Constructor
	 */
	QuestFlag() : _game(nullptr), _state(UNSTARTED) {}

	/**
	 * Constructor
	 */
	QuestFlag(Ultima1Game *game) : _game(game), _state(UNSTARTED) {}

	/**
	 * Synchronize the data for a single flag
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Returns true if the quest is unstarted
	 */
	bool isUnstarted() const { return _state == UNSTARTED; }

	/**
	 * Returns true if the quest is in progress
	 */
	bool isInProgress() const { return _state == IN_PROGRESS; }

	/**
	 * Called when a quest is completed
	 */
	bool isComplete() const { return _state == COMPLETED; }

	/**
	 * Mark a quest as in progress
	 */
	void start();

	/**
	 * Complete an in-progress quest
	 */
	void complete();
};
/**
 * Manages the list of quest flags
 */
class Quests : public Common::Array<QuestFlag> {
public:
	/**
	 * Constructor
	 */
	Quests(Ultima1Game *game);

	/**
	 * Synchronize the data for a single flag
	 */
	void synchronize(Common::Serializer &s);
};

} // End of namespace Ultima1
} // End of namespace Ultima

#endif
