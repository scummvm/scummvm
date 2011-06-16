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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifdef ENABLE_EOB

#include "kyra/eobcommon.h"
#include "kyra/resource.h"
#include "kyra/sound_intern.h"

namespace Kyra {

class CharacterGenerator {
public:
	CharacterGenerator(EobCoreEngine *vm, Screen_Eob *screen);
	~CharacterGenerator();

	void start(EobCharacter *characters, uint8 ***faceShapes);

private:
	void init();
	void initButtonsFromList(int first, int numButtons);
	void initButton(int index, int x, int y, int w, int h, int keyCode);
	void checkForCompleteParty();
	void toggleSpecialButton(int index, int bodyCustom, int pageNum);
	void processSpecialButton(int index);
	void highlightBoxFrame(int index);
	int viewDeleteCharacter();
	void createPartyMember();
	int raceSexMenu();
	int classMenu(int raceSex);
	int alignmentMenu(int cClass);
	void updateMagicShapes();
	void generateStats(int index);
	void modifyMenu();
	void statsAndFacesMenu();
	void faceSelectMenu();
	int getNextFreeFaceShape(int shpIndex, int charSex, int step, int8 *selectedPortraits);
	void processFaceMenuSelection(int index);
	void printStats(int index, int mode);
	void processNameInput(int index, int len, int textColor);
	int rollDice();
	int modifyStat(int index, int8 *stat1, int8 *stat2);
	int getMaxHp(int cclass, int constitution, int level1, int level2, int level3);
	int getMinHp(int cclass, int constitution, int level1, int level2, int level3);
	void finish();

	uint8 **_chargenMagicShapes;
	uint8 **_chargenButtonLabels;
	int _activeBox;
	int _magicShapesBox;
	int _updateBoxIndex;
	int _updateBoxColorIndex;
	int _updateBoxShapesIndex;
	int _lastUpdateBoxShapesIndex;
	uint32 _chargenBoxTimer;
	uint32 _chargenMagicShapeTimer;
	int8 _chargenSelectedPortraits[4];
	int8 _chargenSelectedPortraits2[4];

	uint16 _chargenMinStats[7];
	uint16 _chargenMaxStats[7];

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

	static const EobChargenButtonDef _chargenButtonDefs[];
	static const CreatePartyModButton _chargenModButtons[];
	static const EobRect8 _chargenButtonBodyCoords[];
	static const EobRect16 _chargenPortraitBoxFrames[];
	static const int16 _chargenBoxX[];
	static const int16 _chargenBoxY[];
	static const int16 _chargenNameFieldX[];
	static const int16 _chargenNameFieldY[];

	static const int32 _classMenuMasks[];
	static const int32 _alignmentMenuMasks[];

	static const int16 _raceModifiers[];

	EobCharacter *_characters;
	uint8 **_faceShapes;

	EobCoreEngine *_vm;
	Screen_Eob *_screen;
};

void EobCoreEngine::startCharacterGeneration() {
	CharacterGenerator(this, _screen).start(_characters, &_faceShapes);
}

CharacterGenerator::CharacterGenerator(EobCoreEngine *vm, Screen_Eob *screen) : _vm(vm), _screen(screen),
	_characters(0), _faceShapes(0), _chargenMagicShapes(0), _chargenButtonLabels(0), _updateBoxIndex(-1),
	_chargenBoxTimer(0), _chargenMagicShapeTimer(0), _updateBoxColorIndex(0), _updateBoxShapesIndex(0),
	_lastUpdateBoxShapesIndex(0), _magicShapesBox(6), _activeBox(0) {

	_chargenStatStrings = _vm->_chargenStatStrings;
	_chargenRaceSexStrings = _vm->_chargenRaceSexStrings;
	_chargenClassStrings = _vm->_chargenClassStrings;
	_chargenAlignmentStrings = _vm->_chargenAlignmentStrings;

	memset(_chargenSelectedPortraits, -1, sizeof(_chargenSelectedPortraits));
	memset(_chargenSelectedPortraits2, 0, sizeof(_chargenSelectedPortraits2));
	memset(_chargenMinStats, 0, sizeof(_chargenMinStats));
	memset(_chargenMaxStats, 0, sizeof(_chargenMaxStats));

	int temp;
	_chargenStrings1 = _vm->staticres()->loadStrings(kEobBaseChargenStrings1, temp);
	_chargenStrings2 = _vm->staticres()->loadStrings(kEobBaseChargenStrings2, temp);
	_chargenStartLevels = _vm->staticres()->loadRawData(kEobBaseChargenStartLevels, temp);
	_chargenEnterGameStrings = _vm->staticres()->loadStrings(kEobBaseChargenEnterGameStrings, temp);
	_chargenClassMinStats = _vm->staticres()->loadRawData(kEobBaseChargenClassMinStats, temp);
	_chargenRaceMinStats = _vm->staticres()->loadRawData(kEobBaseChargenRaceMinStats, temp);
	_chargenRaceMaxStats = _vm->staticres()->loadRawDataBe16(kEobBaseChargenRaceMaxStats, temp);
}

CharacterGenerator::~CharacterGenerator() {
	if (_chargenMagicShapes) {
		for (int i = 0; i < 10; i++)
			delete[] _chargenMagicShapes[i];
		delete[] _chargenMagicShapes;
	}

	if (_chargenButtonLabels) {
		for (int i = 0; i < 10; i++)
			delete[] _chargenButtonLabels[i];
		delete[] _chargenButtonLabels;
	}
}

void CharacterGenerator::start(EobCharacter *characters, uint8 ***faceShapes) {
	if (!characters && !faceShapes)
		return;

	_characters = characters;
	_faceShapes = *faceShapes;

	_vm->sound()->playTrack(0);

	_vm->delay(_vm->_tickLength);

	init();

	_screen->setScreenDim(2);

	checkForCompleteParty();
	initButtonsFromList(0, 5);

	_vm->sound()->playTrack(_vm->game() == GI_EOB1 ? 20 : 13);
	_activeBox = 0;

	for (bool loop = true; loop && (!_vm->shouldQuit()); ) {
		highlightBoxFrame(_activeBox + 6);
		_vm->sound()->process();
		int inputFlag = _vm->checkInput(_vm->_activeButtons, false, 0);
		_vm->removeInputTop();

		if (inputFlag) {
			if (inputFlag == _vm->_keyMap[Common::KEYCODE_LEFT] || inputFlag == _vm->_keyMap[Common::KEYCODE_RIGHT])
				_activeBox ^= 1;
			else if (inputFlag == _vm->_keyMap[Common::KEYCODE_UP] || inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN])
				_activeBox ^= 2;
			highlightBoxFrame(-1);
		}

		if (inputFlag & 0x8000) {
			inputFlag = (inputFlag & 0x0f) - 1;
			if (inputFlag == 4) {
				loop = false;
			} else {
				_activeBox = inputFlag;
				inputFlag = 43;
			}
		}

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN] || inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP5]) {
			highlightBoxFrame(-1);
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

	if (!_vm->shouldQuit()) {
		processSpecialButton(15);
		finish();
	}

	_vm->sound()->playTrack(15);

	*faceShapes = _faceShapes;
}

void CharacterGenerator::init() {
	_screen->loadEobBitmap("CHARGENA", 3, 3);
	if (_faceShapes) {
		for (int i = 0; i < 44; i++)
			delete[] _faceShapes[i];
		delete[] _faceShapes;
	}

	_faceShapes = new uint8*[44];
	for (int i = 0; i < 44; i++)
		_faceShapes[i] = _screen->encodeShape((i % 10) << 2, (i / 10) << 5, 4, 32, true);
	_screen->_curPage = 0;

	_screen->loadEobCpsFileToPage("CHARGEN", 0, 3, 3, 0);
	_screen->loadEobBitmap("CHARGENB", 3, 3);
	if (_chargenMagicShapes) {
		for (int i = 0; i < 10; i++)
			delete[] _chargenMagicShapes[i];
		delete[] _chargenMagicShapes;
	}

	_chargenMagicShapes = new uint8*[10];
	for (int i = 0; i < 10; i++)
		_chargenMagicShapes[i] = _screen->encodeShape(i << 2, 0, 4, 32, true);

	_chargenButtonLabels = new uint8*[17];
	for (int i = 0; i < 17; i++) {
		const CreatePartyModButton *c = &_chargenModButtons[i];
		_chargenButtonLabels[i] = c->labelW? _screen->encodeShape(c->encodeLabelX, c->encodeLabelY, c->labelW, c->labelH, true) : 0;
	}

	_screen->copyPage(3, 2);
	_screen->_curPage = 0;
	_screen->copyRegion(144, 64, 0, 0, 180, 128, 0, 2, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
}

void CharacterGenerator::initButtonsFromList(int first, int numButtons) {
	_vm->gui_resetButtonList();

	for (int i = 0; i < numButtons; i++) {
		const EobChargenButtonDef *e = &_chargenButtonDefs[first + i];
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
	b->keyCode2 = keyCode + 0x100;
}

void CharacterGenerator::checkForCompleteParty() {
	_screen->copyRegion(0, 0, 160, 0, 160, 128, 2, 2, Screen::CR_NO_P_CHECK);
	int cp = _screen->setCurPage(2);
	_screen->printShadedText(_chargenStrings1[8], 168, 16, 15, 0);
	_screen->setCurPage(cp);
	_screen->copyRegion(160, 0, 144, 64, 160, 128, 2, 0, Screen::CR_NO_P_CHECK);

	int numChars = 0;
	for (int i = 0; i < 4; i++) {
		if (_characters[i].name[0])
			numChars++;
	}

	if (numChars == 4) {
		_screen->setCurPage(2);
		_screen->printShadedText(_chargenStrings1[0], 168, 61, 15, 0);
		_screen->setCurPage(0);
		_screen->copyRegion(168, 61, 152, 125, 136, 40, 2, 0, Screen::CR_NO_P_CHECK);
		toggleSpecialButton(15, 0, 0);
	} else {
		toggleSpecialButton(14, 0, 0);
	}

	_screen->updateScreen();
}

void CharacterGenerator::toggleSpecialButton(int index, int bodyCustom, int pageNum) {
	if (index >= 17)
		return;

	const CreatePartyModButton *c = &_chargenModButtons[index];
	const EobRect8 *p = &_chargenButtonBodyCoords[c->bodyIndex + bodyCustom];

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

void CharacterGenerator::processSpecialButton(int index) {
	toggleSpecialButton(index, 1, 0);
	_vm->sound()->playSoundEffect(76);
	_vm->_system->delayMillis(80);
	toggleSpecialButton(index, 0, 0);
}

void CharacterGenerator::highlightBoxFrame(int index) {
	static const uint8 colorTable[] = { 0x0F, 0xB0, 0xB2, 0xB4, 0xB6,
		0xB8, 0xBA, 0xBC, 0x0C, 0xBC, 0xBA, 0xB8, 0xB6, 0xB4, 0xB2, 0xB0, 0x00
	};

	if (_updateBoxIndex == index) {
		if (_updateBoxIndex == -1)
			return;

		if (_vm->_system->getMillis() <= _chargenBoxTimer)
			return;

		if (!colorTable[_updateBoxColorIndex])
			_updateBoxColorIndex = 0;

		const EobRect16 *r = &_chargenPortraitBoxFrames[_updateBoxIndex];
		_screen->drawBox(r->x1, r->y1, r->x2, r->y2, colorTable[_updateBoxColorIndex++]);
		_screen->updateScreen();

		_chargenBoxTimer = _vm->_system->getMillis() + _vm->_tickLength;

	} else {
		if (_updateBoxIndex != -1) {
			const EobRect16 *r = &_chargenPortraitBoxFrames[_updateBoxIndex];
			_screen->drawBox(r->x1, r->y1, r->x2, r->y2, 12);
			_screen->updateScreen();
		}

		_updateBoxColorIndex = 0;
		_updateBoxIndex = index;
		_chargenBoxTimer = _vm->_system->getMillis();
	}
}

int CharacterGenerator::viewDeleteCharacter() {
	initButtonsFromList(0, 7);
	_vm->removeInputTop();

	highlightBoxFrame(-1);
	printStats(_activeBox, 2);

	int res = 0;
	for (bool loop = true; loop && _characters[_activeBox].name[0] && !_vm->shouldQuit(); ) {
		highlightBoxFrame(_activeBox + 6);
		_vm->sound()->process();

		int inputFlag = _vm->checkInput(_vm->_activeButtons, false, 0);
		int cbx = _activeBox;
		_vm->removeInputTop();

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_RETURN] || inputFlag == _vm->_keyMap[Common::KEYCODE_SPACE] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP5] || inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE]) {
			processSpecialButton(9);
			res = 0;
			loop = false;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_LEFT] || inputFlag == _vm->_keyMap[Common::KEYCODE_RIGHT]) {
			cbx ^= 1;
		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_UP] || inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN]) {
			cbx ^= 2;
		}

		if (inputFlag & 0x8000) {
			inputFlag = (inputFlag & 0x0f) - 1;
			if (inputFlag == 4) {
				res = 1;
				loop = false;
			} else if (inputFlag == 5) {
				processSpecialButton(9);
				res = 0;
				loop = false;
			} else if (inputFlag == 6) {
				if (_characters[_activeBox].name[0]) {
					processSpecialButton(16);
					_characters[_activeBox].name[0] = 0;
					processNameInput(_activeBox, 1, 12);
					processFaceMenuSelection(_activeBox + 50);
				}
			} else {
				cbx = inputFlag;
			}
		}

		if (loop == false)
			highlightBoxFrame(-1);

		if (!_characters[cbx].name[0])
			loop = false;

		if (cbx != _activeBox) {
			_activeBox = cbx;
			highlightBoxFrame(-1);
			if (loop)
				printStats(_activeBox, 2);
		}
	}

	return res;
}

void CharacterGenerator::createPartyMember() {
	_screen->setScreenDim(2);
	_chargenBoxTimer = 0;

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

		for (_characters[_activeBox].name[0] = 0; _characters[_activeBox].name[0] == 0 && !_vm->shouldQuit(); ) {
			processFaceMenuSelection(_chargenMinStats[6]);
			printStats(_activeBox, 0);
			_screen->printShadedText(_chargenStrings2[11], 149, 100, 9, 0);
			if (!_vm->shouldQuit())
				processNameInput(_activeBox, _vm->_gui->getTextInput(_characters[_activeBox].name, 24, 100, 10, 15, 0, 8), 2);
		}
	}
}

int CharacterGenerator::raceSexMenu() {
	_screen->drawBox(_chargenBoxX[_activeBox], _chargenBoxY[_activeBox], _chargenBoxX[_activeBox] + 32, _chargenBoxY[_activeBox] + 33, 12);
	_screen->copyRegion(0, 0, 144, 64, 160, 128, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->printShadedText(_chargenStrings2[8], 147, 67, 9, 0);
	_vm->removeInputTop();

	_vm->_gui->setupMenu(1, 0, _chargenRaceSexStrings, -1, 0, 0);
	int16 res = -1;

	while (res == -1 && !_vm->shouldQuit()) {
		res = _vm->_gui->handleMenu(1, _chargenRaceSexStrings, 0, -1, 0);
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
	_screen->printShadedText(_chargenStrings2[9], 147, 67, 9, 0);

	toggleSpecialButton(5, 0, 0);

	itemsMask &=_classMenuMasks[raceSex / 2];
	_vm->_gui->setupMenu(2, 15, _chargenClassStrings, itemsMask, 0, 0);

	_vm->_mouseX = _vm->_mouseY = 0;
	int16 res = -1;

	while (res == -1 && !_vm->shouldQuit()) {
		updateMagicShapes();

		int in = _vm->checkInput(0, false, 0) & 0xff;
		Common::Point mp = _vm->getMousePos();

		if (in == _vm->_keyMap[Common::KEYCODE_ESCAPE] || _vm->_gui->_menuLastInFlags == _vm->_keyMap[Common::KEYCODE_ESCAPE] || _vm->_gui->_menuLastInFlags == _vm->_keyMap[Common::KEYCODE_b]) {
			res = _vm->_keyMap[Common::KEYCODE_ESCAPE];
		} else if (_vm->posWithinRect(mp.x, mp.y, 264, 171, 303, 187)) {
			if (in == 199 || in == 201)
				res = _vm->_keyMap[Common::KEYCODE_ESCAPE];
			else
				_vm->removeInputTop();
		} else {
			res = _vm->_gui->handleMenu(2, _chargenClassStrings, 0, itemsMask, 0);
		}
	}

	_vm->removeInputTop();

	if (res == _vm->_keyMap[Common::KEYCODE_ESCAPE])
		processSpecialButton(5);

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
	_screen->printShadedText(_chargenStrings2[10], 147, 67, 9, 0);

	toggleSpecialButton(5, 0, 0);

	itemsMask &=_alignmentMenuMasks[cClass];
	_vm->_gui->setupMenu(3, 9, _chargenAlignmentStrings, itemsMask, 0, 0);

	_vm->_mouseX = _vm->_mouseY = 0;
	int16 res = -1;

	while (res == -1 && !_vm->shouldQuit()) {
		updateMagicShapes();

		int in = _vm->checkInput(0, false, 0) & 0xff;
		Common::Point mp = _vm->getMousePos();

		if (in == _vm->_keyMap[Common::KEYCODE_ESCAPE] || _vm->_gui->_menuLastInFlags == _vm->_keyMap[Common::KEYCODE_ESCAPE] || _vm->_gui->_menuLastInFlags == _vm->_keyMap[Common::KEYCODE_b]) {
			res = _vm->_keyMap[Common::KEYCODE_ESCAPE];
		} else if (_vm->posWithinRect(mp.x, mp.y, 264, 171, 303, 187)) {
			if (in == 199 || in == 201)
				res = _vm->_keyMap[Common::KEYCODE_ESCAPE];
			else
				_vm->removeInputTop();
		} else {
			res = _vm->_gui->handleMenu(3, _chargenAlignmentStrings, 0, itemsMask, 0);
		}
	}

	_vm->removeInputTop();

	if (res == _vm->_keyMap[Common::KEYCODE_ESCAPE])
		processSpecialButton(5);

	return res;
}

void CharacterGenerator::updateMagicShapes() {
	_vm->sound()->process();

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
	EobCharacter *c = &_characters[index];

	for (int i = 0; i < 3; i++) {
		c->level[i] = _chargenStartLevels[(c->cClass << 2) + i];
		c->experience[i] = (_vm->game() == GI_EOB2 ? 69000 : 5000) / _chargenStartLevels[(c->cClass << 2) + 3];
	}

	int rc = c->raceSex >> 1;
	for (int i = 0; i < 6; i++) {
		_chargenMinStats[i] = MAX(_chargenClassMinStats[c->cClass * 6 + i], _chargenRaceMinStats[rc * 6 + i]);
		_chargenMaxStats[i] = _chargenRaceMaxStats[rc * 6 + i];
	}

	if (_vm->_charClassModUnk[c->cClass])
		_chargenMaxStats[0] = 18;

	uint16 sv[6];
	for (int i = 0; i < 6; i++) {
		sv[i] = MAX<uint16>(rollDice() + _raceModifiers[rc * 6 + i], _chargenMinStats[i]);
		if (!i && sv[i] == 18)
			sv[i] |= (uint16)(_vm->rollDice(1, 100) << 8);
		if (sv[i] > _chargenMaxStats[i])
			sv[i] = _chargenMaxStats[i];
	}

	c->strengthCur = c->strengthMax = sv[0] & 0xff;
	c->strengthExtCur = c->strengthExtMax = sv[0] >> 8;
	c->intelligenceCur = c->intelligenceMax = sv[1] & 0xff;
	c->wisdomCur = c->wisdomMax = sv[2] & 0xff;
	c->dexterityCur = c->dexterityMax = sv[3] & 0xff;
	c->constitutionCur = c->constitutionMax = sv[4] & 0xff;
	c->charismaCur = c->charismaMax = sv[5] & 0xff;
	c->armorClass = 10 + _vm->getDexterityArmorClassModifier(sv[3] & 0xff);
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

	EobCharacter *c = &_characters[_activeBox];
	int8 hpLO = c->hitPointsCur;

	for (int i = 0; i >= 0 && i < 7; ) {
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
		in = _vm->checkInput(_vm->_activeButtons, false, 0);
		_vm->removeInputTop();

		if (in == 0x8001) {
			processSpecialButton(4);
			updateMagicShapes();
			generateStats(_activeBox);
			in = -1;
		} else if (in == 0x8002) {
			processSpecialButton(7);
			modifyMenu();
			in = -1;
		} else if (in == 0x8003) {
			processSpecialButton(8);
			faceSelectMenu();
			in = -1;
		} else if (in == 0x8004 || in == _vm->_keyMap[Common::KEYCODE_KP5]) {
			processSpecialButton(6);
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

	highlightBoxFrame(6 + _activeBox);
	highlightBoxFrame(-1);
}

void CharacterGenerator::faceSelectMenu() {
	int8 sp[4];
	memcpy(sp, _chargenSelectedPortraits2, sizeof(sp));
	_vm->removeInputTop();
	initButtonsFromList(21, 6);

	int charSex = _characters[_activeBox].raceSex % 2;
	int8 shp = charSex ? 26 : 0;

	printStats(_activeBox, 4);
	toggleSpecialButton(12, 0, 0);
	toggleSpecialButton(13, 0, 0);
	highlightBoxFrame(-1);

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
			in = _vm->checkInput(_vm->_activeButtons, false, 0);
			_vm->removeInputTop();

			highlightBoxFrame(box + 10);

			if (in == 0x8002 || in == _vm->_keyMap[Common::KEYCODE_RIGHT]) {
				processSpecialButton(13);
				in = 2;
			} else if (in > 0x8002 && in < 0x8007) {
				box = (in & 7) - 3;
				in = 3;
			} else if (in == 0x8001 || in == _vm->_keyMap[Common::KEYCODE_LEFT]) {
				processSpecialButton(12);
				in = 1;
			} else if (in == _vm->_keyMap[Common::KEYCODE_RETURN] || in == _vm->_keyMap[Common::KEYCODE_KP5]) {
				in = 3;
			} else if (in & 0x8000) {
				in &= 0xff;
			} else {
				in = 0;
			}
		}

		highlightBoxFrame(-1);

		if (in == 1)
			shp = getNextFreeFaceShape(shp - 1, charSex, -1, _chargenSelectedPortraits);
		else if (in == 2)
			shp = getNextFreeFaceShape(shp + 1, charSex, 1, _chargenSelectedPortraits);
		else if (in == 3)
			res = box;
	}

	if (!_vm->shouldQuit()) {
		highlightBoxFrame(-1);
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
	highlightBoxFrame(-1);
	if (index <= 48)
		_screen->drawShape(0, _characters[_activeBox].faceShape, _chargenBoxX[_activeBox], _chargenBoxY[_activeBox] + 1, 0);
	else
		toggleSpecialButton(index - 50, 0, 0);
}

void CharacterGenerator::printStats(int index, int mode) {
	_screen->copyRegion(0, 0, 160, 0, 160, 128, 2, 2, Screen::CR_NO_P_CHECK);
	_screen->_curPage = 2;

	EobCharacter *c = &_characters[index];

	if (mode != 4)
		_screen->drawShape(2, c->faceShape, 224, 2, 0);

	_screen->printShadedText(c->name, 160 + ((20 - strlen(c->name)) << 2), 35, 15, 0);
	_screen->printShadedText(_chargenRaceSexStrings[c->raceSex], 160 + ((20 - strlen(_chargenRaceSexStrings[c->raceSex])) << 2), 45, 15, 0);
	_screen->printShadedText(_chargenClassStrings[c->cClass], 160 + ((20 - strlen(_chargenClassStrings[c->cClass])) << 2), 54, 15, 0);

	for (int i = 0; i < 6; i++)
		_screen->printShadedText(_chargenStatStrings[i], 163, (i + 8) << 3, 15, 0);

	_screen->printShadedText(_chargenStrings1[2], 248, 64, 15, 0);

	Common::String str = Common::String::format(_chargenStrings1[3], _vm->getCharStrength(c->strengthCur, c->strengthExtCur).c_str(), c->intelligenceCur, c->wisdomCur, c->dexterityCur, c->constitutionCur, c->charismaCur);
	_screen->printShadedText(str.c_str(), 192, 64, 15, 0);

	str = Common::String::format(_chargenStrings1[4], c->armorClass, c->hitPointsMax);
	_screen->printShadedText(str.c_str(), 280, 64, 15, 0);

	const char *lvlStr = c->level[2] ? _chargenStrings1[7] : (c->level[1] ? _chargenStrings1[6] : _chargenStrings1[5]);
	str = Common::String::format(lvlStr, c->level[0], c->level[1], c->level[2]);
	_screen->printShadedText(str.c_str(), 280, 80, 15, 0);

	switch (mode) {
	case 1:
		toggleSpecialButton(4, 0, 2);
		toggleSpecialButton(7, 0, 2);
		toggleSpecialButton(8, 0, 2);
		toggleSpecialButton(6, 0, 2);
		break;

	case 2:
		toggleSpecialButton(16, 0, 2);
		toggleSpecialButton(9, 0, 2);
		break;

	case 3:
		toggleSpecialButton(10, 0, 2);
		toggleSpecialButton(11, 0, 2);
		toggleSpecialButton(9, 0, 2);
		break;

	default:
		break;
	}

	_screen->copyRegion(160, 0, 144, 64, 160, 128, 2, 0, Screen::CR_NO_P_CHECK);

	if (mode != 3)
		_screen->updateScreen();

	_screen->_curPage = 0;
}

void CharacterGenerator::processNameInput(int index, int len, int textColor) {
	Screen::FontId of = _screen->setFont(Screen::FID_6_FNT);

	// WORKAROUND for bug in original code:
	len = strlen(_characters[index].name);

	int xOffs = (60 - _screen->getFontWidth() * len) >> 1;
	_screen->printText(_chargenStrings1[1], _chargenNameFieldX[index], _chargenNameFieldY[index], 12, 12);
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
	uint8 *s1 = (uint8*) stat1;
	uint8 *s2 = (uint8*) stat2;

	initButtonsFromList(31, 10);
	Button *b = _vm->gui_getButton(_vm->_activeButtons, index + 1);

	printStats(_activeBox, 3);
	_vm->removeInputTop();

	Common::String statStr = index ? Common::String::format("%d", *s1) : _vm->getCharStrength(*s1, *s2);

	_screen->copyRegion(b->x - 112, b->y - 64, b->x + 32, b->y, 40, b->height, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->printShadedText(statStr.c_str(), b->x + 32, b->y, 6, 0);
	_screen->updateScreen();

	EobCharacter *c = &_characters[_activeBox];

	int ci = index;
	uint8 v2 = s2 ? *s2 : 0;

	if (index == 6) {
		_chargenMaxStats[6] = getMaxHp(c->cClass, c->constitutionCur, c->level[0], c->level[1], c->level[2]);
		_chargenMinStats[6] = getMinHp(c->cClass, c->constitutionCur, c->level[0], c->level[1], c->level[2]);
	}

	for (bool loop = true; loop && !_vm->shouldQuit(); ) {
		uint8 v1 = *s1;
		updateMagicShapes();
		int	inputFlag = _vm->checkInput(_vm->_activeButtons, false, 0);
		_vm->removeInputTop();

		if (inputFlag == _vm->_keyMap[Common::KEYCODE_LEFT] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP4] || inputFlag == _vm->_keyMap[Common::KEYCODE_MINUS] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP_MINUS] || inputFlag == 0x8009) {
			processSpecialButton(11);
			v1--;

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_RIGHT] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP6] || inputFlag == _vm->_keyMap[Common::KEYCODE_PLUS] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP_PLUS] || inputFlag == 0x8008) {
			processSpecialButton(10);
			v1++;

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_UP] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP8]) {
			ci = (ci - 1) % 7;
			loop = false;

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_DOWN] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP2]) {
			ci = (ci + 1) % 7;
			loop = false;

		} else if (inputFlag == _vm->_keyMap[Common::KEYCODE_o] || inputFlag == _vm->_keyMap[Common::KEYCODE_KP5] || inputFlag == _vm->_keyMap[Common::KEYCODE_ESCAPE] || inputFlag == 0x800A) {
			processSpecialButton(9);
			loop = false;
			ci = -2;

		} else if (inputFlag & 0x8000) {
			inputFlag = (inputFlag & 0x0f) - 1;
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

			v1 = CLIP<uint8>(v1, _chargenMinStats[index], _chargenMaxStats[index] & 0xff);
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

		statStr = index ? Common::String::format("%d", *s1) : _vm->getCharStrength(*s1, *s2);

		_screen->copyRegion(b->x - 112, b->y - 64, b->x + 32, b->y, 40, b->height, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->printShadedText(statStr.c_str(), b->x + 32, b->y, 6, 0);
		_screen->updateScreen();

		if (index == 4) {
			int oldVal = c->hitPointsCur;
			_chargenMaxStats[6] = getMaxHp(c->cClass, c->constitutionCur, c->level[0], c->level[1], c->level[2]);
			_chargenMinStats[6] = getMinHp(c->cClass, c->constitutionCur, c->level[0], c->level[1], c->level[2]);
			c->hitPointsMax = c->hitPointsCur = CLIP<int16>(c->hitPointsCur, _chargenMinStats[6], _chargenMaxStats[6]);

			if (c->hitPointsCur != oldVal) {
				statStr = Common::String::format("%d", c->hitPointsCur);
				_screen->copyRegion(120, 72, 264, 136, 40, 8, 2, 0, Screen::CR_NO_P_CHECK);
				_screen->printShadedText(statStr.c_str(), 264, 136, 15, 0);
				_screen->updateScreen();
			}

		} else if (index == 3) {
			int oldVal = c->armorClass;
			c->armorClass = _vm->getDexterityArmorClassModifier(v1) + 10;

			if (c->armorClass != oldVal) {
				statStr = Common::String::format("%d", c->armorClass);
				_screen->copyRegion(120, 64, 264, 128, 40, 8, 2, 0, Screen::CR_NO_P_CHECK);
				_screen->printShadedText(statStr.c_str(), 264, 128, 15, 0);
				_screen->updateScreen();
			}
		}

		if (loop == false) {
			statStr = index ? Common::String::format("%d", *s1) : _vm->getCharStrength(*s1, *s2);
			_screen->printText(statStr.c_str(), b->x + 32, b->y, 15, 0);
			_screen->updateScreen();
		}
	}

	return ci;
}

int CharacterGenerator::getMaxHp(int cclass, int constitution, int level1, int level2, int level3) {
	int res = 0;
	constitution = _vm->getClassAndConstHitpointsModifier(cclass, constitution);

	int m = _vm->getClassHpIncreaseType(cclass, 0);
	if (m != -1)
		res = _vm->getModifiedHpLimits(m, constitution, level1, false);

	m = _vm->getClassHpIncreaseType(cclass, 1);
	if (m != -1)
		res += _vm->getModifiedHpLimits(m, constitution, level2, false);

	m = _vm->getClassHpIncreaseType(cclass, 2);
	if (m != -1)
		res += _vm->getModifiedHpLimits(m, constitution, level3, false);

	res /= _vm->_numLevelsPerClass[cclass];

	return res;
}

int CharacterGenerator::getMinHp(int cclass, int constitution, int level1, int level2, int level3) {
	int res = 0;
	constitution = _vm->getClassAndConstHitpointsModifier(cclass, constitution);

	int m = _vm->getClassHpIncreaseType(cclass, 0);
	if (m != -1)
		res = _vm->getModifiedHpLimits(m, constitution, level1, true);

	m = _vm->getClassHpIncreaseType(cclass, 1);
	if (m != -1)
		res += _vm->getModifiedHpLimits(m, constitution, level2, true);

	m = _vm->getClassHpIncreaseType(cclass, 2);
	if (m != -1)
		res += _vm->getModifiedHpLimits(m, constitution, level3, true);

	res /= _vm->_numLevelsPerClass[cclass];

	return res;
}

void CharacterGenerator::finish() {
	_screen->copyRegion(0, 0, 160, 0, 160, 128, 2, 2, Screen::CR_NO_P_CHECK);
	int cp = _screen->setCurPage(2);
	_screen->printShadedText(_chargenEnterGameStrings[0], 168, 32, 15, 0);
	_screen->setCurPage(cp);
	_screen->copyRegion(160, 0, 144, 64, 160, 128, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	if (_vm->game() == GI_EOB1) {
		static const int8 classDefaultItemsList[] = { 1, 17, 2, 17, 46, -1, 4, -1, 5, -1, 6, 2, 7, -1, 8, -1, 9, 21, 10, 2, 31, 2 };
		static const int8 classDefaultItemsListIndex[] = { 4, 8, 0, -1, 4, 3, 0, -1, 4, 10, 0, 8, 3, 6, 1, -1, 2, 7, 0, -1,
			4, 5, 0, -1, 4, 7, 0, 8, 4, 5, 0, 8, 4, 6, 8, 8, 4, 6, 5, 8, 3, 6, 5, -1, 2, 7, 5, 0, 4, 6, 7, 0, 4, 3, 7, 0, 2, 6, 7, 1 };

		_characters[0].inventory[2] = _vm->duplicateItem(35);

		for (int i = 0; i < 4; i++) {
			EobCharacter *c = &_characters[i];
			c->flags = 1;
			c->food = 100;
			c->id = i;
			c->inventory[3] = _vm->duplicateItem(10);

			for (int ii = 0; ii < 4; ii++) {
				int l = classDefaultItemsListIndex[(c->cClass << 2) + ii] << 1;
				if (classDefaultItemsList[l] == -1)
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
		static const int8 *itemList[] = { itemList0, itemList1, itemList2, itemList3 };

		for (int i = 0; i < 4; i++) {
			EobCharacter *c = &_characters[i];
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
			_characters[i].mageSpellsAvailabilityFlags = (_vm->game() == GI_EOB2) ? 0x81CB6 : 0x26C;

		if (_vm->_classModifierFlags[_characters[i].cClass] & 0x14 && _vm->game() == GI_EOB2) {
			// Cleric: Turn Undead
			_characters[i].clericSpells[0] = 29;
		}
	}

	for (int i = 0; i < 4; i++) {
		EobCharacter *c = &_characters[i];
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

	if (_chargenButtonLabels) {
		for (int i = 0; i < 10; i++)
			delete[] _chargenButtonLabels[i];
		delete[] _chargenButtonLabels;
		_chargenButtonLabels = 0;
	}
}

const EobChargenButtonDef CharacterGenerator::_chargenButtonDefs[] = {
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

const EobRect8 CharacterGenerator::_chargenButtonBodyCoords[] = {
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

const EobRect16 CharacterGenerator::_chargenPortraitBoxFrames[] = {
	{ 0x00B7, 0x0001, 0x00F7, 0x0034 },
	{ 0x00FF, 0x0001, 0x013F, 0x0034 },
	{ 0x00B7, 0x0035, 0x00F7, 0x0068 },
	{ 0x00FF, 0x0035, 0x013F, 0x0068 },
	{ 0x00B7, 0x0069, 0x00F7, 0x009C },
	{ 0x00FF, 0x0069, 0x013F, 0x009C },
	{ 0x0010, 0x003F, 0x0030, 0x0060 },
	{ 0x0050, 0x003F, 0x0070, 0x0060 },
	{ 0x0010, 0x007F, 0x0030, 0x00A0 },
	{ 0x0050, 0x007F, 0x0070, 0x00A0 },
	{ 0x00B0, 0x0042, 0x00D0, 0x0061 },
	{ 0x00D0, 0x0042, 0x00F0, 0x0061 },
	{ 0x00F0, 0x0042, 0x0110, 0x0061 },
	{ 0x0110, 0x0042, 0x0130, 0x0061 },
	{ 0x0004, 0x0018, 0x0024, 0x0039 },
	{ 0x00A3, 0x0018, 0x00C3, 0x0039 },
	{ 0x0004, 0x0040, 0x0024, 0x0061 },
	{ 0x00A3, 0x0040, 0x00C3, 0x0061 },
	{ 0x0004, 0x0068, 0x0024, 0x0089 },
	{ 0x00A3, 0x0068, 0x00C3, 0x0089 }
};

const int16 CharacterGenerator::_chargenBoxX[] = { 0x10, 0x50, 0x10, 0x50 };
const int16 CharacterGenerator::_chargenBoxY[] = { 0x3F, 0x3F, 0x7F, 0x7F };
const int16 CharacterGenerator::_chargenNameFieldX[] = { 0x02, 0x42, 0x02, 0x42 };
const int16 CharacterGenerator::_chargenNameFieldY[] = { 0x6B, 0x6B, 0xAB, 0xAB };

const int32 CharacterGenerator::_classMenuMasks[] = {
	0x003F, 0x07BB, 0x77FB, 0x00F1, 0x08F1, 0x00B1
};

const int32 CharacterGenerator::_alignmentMenuMasks[] = {
	0x01FF, 0x0007, 0x0001, 0x01FF,	0x01FF, 0x01FE, 0x01FF, 0x01FE,
	0x01FF, 0x01FE, 0x01FE, 0x01FE, 0x01FF,	0x0007,	0x01FF
};

const int16 CharacterGenerator::_raceModifiers[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, -1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,	1, -1, 0, 1, -1, 0, 0, 0, -1, 0, 0, 1, 0, 0
};

}	// End of namespace Kyra

#endif // ENABLE_EOB
