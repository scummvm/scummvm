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

#ifndef XEEN_TOWN_H
#define XEEN_TOWN_H

#include "common/scummsys.h"
#include "common/str-array.h"
#include "xeen/dialogs.h"
#include "xeen/dialogs_error.h"
#include "xeen/party.h"

namespace Xeen {

enum TownAction {
	BANK = 0, BLACKSMITH = 1, GUILD = 2, TAVERN = 3, TEMPLE = 4,
	TRAINING = 5, ARENA = 6, NO_ACTION = 7, REAPER = 8, GOLEM = 9,
	DWARF1 = 10, SPHINX = 11, ACTION12 = 12, DWARF2 = 13
};

class XeenEngine;
class TownMessage;

class Town: public ButtonContainer {
	friend class TownMessage;
private:
	SpriteResource _icons1, _icons2;
	Common::StringArray _textStrings;
	Common::Array<SpriteResource> _townSprites;
	int _townMaxId;
	TownAction _townActionId;
	int _v1, _v2;
	int _donation;
	int _healCost;
	int _v5, _v6;
	int _v10, _v11, _v12;
	int _v13, _v14;
	uint _maxLevel;
	int _v21;
	uint _v22;
	int _v23;
	int _v24;
	int _dayOfWeek;
	int _uncurseCost;
	Common::Point _townPos;
	int _arr1[6];
	int _currentCharLevel;
	bool _flag1;
	uint _experienceToNextLevel;
	int _drawFrameIndex;
	int _drawCtr1, _drawCtr2;

	void loadStrings(const Common::String &name);

	void pyramidEvent();

	void arenaEvent();

	void reaperEvent();

	void golemEvent();

	void sphinxEvent();

	void dwarfEvent();

	Common::String createTownText(Character &ch);

	int townWait();

	Character *doTownOptions(Character *c);

	Character *doBankOptions(Character *c);

	Character *doBlacksmithOptions(Character *c);

	Character *doGuildOptions(Character *c);

	Character *doTavernOptions(Character *c);

	Character *doTempleOptions(Character *c);

	Character *doTrainingOptions(Character *c);

	void depositWithdrawl(PartyBank whereId);
public:
	Town(XeenEngine *vm);

	int townAction(TownAction actionId);

	void drawTownAnim(bool flag);

	/**
	 * Returns true if a town location (bank, blacksmith, etc.) is currently active
	 */
	bool isActive() const;

	void clearSprites();
};

class TownMessage : public ButtonContainer {
private:
	SpriteResource _iconSprites;

	TownMessage(XeenEngine *vm) : ButtonContainer(vm) {}

	bool execute(int portrait, const Common::String &name,
		const Common::String &text, int confirm);

	void loadButtons();
public:
	static bool show(XeenEngine *vm, int portrait, const Common::String &name,
		const Common::String &text, int confirm);
};

} // End of namespace Xeen

#endif /* XEEN_SPELLS_H */
