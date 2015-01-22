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

#ifndef XEEN_INTERFACE_H
#define XEEN_INTERFACE_H

#include "common/scummsys.h"
#include "xeen/dialogs.h"
#include "xeen/interface_map.h"
#include "xeen/party.h"
#include "xeen/screen.h"

namespace Xeen {

class XeenEngine;

#define MINIMAP_SIZE 7

class Interface: public ButtonContainer, public InterfaceMap {
private:
	XeenEngine *_vm;
	SpriteResource _dseFace;
	SpriteResource _globalSprites;
	SpriteResource _borderSprites;
	SpriteResource _spellFxSprites;
	SpriteResource _fecpSprites;
	SpriteResource _blessSprites;
	SpriteResource _restoreSprites;
	SpriteResource _hpSprites;
	SpriteResource _uiSprites;
	SpriteResource _iconSprites;
	SpriteResource _charPowSprites;
	SpriteResource _charFaces[TOTAL_CHARACTERS];
	SpriteResource *_partyFaces[MAX_ACTIVE_PARTY];
	DrawStruct _faceDrawStructs[4];
	DrawStruct _mainList[16];
	int _combatCharIds[8];

	int _batUIFrame;
	int _spotDoorsUIFrame;
	int _dangerSenseUIFrame;
	int _face1UIFrame;
	int _face2UIFrame;
	int _blessedUIFrame;
	int _powerShieldUIFrame;
	int _holyBonusUIFrame;
	int _heroismUIFrame;
	int _flipUIFrame;
	bool _buttonsLoaded;
	int _hiliteChar;
	bool _flag1;
	bool _isAnimReset;
	byte _tillMove;
	int _overallFrame;
	bool _upDoorText;
	int _steppingFX;

	void initDrawStructs();

	void loadSprites();

	void assembleBorder();

	void setupBackground();

	void setupFaces(int charIndex, Common::Array<int> xeenSideChars, bool updateFlag);

	void drawViewBackground(int bgType);

	void moveCharacterToRoster();

	void animate3d();

	void moveMonsters();

	void setMainButtons();

	void drawMiniMap();

	void chargeStep();

	void stepTime();

	void doStepCode();

	bool checkMoveDirection(int key);
public:
	int _intrIndex1;
	Common::String _interfaceText;
public:
	Interface(XeenEngine *vm);

	void setup();

	void manageCharacters(bool soundPlayed);

	void loadCharIcons();

	void loadPartyIcons();

	void draw3d(bool updateFlag);

	void startup();

	void mainIconsPrint();

	void charIconsPrint(bool updateFlag);

	void doFalling();

	void perform();
};

} // End of namespace Xeen

#endif /* XEEN_INTERFACE_H */
