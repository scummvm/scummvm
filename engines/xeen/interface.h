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
#include "xeen/party.h"
#include "xeen/screen.h"

namespace Xeen {

class XeenEngine;

class Interface: public ButtonContainer {
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
	SpriteResource _charFaces[TOTAL_CHARACTERS];
	SpriteResource *_partyFaces[MAX_ACTIVE_PARTY];
	DrawStruct _faceDrawStructs[4];
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
	bool _isEarlyGame;
	bool _buttonsLoaded;
	Common::String _interfaceText;
	int _hiliteChar;
	int _intrIndex1;

	void assembleBorder();

	void setupBackground();

	void setupFaces(int charIndex, Common::Array<int> xeenSideChars, bool updateFlag);

	void charIconsPrint(bool updateFlag);

	void drawViewBackground(int bgType);

	void addCharacterToRoster();
public:
	Interface(XeenEngine *vm);

	void manageCharacters(bool soundPlayed);

	void loadCharIcons(int numChars);

	void draw3d(bool flag);
};

} // End of namespace Xeen

#endif /* XEEN_INTERFACE_H */
