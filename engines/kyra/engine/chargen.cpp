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

#ifdef ENABLE_EOB

#include "kyra/engine/eobcommon.h"
#include "kyra/graphics/screen_eob_segacd.h"
#include "kyra/resource/resource.h"
#include "kyra/sound/sound_intern.h"

#include "common/savefile.h"
#include "common/str-array.h"

#include "common/config-manager.h"
#include "base/plugins.h"
#include "engines/metaengine.h"
#include "engines/game.h"

namespace Kyra {

// Character Generator

class CharacterGenerator {
public:
	CharacterGenerator(EoBCoreEngine *vm, Screen_EoB *screen);
	~CharacterGenerator();

	bool start(EoBCharacter *characters, const uint8 ***faceShapes, bool defaultParty);

private:
	void init(bool defaultParty);
	bool createCustomParty(const uint8 ***faceShapes);
	void createDefaultParty();
	void initButtonsFromList(int first, int numButtons);
	void initButton(int index, int x, int y, int w, int h, int keyCode);
	void checkForCompleteParty();
	void drawButton(int index, int buttonState, int pageNum);
	void processButtonClick(int index);
	int viewDeleteCharacter();
	void createPartyMember();
	int raceSexMenu();
	int classMenu(int raceSex);
	int alignmentMenu(int cClass);
	int getInput(Button *buttonList);
	void updateMagicShapes();
	void generateStats(int index);
	void modifyMenu();
	void statsAndFacesMenu();
	void faceSelectMenu();
	int getNextFreeFaceShape(int shpIndex, int charSex, int step, int8 *selectedPortraits);
	void processFaceMenuSelection(int index);
	void printStats(int index, int mode);
	void processNameInput(int index, int textColor);
	int rollDice();
	int modifyStat(int index, int8 *stat1, int8 *stat2);
	int getMaxHp(int cclass, int constitution, int level1, int level2, int level3);
	int getMinHp(int cclass, int constitution, int level1, int level2, int level3);
	void finish();

	uint8 **_chargenMagicShapes;
	uint8 *_chargenButtonLabels[17];
	int _activeBox;
	int _magicShapesBox;
	int _updateBoxShapesIndex;
	int _lastUpdateBoxShapesIndex;
	uint32 _chargenMagicShapeTimer;
	int8 _chargenSelectedPortraits[4];
	int8 _chargenSelectedPortraits2[4];

	uint16 _chargenMinStats[7];
	uint16 _chargenMaxStats[7];

	const uint8 _menuColor1, _menuColor2, _menuColor3;

	const char *const *_chargenStrings1;
	const char *const *_chargenStrings2;
	const char *const *_chargenStatStrings;
	const char *const *_chargenRaceSexStrings;
	const char *const *_chargenClassStrings;
	const char *const *_chargenAlignmentStrings;
	const char *const *_chargenEnterGameStrings;

	const uint8 *_chargenStartLevels;
	const uint8 *_chargenClassMinStats;
	const uint8 *_chargenRaceMinStats;
	const uint16 *_chargenRaceMaxStats;

	const char *const *_chargenDefaultNames;
	const uint8 *_chargenDefaultStats;

	const EoBChargenButtonDef *_chargenButtonDefs;

	static const EoBChargenButtonDef _chargenButtonDefsDOS[];
	static const uint16 _chargenButtonKeyCodesFMTOWNS[];
	static const CreatePartyModButton _chargenModButtons[];
	static const EoBRect8 _chargenButtonBodyCoords[];
	static const uint8 _chargenSegaButtonCoords[60];
	static const int16 _chargenBoxX[];
	static const int16 _chargenBoxY[];
	static const int16 _chargenNameFieldX[];
	static const int16 _chargenNameFieldY[];

	static const int32 _classMenuMasks[];
	static const int32 _alignmentMenuMasks[];

	static const int16 _raceModifiers[];

	EoBCharacter *_characters;
	const uint8 **_faceShapes;

	uint8 *_wndBackgrnd;

	EoBCoreEngine *_vm;
	Screen_EoB *_screen;
};

CharacterGenerator::CharacterGenerator(EoBCoreEngine *vm, Screen_EoB *screen) : _vm(vm), _screen(screen),
	_characters(0), _faceShapes(0), _chargenMagicShapes(0), _chargenMagicShapeTimer(0), _wndBackgrnd(0),
	_updateBoxShapesIndex(0), _lastUpdateBoxShapesIndex(0), _magicShapesBox(6), _activeBox(0),
	_menuColor1(vm->gameFlags().platform == Common::kPlatformSegaCD ? 0xFF : (vm->_configRenderMode == Common::kRenderCGA ? 1 : vm->guiSettings()->colors.guiColorWhite)),
	_menuColor2(vm->gameFlags().platform == Common::kPlatformSegaCD ? 0x55 : vm->guiSettings()->colors.guiColorLightRed),
	_menuColor3(vm->gameFlags().platform == Common::kPlatformSegaCD ? 0x99 : vm->guiSettings()->colors.guiColorBlack) {

	_chargenStatStrings = _vm->_chargenStatStrings;
	_chargenRaceSexStrings = _vm->_chargenRaceSexStrings;
	_chargenClassStrings = _vm->_chargenClassStrings;
	_chargenAlignmentStrings = _vm->_chargenAlignmentStrings;

	memset(_chargenSelectedPortraits, -1, sizeof(_chargenSelectedPortraits));
	memset(_chargenSelectedPortraits2, 0, sizeof(_chargenSelectedPortraits2));
	memset(_chargenMinStats, 0, sizeof(_chargenMinStats));
	memset(_chargenMaxStats, 0, sizeof(_chargenMaxStats));
	memset(_chargenButtonLabels, 0, sizeof(_chargenButtonLabels));

	int temp;
	_chargenStrings1 = _vm->staticres()->loadStrings(kEoBBaseChargenStrings1, temp);
	_chargenStrings2 = _vm->staticres()->loadStrings(kEoBBaseChargenStrings2, temp);
	_chargenStartLevels = _vm->staticres()->loadRawData(kEoBBaseChargenStartLevels, temp);
	_chargenEnterGameStrings = _vm->staticres()->loadStrings(kEoBBaseChargenEnterGameStrings, temp);
	_chargenClassMinStats = _vm->staticres()->loadRawData(kEoBBaseChargenClassMinStats, temp);
	_chargenRaceMinStats = _vm->staticres()->loadRawData(kEoBBaseChargenRaceMinStats, temp);
	_chargenRaceMaxStats = _vm->staticres()->loadRawDataBe16(kEoBBaseChargenRaceMaxStats, temp);
	_chargenDefaultNames = _vm->staticres()->loadStrings(kEoB1DefaultPartyNames, temp);
	_chargenDefaultStats = _vm->staticres()->loadRawData(kEoB1DefaultPartyStats, temp);

	EoBChargenButtonDef *chargenButtonDefs = new EoBChargenButtonDef[41];
	memcpy(chargenButtonDefs, _chargenButtonDefsDOS, 41 * sizeof(EoBChargenButtonDef));

	if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
		const uint16 *c = _chargenButtonKeyCodesFMTOWNS;
		for (int i = 0; i < 41; ++i) {
			if (chargenButtonDefs[i].keyCode)
				chargenButtonDefs[i].keyCode = *c++;
		}
	}

	if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
		// Adjust modify menu buttons for SegaCD
		for (int i = 31; i < 38; ++i)
			chargenButtonDefs[i].y += 8;
		chargenButtonDefs[37].x--;
	}

	_chargenButtonDefs = chargenButtonDefs;
}

CharacterGenerator::~CharacterGenerator() {
	_vm->_gui->updateBoxFrameHighLight(-1);

	if (_chargenMagicShapes) {
		for (int i = 0; i < 10; i++)
			delete[] _chargenMagicShapes[i];
		delete[] _chargenMagicShapes;
	}

	for (int i = 0; i < 17; i++)
		delete[] _chargenButtonLabels[i];

	delete[] _chargenButtonDefs;
	delete[] _wndBackgrnd;
	_vm->_wndBackgrnd = 0;

	_screen->clearPage(2);
}

bool CharacterGenerator::start(EoBCharacter *characters, const uint8 ***faceShapes, bool defaultParty) {
	if (!characters || !faceShapes) {
		warning("CharacterGenerator::start(): Called without character data");
		return true;
	}

	_characters = characters;
	_faceShapes = *faceShapes;

	_vm->snd_stopSound();
	_vm->delay(_vm->_tickLength);

	init(defaultParty);

	if (defaultParty)
		createDefaultParty();
	else if (!createCustomParty(faceShapes))
		return false;

	if (!_vm->shouldQuit()) {
		if (!defaultParty)
			processButtonClick(15);
		finish();
	}

	if (_vm->game() == GI_EOB2)
		_vm->snd_fadeOut();

	*faceShapes = _faceShapes;
	return true;
}

void CharacterGenerator::init(bool defaultParty) {
	if (_faceShapes) {
		for (int i = 0; i < 44; i++)
			delete[] _faceShapes[i];
		delete[] _faceShapes;
	}

	_faceShapes = new const uint8 *[44];
	if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
		uint8 *in = _vm->resource()->fileData("FACE", 0);
		_screen->sega_encodeShapesFromSprites(_faceShapes, in, 44, 32, 32, 3);
		delete[] in;
	} else {
		_screen->loadShapeSetBitmap("CHARGENA", 5, 3);
		for (int i = 0; i < 44; i++)
			_faceShapes[i] = _screen->encodeShape((i % 10) << 2, (i / 10) << 5, 4, 32, true, _vm->_cgaMappingDefault);
	}
	_screen->_curPage = 0;

	if (_vm->gameFlags().platform == Common::kPlatformAmiga || (_vm->game() == GI_EOB1 && _vm->gameFlags().platform == Common::kPlatformPC98))
		_screen->fadeToBlack(32);

	// If we start with a default party we only need the face shapes
	if (defaultParty)
		return;

	if (_chargenMagicShapes) {
		for (int i = 0; i < 10; i++)
			delete[] _chargenMagicShapes[i];
		delete[] _chargenMagicShapes;
	}

	if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
		_screen->sega_fadeToBlack(0);
		_screen->sega_selectPalette(13, 0);
		_screen->sega_selectPalette(14, 1);
		_screen->sega_selectPalette(15, 2);
		_screen->sega_selectPalette(15, 3);
		_vm->_txt->clearDim(1);

		Common::SeekableReadStream *in = _vm->resource()->createReadStream("CHARGEN");
		_screen->sega_getRenderer()->loadStreamToVRAM(in, 0x20);
		_wndBackgrnd = new uint8[10240];
		_vm->_wndBackgrnd = _wndBackgrnd;
		for (int i = 0; i < 16; ++i) {
			in->seek(((8 + i) * 40 + 18) << 5);
			in->read(&_wndBackgrnd[i * 640], 640);
		}
		delete in;

		uint8 *cgb = _vm->resource()->fileData("CGBUTTON", 0);
		_screen->sega_getRenderer()->loadToVRAM(&cgb[0x21E0], 0x1400, 0x8220);
		for (int i = 0; i < 10; i++)
			_screen->sega_getRenderer()->fillRectWithTiles(1, i << 2, 0, 4, 4, 0x4411 + (i << 4), true);
		_screen->sega_getRenderer()->render(2);
		_screen->_curPage = 2;
		_chargenMagicShapes = new uint8*[10];
		for (int i = 0; i < 10; i++)
			_chargenMagicShapes[i] = _screen->encodeShape(i << 2, 0, 4, 32);
		_screen->_curPage = 0;

		_screen->sega_getRenderer()->loadToVRAM(&cgb[0], 0x1900, 0x8220);
		_screen->sega_getRenderer()->loadToVRAM(&cgb[0x1900], 0x8E0, 0xB080);
		delete[] cgb;

		_screen->sega_getRenderer()->fillRectWithTiles(0, 0, 0, 40, 28, 0);
		_screen->sega_getRenderer()->fillRectWithTiles(1, 0, 0, 40, 28, 0);
		_screen->sega_getRenderer()->fillRectWithTiles(1, 0, 0, 40, 26, 1, true);
		_screen->sega_getRenderer()->render(0);

		for (int i = 0; i < 4; ++i)
			_screen->copyRegion(_chargenBoxX[i], _chargenBoxY[i] + 1, i << 5, 128, 32, 32, 0, 2, Screen::CR_NO_P_CHECK);

		_screen->sega_fadeToNeutral(4);

	} else {
		_screen->loadEoBBitmap((_vm->game() == GI_EOB1 && _vm->_flags.lang == Common::ES_ESP) ? "CCARGEN" : "CHARGEN", _vm->_cgaMappingDefault, 5, 3, 0);
		_screen->selectPC98Palette(4, _screen->getPalette(0));

		if (_vm->gameFlags().platform == Common::kPlatformAmiga || (_vm->game() == GI_EOB1 && _vm->gameFlags().platform == Common::kPlatformPC98))
			_screen->fadeFromBlack(32);

		_screen->loadShapeSetBitmap((_vm->game() == GI_EOB1 && _vm->_flags.lang == Common::ES_ESP) ? "CCARGENB" : "CHARGENB", 5, 3);
		_chargenMagicShapes = new uint8*[10];
		for (int i = 0; i < 10; i++)
			_chargenMagicShapes[i] = _screen->encodeShape(i << 2, 0, 4, 32, true, _vm->_cgaMappingDefault);

		for (int i = 0; i < 17; i++) {
			const CreatePartyModButton *c = &_chargenModButtons[i];
			_chargenButtonLabels[i] = c->labelW?_screen->encodeShape(c->encodeLabelX, c->encodeLabelY, c->labelW, c->labelH, true, _vm->_cgaMappingDefault):0;
		}

		_screen->convertPage(3, 2, _vm->_cgaMappingDefault);
	}

	_screen->setScreenDim(2);
	_screen->_curPage = 0;
	_screen->convertToHiColor(2);
	_screen->shadeRect(142, 63, 306, 193, 4);
	_screen->copyRegion(144, 64, 0, 0, 180, 128, 0, 2, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
}

bool CharacterGenerator::createCustomParty(const uint8 ***faceShapes) {
	checkForCompleteParty();
	initButtonsFromList(0, 5);

	_vm->snd_playSong(_vm->game() == GI_EOB1 ? (_vm->gameFlags().platform == Common::kPlatformPC98 ? 1 : (_vm->gameFlags().platform == Common::kPlatformSegaCD ? 8: 20)) : 13);
	_activeBox = 0;

	for (bool loop = true; loop && (!_vm->shouldQuit());) {
		_vm->_gui->updateBoxFrameHighLight(_activeBox + 6);
		int inputFlag = getInput(_vm->_activeButtons);
		_vm->removeInputTop();

		if (inputFlag) {
			if (inputFlag == _vm->_keyMap[Common::KEYCODE_LEFT] || inputFlag == _vm->_keyMap[Common::KEYCODE_RIGHT]) {
				_activeBox ^= 1;
			} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_UP] || inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN]) {
				_activeBox ^= 2;
			} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE]) {
				// Unlike the original we allow returning to the main menu if no character has been created yet or all characters have been deleted
				if (!_characters[0].name[0] && !_characters[1].name[0] && !_characters[2].name[0] && !_characters[3].name[0]) {
					_vm->snd_stopSound();
					*faceShapes = _faceShapes;
					return false;
				}
			}
			_vm->_gui->updateBoxFrameHighLight(-1);
		}

		if (inputFlag & 0x8000) {
			inputFlag = (inputFlag & 0x0F) - 1;
			if (inputFlag == 4) {
				loop = false;
			} else {
				_activeBox = inputFlag;
				inputFlag = _vm->_keyMap[Common::KEYCODE_RETURN];
			}
		}

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN] || inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP5]) {
			_vm->_gui->updateBoxFrameHighLight(-1);
			if (_characters[_activeBox].name[0]) {
				int b = _activeBox;
				if (viewDeleteCharacter())
					loop = false;
				if (b != _activeBox && !_characters[_activeBox].name[0])
					createPartyMember();
			} else {
				createPartyMember();
			}

			initButtonsFromList(0, 5);
			checkForCompleteParty();
		}

		if (loop == false) {
			for (int i = 0; i < 4; i++) {
				if (!_characters[i].name[0])
					loop = true;
			}
		}
	}

	return true;
}

void CharacterGenerator::createDefaultParty() {
	assert(_chargenDefaultNames);
	assert(_chargenDefaultStats);
	const uint8 *pos = _chargenDefaultStats;
	for (int i = 0; i < 4; ++i) {
		EoBCharacter &c = _characters[i];
		c.raceSex = *pos++;
		c.cClass = *pos++;
		c.alignment = *pos++;
		generateStats(i);
		c.portrait = *pos++;
		c.faceShape = _faceShapes[c.portrait];
		c.strengthCur = *pos++;
		c.intelligenceCur = *pos++;
		c.wisdomCur = *pos++;
		c.dexterityCur = *pos++;
		c.constitutionCur = *pos++;
		c.charismaCur = *pos++;
		c.armorClass = *pos++;
		c.hitPointsCur = *pos++;
		Common::strlcpy(c.name, _chargenDefaultNames[i], 11);
	}
}

void CharacterGenerator::initButtonsFromList(int first, int numButtons) {
	_vm->gui_resetButtonList();

	for (int i = 0; i < numButtons; i++) {
		const EoBChargenButtonDef *e = &_chargenButtonDefs[first + i];
		initButton(i, e->x, e->y, e->w, e->h, e->keyCode);
	}

	_vm->gui_notifyButtonListChanged();
}

void CharacterGenerator::initButton(int index, int x, int y, int w, int h, int keyCode) {
	Button *b = 0;
	int cnt = 1;

	if (_vm->_activeButtons) {
		Button *n = _vm->_activeButtons;
		while (n->nextButton) {
			++cnt;
			n = n->nextButton;
		}

		++cnt;
		b = n->nextButton = &_vm->_activeButtonData[cnt];
	} else {
		b = &_vm->_activeButtonData[0];
		_vm->_activeButtons = b;
	}

	*b = Button();
	b->flags = 0x1100;
	b->data0Val2 = 12;
	b->data1Val2 = b->data2Val2 = 15;
	b->data3Val2 = 8;

	b->index = index + 1;
	b->x = x << 3;
	b->y = y;
	b->width = w;
	b->height = h;
	b->keyCode = keyCode;
	b->keyCode2 = keyCode | 0x100;
}

void CharacterGenerator::checkForCompleteParty() {
	_screen->copyRegion(0, 0, 160, 0, 160, 128, 2, 2, Screen::CR_NO_P_CHECK);
	int cp = _screen->setCurPage(2);
	int x = (_vm->gameFlags().platform == Common::kPlatformFMTowns) ? 184 : 168;
	int cs = 0;

	if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
		_screen->sega_loadTextBackground(_wndBackgrnd, 10240);
		_screen->sega_getRenderer()->fillRectWithTiles(0, 18, 8, 20, 16, 0);
		cs = _screen->setFontStyles(_screen->_currentFont, _vm->gameFlags().lang == Common::JA_JPN ? Font::kStyleNone : Font::kStyleFullWidth);
		_vm->_txt->printShadedText(_chargenStrings1[8], 0, 0, -1, 0x99);
	} else {
		_screen->printShadedText(_chargenStrings1[8], x, 16, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
		_screen->copyRegion(160, 0, 144, 64, 160, 128, 2, 0, Screen::CR_NO_P_CHECK);
	}
	_screen->setCurPage(cp);

	int numChars = 0;
	for (int i = 0; i < 4; i++) {
		if (_characters[i].name[0])
			numChars++;
	}

	if (numChars == 4) {
		if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
			_vm->_txt->printShadedText(_chargenStrings1[0], 0, 60, -1, 0x99);
		} else {
			_screen->setCurPage(2);
			_screen->printShadedText(_chargenStrings1[0], x, 61, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
			_screen->setCurPage(0);
			_screen->copyRegion(168, 61, 152, 125, 136, 40, 2, 0, Screen::CR_NO_P_CHECK);
		}
		drawButton(15, 0, 0);
	} else {
		drawButton(14, 0, 0);
	}

	if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
		_screen->setFontStyles(_screen->_currentFont, cs);
		_screen->sega_getRenderer()->render(0, 18, 8, 20, 16);
	}

	_screen->updateScreen();
}

void CharacterGenerator::drawButton(int index, int buttonState, int pageNum) {
	if (index >= 17)
		return;

	if (_vm->_flags.platform == Common::kPlatformSegaCD && index > 3) {
		static const int8 buttonOrder[] = { -1, -1, -1, -1, 9, 10, 8, 6, 7, 11, 4, 5, 2, 3, -2, 0, 1 };
		index = buttonOrder[index];
		if (index < 0) {
			if (index == -2) {
				// The original SegaCD version doesn't remove the 'PLAY' button here. We do it nonetheless,
				// since it seems weird to have the button still there, even when it is dysfunctional.
				_screen->sega_getRenderer()->fillRectWithTiles(0, 3, 23, 11, 1, 0x39C, true);
				_screen->sega_getRenderer()->fillRectWithTiles(0, 3, 24, 11, 1, 0x3C4, true);
				_screen->sega_getRenderer()->render(0, 3, 23, 11, 2);
			}
			return;
		}
		const uint8 *bt = &_chargenSegaButtonCoords[index * 5];
		_screen->sega_getRenderer()->fillRectWithTiles(0, bt[0], bt[1], bt[2], bt[3], (index > 9 ? 0x24BC : 0x2411) + bt[4] + (buttonState ? (bt[2] * bt[3]) : 0), true);
		_screen->sega_getRenderer()->render(0, bt[0], bt[1], bt[2], bt[3]);
		_screen->updateScreen();
		return;
	}

	const CreatePartyModButton *c = &_chargenModButtons[index];
	const EoBRect8 *p = &_chargenButtonBodyCoords[c->bodyIndex + buttonState];

	int x2 = 20;
	int y2 = 0;

	if (pageNum) {
		x2 = c->destX + 2;
		y2 = c->destY - 64;
	}

	_screen->copyRegion(p->x << 3, p->y, x2 << 3, y2, p->w << 3, p->h, 2, 2, Screen::CR_NO_P_CHECK);
	if (c->labelW)
		_screen->drawShape(2, _chargenButtonLabels[index], (x2 << 3) + c->labelX, y2 + c->labelY, 0);

	if (pageNum == 2)
		return;

	_screen->copyRegion(160, 0, c->destX << 3, c->destY, p->w << 3, p->h, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
}

void CharacterGenerator::processButtonClick(int index) {
	drawButton(index, 1, 0);
	if (!(_vm->game() == GI_EOB1 && _vm->_flags.platform == Common::kPlatformPC98))
		_vm->snd_playSoundEffect(76);
	_vm->_system->delayMillis(80);
	drawButton(index, 0, 0);
}

int CharacterGenerator::viewDeleteCharacter() {
	initButtonsFromList(0, 7);
	_vm->removeInputTop();

	_vm->_gui->updateBoxFrameHighLight(-1);
	printStats(_activeBox, 2);

	int res = 0;
	for (bool loop = true; loop && _characters[_activeBox].name[0] && !_vm->shouldQuit();) {
		_vm->_gui->updateBoxFrameHighLight(_activeBox + 6);
		int inputFlag = getInput(_vm->_activeButtons);
		int cbx = _activeBox;
		_vm->removeInputTop();

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN] || inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP5] || inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE]) {
			processButtonClick(9);
			res = 0;
			loop = false;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_LEFT] || inputFlag == _vm->_keyMap[Common::KEYCODE_RIGHT]) {
			cbx ^= 1;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_UP] || inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN]) {
			cbx ^= 2;
		}

		if (inputFlag & 0x8000) {
			inputFlag = (inputFlag & 0x0F) - 1;
			if (inputFlag == 4) {
				res = 1;
				loop = false;
			} else if (inputFlag == 5) {
				processButtonClick(9);
				res = 0;
				loop = false;
			} else if (inputFlag == 6) {
				if (_characters[_activeBox].name[0]) {
					processButtonClick(16);
					_characters[_activeBox].name[0] = 0;
					_characters[_activeBox].faceShape = 0;
					if (_vm->_flags.platform == Common::kPlatformSegaCD) {
						_screen->sega_getRenderer()->memsetVRAM((((_chargenBoxY[_activeBox] + 41) >> 3) * 40 + (_chargenBoxX[_activeBox] >> 3)) << 5, 0, 224);
						_screen->sega_getRenderer()->render(0, (_chargenBoxX[_activeBox] >> 3) - 1, (_chargenBoxY[_activeBox] + 41) >> 3, 7, 1);
					} else {
						processNameInput(_activeBox, _vm->guiSettings()->colors.guiColorBlack);
					}
					processFaceMenuSelection(_activeBox + 50);
				}
			} else {
				cbx = inputFlag;
			}
		}

		if (loop == false)
			_vm->_gui->updateBoxFrameHighLight(-1);

		if (!_characters[cbx].name[0])
			loop = false;

		if (cbx != _activeBox) {
			_activeBox = cbx;
			_vm->_gui->updateBoxFrameHighLight(-1);
			if (loop)
				printStats(_activeBox, 2);
		}
	}

	return res;
}

void CharacterGenerator::createPartyMember() {
	_screen->setScreenDim(2);
	assert(_vm->_gui);

	for (int i = 0; i != 3 && !_vm->shouldQuit(); i++) {
		bool bck = false;

		switch (i) {
		case 0:
			_characters[_activeBox].raceSex = raceSexMenu();
			break;
		case 1:
			_characters[_activeBox].cClass = classMenu(_characters[_activeBox].raceSex);
			if (_characters[_activeBox].cClass == _vm->_keyMap[Common::KEYCODE_ESCAPE])
				bck = true;
			break;
		case 2:
			_characters[_activeBox].alignment = alignmentMenu(_characters[_activeBox].cClass);
			if (_characters[_activeBox].alignment == _vm->_keyMap[Common::KEYCODE_ESCAPE])
				bck = true;
			break;
		default:
			break;
		}

		if (bck)
			i -= 2;
	};

	if (!_vm->shouldQuit()) {
		generateStats(_activeBox);
		statsAndFacesMenu();

		for (_characters[_activeBox].name[0] = 0; _characters[_activeBox].name[0] == 0 && !_vm->shouldQuit();) {
			processFaceMenuSelection(_chargenMinStats[6]);
			printStats(_activeBox, 0);
			if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
				_screen->sega_loadTextBackground(_wndBackgrnd, 10240);
				_vm->_txt->printShadedText(_chargenStrings2[11], 0, 0);
				_screen->sega_getRenderer()->render(0, 18, 8, 20, 2);
				if (!_vm->shouldQuit())
					_vm->_gui->getTextInput(_characters[_activeBox].name, (_chargenBoxX[_activeBox] >> 3) - 1, _chargenBoxY[_activeBox] + 41, 7, 0xFF, 0x00, 0xFF);
			} else {
				_screen->printShadedText(_chargenStrings2[11], 149, 100, _vm->guiSettings()->colors.guiColorLightBlue, 0, _vm->guiSettings()->colors.guiColorBlack);
				if (!_vm->shouldQuit()) {
					_vm->_gui->getTextInput(_characters[_activeBox].name, 24, 100, 10, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorDarkRed);
					processNameInput(_activeBox, _vm->guiSettings()->colors.guiColorBlue);
				}
			}
		}
	}
}

int CharacterGenerator::raceSexMenu() {
	_screen->drawBox(_chargenBoxX[_activeBox], _chargenBoxY[_activeBox], _chargenBoxX[_activeBox] + 32, _chargenBoxY[_activeBox] + 33, _vm->guiSettings()->colors.guiColorBlack);
	_screen->copyRegion(0, 0, 144, 64, 160, 128, 2, 0, Screen::CR_NO_P_CHECK);

	if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
		_screen->sega_loadTextBackground(_wndBackgrnd, 10240);
		_screen->sega_getRenderer()->fillRectWithTiles(0, 18, 8, 20, 16, 0);
		_vm->_txt->printShadedText(_chargenStrings2[8], 0, 0, -1, 0x99);
	} else {
		_screen->printShadedText(_chargenStrings2[8], 147, 67, _vm->guiSettings()->colors.guiColorLightBlue, 0, _vm->guiSettings()->colors.guiColorBlack);
	}
	_vm->removeInputTop();

	_vm->_gui->simpleMenu_setup(1, 0, _chargenRaceSexStrings, -1, 0, 0, _menuColor1, _menuColor2, _menuColor3);
	if (_vm->_flags.platform == Common::kPlatformSegaCD)
		_screen->sega_getRenderer()->render(0, 18, 8, 20, 16);
	_screen->updateScreen();
	int16 res = -1;

	while (res == -1 && !_vm->shouldQuit()) {
		res = _vm->_gui->simpleMenu_process(1, _chargenRaceSexStrings, 0, -1, 0);
		if (_vm->_flags.platform == Common::kPlatformSegaCD)
			_screen->sega_getRenderer()->render(0, 18, 8, 20, 16);
		_screen->updateScreen();
		updateMagicShapes();
	}

	return res;
}

int CharacterGenerator::classMenu(int raceSex) {
	int32 itemsMask = -1;

	for (int i = 0; i < 4; i++) {
		// check for evil characters
		if (_characters[i].name[0] && _characters[i].alignment > 5)
			itemsMask = 0xFFFB;
	}

	_vm->removeInputTop();
	updateMagicShapes();

	_screen->copyRegion(0, 0, 144, 64, 160, 128, 2, 0, Screen::CR_NO_P_CHECK);
	if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
		_screen->sega_loadTextBackground(_wndBackgrnd, 10240);
		_screen->sega_getRenderer()->fillRectWithTiles(0, 18, 8, 20, 16, 0);
		_vm->_txt->printShadedText(_chargenStrings2[9], 0, 0, -1, 0x99);
	} else {
		_screen->printShadedText(_chargenStrings2[9], 147, 67, _vm->guiSettings()->colors.guiColorLightBlue, 0, _vm->guiSettings()->colors.guiColorBlack);
	}
	drawButton(5, 0, 0);

	itemsMask &= _classMenuMasks[raceSex / 2];
	_vm->_gui->simpleMenu_setup(2, 15, _chargenClassStrings, itemsMask, 0, 0, _menuColor1, _menuColor2, _menuColor3);
	if (_vm->_flags.platform == Common::kPlatformSegaCD)
		_screen->sega_getRenderer()->render(0, 18, 8, 20, 16);
	_screen->updateScreen();

	_vm->_mouseX = _vm->_mouseY = 0;
	int16 res = -1;

	while (res == -1 && !_vm->shouldQuit()) {
		updateMagicShapes();
		int in = getInput(0) & 0xFF;
		Common::Point mp = _vm->getMousePos();

		if (in == _vm->_keyMap[Common::KEYCODE_ESCAPE] || _vm->_gui->_menuLastInFlags == _vm->_keyMap[Common::KEYCODE_ESCAPE] || _vm->_gui->_menuLastInFlags == _vm->_keyMap[Common::KEYCODE_b]) {
			res = _vm->_keyMap[Common::KEYCODE_ESCAPE];
		} else if (_vm->posWithinRect(mp.x, mp.y, 264, 171, 303, 187)) {
			if (in == 199 || in == 201)
				res = _vm->_keyMap[Common::KEYCODE_ESCAPE];
			else
				_vm->removeInputTop();
		} else {
			res = _vm->_gui->simpleMenu_process(2, _chargenClassStrings, 0, itemsMask, 0);
			if (_vm->_flags.platform == Common::kPlatformSegaCD)
				_screen->sega_getRenderer()->render(0, 18, 8, 20, 16);
			_screen->updateScreen();
		}
	}

	_vm->removeInputTop();

	if (res == _vm->_keyMap[Common::KEYCODE_ESCAPE])
		processButtonClick(5);

	return res;
}

int CharacterGenerator::alignmentMenu(int cClass) {
	int32 itemsMask = -1;

	for (int i = 0; i < 4; i++) {
		// check for paladins
		if (_characters[i].name[0] && _characters[i].cClass == 2)
			itemsMask = 0xFE3F;
	}

	_vm->removeInputTop();
	updateMagicShapes();

	_screen->copyRegion(0, 0, 144, 64, 160, 128, 2, 0, Screen::CR_NO_P_CHECK);
	if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
		_screen->sega_loadTextBackground(_wndBackgrnd, 10240);
		_screen->sega_getRenderer()->fillRectWithTiles(0, 18, 8, 20, 16, 0);
		_vm->_txt->printShadedText(_chargenStrings2[10], 0, 0, -1, 0x99);
	} else {
		_screen->printShadedText(_chargenStrings2[10], 147, 67, _vm->guiSettings()->colors.guiColorLightBlue, 0, _vm->guiSettings()->colors.guiColorBlack);
	}

	drawButton(5, 0, 0);

	itemsMask &= _alignmentMenuMasks[cClass];
	_vm->_gui->simpleMenu_setup(3, 9, _chargenAlignmentStrings, itemsMask, 0, 0, _menuColor1, _menuColor2, _menuColor3);
	if (_vm->_flags.platform == Common::kPlatformSegaCD)
		_screen->sega_getRenderer()->render(0, 18, 8, 20, 16);
	_screen->updateScreen();

	_vm->_mouseX = _vm->_mouseY = 0;
	int16 res = -1;

	while (res == -1 && !_vm->shouldQuit()) {
		updateMagicShapes();
		int in = getInput(0) & 0xFF;
		Common::Point mp = _vm->getMousePos();

		if (in == _vm->_keyMap[Common::KEYCODE_ESCAPE] || _vm->_gui->_menuLastInFlags == _vm->_keyMap[Common::KEYCODE_ESCAPE] || _vm->_gui->_menuLastInFlags == _vm->_keyMap[Common::KEYCODE_b]) {
			res = _vm->_keyMap[Common::KEYCODE_ESCAPE];
		} else if (_vm->posWithinRect(mp.x, mp.y, 264, 171, 303, 187)) {
			if (in == 199 || in == 201)
				res = _vm->_keyMap[Common::KEYCODE_ESCAPE];
			else
				_vm->removeInputTop();
		} else {
			res = _vm->_gui->simpleMenu_process(3, _chargenAlignmentStrings, 0, itemsMask, 0);
			if (_vm->_flags.platform == Common::kPlatformSegaCD)
				_screen->sega_getRenderer()->render(0, 18, 9, 20, 16);
			_screen->updateScreen();
		}
	}

	_vm->removeInputTop();

	if (res == _vm->_keyMap[Common::KEYCODE_ESCAPE])
		processButtonClick(5);

	return res;
}

int CharacterGenerator::getInput(Button *buttonList) {
	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		return _vm->checkInput(buttonList, false, 0);

	if (_vm->game() == GI_EOB1 && _vm->sound()->checkTrigger()) {
		_vm->sound()->resetTrigger();
		_vm->snd_playSong(20);
	} else if (_vm->game() == GI_EOB2 && !_vm->sound()->isPlaying()) {
		// WORKAROUND for EOB II: The original implements the same sound trigger check as in EOB I.
		// However, Westwood seems to have forgotten to set the trigger at the end of the AdLib song,
		// so that the music will not loop. We simply check whether the sound driver is still playing.
		_vm->delay(3 * _vm->_tickLength);
		_vm->snd_playSong(13);
	}

	return _vm->checkInput(buttonList, false, 0);
}

void CharacterGenerator::updateMagicShapes() {
	if (_magicShapesBox != _activeBox) {
		_chargenMagicShapeTimer = 0;
		_magicShapesBox = _activeBox;
	}

	if (_chargenMagicShapeTimer < _vm->_system->getMillis()) {
		if (++_updateBoxShapesIndex > 9)
			_updateBoxShapesIndex = 0;
		_chargenMagicShapeTimer = _vm->_system->getMillis() + 2 * _vm->_tickLength;
	}

	if (_updateBoxShapesIndex == _lastUpdateBoxShapesIndex)
		return;

	_screen->copyRegion(_activeBox << 5, 128, 288, 128, 32, 32, 2, 2, Screen::CR_NO_P_CHECK);
	_screen->drawShape(2, _chargenMagicShapes[_updateBoxShapesIndex], 288, 128, 0);
	_screen->copyRegion(288, 128, _chargenBoxX[_activeBox], _chargenBoxY[_activeBox] + 1, 32, 32, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	_lastUpdateBoxShapesIndex = _updateBoxShapesIndex;
}

void CharacterGenerator::generateStats(int index) {
	EoBCharacter *c = &_characters[index];

	for (int i = 0; i < 3; i++) {
		c->level[i] = _chargenStartLevels[(c->cClass << 2) + i];
		c->experience[i] = (_vm->game() == GI_EOB2 ? 69000 : 5000) / _chargenStartLevels[(c->cClass << 2) + 3];
	}

	int rc = c->raceSex >> 1;
	for (int i = 0; i < 6; i++) {
		_chargenMinStats[i] = MAX(_chargenClassMinStats[c->cClass * 6 + i], _chargenRaceMinStats[rc * 6 + i]);
		_chargenMaxStats[i] = _chargenRaceMaxStats[rc * 6 + i];
	}

	if (_vm->_charClassModifier[c->cClass])
		_chargenMaxStats[0] = 18;

	uint16 sv[6];
	for (int i = 0; i < 6; i++) {
		sv[i] = MAX<uint16>(rollDice() + _raceModifiers[rc * 6 + i], _chargenMinStats[i]);
		if (!i && sv[i] == 18)
			sv[i] |= (uint16)(_vm->rollDice(1, 100) << 8);
		if (sv[i] > _chargenMaxStats[i])
			sv[i] = _chargenMaxStats[i];
	}

	c->strengthCur = c->strengthMax = sv[0] & 0xFF;
	c->strengthExtCur = c->strengthExtMax = sv[0] >> 8;
	c->intelligenceCur = c->intelligenceMax = sv[1] & 0xFF;
	c->wisdomCur = c->wisdomMax = sv[2] & 0xFF;
	c->dexterityCur = c->dexterityMax = sv[3] & 0xFF;
	c->constitutionCur = c->constitutionMax = sv[4] & 0xFF;
	c->charismaCur = c->charismaMax = sv[5] & 0xFF;
	c->armorClass = 10 + _vm->getDexterityArmorClassModifier(sv[3] & 0xFF);
	c->hitPointsCur = 0;

	for (int l = 0; l < 3; l++) {
		for (int i = 0; i < c->level[l]; i++)
			c->hitPointsCur += _vm->generateCharacterHitpointsByLevel(index, 1 << l);
	}

	c->hitPointsMax = c->hitPointsCur;
}

void CharacterGenerator::modifyMenu() {
	_vm->removeInputTop();
	printStats(_activeBox, 3);

	EoBCharacter *c = &_characters[_activeBox];
	int8 hpLO = c->hitPointsCur;

	for (int i = 0; i >= 0 && i < 7;) {
		switch (i) {
		case 0:
			i = modifyStat(i, &c->strengthCur, &c->strengthExtCur);
			break;
		case 1:
			i = modifyStat(i, &c->intelligenceCur, 0);
			break;
		case 2:
			i = modifyStat(i, &c->wisdomCur, 0);
			break;
		case 3:
			i = modifyStat(i, &c->dexterityCur, 0);
			break;
		case 4:
			i = modifyStat(i, &c->constitutionCur, 0);
			break;
		case 5:
			i = modifyStat(i, &c->charismaCur, 0);
			break;
		case 6:
			hpLO = c->hitPointsCur;
			i = modifyStat(i, &hpLO, 0);
			c->hitPointsCur = hpLO;
			break;
		default:
			break;
		}

		if (i == -2 || _vm->shouldQuit())
			break;
		else if (i < 0)
			i = 6;
		i %= 7;

		printStats(_activeBox, 3);
	}

	printStats(_activeBox, 1);
}

void CharacterGenerator::statsAndFacesMenu() {
	faceSelectMenu();
	printStats(_activeBox, 1);
	initButtonsFromList(27, 4);
	_vm->removeInputTop();
	int in = 0;

	while (!in && !_vm->shouldQuit()) {
		updateMagicShapes();
		in = getInput(_vm->_activeButtons);
		_vm->removeInputTop();

		if (in == 0x8001) {
			processButtonClick(4);
			updateMagicShapes();
			generateStats(_activeBox);
			in = -1;
		} else if (in == 0x8002) {
			processButtonClick(7);
			modifyMenu();
			in = -1;
		} else if (in == 0x8003) {
			processButtonClick(8);
			faceSelectMenu();
			in = -1;
		} else if (in == 0x8004 || in == _vm->_keyMap[Common::KEYCODE_KP5]) {
			processButtonClick(6);
			in = 1;
		} else {
			in = 0;
		}

		if (in & 0x8000) {
			printStats(_activeBox, 1);
			initButtonsFromList(27, 4);
			in = 0;
		}
	}

	_vm->_gui->updateBoxFrameHighLight(6 + _activeBox);
	_vm->_gui->updateBoxFrameHighLight(-1);
}

void CharacterGenerator::faceSelectMenu() {
	int8 sp[4];
	memcpy(sp, _chargenSelectedPortraits2, sizeof(sp));
	_vm->removeInputTop();
	initButtonsFromList(21, 6);

	int charSex = _characters[_activeBox].raceSex % 2;
	int8 shp = charSex ? 26 : 0;

	printStats(_activeBox, 4);
	drawButton(12, 0, 0);
	drawButton(13, 0, 0);
	_vm->_gui->updateBoxFrameHighLight(-1);

	shp = getNextFreeFaceShape(shp, charSex, 1, _chargenSelectedPortraits);

	int res = -1;
	int box = 1;

	while (res == -1 && !_vm->shouldQuit()) {
		int8 shpOld = shp;

		for (int i = 0; i < 4; i++) {
			sp[i] = shp;
			_screen->drawShape(0, _faceShapes[sp[i]], 176 + (i << 5), 66, 0);
			shp = getNextFreeFaceShape(shp + 1, charSex, 1, _chargenSelectedPortraits);
		}

		shp = shpOld;
		int in = 0;

		while (!in && !_vm->shouldQuit()) {
			updateMagicShapes();
			in = getInput(_vm->_activeButtons);
			_vm->removeInputTop();

			_vm->_gui->updateBoxFrameHighLight(box + 10);

			if (in == 0x8002 || in == _vm->_keyMap[Common::KEYCODE_RIGHT]) {
				processButtonClick(13);
				in = 2;
			} else if (in > 0x8002 && in < 0x8007) {
				box = (in & 7) - 3;
				in = 3;
			} else if (in == 0x8001 || in == _vm->_keyMap[Common::KEYCODE_LEFT]) {
				processButtonClick(12);
				in = 1;
			} else if (in == _vm->_keyMap[Common::KEYCODE_RETURN] || in == _vm->_keyMap[Common::KEYCODE_KP5]) {
				in = 3;
			} else if (in & 0x8000) {
				in &= 0xFF;
			} else {
				in = 0;
			}
		}

		_vm->_gui->updateBoxFrameHighLight(-1);

		if (in == 1)
			shp = getNextFreeFaceShape(shp - 1, charSex, -1, _chargenSelectedPortraits);
		else if (in == 2)
			shp = getNextFreeFaceShape(shp + 1, charSex, 1, _chargenSelectedPortraits);
		else if (in == 3)
			res = box;
	}

	if (!_vm->shouldQuit()) {
		_vm->_gui->updateBoxFrameHighLight(-1);
		updateMagicShapes();

		_chargenSelectedPortraits[_activeBox] = sp[res];
		_characters[_activeBox].portrait = sp[res];
		_characters[_activeBox].faceShape = _faceShapes[sp[res]];

		printStats(_activeBox, 1);
	}
}

int CharacterGenerator::getNextFreeFaceShape(int shpIndex, int charSex, int step, int8 *selectedPortraits) {
	int shpCur = ((shpIndex < 0) ? 43 : shpIndex) % 44;
	bool notUsable = false;

	do {
		notUsable = false;
		for (int i = 0; i < 4; i++) {
			if (_characters[i].name[0] && selectedPortraits[i] == shpCur)
				notUsable = true;
		}

		if ((charSex && (shpCur < 26)) || (!charSex && (shpCur > 28)))
			notUsable = true;

		if (notUsable) {
			shpCur += step;
			shpCur = ((shpCur < 0) ? 43 : shpCur) % 44;
		}
	} while (notUsable);

	return shpCur;
}

void CharacterGenerator::processFaceMenuSelection(int index) {
	_vm->_gui->updateBoxFrameHighLight(-1);
	if (index <= 48)
		_screen->drawShape(0, _characters[_activeBox].faceShape, _chargenBoxX[_activeBox], _chargenBoxY[_activeBox] + 1, 0);
	else
		drawButton(index - 50, 0, 0);
}

void CharacterGenerator::printStats(int index, int mode) {
	EoBCharacter *c = &_characters[index];

	if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
		_screen->sega_loadTextBackground(_wndBackgrnd, 10240);
		_screen->sega_getRenderer()->fillRectWithTiles(0, 18, 8, 20, 16, 0);
	} else {
		_screen->copyRegion(0, 0, 160, 0, 160, 128, 2, 2, Screen::CR_NO_P_CHECK);
		_screen->_curPage = 2;
		if (mode != 4)
			_screen->drawShape(2, c->faceShape, 224, 2, 0);
	}

	Common::String str1 = Common::String::format(_chargenStrings1[3], _vm->getCharStrength(c->strengthCur, c->strengthExtCur, _vm->gameFlags().platform == Common::kPlatformSegaCD).c_str(), c->intelligenceCur, c->wisdomCur, c->dexterityCur, c->constitutionCur, c->charismaCur);
	Common::String str2 = Common::String::format(_chargenStrings1[4], c->armorClass, c->hitPointsMax);
	const char *lvlStr = c->level[2] ? _chargenStrings1[7] : (c->level[1] ? _chargenStrings1[6] : _chargenStrings1[5]);
	Common::String str3 = Common::String::format(lvlStr, c->level[0], c->level[1], c->level[2]);

	if (_vm->gameFlags().platform == Common::kPlatformSegaCD) {
		_vm->_txt->printShadedText(_chargenStatStrings[0], 0, 72);
		_vm->_txt->printShadedText(_chargenRaceSexStrings[c->raceSex], 80 - _screen->getTextWidth(_chargenRaceSexStrings[c->raceSex]) / 2, 48);
		_vm->_txt->printShadedText(_chargenClassStrings[c->cClass], 80 - _screen->getTextWidth(_chargenClassStrings[c->cClass]) / 2, 56);
		_vm->_txt->printShadedText(str1.c_str(), 32, 72);
		_vm->_txt->printShadedText(str2.c_str(), 112, 72);
		_vm->_txt->printShadedText(str3.c_str(), 120, 88);
	} else {
		_screen->printShadedText(c->name, 160 + ((160 - _screen->getTextWidth(c->name)) / 2), 35, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
		_screen->printShadedText(_chargenRaceSexStrings[c->raceSex], 160 + ((160 - _screen->getTextWidth(_chargenRaceSexStrings[c->raceSex])) / 2), 45, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
		_screen->printShadedText(_chargenClassStrings[c->cClass], 160 + ((160 - _screen->getTextWidth(_chargenClassStrings[c->cClass])) / 2), 54, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);

		for (int i = 0; i < 6; i++)
			_screen->printShadedText(_chargenStatStrings[i], 163, (i + 8) << 3, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);

		_screen->printShadedText(_chargenStrings1[2], 248, 64, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
		_screen->printShadedText(str1.c_str(), 192, 64, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
		_screen->printShadedText(str2.c_str(), 280, 64, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
		_screen->printShadedText(str3.c_str(), 280, 80, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
	}

	switch (mode) {
	case 1:
		drawButton(4, 0, 2);
		drawButton(7, 0, 2);
		drawButton(8, 0, 2);
		drawButton(6, 0, 2);
		break;

	case 2:
		drawButton(16, 0, 2);
		drawButton(9, 0, 2);
		break;

	case 3:
		drawButton(10, 0, 2);
		drawButton(11, 0, 2);
		drawButton(9, 0, 2);
		break;

	default:
		break;
	}

	if (_vm->_flags.platform == Common::kPlatformSegaCD) {
		_screen->sega_getRenderer()->render(0, 18, 8, 20, 16);
		if (mode != 4)
			_screen->drawShape(0, c->faceShape, 208, 66, 0);
	} else
		_screen->copyRegion(160, 0, 144, 64, 160, 128, 2, 0, Screen::CR_NO_P_CHECK);

	if (mode != 3)
		_screen->updateScreen();

	_screen->_curPage = 0;
}

void CharacterGenerator::processNameInput(int index, int textColor) {
	Screen::FontId of = _screen->setFont(_vm->_conFont);
	_screen->fillRect(_chargenNameFieldX[index], _chargenNameFieldY[index], _chargenNameFieldX[index] + 59, _chargenNameFieldY[index] + 5, _vm->guiSettings()->colors.guiColorBlack);
	_screen->setFont(_vm->_invFont1);
	int xOffs = (60 - _screen->getTextWidth(_characters[index].name)) >> 1;
	_screen->printText(_characters[index].name, _chargenNameFieldX[index] + xOffs, _chargenNameFieldY[index], textColor, 0);
	_screen->updateScreen();
	_screen->setFont(of);
}

int CharacterGenerator::rollDice() {
	int res = 0;
	int min = 10;

	for (int i = 0; i < 4; i++) {
		int d = _vm->rollDice(1, 6, 0);
		res += d;
		if (d < min)
			min = d;
	}

	res -= min;
	return res;
}

int CharacterGenerator::modifyStat(int index, int8 *stat1, int8 *stat2) {
	uint8 *s1 = (uint8 *)stat1;
	uint8 *s2 = (uint8 *)stat2;

	initButtonsFromList(31, 10);
	Button *b = _vm->gui_getButton(_vm->_activeButtons, index + 1);

	printStats(_activeBox, 3);
	_vm->removeInputTop();

	if (_vm->_flags.platform == Common::kPlatformSegaCD) {
		Common::String statStr = index ? Common::String::format("%02d", *s1) : _vm->getCharStrength(*s1, *s2, true);
		_vm->_txt->printShadedText(statStr.c_str(), b->x - 112, b->y - 64, 0x55);
		_screen->sega_getRenderer()->render(0, (b->x + 32) >> 3, b->y >> 3, 5, 1);
	} else {
		Common::String statStr = index ? Common::String::format("%d", *s1) : _vm->getCharStrength(*s1, *s2);
		_screen->copyRegion(b->x - 112, b->y - 64, b->x + 32, b->y, 40, b->height, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->printShadedText(statStr.c_str(), b->x + 32, b->y, _vm->guiSettings()->colors.guiColorLightRed, 0, _vm->guiSettings()->colors.guiColorBlack);
	}
	_screen->updateScreen();

	EoBCharacter *c = &_characters[_activeBox];

	int ci = index;
	uint8 v2 = s2 ? *s2 : 0;

	if (index == 6) {
		_chargenMaxStats[6] = getMaxHp(c->cClass, c->constitutionCur, c->level[0], c->level[1], c->level[2]);
		_chargenMinStats[6] = getMinHp(c->cClass, c->constitutionCur, c->level[0], c->level[1], c->level[2]);
	}

	for (bool loop = true; loop && !_vm->shouldQuit();) {
		uint8 v1 = *s1;
		updateMagicShapes();
		int inputFlag = getInput(_vm->_activeButtons);
		_vm->removeInputTop();

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_LEFT] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP4] || inputFlag == _vm->_keyMap[Common::KEYCODE_MINUS] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP_MINUS] || inputFlag == 0x8009) {
			processButtonClick(11);
			v1--;

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_RIGHT] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP6] || inputFlag == _vm->_keyMap[Common::KEYCODE_PLUS] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP_PLUS] || inputFlag == 0x8008) {
			processButtonClick(10);
			v1++;

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_UP] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP8]) {
			ci = (ci - 1) % 7;
			loop = false;

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP2]) {
			ci = (ci + 1) % 7;
			loop = false;

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_o] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP5] || inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE] || inputFlag == 0x800A) {
			processButtonClick(9);
			loop = false;
			ci = -2;

		} else if (inputFlag & 0x8000) {
			inputFlag = (inputFlag & 0x0F) - 1;
			if (index != inputFlag) {
				ci = inputFlag;
				loop = false;
			}
		}

		if (v1 == *s1)
			continue;

		if (!index) {
			while (v1 > 18) {
				v1--;
				v2++;
			}
			while (v2 > 0 && v1 < 18) {
				v1++;
				v2--;
			}

			v1 = CLIP<uint8>(v1, _chargenMinStats[index], _chargenMaxStats[index] & 0xFF);
			v2 = (v1 == 18 && _chargenMaxStats[index] >= 19) ? CLIP<uint8>(v2, 0, 100) : 0;
			if (s2)
				*s2 = v2;
			else
				error("CharacterGenerator::modifyStat:...");
		} else {
			v1 = CLIP<uint8>(v1, _chargenMinStats[index], _chargenMaxStats[index]);
		}

		*s1 = v1;

		if (index == 6)
			_characters[_activeBox].hitPointsMax = v1;

		bool hpChanged = false;
		bool acChanged = false;

		if (index == 4) {
			int oldVal = c->hitPointsCur;
			_chargenMaxStats[6] = getMaxHp(c->cClass, c->constitutionCur, c->level[0], c->level[1], c->level[2]);
			_chargenMinStats[6] = getMinHp(c->cClass, c->constitutionCur, c->level[0], c->level[1], c->level[2]);
			c->hitPointsMax = c->hitPointsCur = CLIP<int16>(c->hitPointsCur, _chargenMinStats[6], _chargenMaxStats[6]);
			hpChanged = c->hitPointsCur != oldVal;
		} else if (index == 3) {
			int oldVal = c->armorClass;
			c->armorClass = _vm->getDexterityArmorClassModifier(v1) + 10;
			acChanged = c->armorClass != oldVal;
		}

		if (_vm->_flags.platform == Common::kPlatformSegaCD) {
			Common::String statStr = index ? Common::String::format("%02d", *s1) : _vm->getCharStrength(*s1, *s2, true);
			printStats(_activeBox, 3);
			_vm->_txt->printShadedText(statStr.c_str(), b->x - 112, b->y - 64, 0x55);
			_screen->sega_getRenderer()->render(0, (b->x + 32) >> 3, b->y >> 3, 5, 1);
		} else {
			Common::String statStr = index ? Common::String::format("%d", *s1) : _vm->getCharStrength(*s1, *s2);
			_screen->copyRegion(b->x - 112, b->y - 64, b->x + 32, b->y, 40, b->height, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->printShadedText(statStr.c_str(), b->x + 32, b->y, _vm->guiSettings()->colors.guiColorLightRed, 0, _vm->guiSettings()->colors.guiColorBlack);
			if (hpChanged) {
				statStr = Common::String::format("%d", c->hitPointsCur);
				_screen->copyRegion(120, 72, 264, 136, 40, 8, 2, 0, Screen::CR_NO_P_CHECK);
				_screen->printShadedText(statStr.c_str(), 264, 136, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
			} else if (acChanged) {
				statStr = Common::String::format("%d", c->armorClass);
				_screen->copyRegion(120, 64, 264, 128, 40, 8, 2, 0, Screen::CR_NO_P_CHECK);
				_screen->printShadedText(statStr.c_str(), 264, 128, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
			}
		}

		_screen->updateScreen();

		if (loop == false) {
			if (_vm->_flags.platform == Common::kPlatformSegaCD) {
				printStats(_activeBox, 3);
			} else {
				Common::String statStr = index ? Common::String::format("%d", *s1) : _vm->getCharStrength(*s1, *s2);
				_screen->printText(statStr.c_str(), b->x + 32, b->y, _vm->guiSettings()->colors.guiColorWhite, 0);
			}
			_screen->updateScreen();
		}
	}

	return ci;
}

int CharacterGenerator::getMaxHp(int cclass, int constitution, int level1, int level2, int level3) {
	int res = 0;
	constitution = _vm->getClassAndConstHitpointsModifier(cclass, constitution);

	int m = _vm->getCharacterClassType(cclass, 0);
	if (m != -1)
		res = _vm->getModifiedHpLimits(m, constitution, level1, false);

	m = _vm->getCharacterClassType(cclass, 1);
	if (m != -1)
		res += _vm->getModifiedHpLimits(m, constitution, level2, false);

	m = _vm->getCharacterClassType(cclass, 2);
	if (m != -1)
		res += _vm->getModifiedHpLimits(m, constitution, level3, false);

	res /= _vm->_numLevelsPerClass[cclass];

	return res;
}

int CharacterGenerator::getMinHp(int cclass, int constitution, int level1, int level2, int level3) {
	int res = 0;
	constitution = _vm->getClassAndConstHitpointsModifier(cclass, constitution);

	int m = _vm->getCharacterClassType(cclass, 0);
	if (m != -1)
		res = _vm->getModifiedHpLimits(m, constitution, level1, true);

	m = _vm->getCharacterClassType(cclass, 1);
	if (m != -1)
		res += _vm->getModifiedHpLimits(m, constitution, level2, true);

	m = _vm->getCharacterClassType(cclass, 2);
	if (m != -1)
		res += _vm->getModifiedHpLimits(m, constitution, level3, true);

	res /= _vm->_numLevelsPerClass[cclass];

	return res;
}

void CharacterGenerator::finish() {
	_screen->copyRegion(0, 0, 160, 0, 160, 128, 2, 2, Screen::CR_NO_P_CHECK);
	if (_chargenEnterGameStrings) {
		int cp = _screen->setCurPage(2);
		_screen->printShadedText(_chargenEnterGameStrings[0], (_vm->gameFlags().platform == Common::kPlatformFMTowns) ? 184 : 168, 32, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
		_screen->setCurPage(cp);
	}
	_screen->copyRegion(160, 0, 144, 64, 160, 128, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	if (_vm->game() == GI_EOB1) {
		static const int8 classDefaultItemsList[] = {
			 1, 17,  2, 17, 46, -1,  4, -1,  5, -1,  6,
			 2,  7, -1,  8, -1,  9, 21, 10,  2, 31,  2
		};

		static const int8 classDefaultItemsListIndex[] = {
			 4,  8,  0, -1,  4,  3,  0, -1,  4, 10,
			 0,  8,  3,  6,  1, -1,  2,  7,  0, -1,
			 4,  5,  0, -1,  4,  7,  0,  8,  4,  5,
			 0,  8,  4,  6,  8,  8,  4,  6,  5,  8,
			 3,  6,  5, -1,  2,  7,  5,  0,  4,  6,
			 7,  0,  4,  3,  7,  0,  2,  6,  7,  1
		};

		_characters[0].inventory[2] = _vm->duplicateItem(35);

		for (int i = 0; i < 4; i++) {
			EoBCharacter *c = &_characters[i];
			c->flags = 1;
			c->food = 100;
			c->id = i;
			c->inventory[3] = _vm->duplicateItem(10);

			for (int ii = 0; ii < 4; ii++) {
				int l = classDefaultItemsListIndex[(c->cClass << 2) + ii] << 1;
				if (l < 0 || classDefaultItemsList[l] == -1)
					continue;

				int d = classDefaultItemsList[l];
				int slot = classDefaultItemsList[l + 1];

				if (slot < 0) {
					slot = 0;
					if (c->inventory[slot])
						slot++;
					if (c->inventory[slot])
						slot++;
				}

				if (slot != 2 && c->inventory[slot])
					continue;

				if (d == 5 && (c->raceSex >> 1) == 3)
					d = 36;

				if (slot == 2) {
					while (c->inventory[slot])
						slot++;
				}

				c->inventory[slot] = _vm->duplicateItem(d);
			}

			_vm->recalcArmorClass(i);
		}

	} else {
		static const uint8 classDefaultItemsListIndex[] = { 0, 0, 0, 1, 2, 3, 0, 0, 0, 0, 3, 2, 0, 0, 2 };
		static const int8 itemList0[] = { 3, 36, 0, 17, -1, 0, 0, 56, 1, 17, 31, 0, 1, 23, 1, 17, 31, 1, 1 };
		static const int8 itemList1[] = { 1, 2, 0, 17, -1, 0, 0 };
		static const int8 itemList2[] = { 2, 56, 1, 17, 31, 0, 1, 23, 1, 17, 31, 0, 1 };
		static const int8 itemList3[] = { 2, 1, 1, 17, 31, 1, 1, 1, 0, 17, 31, 2, 1 };
		static const int8 *const itemList[] = { itemList0, itemList1, itemList2, itemList3 };

		for (int i = 0; i < 4; i++) {
			EoBCharacter *c = &_characters[i];
			c->flags = 1;
			c->food = 100;
			c->id = i;
			const int8 *df = itemList[classDefaultItemsListIndex[c->cClass]];
			int v1 = _vm->rollDice(1, *df++, -1);

			df = &df[v1 * 6];
			for (int ii = 0; ii < 2; ii++) {
				if (df[0] == -1)
					break;
				_vm->createInventoryItem(c, df[0], df[1], df[2]);
				df = &df[3];
			}

			uint16 m = _vm->_classModifierFlags[c->cClass];
			v1 = _vm->rollDice(1, 2, -1);

			int ival = 0;
			int itype = 0;

			if (m & 0x31) {
				if ((c->raceSex >> 1) == 3) {
					itype = 22;
					ival = 1;
				} else {
					if (v1 == 0) {
						itype = 5;
						ival = 1;
					} else {
						itype = 34;
					}
				}
			} else if (m & 0x04) {
				itype = 26;
				if (v1 != 0)
					ival = 1;
			} else if (m & 0x08) {
				ival = 1;
				itype = ((c->raceSex >> 1) == 3) ? 22 : 5;
			} else {
				if (v1 == 0) {
					itype = 3;
				} else {
					itype = 4;
					ival = 1;
				}
			}

			_vm->createInventoryItem(c, itype, ival, 0);
			_vm->createInventoryItem(c, 10, -1, 2);
			_vm->createInventoryItem(c, 10, -1, 2);
			_vm->createInventoryItem(c, 24, 2, 2);

			if (_vm->_classModifierFlags[c->cClass] & 2) {
				_vm->createInventoryItem(c, 7, -1, 1);
				_vm->createInventoryItem(c, 21, 4, 2);
				_vm->createInventoryItem(c, 21, 13, 2);
			}

			if (_vm->_classModifierFlags[c->cClass] & 0x14) {
				if (c->cClass == 2)
					_vm->createInventoryItem(c, 27, -1, 1);
				else
					_vm->createInventoryItem(c, 8, -1, 1);

				_vm->createInventoryItem(c, 20, 49, 1);
			}

			if (_vm->_classModifierFlags[c->cClass] & 8)
				_vm->createInventoryItem(c, 6, -1, 1);

			if (i == 0)
				_vm->createInventoryItem(c, 93, -1, 2);

			_vm->recalcArmorClass(i);
		}
	}

	for (int i = 0; i < 4; i++) {
		if (_vm->_classModifierFlags[_characters[i].cClass] & 2)
			_characters[i].mageSpellsAvailableFlags = (_vm->game() == GI_EOB2) ? 0x81CB6 : 0x26C;

		if (_vm->_classModifierFlags[_characters[i].cClass] & 0x14 && _vm->game() == GI_EOB2) {
			// Cleric/Paladin: Add Turn Undead spell
			_characters[i].clericSpells[0] = 29;
		}
	}

	for (int i = 0; i < 4; i++) {
		EoBCharacter *c = &_characters[i];
		c->strengthMax = c->strengthCur;
		c->strengthExtMax = c->strengthExtCur;
		c->intelligenceMax = c->intelligenceCur;
		c->wisdomMax = c->wisdomCur;
		c->dexterityMax = c->dexterityCur;
		c->constitutionMax = c->constitutionCur;
		c->charismaMax = c->charismaCur;
		c->hitPointsMax = c->hitPointsCur;
	}

	_vm->gui_resetButtonList();

	if (_faceShapes) {
		for (int i = 0; i < 44; i++) {
			bool del = true;
			for (int ii = 0; ii < 4; ii++) {
				if (_characters[ii].faceShape == _faceShapes[i])
					del = false;
			}
			if (del)
				delete[] _faceShapes[i];
		}
		delete[] _faceShapes;
		_faceShapes = 0;
	}

	if (_chargenMagicShapes) {
		for (int i = 0; i < 10; i++)
			delete[] _chargenMagicShapes[i];
		delete[] _chargenMagicShapes;
		_chargenMagicShapes = 0;
	}

	for (int i = 0; i < 17; i++) {
		delete[] _chargenButtonLabels[i];
		_chargenButtonLabels[i] = 0;
	}
}

const EoBChargenButtonDef CharacterGenerator::_chargenButtonDefsDOS[] = {
	{ 0x01, 0x37, 0x31, 0x32, 0x70 },
	{ 0x09, 0x37, 0x31, 0x32, 0x71 },
	{ 0x01, 0x77, 0x31, 0x32, 0x72 },
	{ 0x09, 0x77, 0x31, 0x32, 0x73 },
	{ 0x03, 0xB5, 0x53, 0x10, 0x1A },
	{ 0x21, 0xAC, 0x26, 0x10, 0x19 },
	{ 0x1C, 0xAC, 0x26, 0x10, 0x21 },
	{ 0x21, 0xAC, 0x26, 0x10, 0x32 },
	{ 0x13, 0x50, 0x9A, 0x08, 0x00 },
	{ 0x13, 0x58, 0x9A, 0x08, 0x00 },
	{ 0x13, 0x60, 0x9A, 0x08, 0x00 },
	{ 0x13, 0x68, 0x9A, 0x08, 0x00 },
	{ 0x13, 0x70, 0x9A, 0x08, 0x00 },
	{ 0x13, 0x78, 0x9A, 0x08, 0x00 },
	{ 0x13, 0x80, 0x9A, 0x08, 0x00 },
	{ 0x13, 0x88, 0x9A, 0x08, 0x00 },
	{ 0x13, 0x90, 0x9A, 0x08, 0x00 },
	{ 0x13, 0x98, 0x9A, 0x08, 0x00 },
	{ 0x13, 0xA0, 0x9A, 0x08, 0x00 },
	{ 0x13, 0xA8, 0x9A, 0x08, 0x00 },
	{ 0x13, 0xB0, 0x9A, 0x08, 0x00 },
	{ 0x12, 0x42, 0x20, 0x10, 0x00 },
	{ 0x12, 0x52, 0x20, 0x10, 0x00 },
	{ 0x16, 0x42, 0x20, 0x20, 0x00 },
	{ 0x1A, 0x42, 0x20, 0x20, 0x00 },
	{ 0x1E, 0x42, 0x20, 0x20, 0x00 },
	{ 0x22, 0x42, 0x20, 0x20, 0x00 },
	{ 0x1C, 0x9C, 0x26, 0x10, 0x14 },
	{ 0x21, 0x9C, 0x26, 0x10, 0x34 },
	{ 0x1C, 0xAC, 0x26, 0x10, 0x22 },
	{ 0x21, 0xAC, 0x26, 0x10, 0x26 },
	{ 0x12, 0x80, 0x35, 0x08, 0x00 },
	{ 0x12, 0x88, 0x35, 0x08, 0x00 },
	{ 0x12, 0x90, 0x35, 0x08, 0x00 },
	{ 0x12, 0x98, 0x35, 0x08, 0x00 },
	{ 0x12, 0xA0, 0x35, 0x08, 0x00 },
	{ 0x12, 0xA8, 0x35, 0x08, 0x00 },
	{ 0x1D, 0x88, 0x35, 0x08, 0x00 },
	{ 0x1B, 0xAC, 0x15, 0x10, 0x0D },
	{ 0x1E, 0xAC, 0x15, 0x10, 0x0C },
	{ 0x21, 0xAC, 0x25, 0x10, 0x19 }
};

const uint16 CharacterGenerator::_chargenButtonKeyCodesFMTOWNS[] = {
	93, 94, 95, 96, 80, 79, 68, 66, 82, 77, 70, 75, 43, 45, 79
};

const CreatePartyModButton CharacterGenerator::_chargenModButtons[] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0A, 0x40 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x80 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x0A, 0x80 },
	{ 0x00, 0xC0, 0x04, 0x05, 0x07, 0x05, 0x08, 0x1C, 0x9C },
	{ 0x04, 0xC0, 0x03, 0x05, 0x0A, 0x05, 0x08, 0x21, 0xAC },
	{ 0x07, 0xC0, 0x03, 0x05, 0x0B, 0x05, 0x08, 0x21, 0xAC },
	{ 0x0A, 0xC0, 0x04, 0x05, 0x06, 0x05, 0x08, 0x21, 0x9C },
	{ 0x18, 0xC0, 0x03, 0x05, 0x09, 0x05, 0x08, 0x1C, 0xAC },
	{ 0x0E, 0xC0, 0x02, 0x05, 0x0F, 0x05, 0x08, 0x21, 0xAC },
	{ 0x10, 0xC0, 0x01, 0x05, 0x09, 0x05, 0x04, 0x1B, 0xAC },
	{ 0x11, 0xC0, 0x01, 0x01, 0x09, 0x07, 0x04, 0x1E, 0xAC },
	{ 0x12, 0xC0, 0x03, 0x07, 0x07, 0x04, 0x06, 0x12, 0x42 },
	{ 0x15, 0xC0, 0x03, 0x07, 0x07, 0x04, 0x06, 0x12, 0x52 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x03, 0xB5 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x03, 0xB5 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x1C, 0xAC }
};

const EoBRect8 CharacterGenerator::_chargenButtonBodyCoords[] = {
	{ 0x00, 0x80, 0x04, 0x20 },
	{ 0x04, 0x80, 0x04, 0x20 },
	{ 0x08, 0x80, 0x04, 0x20 },
	{ 0x0C, 0x80, 0x04, 0x20 },
	{ 0x0E, 0xA0, 0x03, 0x10 },
	{ 0x0B, 0xA0, 0x03, 0x10 },
	{ 0x10, 0x80, 0x04, 0x10 },
	{ 0x10, 0x90, 0x04, 0x10 },
	{ 0x11, 0xA0, 0x05, 0x10 },
	{ 0x11, 0xB0, 0x05, 0x10 },
	{ 0x16, 0xA0, 0x05, 0x10 },
	{ 0x16, 0xB0, 0x05, 0x10 },
	{ 0x00, 0xA0, 0x0B, 0x10 },
	{ 0x14, 0x80, 0x0B, 0x10 },
	{ 0x14, 0x90, 0x0B, 0x10 }
};

const uint8 CharacterGenerator::_chargenSegaButtonCoords[60] = {
	0x03, 0x17, 0x0b, 0x02, 0x00,
	0x1c, 0x16, 0x05, 0x02, 0x2c,
	0x12, 0x08, 0x04, 0x02, 0x40,
	0x12, 0x0a, 0x04, 0x02, 0x50,
	0x1b, 0x16, 0x03, 0x02, 0x60,
	0x1e, 0x16, 0x03, 0x02, 0x6c,
	0x21, 0x14, 0x05, 0x02, 0x78,
	0x1c, 0x16, 0x05, 0x02, 0x8c,
	0x21, 0x16, 0x05, 0x02, 0xa0,
	0x1c, 0x14, 0x05, 0x02, 0xb4,
	0x21, 0x16, 0x05, 0x02, 0xc8,
	0x21, 0x16, 0x05, 0x02, 0xdc
};

const int16 CharacterGenerator::_chargenBoxX[] = { 0x10, 0x50, 0x10, 0x50 };
const int16 CharacterGenerator::_chargenBoxY[] = { 0x3F, 0x3F, 0x7F, 0x7F };
const int16 CharacterGenerator::_chargenNameFieldX[] = { 0x02, 0x42, 0x02, 0x42 };
const int16 CharacterGenerator::_chargenNameFieldY[] = { 0x6B, 0x6B, 0xAB, 0xAB };

const int32 CharacterGenerator::_classMenuMasks[] = {
	0x003F, 0x07BB, 0x77FB, 0x00F1, 0x08F1, 0x00B1
};

const int32 CharacterGenerator::_alignmentMenuMasks[] = {
	0x01FF, 0x0007, 0x0001, 0x01FF, 0x01FF, 0x01FE, 0x01FF, 0x01FE,
	0x01FF, 0x01FE, 0x01FE, 0x01FE, 0x01FF, 0x0007, 0x01FF
};

const int16 CharacterGenerator::_raceModifiers[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, -1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, -1, 0, 1, -1, 0, 0, 0, -1, 0, 0, 1, 0, 0
};

// Transfer Party

class TransferPartyWiz {
public:
	TransferPartyWiz(EoBCoreEngine *vm, Screen_EoB *screen);
	~TransferPartyWiz();

	bool start();

private:
	bool selectAndLoadTransferFile();
	bool transferFileDialogue(Common::String &dest);


	int selectCharactersMenu();
	void drawCharPortraitWithStats(int charIndex, bool enabled);
	void updateHighlight(int index);

	void convertStats();
	void convertInventory();
	Item convertItem(Item eob1Item);
	void giveKhelbensCoin();

	EoBCoreEngine *_vm;
	Screen_EoB *_screen;

	int _highlight;
	EoBItem *_oldItems;

	const uint16 *_portraitFrames;
	const uint8 *_convertTable;
	const uint8 *_itemTable;
	const uint32 *_expTable;
	const char *const *_strings1;
	const char *const *_strings2;
	const char *const *_labels;
};

TransferPartyWiz::TransferPartyWiz(EoBCoreEngine *vm, Screen_EoB *screen) : _vm(vm), _screen(screen) {
	int temp;
	_portraitFrames = _vm->staticres()->loadRawDataBe16(kEoB2TransferPortraitFrames, temp);
	_convertTable = _vm->staticres()->loadRawData(kEoB2TransferConvertTable, temp);
	_itemTable = _vm->staticres()->loadRawData(kEoB2TransferItemTable, temp);
	_expTable = _vm->staticres()->loadRawDataBe32(kEoB2TransferExpTable, temp);
	_strings1 = _vm->staticres()->loadStrings(kEoB2TransferStrings1, temp);
	_strings2 = _vm->staticres()->loadStrings(kEoB2TransferStrings2, temp);
	_labels = _vm->staticres()->loadStrings(kEoB2TransferLabels, temp);
	_highlight = -1;
	_oldItems = 0;
}

TransferPartyWiz::~TransferPartyWiz() {
	_vm->gui()->notifyUpdateSaveSlotsList();
	delete[] _oldItems;
}

bool TransferPartyWiz::start() {
	_screen->copyPage(0, 12);

	if (!selectAndLoadTransferFile())
		return false;

	convertStats();

	_oldItems = new EoBItem[600];
	memcpy(_oldItems, _vm->_items, sizeof(EoBItem) * 600);
	_vm->loadItemDefs();

	int selection = selectCharactersMenu();
	if (selection == 0) {
		for (int i = 0; i < 6; i++)
			delete[] _vm->_characters[i].faceShape;
		memset(_vm->_characters, 0, sizeof(EoBCharacter) * 6);
		return false;
	}

	int ch = 0;
	for (int i = 0; i < 6; i++) {
		if (selection & (1 << i)) {
			if (ch != i) {
				delete[] _vm->_characters[ch].faceShape;
				memcpy(&_vm->_characters[ch], &_vm->_characters[i], sizeof(EoBCharacter));
				_vm->_characters[i].faceShape = 0;
			}
			ch++;
		}
	}

	delete[] _vm->_characters[4].faceShape;
	delete[] _vm->_characters[5].faceShape;
	memset(&_vm->_characters[4], 0, sizeof(EoBCharacter) * 2);

	convertInventory();
	giveKhelbensCoin();

	return true;
}

bool TransferPartyWiz::selectAndLoadTransferFile() {
	do {
		_screen->copyPage(12, 0);
		 if (transferFileDialogue(_vm->_savegameFilename))
			 break;
	} while (_vm->_gui->confirmDialogue2(15, 68, 1));

	if (_vm->_savegameFilename.empty())
		return false;

	if (_vm->loadGameState(-1).getCode() != Common::kNoError)
		return false;

	return true;
}

 bool TransferPartyWiz::transferFileDialogue(Common::String &dest) {
	_vm->_gui->transferWaitBox();

	Common::Array<Common::String> eobTargets;
	const Common::ConfigManager::DomainMap dom = ConfMan.getGameDomains();

	for (Common::ConfigManager::DomainMap::const_iterator i = dom.begin(); i != dom.end(); ++i) {
		if (ConfMan.get("gameid", i->_key).equals("eob"))
			eobTargets.push_back(i->_key);
		_vm->updateInput();
	}

	if (eobTargets.empty())
		return false;

	Common::String target = _vm->_gui->transferTargetMenu(eobTargets);
	_screen->copyPage(12, 0);

	if (target.empty())
		return true;

	dest = target + ".fin";
	Common::InSaveFile *in = _vm->_saveFileMan->openForLoading(dest);
	if (in) {
		delete in;
		if (_vm->_gui->confirmDialogue2(15, -2, 1))
			return true;
	}

	_screen->copyPage(12, 0);

	bool result = _vm->_gui->transferFileMenu(target, dest);
	_screen->copyPage(12, 0);

	return result;
}

int TransferPartyWiz::selectCharactersMenu() {
	_screen->setCurPage(2);
	_screen->setFont(Screen::FID_6_FNT);
	_screen->clearCurPage();

	_vm->gui_drawBox(0, 0, 320, 163, _vm->guiSettings()->colors.frame1, _vm->guiSettings()->colors.frame2, _vm->guiSettings()->colors.fill);
	_screen->printText(_strings2[0], 5, 3, _vm->guiSettings()->colors.guiColorWhite, 0);
	_screen->printText(_strings2[1], 5, 10, _vm->guiSettings()->colors.guiColorWhite, 0);

	for (int i = 0; i < 6; i++)
		drawCharPortraitWithStats(i, 0);

	_vm->gui_drawBox(4, 148, 43, 12, _vm->guiSettings()->colors.frame1, _vm->guiSettings()->colors.frame2, _vm->guiSettings()->colors.fill);
	_vm->gui_drawBox(272, 148, 43, 12, _vm->guiSettings()->colors.frame1, _vm->guiSettings()->colors.frame2, _vm->guiSettings()->colors.fill);

	_screen->printShadedText(_labels[0], 9, 151, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);
	_screen->printShadedText(_labels[1], 288, 151, _vm->guiSettings()->colors.guiColorWhite, 0, _vm->guiSettings()->colors.guiColorBlack);

	_screen->setCurPage(0);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	int selection = 0;
	int highlight = 0;
	bool update = false;

	for (bool loop = true; loop && (!_vm->shouldQuit());) {
		int inputFlag = _vm->checkInput(0, false, 0) & 0x8FF;
		_vm->removeInputTop();

		if (inputFlag) {
			if (inputFlag == _vm->_keyMap[Common::KEYCODE_LEFT] || inputFlag == _vm->_keyMap[Common::KEYCODE_RIGHT]) {
				highlight ^= 1;
			} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_UP]) {
				highlight -= 2;
				if (highlight < 0)
					highlight += 8;
			} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN]) {
				highlight += 2;
				if (highlight >= 8)
					highlight -= 8;
			} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN] || inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE]) {
				update = true;
			} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE]) {
				update = true;
				highlight = 6;
			} else if (inputFlag == 199) {
				for (int i = 0; i < 8; i++) {
					int t = i << 2;
					if (_vm->posWithinRect(_vm->_mouseX, _vm->_mouseY, _portraitFrames[t], _portraitFrames[t + 1], _portraitFrames[t + 2], _portraitFrames[t + 3])) {
						highlight = i;
						update = true;
						break;
					}
				}
			}
		}

		updateHighlight(highlight);

		if (!update)
			continue;

		update = false;

		if (highlight < 6) {
			if (_vm->_characters[highlight].flags & 1) {
				selection ^= (1 << highlight);
				drawCharPortraitWithStats(highlight, (selection & (1 << highlight)) ? true : false);
				_screen->updateScreen();
			}
			continue;
		}

		int x = (highlight - 6) * 268 + 4;
		_vm->gui_drawBox(x, 148, 43, 12, _vm->guiSettings()->colors.fill, _vm->guiSettings()->colors.fill, -1);
		_screen->updateScreen();
		_vm->_system->delayMillis(80);
		_vm->gui_drawBox(x, 148, 43, 12, _vm->guiSettings()->colors.frame1, _vm->guiSettings()->colors.frame2, -1);
		_screen->updateScreen();

		if (highlight == 6 || _vm->shouldQuit()) {
			_screen->setFont(Screen::FID_8_FNT);
			return 0;
		}

		int count = 0;
		for (int i = 0; i < 6; i++) {
			if (selection & (1 << i))
				count++;
		}

		if (count == 4 || _vm->shouldQuit())
			loop = false;
		else
			_vm->_gui->messageDialogue(16, count < 4 ? 69 : 70, _vm->guiSettings()->colors.guiColorLightRed);

		_screen->updateScreen();
	}

	_screen->setFont(Screen::FID_8_FNT);
	if (_vm->shouldQuit())
		return 0;
	else
		_vm->_gui->messageDialogue(16, 71, _vm->guiSettings()->colors.guiColorLightRed);

	return selection;
}

void TransferPartyWiz::drawCharPortraitWithStats(int charIndex, bool enabled) {
	int16 x = (charIndex % 2) * 159;
	int16 y = (charIndex / 2) * 40;
	EoBCharacter *c = &_vm->_characters[charIndex];

	_screen->fillRect(x + 4, y + 24, x + 36, y + 57, 12);
	_vm->gui_drawBox(x + 40, y + 24, 118, 34, _vm->guiSettings()->colors.frame1, _vm->guiSettings()->colors.frame2, _vm->guiSettings()->colors.fill);

	if (!(c->flags & 1))
		return;

	_screen->drawShape(_screen->_curPage, c->faceShape, x + 4, y + 25, 0);

	int color1 = _vm->guiSettings()->colors.guiColorWhite;
	int color2 = _vm->guiSettings()->colors.guiColorBlack;

	if (enabled) {
		color1 = _vm->guiSettings()->colors.guiColorLightRed;
		color2 = _vm->guiSettings()->colors.guiColorWhite;
	} else {
		_screen->drawShape(_screen->_curPage, _vm->_disabledCharGrid, x + 4, y + 25, 0);
	}

	_screen->printShadedText(c->name, x + 44, y + 27, color1, 0, _vm->guiSettings()->colors.guiColorBlack);
	_screen->printText(_vm->_chargenRaceSexStrings[c->raceSex], x + 43, y + 36, color2, 0);
	_screen->printText(_vm->_chargenClassStrings[c->cClass], x + 43, y + 43, color2, 0);

	Common::String tmp = Common::String::format(_strings1[0], c->level[0]);
	for (int i = 1; i < _vm->_numLevelsPerClass[c->cClass]; i++)
		tmp += Common::String::format(_strings1[1], c->level[i]);
	_screen->printText(tmp.c_str(), x + 43, y + 50, color2, 0);
}

void TransferPartyWiz::updateHighlight(int index) {
	static const int16 xPos[] = { 9, 288 };
	if (_highlight > 5 && _highlight != index)
		_screen->printText(_labels[_highlight - 6], xPos[_highlight - 6], 151, _vm->guiSettings()->colors.guiColorWhite, 0);

	if (index < 6) {
		_vm->_gui->updateBoxFrameHighLight(14 + index);
		_highlight = index;
		return;
	}

	if (_highlight == index)
		return;

	if (_highlight < 6)
		_vm->_gui->updateBoxFrameHighLight(-1);

	_screen->printText(_labels[index - 6], xPos[index - 6], 151, _vm->guiSettings()->colors.guiColorLightRed, 0);
	_screen->updateScreen();
	_highlight = index;
}

void TransferPartyWiz::convertStats() {
	for (int i = 0; i < 6; i++) {
		EoBCharacter *c = &_vm->_characters[i];
		uint32 aflags = 0;

		for (int ii = 0; ii < 25; ii++) {
			if (c->mageSpellsAvailableFlags & (1 << ii)) {
				int8 f = (int8)_convertTable[ii + 1] - 1;
				if (f != -1)
					aflags |= (1 << f);
			}
		}
		c->mageSpellsAvailableFlags = aflags;

		c->armorClass = 0;
		c->disabledSlots = 0;
		c->flags &= 1;
		c->hitPointsCur = c->hitPointsMax;
		c->food = 100;

		c->effectFlags = 0;
		c->damageTaken = 0;
		memset(c->clericSpells, 0, sizeof(int8) * 80);
		memset(c->mageSpells, 0, sizeof(int8) * 80);
		memset(c->timers, 0, sizeof(uint32) * 10);
		memset(c->events, 0, sizeof(int8) * 10);
		memset(c->effectsRemainder, 0, sizeof(uint8) * 4);
		memset(c->slotStatus, 0, sizeof(int8) * 5);

		for (int ii = 0; ii < 3; ii++) {
			int t = _vm->getCharacterClassType(c->cClass, ii);
			if (t == -1)
				continue;
			if (c->experience[ii] > _expTable[t])
				c->experience[ii] = _expTable[t];
		}
	}
}

void TransferPartyWiz::convertInventory() {
	for (int i = 0; i < 4; i++) {
		EoBCharacter *c = &_vm->_characters[i];

		for (int slot = 0; slot < 27; slot++) {
			Item itm = c->inventory[slot];
			if (slot == 16) {
				Item first = itm;
				c->inventory[slot] = 0;

				for (bool forceLoop = true; (itm && (itm != first)) || forceLoop; itm = _oldItems[itm].prev) {
					forceLoop = false;
					_vm->setItemPosition(&c->inventory[slot], -2, convertItem(itm), 0);
				}
			} else {
				c->inventory[slot] = convertItem(itm);
			}
		}

		_vm->recalcArmorClass(i);
	}
}

Item TransferPartyWiz::convertItem(Item eob1Item) {
	if (!eob1Item)
		return 0;

	EoBItem *itm1 = &_oldItems[eob1Item];

	if (!_itemTable[itm1->type])
		return 0;

	Item newItem = _vm->duplicateItem(1);
	EoBItem *itm2 = &_vm->_items[newItem];
	bool match = false;

	itm2->flags = itm1->flags | 0x40;
	itm2->icon = itm1->icon;
	itm2->type = itm1->type;
	itm2->level = 0xFF;

	switch (itm2->type) {
	case 35:
		itm1->value += 25;
		// fall through
	case 34:
		itm2->value = _convertTable[itm1->value];
		if (!itm2->value) {
			itm2->block = -1;
			return 0;
		}
		break;
	case 39:
		itm2->value = itm1->value - 1;
		break;
	case 48:
		if (itm1->value == 5) {
			memset(itm2, 0, sizeof(EoBItem));
			itm2->block = -1;
			return 0;
		}
		itm2->value = itm1->value;
		itm2->flags = ((itm1->flags & 0x3F) + 3) | 0x40;
		break;
	case 18:
		itm2->icon = 19;
		// fall through
	default:
		itm2->value = itm1->value;
		break;
	}

	switch ((_vm->_itemTypes[itm2->type].extraProperties & 0x7F) - 1) {
	case 0:
	case 1:
	case 2:
		if (itm2->value)
			itm2->flags |= 0x80;
		break;
	case 4:
	case 5:
	case 8:
	case 9:
	case 13:
	case 15:
	case 17:
		itm2->flags |= 0x80;
		break;
	default:
		break;
	}

	for (int i = 1; i < 600; i++) {
		if (i == 60 || i == 62 || i == 63 || i == 83)
			continue;
		EoBItem *tmp = &_vm->_items[i];
		if (tmp->level || tmp->block == -2 || tmp->type != itm2->type || tmp->icon != itm2->icon)
			continue;
		itm2->nameUnid = tmp->nameUnid;
		itm2->nameId = tmp->nameId;
		match = true;
		break;
	}

	if (!match) {
		for (int i = 1; i < 600; i++) {
			if (i == 60 || i == 62 || i == 63 || i == 83)
				continue;
			EoBItem *tmp = &_vm->_items[i];
			if (tmp->level || tmp->block == -2 || tmp->type != itm2->type)
				continue;
			itm2->nameUnid = tmp->nameUnid;
			itm2->nameId = tmp->nameId;
			match = true;
			break;
		}
	}

	if (!match) {
		memset(itm2, 0, sizeof(EoBItem));
		itm2->block = -1;
		return 0;
	}

	itm2->level = 0;
	return newItem;
}

void TransferPartyWiz::giveKhelbensCoin() {
	bool success = false;
	for (int i = 0; i < 4 && !success; i++) {
		EoBCharacter *c = &_vm->_characters[i];

		for (int slot = 2; slot < 16; slot++) {
			if (c->inventory[slot])
				continue;
			_vm->createInventoryItem(c, 93, -1, slot);
			success = true;
			break;
		}
	}

	if (!success) {
		_vm->_characters[0].inventory[2] = 0;
		_vm->createInventoryItem(&_vm->_characters[0], 93, -1, 2);
	}
}

// Start functions

bool EoBCoreEngine::startCharacterGeneration(bool defaultParty) {
	_sound->selectAudioResourceSet(_flags.platform == Common::kPlatformAmiga ? kMusicIntro : kMusicIngame);
	return CharacterGenerator(this, _screen).start(_characters, &_faceShapes, defaultParty);
}

bool EoBCoreEngine::startPartyTransfer() {
	return TransferPartyWiz(this, _screen).start();
}

} // End of namespace Kyra

#endif // ENABLE_EOB
