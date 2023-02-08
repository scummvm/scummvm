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

#ifndef MM1_VIEWS_CREATE_CHARACTERS_H
#define MM1_VIEWS_CREATE_CHARACTERS_H

#include "mm/mm1/data/roster.h"
#include "mm/mm1/views/text_view.h"

namespace MM {
namespace MM1 {
namespace Views {

class CreateCharacters : public TextView {
	enum State {
		SELECT_CLASS, SELECT_RACE, SELECT_ALIGNMENT,
		SELECT_SEX, SELECT_NAME, SAVE_PROMPT
	};
	enum Attribute {
		INTELLECT, MIGHT, PERSONALITY, ENDURANCE, SPEED,
		ACCURACY, LUCK
	};
	struct NewCharacter {
	private:
		void setHP(int hp);
		void setSP(int sp);
	public:
		uint8 _attribs1[LUCK + 1] = { 0 };
		uint8 _attribs2[LUCK + 1] = { 0 };
		CharacterClass _class = KNIGHT;
		Race _race = HUMAN;
		Alignment _alignment = GOOD;
		Sex _sex = MALE;
		Common::String _name;

		bool _classesAllowed[7] = { false };

		void clear();
		void reroll();
		void save();
	};

private:
	State _state = SELECT_CLASS;
	NewCharacter _newChar;

	/**
	 * Displays the new character attributes
	 */
	void printAttributes();

	/**
	 * Display the available classes
	 */
	void printClasses();

	/**
	 * Display the races
	 */
	void printRaces();

	/**
	 * Display the alignments
	 */
	void printAlignments();

	/**
	 * Display the sexes
	 */
	void printSexes();

	/**
	 * Display the selection summaries and either name entry
	 * or prompt to save character
	 */
	void printSummary(bool promptToSave);
public:
	CreateCharacters() : TextView("CreateCharacters") {}
	virtual ~CreateCharacters() {}

	bool msgFocus(const FocusMessage &msg) override {
		_newChar.reroll();
		return true;
	}
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
