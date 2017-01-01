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

#ifndef XEEN_DIALOGS_PARTY_H
#define XEEN_DIALOGS_PARTY_H

#include "common/array.h"
#include "xeen/dialogs.h"
#include "xeen/interface.h"
#include "xeen/screen.h"
#include "xeen/sprites.h"

namespace Xeen {

class PartyDialog : public ButtonContainer, public PartyDrawer {
private:
	XeenEngine *_vm;
	SpriteResource _uiSprites;
	DrawStruct _faceDrawStructs[4];
	Common::String _partyDetails;
	Common::Array<int> _charList;
	int _diceFrame[3];
	Common::Point _dicePos[3];
	Common::Point _diceInc[3];

	PartyDialog(XeenEngine *vm);

	void execute();

	void loadButtons();

	void initDrawStructs();

	void setupBackground();

	/**
	 * Sets up the faces from the avaialble roster for display in the party dialog
	 */
	void setupFaces(int firstDisplayChar, bool updateFlag);

	void startingCharChanged(int firstDisplayChar);

	void createChar();

	int selectCharacter(bool isDelete, int firstDisplayChar);

	/**
	 * Roll up some random values for the attributes, and return both them as
	 * well as a list of classes that the attributes meet the requirements for
	 */
	void throwDice(uint attribs[TOTAL_ATTRIBUTES], bool allowedClasses[TOTAL_CLASSES]);

	/**
	 * Set a list of flags for which classes the passed attribute set meet the
	 * minimum requirements of
	 */
	void checkClass(const uint attribs[TOTAL_ATTRIBUTES], bool allowedClasses[TOTAL_CLASSES]);

	/**
	 * Return details of the generated character
	 */
	int newCharDetails(const uint attribs[TOTAL_ATTRIBUTES],
		bool allowedClasses[TOTAL_CLASSES], Race race, Sex sex, int classId,
		int selectedClass, Common::String &msg);

	/**
	 * Print the selection arrow to indicate the selected class
	 */
	void printSelectionArrow(SpriteResource &icons, int selectedClass);

	/**
	 * Print the dice animation
	 */
	void drawDice(SpriteResource &dice);

	/**
	 * Exchanging two attributes for the character being rolled
	 */
	int exchangeAttribute(int srcAttr);

	/**
	 * Saves the rolled character into the roster
	 */
	bool saveCharacter(Character &c, int classId, Race race,
		Sex sex, uint attribs[TOTAL_ATTRIBUTES]);
public:
	static void show(XeenEngine *vm);
};

} // End of namespace Xeen

#endif /* XEEN_DIALOGS_PARTY_H */
