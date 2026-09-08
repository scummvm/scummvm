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

#include "ultima/ultima1/views/interactions/pillar.h"
#include "ultima/ultima1/core/strings.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

constexpr int COLOR_TEXT = 11;   // light cyan - matches Commands/Stats
constexpr int COLOR_BORDER = 1;  // dark blue
constexpr int COLOR_EDGE = 15;   // white - brief "success" flash colour

// The dialog box's own area, and the thin border line inset within it -
// both given here in absolute screen pixels, as in the original
constexpr int DIALOG_X1 = 31, DIALOG_Y1 = 23, DIALOG_X2 = 287, DIALOG_Y2 = 127;
constexpr int BORDER_X1 = 34, BORDER_Y1 = 26, BORDER_X2 = 284, BORDER_Y2 = 124;

// Which attribute (into ATTRIBUTE_NAMES/attributeRef below) each of the 8
// pillars raises; -1 marks the two special-cased pillars - location 2
// (finds a weapon instead) and location 6 (does nothing at all)
static const int8 ATTRIBUTE_INDEXES[8] = { 2, 6, -1, 5, 3, 4, -1, 3 };

static const char *const ATTRIBUTE_NAMES[7] = {
	"Hit Points", "Strength", "Agility", "Stamina", "Charisma", "Wisdom", "Intelligence"
};

int16 &attributeRef(Data::Savegame &sg, int attrIndex) {
	switch (attrIndex) {
	case 0:
		return sg._hits;
	case 1:
		return sg._strength;
	case 2:
		return sg._agility;
	case 3:
		return sg._stamina;
	case 4:
		return sg._charisma;
	case 5:
		return sg._wisdom;
	default:
		return sg._intelligence;
	}
}

bool Pillar::msgGame(const GameMessage &msg) {
	if (msg._name == "PILLAR") {
		_locationNum = msg._value;
		addView();
		return true;
	}

	return false;
}

bool Pillar::msgFocus(const FocusMessage &msg) {
	setBounds(TextRect(0, 0, 39, 19));
	_borderColor = COLOR_BORDER;

	auto &sg = _G(savegame);
	bool success;

	if ((_locationNum % 2) != 0)
		questCompleted(_locationNum);

	if (_locationNum == sg._signMarker) {
		// Already read this particular pillar before - no repeat reward
		success = false;
	} else {
		sg._signMarker = _locationNum;

		int attrIndex = ATTRIBUTE_INDEXES[_locationNum];
		if (attrIndex < 0)
			success = (_locationNum == 2) && findWeapon();
		else
			success = gainAttribute(attrIndex);
	}

	if (success) {
		// Flash the border briefly to celebrate the reward
		_borderColor = COLOR_EDGE;
		g_engine->playFX(5);
		delaySeconds(1);
	} else if (_locationNum != 6) {
		writeString("Hmmmm...no effect!");
		g_engine->playFX(6);
	}

	redraw();
	return true;
}

void Pillar::timeout() {
	_borderColor = COLOR_BORDER;
	redraw();
}

void Pillar::draw() {
	auto s = getSurface();
	s.fillRect(Rect(DIALOG_X1, DIALOG_Y1, DIALOG_X2, DIALOG_Y2), 0);
	s.frameRect(Rect(BORDER_X1, BORDER_Y1, BORDER_X2, BORDER_Y2), _borderColor);

	s.setColor(COLOR_TEXT);
	drawInscription();
}

void Pillar::drawInscription() {
	auto s = getSurface();

	switch (_locationNum) {
	case 0:
		s.writeString(Point(8, 8), "You feel a strong magic");
		s.writeString(Point(12, 9), "surrounding you!");
		break;
	case 1:
		s.writeString(Point(10, 7), "You hear someone say,");
		s.writeString(Point(6, 9), " \"TURIS-SCIENTIA-MAGNOPERE\"");
		break;
	case 2:
		s.writeString(Point(13, 7), "A sign reads:");
		s.writeString(Point(8, 9), "\"FORTES FORTUNA ADIUVAT!\"");
		break;
	case 3:
		s.writeString(Point(13, 5), "On a pedestal,");
		s.writeString(Point(10, 6), "these words appear:");
		s.writeString(Point(8, 7), "\"MY NAME IS OZYMANDIAS,");
		s.writeString(Point(13, 8), "KING OF KINGS:");
		s.writeString(Point(11, 9), "LOOK AT MY WORKS,");
		s.writeString(Point(8, 10), "YE MIGHTY, AND DESPAIR!\"");
		s.writeString(Point(8, 12), "Nothing beside remains.");
		s.writeString(Point(7, 13), "You feel a strange force!");
		break;
	case 4:
		s.writeString(Point(12, 6), "The sign reads:");
		s.writeString(Point(12, 8), "\"ULTIMA THULE!\"");
		s.writeString(Point(7, 10), "The sky grows dark, and a");
		s.writeString(Point(7, 11), "strong magic engulfs you!");
		break;
	case 5:
		s.writeString(Point(12, 7), "The sign reads:");
		s.writeString(Point(11, 9), "\"OMNIA MUTANTUR!\"");
		break;
	case 6:
		s.writeString(Point(12, 7), "The sign reads:");
		s.writeString(Point(9, 9), "\"GO EAST TO GO EAST!\"");
		break;
	case 7:
		s.writeString(Point(10, 7), "The grave is marked:");
		s.writeString(Point(14, 9), "\"VAE VICTIS\"");
		break;
	default:
		break;
	}
}

void Pillar::questCompleted(int locationNum) {
	auto &sg = _G(savegame);
	int castleIdx = 8 - locationNum;

	if (sg._questStatus[castleIdx] == -1) {
		sg._questStatus[castleIdx] = 1;
		writeString("A quest has been completed!\n");
		g_engine->playFX(5);
		writeString("\n");
	}
}

bool Pillar::findWeapon() {
	auto &sg = _G(savegame);

	int idx = 0;
	do {
		++idx;
	} while (idx <= 15 && sg._weapons[idx] != 0);

	if (idx > 15)
		return false;

	++sg._weapons[idx];
	const char *name = Data::WEAPON_NAMES[idx];

	writeString("You find a");
	if (isVowel(*name))
		writeString("n");
	writeString(" %s!", name);
	return true;
}

bool Pillar::gainAttribute(int attrIndex) {
	auto &sg = _G(savegame);
	int16 &stat = attributeRef(sg, attrIndex);

	if (stat == 99)
		return false;

	int value = (stat + 8) / 10;
	if (stat + value > 99)
		value = 99 - stat;

	writeString("Thou dost gain %d %s", value, ATTRIBUTE_NAMES[attrIndex]);
	stat += value;
	return true;
}

bool Pillar::msgAction(const ActionMessage &msg) {
	finish();
	return true;
}

bool Pillar::msgKeypress(const KeypressMessage &msg) {
	finish();
	return true;
}

void Pillar::finish() {
	cancelDelay();

	if (_locationNum != 6)
		writeString("\n");
	prompt();
	close();
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
