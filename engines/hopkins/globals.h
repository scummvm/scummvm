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

#ifndef HOPKINS_GLOBALS_H
#define HOPKINS_GLOBALS_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/util.h"
#include "common/events.h"

namespace Hopkins {

struct HopkinsItem {
	int _speedX;
	int _speedY;
};

struct CharacterLocation {
	Common::Point _pos;
	int _startSpriteIndex;
	int _location;
	int _zoomFactor;

	void reset() {
		_pos.x = 0;
		_pos.y = 0;
		_startSpriteIndex = 0;
		_location = 0;
		_zoomFactor = 0;
	}

	CharacterLocation() { reset(); }
};

enum SauvegardeOffset {
	svLastMouseCursor = 1
	, svLastZoneNum = 2
	, svLastObjectIndex = 3
	, svDialogField4 = 4
	, svLastScreenId = 5
	, svLastPrevScreenId = 6
	, svLastInventoryItem = 8
	, svLastInvMouseCursor = 9
	, svLastSavegameSlot = 10
	, svFreedHostageFl = 80
	, svField94 = 94
	, svField95 = 95
	, svForestAvailableFl = 113
	, svHutBurningFl = 117
	, svHopkinsCloneFl = 121
	, svAlternateSpriteFl = 122
	, svHeavenGuardGoneFl = 123
	, svField132 = 132
	, svField133 = 133
	, svGameWonFl = 135
	, svCinemaCurtainCond1 = 166
	, svCinemaCurtainCond2 = 167
	, svBankAttackAnimPlayedFl = 170
	, svCopCall1PlayedFl = 171
	, svCopCall2PlayedFl = 172
	, svField173 = 173
	, svField176 = 176
	, svPoolDogGoneFl = 177
	, svCinemaDogGoneFl = 181
	, svField183 = 183
	, svField184 = 184
	, svField186 = 186
	, svField188 = 188
	, svField200 = 200
	, svField214 = 214
	, svBombBoxOpenedFl = 220
	, svBombDisarmedFl = 225
	, svField228 = 228
	, svField231 = 231
	, svField253 = 253
	, svField261 = 261
	, svField270 = 270
	, svField300 = 300
	, svBaseElevatorCond1 = 311
	, svBaseFireFl = 312
	, svSecondElevatorAvailableFl = 318
	, svField320 = 320
	, svEscapeLeftJailFl = 330
	, svField333 = 333
	, svField338 = 338
	, svField339 = 339
	, svField340 = 340
	, svField341 = 341
	, svField352 = 352
	, svField353 = 353
	, svField354 = 354
	, svField355 = 355
	, svField356 = 356
	, svField357 = 357
	, svField399 = 399
	, svField401 = 401
};

// As Script engine directly access savegame fields,
// refactoring it in separated fields properly named is impossible
struct Savegame {
	byte _data[2050];
	CharacterLocation _cloneHopkins;
	CharacterLocation _realHopkins;
	CharacterLocation _samantha;
	int16 _inventory[35];	// Originally at offset 1300 of data array
	int16 _mapCarPosX;
	int16 _mapCarPosY;

	void reset() {
		for (uint16 i = 0; i < ARRAYSIZE(_data); i++) {
			_data[i] = 0;
		}
		_cloneHopkins.reset();
		_realHopkins.reset();
		_samantha.reset();
		for (uint16 i = 0; i < ARRAYSIZE(_inventory); i++) {
			_inventory[i] = 0;
		}
		_mapCarPosX = 0;
		_mapCarPosY = 0;
	}

	Savegame() { reset(); }
};

struct CreditItem {
	bool _actvFl;
	int _color;
	int _linePosY;
	int _lineSize;
	byte _line[50];
};

enum Language { LANG_EN = 0, LANG_FR = 1, LANG_SP = 2};

enum PlayerCharacter { CHARACTER_HOPKINS = 0, CHARACTER_HOPKINS_CLONE = 1, CHARACTER_SAMANTHA = 2 };

enum Directions {
	DIR_NONE = -1,
	DIR_UP = 1,
	DIR_UP_RIGHT = 2,
	DIR_RIGHT = 3,
	DIR_DOWN_RIGHT = 4,
	DIR_DOWN = 5,
	DIR_DOWN_LEFT = 6,
	DIR_LEFT = 7,
	DIR_UP_LEFT = 8
};

enum EventMode {
	EVENTMODE_DEFAULT = 0,
	EVENTMODE_IGNORE = 1,
	EVENTMODE_CREDITS = 3,
	EVENTMODE_ALT = 4
};

class HopkinsEngine;

/**
 * Engine Globals
 */
class Globals {
private:
	HopkinsEngine *_vm;

public:
	bool _disableInventFl;
	bool _cityMapEnabledFl;
	bool _linuxEndDemoFl;
	bool _censorshipFl;
	bool _introSpeechOffFl;
	int _exitId;
	Directions _oceanDirection;
	int _actionDirection;
	int _inventory[36];
	int _screenId;
	int _prevScreenId;
	int _characterMaxPosY;
	int _baseMapColor;
	int _spriteSize[500];
	PlayerCharacter _characterType;
	uint _speed;
	byte *_answerBuffer;
	Savegame *_saveData;
	Language _language;
	HopkinsItem _hopkinsItem[70];

	CreditItem _creditsItem[200];
	int _creditsLineNumb;
	int _creditsStep;
	int _creditsPosY;
	int _creditsStartX;
	int _creditsEndX;
	int _creditsStartY;
	int _creditsEndY;

	int _menuSpeed;
	int _menuSoundOff;
	int _menuTextOff;
	int _menuVoiceOff;
	int _menuMusicOff;
	int _menuDisplayType;
	int _menuScrollSpeed;

	byte *_optionDialogSpr;
	bool _optionDialogFl;

	bool _actionMoveTo;
	bool _freezeCharacterFl;
	bool _checkDistanceFl;
	byte *_characterSpriteBuf;
	Common::String _zoneFilename;
	Common::String _textFilename;
	byte *_levelSpriteBuf;

	EventMode _eventMode;

	Globals(HopkinsEngine *vm);
	~Globals();
	byte *allocMemory(int count);
	byte *freeMemory(byte *p);
	void setConfig();
	void clearAll();
	void loadCharacterData();

	int _curRoomNum;
};

} // End of namespace Hopkins

#endif /* HOPKINS_GLOBALS_H */
