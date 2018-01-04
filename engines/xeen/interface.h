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
#include "xeen/interface_minimap.h"
#include "xeen/interface_scene.h"
#include "xeen/party.h"
#include "xeen/window.h"

namespace Xeen {

class XeenEngine;

enum Obscurity {
	OBSCURITY_BLACK = 0,
	OBSCURITY_3 = 1,
	OBSCURITY_2 = 2,
	OBSCURITY_1 = 3,
	OBSCURITY_NONE = 4
};

#define HILIGHT_CHAR_DISABLED -2
#define HILIGHT_CHAR_NONE -1

/**
 * Class responsible for drawing the images of the characters in the party
 */
class PartyDrawer {
private:
	XeenEngine *_vm;
	SpriteResource _dseFace;
	SpriteResource _hpSprites;
	SpriteResource _restoreSprites;
	int _hiliteChar;
public:
	PartyDrawer(XeenEngine *vm);

	void drawParty(bool updateFlag);

	void highlightChar(int charId);

	void unhighlightChar();

	void resetHighlight();
};

/**
 * Implements the main in-game interface
 */
class Interface: public ButtonContainer, public InterfaceScene,
		public InterfaceMinimap, public PartyDrawer {
private:
	XeenEngine *_vm;
	SpriteResource _uiSprites;
	SpriteResource _iconSprites;
	SpriteResource _borderSprites;
	SpriteResource _spellFxSprites;
	SpriteResource _fecpSprites;
	SpriteResource _blessSprites;
	DrawStruct _mainList[16];

	bool _buttonsLoaded;
	int _steppingFX;
	int _blessedUIFrame;
	int _powerShieldUIFrame;
	int _holyBonusUIFrame;
	int _heroismUIFrame;
	int _flipUIFrame;

	void initDrawStructs();

	void loadSprites();

	void setupBackground();

	void setMainButtons(bool combatMode = false);

	void chargeStep();

	/**
	 * Handles incrementing game time
	 */
	void stepTime();

	void doStepCode();

	/**
	 * Check movement in the given direction
	 */
	bool checkMoveDirection(int key);

	/**
	 * Handle doing the falling
	 */
	void handleFalling();

	void saveFall();

	void fall(int v);

	/**
	 * Shake the screen
	 */
	void shake(int time);

	/**
	 * Select next character or monster to be attacking
	 */
	void nextChar();
public:
	Obscurity _obscurity;
	Common::String _interfaceText;
	int _falling;
	int _face1State, _face2State;
	int _face1UIFrame, _face2UIFrame;
	int _spotDoorsUIFrame;
	int _dangerSenseUIFrame;
	int _levitateUIFrame;
	bool _upDoorText;
	Common::String _screenText;
	byte _tillMove;
	int _charFX[6];
public:
	Interface(XeenEngine *vm);

	virtual ~Interface() {}

	void setup();

	void manageCharacters(bool soundPlayed);

	void startup();

	void mainIconsPrint();

	/**
	 * Start the party falling
	 */
	void startFalling(bool v);

	/**
	 * Waits for a keypress or click, whilst still allowing the game scene to
	 * be animated.
	 */
	void perform();

	void rest();

	/**
	 * Handles bash actions
	 */
	void bash(const Common::Point &pt, Direction direction);

	/**
	 * Handles drawing the elements of the interface and game scene
	 * @param updateFlag		Updates UI windows 1 & 3
	 * @param pauseFlag			Does a brief pause at the end of drawing
	 */
	void draw3d(bool updateFlag, bool pauseFlag = true);

	/**
	 * Draw the display borders
	 */
	void assembleBorder();

	void doCombat();

	void spellFX(Character *c);

	/**
	 * Optionally obscures the scene due to low light conditions
	 */
	void obscureScene(Obscurity obscurity);
};

} // End of namespace Xeen

#endif /* XEEN_INTERFACE_H */
