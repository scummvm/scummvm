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

#ifndef TESORO_KINDS_STORYCHARACTER_H
#define TESORO_KINDS_STORYCHARACTER_H

#include "tesoro/kinds/character.h"

namespace Tesoro {

class World;

/**
 * Real object kind 5.
 * A person of the story: a character whose looking, talking and use of items start the game's scripts.
 * The subtype (0 to 16, the byte after the colour and width in the record) picks the person, determining
 * what they do in verb and useItem
 */
class StoryCharacter : public Character {
public:
	/** Built like a character; the subtype decides how much state it keeps and how it starts. */
	void init(int subtype, const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world);

	int verb(int verb) override;
	int useItem(int item) override;
	void prepareToSpeak(uint32 pose) override;
	void speechCleared() override;
	void draw(const Camera &camera, Display &display) override;

	int subtype() const { return _subtype; }

private:
	int run(const char *block, const char *label, bool withState);
	/** The byte of the state of another object. */
	byte stateOf(int level, const char *name, uint index);

	int _subtype = 0;
};

} // End of namespace Tesoro

#endif
