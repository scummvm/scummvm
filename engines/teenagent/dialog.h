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

#ifndef TEENAGENT_DIALOG_H
#define TEENAGENT_DIALOG_H

#include "common/endian.h"
#include "common/str.h"

namespace TeenAgent {

// Text Color Symbols
enum {
	textColorJohnNoty = 0xd0,
	textColorCampGuard = 0xd0,
	textColorShockedCaptain = 0xd0,
	textColorMark = 0xd1,
	textColorCredits = 0xd1,
	textColorBankGuard = 0xd7,
	textColorGrandpa = 0xd8,
	textColorMansionGuard = 0xd9,
	textColorMarkEnd = 0xe3,
	textColorProfessor = 0xe5,
	textColorOldLady = 0xe5,
	textColorAnne = 0xe5,
	textColorWellEcho = 0xe5,
	textColorSonny = 0xe5,
	textColorEskimo = 0xe5,
	textColorRGBBoss = 0xe7,
	textColorGoldDriver = 0xe7,
	textColorFortuneTeller = 0xeb,
	textColorCaptain = 0xec,
	textColorMike = 0xef,
	textColorCook = 0xef,
	textColorBarman = 0xef
};

enum CharacterID {
	kMark = 0,
	kGoldDriver = 1,
	kBankGuard = 2,
	kRGBBoss = 3,
	kFortuneTeller = 4,
	kCampGuard = 5,
	kCaptain = 6,
	kShockedCaptain = 7,
	kBarman = 8,
	kSonny = 9,
	kGrandpa = 10,
	kAnne = 11,
	kWellEcho = 12,
	kOldLady = 13,
	kMansionGuard = 14,
	kJohnNoty = 15,
	kProfessor = 16,
	kCook = 17,
	kEskimo = 18,
	kMike = 19,
	kMarkEnd = 20,
	kCreditsText = 21
};

struct CharacterDialogData {
	int voiceID;
	bool male;
	byte textColor;
};

static const CharacterDialogData characterDialogData[] = {
	{ 0, true, textColorMark },
	{ 1, true, textColorGoldDriver },
	{ 2, true, textColorBankGuard },
	{ 3, true, textColorRGBBoss },
	{ 0, false, textColorFortuneTeller },
	{ 4, true, textColorCampGuard },
	{ 5, true, textColorCaptain },
	{ 5, true, textColorShockedCaptain },	// Same voice as captain
	{ 6, true, textColorBarman },
	{ 7, true, textColorSonny, },
	{ 8, true, textColorGrandpa },
	{ 1, false, textColorAnne },
	{ 9, true, textColorWellEcho },
	{ 2, false, textColorOldLady },
	{ 10, true, textColorMansionGuard },
	{ 11, true, textColorJohnNoty },
	{ 12, true, textColorProfessor },
	{ 13, true, textColorCook },
	{ 14, true, textColorEskimo },
	{ 15, true, textColorMike },
	{ 0, true, textColorMarkEnd },	// Same voice as Mark
	{ 0, true, textColorCredits }	// Same voice as Mark
};

class Scene;
class TeenAgentEngine;

class Dialog {
public:
	Dialog(TeenAgentEngine *vm) : _vm(vm) { }

	uint16 pop(Scene *scene, uint16 addr, uint16 animation1, uint16 animation2, CharacterID character1ID, CharacterID character2ID, byte slot1, byte slot2);

	uint16 popMark(Scene *scene, uint16 addr) {
		return pop(scene, addr, 0, 0, kMark, kMark, 0, 0);
	}

	void show(uint16 dialogNum, Scene *scene, uint16 animation1, uint16 animation2, CharacterID character1ID, CharacterID character2ID, byte slot1, byte slot2);

	void showMono(uint16 dialogNum, Scene *scene, uint16 animation, CharacterID characterID, byte slot) {
		show(dialogNum, scene, animation, animation, characterID, characterID, slot, slot);
	}

	void showMark(uint16 dialogNum, Scene *scene) {
		show(dialogNum, scene, 0, 0, kMark, kMark, 0, 0);
	}

private:
	TeenAgentEngine *_vm;

	void show(Scene *scene, uint16 addr, uint16 animation1, uint16 animation2, CharacterID character1ID, CharacterID character2ID, byte slot1, byte slot2);
};

} // End of namespace TeenAgent

#endif
