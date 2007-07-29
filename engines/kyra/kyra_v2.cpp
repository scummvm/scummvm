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
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra.h"
#include "kyra/kyra_v2.h"
#include "kyra/screen.h"
#include "kyra/resource.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"
#include "kyra/script.h"
#include "kyra/text.h"
#include "kyra/timer.h"

#include "common/system.h"

namespace Kyra {

KyraEngine_v2::KyraEngine_v2(OSystem *system, const GameFlags &flags) : KyraEngine(system, flags) {
	memset(_defaultShapeTable, 0, sizeof(_defaultShapeTable));
	_mouseSHPBuf = 0;
	
	_gamePlayBuffer = 0;
	_cCodeBuffer = _optionsBuffer = _chapterBuffer = 0;
	
	_overwriteSceneFacing = false;
	_mainCharX = _mainCharY = -1;
	_drawNoShapeFlag = false;
	_charPalEntry = 0;
	_itemInHand = -1;
	_unkSceneScreenFlag1 = false;
	_noScriptEnter = true;
	_currentChapter = 0;
	_newChapterFile = 1;
	_handItemSet = -1;
	_lastProcessedSceneScript = 0;
	_specialSceneScriptRunFlag = false;
	memset(_animObjects, 0, sizeof(_animObjects));
	_unkHandleSceneChangeFlag = false;
	_pathfinderFlag = 0;
	
	memset(&_sceneScriptData, 0, sizeof(_sceneScriptData));
}

KyraEngine_v2::~KyraEngine_v2() {
	delete [] _mouseSHPBuf;
	delete _screen;
}

Movie *KyraEngine_v2::createWSAMovie() {
	return new WSAMovieV2(this);
}

int KyraEngine_v2::init() {
	_screen = new Screen_v2(this, _system);
	assert(_screen);
	if (!_screen->init())
		error("_screen->init() failed");

	KyraEngine::init();
	
	setupTimers();

	_screen->loadFont(_screen->FID_6_FNT, "6.FNT");
	_screen->loadFont(_screen->FID_8_FNT, "8FAT.FNT");
	_screen->loadFont(_screen->FID_GOLDFONT_FNT, "GOLDFONT.FNT");
	_screen->loadFont(_screen->FID_BOOKFONT_FNT, "BOOKFONT.FNT");
	_screen->setAnimBlockPtr(3504);
	_screen->setScreenDim(0);
	
	assert(_introStringsSize == 21);
	for (int i = 0; i < 21; i++)
		_introStringsDuration[i] = strlen(_introStrings[i]) * 8;
	
	// No mouse display in demo
	if (_flags.isDemo)
		return 0;

	_mouseSHPBuf = _res->fileData("PWGMOUSE.SHP", 0);
	assert(_mouseSHPBuf);

	for (int i = 0; i < 2; i++) {
		_defaultShapeTable[i] = _screen->getPtrToShape(_mouseSHPBuf, i);
		assert(_defaultShapeTable[i]);
	}

	_screen->setMouseCursor(0, 0, _defaultShapeTable[0]);
	return 0;
}

int KyraEngine_v2::go() {	
	if (_flags.isDemo) {
		static const char *soundFileList[] = {
			"K2_DEMO",
			"LOLSYSEX"
		};
		_sound->setSoundFileList(soundFileList, 2);
	} else {
		// TODO: move this to proper place
		static const char *soundFileList[] = {
			"K2INTRO"
		};
		_sound->setSoundFileList(soundFileList, 1);
	}
	_sound->loadSoundFile(0);

	// Temporary measure to work around the fact that there's 
	// several WSA files with identical names in different PAK files.
	_res->unloadPakFile("OUTFARM.PAK");
	_res->unloadPakFile("FLYTRAP.PAK");

	//seq_playSequences(kSequenceVirgin, kSequenceWestwood);
	mainMenu();

	return 0;
}

void KyraEngine_v2::mainMenu() {
	bool running = true;

	while (running && !_quitFlag) {
		seq_playSequences(kSequenceTitle);
		_screen->showMouse();
		
		switch (gui_handleMainMenu()) {
			case 0:
				_screen->showMouse();
				startup();
				runLoop();
				cleanup();
				running = false;
				break;
			case 1:
				seq_playSequences(kSequenceOverview, kSequenceZanFaun); 
				break;
			case 2:
				break;
			case 3:
				running = false;
				break;
			default:
				break;
		}
		_screen->hideMouse();
	}
}

void KyraEngine_v2::startup() {
	_screen->_curPage = 0;
	delete [] _mouseSHPBuf;
	_mouseSHPBuf = 0;

	memset(_defaultShapeTable, 0, sizeof(_defaultShapeTable));
	memset(_sceneShapeTable, 0, sizeof(_sceneShapeTable));
	_gamePlayBuffer = new uint8[46080];
	_unkBuf500Bytes = new uint8[500];
	
	loadMouseShapes();
	loadItemShapes();
	
	_screen->setMouseCursor(0, 0, getShapePtr(0));
	
	_screenBuffer = new uint8[64000];
	
	loadCCodeBuffer("C_CODE.XXX");
	loadOptionsBuffer("OPTIONS.XXX");
	loadChapterBuffer(_newChapterFile);
	
	_unkBuf200kByte = new uint8[200000];
	
	showMessageFromCCode(265, 150, 0);
	
	// XXX
	
	showMessageFromCCode(0, 0, 207);
	
	// XXX
	
	_screen->setShapePages(5, 3);

	memset(&_mainCharacter, 0, sizeof(_mainCharacter));
	_mainCharacter.height = 0x30;
	_mainCharacter.facing = 4;
	_mainCharacter.animFrame = 0x12;
	memset(_mainCharacter.inventory, -1, sizeof(_mainCharacter.inventory));
	
	memset(_sceneAnims, 0, sizeof(_sceneAnims));
	for (int i = 0; i < ARRAYSIZE(_sceneAnimMovie); ++i)
		_sceneAnimMovie[i] = new WSAMovieV2(this);
	memset(_wsaSlots, 0, sizeof(_wsaSlots));
	for (int i = 0; i < ARRAYSIZE(_wsaSlots); ++i)
		_wsaSlots[i] = new WSAMovieV2(this);
	
	_maskPage = 0;//_screen->getPagePtr(5);
	_screen->_curPage = 0;
	
	_objectList = new Object[72];
	memset(_objectList, 0, sizeof(Object)*72);
	_shapeDescTable = new ShapeDesc[55];
	memset(_shapeDescTable, 0, sizeof(ShapeDesc)*55);
	
	for (int i = 9; i <= 32; ++i) {
		_shapeDescTable[i-9].unk5 = 30;
		_shapeDescTable[i-9].unk7 = 55;
		_shapeDescTable[i-9].xAdd = -15;
		_shapeDescTable[i-9].yAdd = -50;
	}
	
	for (int i = 19; i <= 24; ++i) {
		_shapeDescTable[i-9].unk7 = 53;
		_shapeDescTable[i-9].yAdd = -51;
	}
	
	_gfxBackUpRect = new uint8[_screen->getRectSize(32, 32)];
	_itemList = new Item[30];
	resetItemList();
	//loadButtonShapes();
	_loadedZTable = 1;
	loadZShapes(_loadedZTable);
	loadInventoryShapes();

	_res->loadFileToBuf("PALETTE.COL", _screen->_currentPalette, 0x300);
	_screen->loadBitmap("_PLAYFLD.CPS", 3, 3, 0);
	_screen->copyPage(3, 0);
	_screen->showMouse();
	_screen->hideMouse();
	
	clearAnimObjects();
	
	// XXX
	
	_sceneList = new SceneDesc[86];
	runStartScript(1, 0);
	loadNPCScript();
	
	// XXX
	
	enterNewScene(_mainCharacter.sceneId, _mainCharacter.facing, 0, 0, 1);
	_screen->showMouse();
	
	//sub_20EE8(1);
	//setNextIdleAnimTimer();
	//XXX
	_timer->setDelay(0, 5);
}

void KyraEngine_v2::runLoop() {
	_screen->updateScreen();
	
	_quitFlag = false;
	while (!_quitFlag) {
		//XXX
		int inputFlag = checkInput(0/*dword_324C5*/);
		update();
		if (inputFlag == 198 || inputFlag == 199) {
			_unk3 = _handItemSet;
			Common::Point mouse = getMousePos();
			handleInput(mouse.x, mouse.y);
		}
		//XXX
	}
}

void KyraEngine_v2::handleInput(int x, int y) {
	//setNextIdleAnimTimer();
	if (_unk5) {
		_unk5 = 0;
		return;
	}
	
	if (!_screen->isMouseVisible())
		return;
	
	if (_unk3 == -2) {
		//snd_playSfx(13);
		return;
	}
	
	//setNextIdleAnimTimer();
	
	if (x <= 6 || x >= 312 || y <= 6 || y >= 135) {
		bool exitOk = false;
		assert(_unk3 + 6 >= 0);
		switch (_unk3 + 6) {
		case 0:
			if (_sceneExit1 != 0xFFFF)
				exitOk = true;
			break;
		
		case 1:
			if (_sceneExit2 != 0xFFFF)
				exitOk = true;
			break;
		
		case 2:
			if (_sceneExit3 != 0xFFFF)
				exitOk = true;
			break;
		
		case 3:
			if (_sceneExit4 != 0xFFFF)
				exitOk = true;
			break;
		
		default:
			break;
		}
		
		if (exitOk) {
			inputSceneChange(x, y, 1, 1);
			return;
		}
	}
	
	if (checkCharCollision(x, y) >= 0 && _unk3 >= -1) {
		runSceneScript2();
		return;
	} else {
		//XXX
	}
	
	//XXX
	
	inputSceneChange(x, y, 1, 1);
}

int KyraEngine_v2::update() {
	refreshAnimObjectsIfNeed();
	updateMouse();
	updateSpecialSceneScripts();
	_timer->update();
	//sub_274C0();
	//updateInvWsa();
	//sub_1574C();
	//XXX
	_screen->updateScreen();
	return 0;
}

void KyraEngine_v2::updateMouse() {
	int shapeIndex = 0;
	int type = 0;
	int xOffset = 0, yOffset = 0;
	Common::Point mouse = getMousePos();
	
	if (mouse.y <= 145) {
		if (mouse.x <= 6) {
			if (_sceneExit4 != 0xFFFF) {
				type = -3;
				shapeIndex = 4;
				xOffset = 1;
				yOffset = 5;
			} else {
				type = -2;
			}
		} else if (mouse.x >= 312) {
			if (_sceneExit2 != 0xFFFF) {
				type = -5;
				shapeIndex = 2;
				xOffset = 7;
				yOffset = 5;
			} else {
				type = -2;
			}			
		} else if (mouse.y >= 135) {
			if (_sceneExit3 != 0xFFFF) {
				type = -4;
				shapeIndex = 3;
				xOffset = 5;
				yOffset = 10;
			} else {
				type = -2;
			}			
		} else if (mouse.y <= 6) {
			if (_sceneExit1 != 0xFFFF) {
				type = -6;
				shapeIndex = 1;
				xOffset = 5;
				yOffset = 1;
			} else {
				type = -2;
			}			
		}
	}
	
	for (int i = 0; i < _specialExitCount; ++i) {
		if (checkSpecialSceneExit(i, mouse.x, mouse.y)) {
			switch (_specialExitTable[20+i]) {
			case 0:
				type = -6;
				shapeIndex = 1;
				xOffset = 5;
				yOffset = 1;
				break;
			
			case 2:
				type = -5;
				shapeIndex = 2;
				xOffset = 7;
				yOffset = 5;
				break;
			
			case 4:
				type = -4;
				shapeIndex = 3;
				xOffset = 5;
				yOffset = 7;
				break;
			
			case 6:
				type = -3;
				shapeIndex = 4;
				xOffset = 1;
				yOffset = 5;
				break;
			
			default:
				break;
			}
		}
	}
	
	if (type == -2) {
		shapeIndex = 5;
		xOffset = 5;
		yOffset = 9;
	}
	
	if (type != 0 && _handItemSet != type) {
		_handItemSet = type;
		_screen->hideMouse();
		_screen->setMouseCursor(xOffset, yOffset, getShapePtr(shapeIndex));
		_screen->showMouse();
	}
	
	if (type == 0 && _handItemSet != _itemInHand) {
		if ((mouse.y > 145) || (mouse.x > 6 && mouse.x < 312 && mouse.y > 6 && mouse.y < 135)) {
			_handItemSet = _itemInHand;
			_screen->hideMouse();
			if (_itemInHand == -1)
				_screen->setMouseCursor(0, 0, getShapePtr(0));
			else
				_screen->setMouseCursor(8, 15, getShapePtr(_itemInHand+64));
			_screen->showMouse();
		}
	}
}

int KyraEngine_v2::checkInput(void *p) {
	Common::Event event;
	int keys = 0;
	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_RETURN)
				keys = 199;
			break;

		case Common::EVENT_LBUTTONUP:
			keys = 198;
			break;

		case Common::EVENT_QUIT:
			_quitFlag = true;
			break;
		
		default:
			break;
		}
	
		//if ( _debugger->isAttached())
		//	_debugger->onFrame();
	}
	
	_system->delayMillis(10);
	return keys;
}

void KyraEngine_v2::cleanup() {
	delete [] _gamePlayBuffer;
	delete [] _unkBuf500Bytes;
	delete [] _screenBuffer;
	delete [] _unkBuf200kByte;
	
	for (int i = 0; i < ARRAYSIZE(_defaultShapeTable); ++i)
		delete [] _defaultShapeTable[i];
	freeSceneShapePtrs();
	
	delete [] _cCodeBuffer;
	delete [] _optionsBuffer;
	delete [] _chapterBuffer;
	
	delete [] _objectList;
	delete [] _shapeDescTable;
	
	delete [] _gfxBackUpRect;
	
	delete [] _sceneList;
	
	for (int i = 0; i < ARRAYSIZE(_sceneAnimMovie); ++i)
		delete _sceneAnimMovie[i];
	for (int i = 0; i < ARRAYSIZE(_wsaSlots); ++i)
		delete _wsaSlots[i];
}

#pragma mark - Localization

void KyraEngine_v2::loadCCodeBuffer(const char *file) {
	char tempString[13];
	strcpy(tempString, file);
	changeFileExtension(tempString);
	
	delete [] _cCodeBuffer;
	_cCodeBuffer = _res->fileData(tempString, 0);
}

void KyraEngine_v2::loadOptionsBuffer(const char *file) {
	char tempString[13];
	strcpy(tempString, file);
	changeFileExtension(tempString);
	
	delete [] _optionsBuffer;
	_optionsBuffer = _res->fileData(tempString, 0);
}

void KyraEngine_v2::loadChapterBuffer(int chapter) {
	char tempString[14];

	static const char *chapterFilenames[] = {
		"CH1.XXX", "CH2.XXX", "CH3.XXX", "CH4.XXX", "CH5.XXX"
	};

	assert(chapter >= 1 && chapter <= ARRAYSIZE(chapterFilenames));
	strcpy(tempString, chapterFilenames[chapter-1]);
	changeFileExtension(tempString);
	
	delete [] _chapterBuffer;
	_chapterBuffer = _res->fileData(tempString, 0);
	_currentChapter = chapter;
}

void KyraEngine_v2::changeFileExtension(char *buffer) {
	while (*buffer != '.') ++buffer;

	++buffer;
	strcpy(buffer, _languageExtension[_lang]);
}

const uint8 *KyraEngine_v2::getTableEntry(const uint8 *buffer, int id) {
	return buffer + READ_LE_UINT16(buffer + (id<<1));
}

const char *KyraEngine_v2::getTableString(int id, const uint8 *buffer, int decode) {
	const char *string = (const char*)getTableEntry(buffer, id);
	
	if (decode) {
		decodeString1(string, _internStringBuf);
		decodeString2(_internStringBuf, _internStringBuf);
		string = _internStringBuf;
	}
	
	return string;
}

const char *KyraEngine_v2::getChapterString(int id) {
	if (_currentChapter != _newChapterFile)
		loadChapterBuffer(_newChapterFile);
	
	return getTableString(id, _chapterBuffer, 1);
}

int KyraEngine_v2::decodeString1(const char *src, char *dst) {
	static const uint8 decodeTable1[] = {
		0x20, 0x65, 0x74, 0x61, 0x69, 0x6E, 0x6F, 0x73, 0x72, 0x6C, 0x68,
		0x63, 0x64, 0x75, 0x70, 0x6D
	};
	
	static const uint8 decodeTable2[] = {
		0x74, 0x61, 0x73, 0x69, 0x6F, 0x20, 0x77, 0x62, 0x20, 0x72, 0x6E,
		0x73, 0x64, 0x61, 0x6C, 0x6D, 0x68, 0x20, 0x69, 0x65, 0x6F, 0x72,
		0x61, 0x73, 0x6E, 0x72, 0x74, 0x6C, 0x63, 0x20, 0x73, 0x79, 0x6E,
		0x73, 0x74, 0x63, 0x6C, 0x6F, 0x65, 0x72, 0x20, 0x64, 0x74, 0x67,
		0x65, 0x73, 0x69, 0x6F, 0x6E, 0x72, 0x20, 0x75, 0x66, 0x6D, 0x73,
		0x77, 0x20, 0x74, 0x65, 0x70, 0x2E, 0x69, 0x63, 0x61, 0x65, 0x20,
		0x6F, 0x69, 0x61, 0x64, 0x75, 0x72, 0x20, 0x6C, 0x61, 0x65, 0x69,
		0x79, 0x6F, 0x64, 0x65, 0x69, 0x61, 0x20, 0x6F, 0x74, 0x72, 0x75,
		0x65, 0x74, 0x6F, 0x61, 0x6B, 0x68, 0x6C, 0x72, 0x20, 0x65, 0x69,
		0x75, 0x2C, 0x2E, 0x6F, 0x61, 0x6E, 0x73, 0x72, 0x63, 0x74, 0x6C,
		0x61, 0x69, 0x6C, 0x65, 0x6F, 0x69, 0x72, 0x61, 0x74, 0x70, 0x65,
		0x61, 0x6F, 0x69, 0x70, 0x20, 0x62, 0x6D
	};

	int size = 0;
	uint cChar = 0;
	while ((cChar = *src++) != 0) {
		if (cChar & 0x80) {
			cChar &= 0x7F;
			int index = (cChar & 0x78) >> 3;
			*dst++ = decodeTable1[index];
			++size;
			assert(cChar < sizeof(decodeTable2));
			cChar = decodeTable2[cChar];
		}

		*dst++ = cChar;
		++size;
	}

	*dst++ = 0;
	return size;
}

void KyraEngine_v2::decodeString2(const char *src, char *dst) {
	if (!src || !dst)
		return;

	char out = 0;
	while ((out = *src) != 0) {
		if (*src == 0x1B) {
			++src;
			out = *src + 0x7F;
		}
		*dst++ = out;
		++src;
	}

	*dst = 0;
}

#pragma mark -

void KyraEngine_v2::showMessageFromCCode(int id, int16 palIndex, int) {
	const char *string = getTableString(id, _cCodeBuffer, 1);
	showMessage(string, palIndex);
}

void KyraEngine_v2::showMessage(const char *string, int16 palIndex) {
	_shownMessage = string;
	_screen->hideMouse();
	_screen->fillRect(0, 190, 319, 199, 0xCF);
	
	if (string) {
		if (palIndex != -1 || _msgUnk1) {
			palIndex *= 3;
			memcpy(_messagePal, _screen->_currentPalette + palIndex, 3);
			memmove(_screen->_currentPalette + 765, _screen->_currentPalette + palIndex, 3);
			_screen->setScreenPalette(_screen->_currentPalette);
		}

		int x = _text->getCenterStringX(string, 0, 320);
		_text->printText(string, x, 190, 255, 207, 0);

		setTimer1DelaySecs(7);
	}
	
	_msgUnk1 = 0;
	_screen->showMouse();
}

void KyraEngine_v2::showChapterMessage(int id, int16 palIndex) {
	showMessage(getChapterString(id), palIndex);
}

void KyraEngine_v2::loadMouseShapes() {
	_screen->loadBitmap("_MOUSE.CSH", 3, 3, 0);
	
	for (int i = 0; i <= 8; ++i) {
		_defaultShapeTable[i] = _screen->makeShapeCopy(_screen->getCPagePtr(3), i);
		assert(_defaultShapeTable[i]);
	}
}

void KyraEngine_v2::loadItemShapes() {
	_screen->loadBitmap("_ITEMS.CSH", 3, 3, 0);
	
	for (int i = 64; i <= 239; ++i) {
		_defaultShapeTable[i] = _screen->makeShapeCopy(_screen->getCPagePtr(3), i-64);
		assert(_defaultShapeTable[i]);
	}
	
	_res->loadFileToBuf("_ITEMHT.DAT", _itemHtDat, sizeof(_itemHtDat));
	assert(_res->getFileSize("_ITEMHT.DAT") == sizeof(_itemHtDat));
	
	_screen->_curPage = 0;
}

void KyraEngine_v2::loadZShapes(int shapes) {
	char file[10];
	strcpy(file, "_ZX.SHP");
	
	_loadedZTable = shapes;
	file[2] = '0' + shapes;
	
	uint8 *data = _res->fileData(file, 0);
	for (int i = 9; i <= 32; ++i) {
		delete [] _defaultShapeTable[i];
		_defaultShapeTable[i] = _screen->makeShapeCopy(data, i-9);
	}
	delete [] data;
	
	_loadedZTable = shapes;
}

void KyraEngine_v2::loadInventoryShapes() {
	int curPageBackUp = _screen->_curPage;
	_screen->_curPage = 2;
	
	_screen->loadBitmap("_PLAYALL.CPS", 3, 3, 0);
	
	for (int i = 0; i < 10; ++i)
		_defaultShapeTable[240+i] = _screen->encodeShape(_inventoryX[i], _inventoryY[i], 16, 16, 0);
	
	_screen->_curPage = curPageBackUp;
}

void KyraEngine_v2::runStartScript(int script, int unk1) {
	char filename[14];
	strcpy(filename, "_START0X.EMC");
	filename[7] = script + '0';

	ScriptData scriptData;
	ScriptState scriptState;
	
	_scriptInterpreter->loadScript(filename, &scriptData, &_opcodes);
	_scriptInterpreter->initScript(&scriptState, &scriptData);
	scriptState.regs[6] = unk1;
	_scriptInterpreter->startScript(&scriptState, 0);
	while (_scriptInterpreter->validScript(&scriptState))
		_scriptInterpreter->runScript(&scriptState);
	_scriptInterpreter->unloadScript(&scriptData);
}

void KyraEngine_v2::loadNPCScript() {
	char filename[12];
	strcpy(filename, "_NPC.EMC");
	
	switch (_lang) {
	case 0:
		filename[5] = 'E';
		break;
	
	case 1:
		filename[5] = 'F';
		break;
	
	case 2:
		filename[5] = 'G';
		break;
	
	default:
		break;
	};
	
	_scriptInterpreter->loadScript(filename, &_npcScriptData, &_opcodes);
}

void KyraEngine_v2::resetScaleTable() {
	for (int i = 0; i < ARRAYSIZE(_scaleTable); ++i)
		_scaleTable[i] = 0x100;
}

void KyraEngine_v2::setScaleTableItem(int item, int data) {
	if (item >= 1 || item <= 15)
		_scaleTable[item-1] = (data << 8) / 100;
}

int KyraEngine_v2::getScale(int x, int y) {
	return _scaleTable[_screen->getLayer(x, y) - 1];
}

void KyraEngine_v2::setDrawLayerTableEntry(int entry, int data) {
	if (entry >= 1 || entry <= 15)
		_drawLayerTable[entry-1] = data;
}

int KyraEngine_v2::getDrawLayer(int x, int y) {
	int layer = _screen->getLayer(x, y);
	layer = _drawLayerTable[layer-1];
	if (layer < 0)
		layer = 0;
	else if (layer >= 7)
		layer = 6;
	return layer;
}

void KyraEngine_v2::restorePage3() {
	_screen->copyBlockToPage(2, 0, 0, 320, 144, _gamePlayBuffer);
}

void KyraEngine_v2::updateCharPal(int unk1) {
	static bool unkVar1 = false;

	if (!_useCharPal)
		return;

	int layer = _screen->getLayer(_mainCharacter.x1, _mainCharacter.y1);
	int palEntry = _charPalTable[layer];

	if (palEntry != _charPalEntry && unk1) {
		const uint8 *src = &_scenePal[(palEntry << 4) * 3];
		uint8 *ptr = _screen->getPalette(0) + 336;
		for (int i = 0; i < 48; ++i) {
			*ptr -= (*ptr - *src) >> 1;
			++ptr;
			++src;
		}
		_screen->setScreenPalette(_screen->getPalette(0));
		unkVar1 = true;
		_charPalEntry = palEntry;
	} else if (unkVar1 && !unk1) {
		memcpy(_screen->getPalette(0) + 336, &_scenePal[(palEntry << 4) * 3], 48);
		_screen->setScreenPalette(_screen->getPalette(0));
		unkVar1 = false;
	}
}

int KyraEngine_v2::inputSceneChange(int x, int y, int unk1, int unk2) {
	bool refreshNPC = false;
	uint16 curScene = _mainCharacter.sceneId;
	_pathfinderFlag = 15;

	if (!_unkHandleSceneChangeFlag) {
		if (_unk3 == -3) {
			if (_sceneList[curScene].exit4 != 0xFFFF) {
				x = 4;
				y = _sceneEnterY4;
				_pathfinderFlag = 7;
			}				
		} else if (_unk3 == -5) {
			if (_sceneList[curScene].exit2 != 0xFFFF) {
				x = 316;
				y = _sceneEnterY2;
				_pathfinderFlag = 7;
			}		
		} else if (_unk3 == -6) {
			if (_sceneList[curScene].exit1 != 0xFFFF) {
				x = _sceneEnterX1;
				y = _sceneEnterY1 - 2;
				_pathfinderFlag = 14;
			}
		} else if (_unk3 == -4) {
			if (_sceneList[curScene].exit3 != 0xFFFF) {
				x = _sceneEnterX3;
				y = 147;
				_pathfinderFlag = 11;
			}
		}
	}
	
	if (_pathfinderFlag) {
		if (findItem(curScene, 13) >= 0 && _unk3 <= -3) {
			//XXX
			_pathfinderFlag = 0;
			return 0;
		} else if (_itemInHand == 72) {
			//XXX
			_pathfinderFlag = 0;
			return 0;
		} else if (findItem(curScene, 72) >= 0 && _unk3 <= -3) {
			//XXX
			_pathfinderFlag = 0;
			return 0;
		} else if (0/*XXX*/) {
			//XXX
			_pathfinderFlag = 0;
			return 0;
		}
	}
	
	if (ABS(_mainCharacter.x1 - x) < 4 || ABS(_mainCharacter.y1 - y) < 2)
		return 0;
	
	int curX = _mainCharacter.x1 & ~3;
	int curY = _mainCharacter.y1 & ~1;
	int dstX = x & ~3;
	int dstY = y & ~1;
	
	int wayLength = findWay(curX, curY, dstX, dstY, _movFacingTable, 600);
	_pathfinderFlag = 0;
	_timer->disable(5);
	
	if (wayLength != 0 && wayLength != 0x7D00)
		refreshNPC = trySceneChange(_movFacingTable, unk1, unk2);

	//XXX
	
	if (refreshNPC)
		enterNewSceneUnk2(0);
	
	_pathfinderFlag = 0;
	return refreshNPC;
}

bool KyraEngine_v2::checkSpecialSceneExit(int num, int x, int y) {
	if (_specialExitTable[0+num] > x || _specialExitTable[5+num] > y ||
		_specialExitTable[10+num] < x || _specialExitTable[15+num] < y)
		return 0;
	return 1;
}

void KyraEngine_v2::moveCharacter(int facing, int x, int y) {
	_mainCharacter.facing = facing;
	x &= ~3;
	y &= ~1;
	
	_screen->hideMouse();
	switch (facing) {
	case 0:
		while (y < _mainCharacter.y1)
			updateCharPosWithUpdate();
		break;
		
	case 2:	
		while (_mainCharacter.x1 < x)
			updateCharPosWithUpdate();
		break;
		
	case 4:
		while (y > _mainCharacter.y1)
			updateCharPosWithUpdate();
		break;
		
	case 6:
		while (_mainCharacter.x1 > x)
			updateCharPosWithUpdate();
		break;
		
	default:
		break;
	}

	_screen->showMouse();
}

int KyraEngine_v2::updateCharPos(int *table) {
	static uint32 nextUpdate = 0;
	static const int updateX[] = { 0, 4, 4, 4, 0, -4, -4, -4 };
	static const int updateY[] = { -2, -2, 0, 2, 2, 2, 0, -2 };

	if (_system->getMillis() < nextUpdate)
		return 0;
	
	int facing = _mainCharacter.facing;
	_mainCharacter.x1 += updateX[facing];
	_mainCharacter.y1 += updateY[facing];
	updateCharAnimFrame(0, table);
	nextUpdate = _system->getMillis() + _timer->getDelay(0) * _tickLength;
	return 1;
}

void KyraEngine_v2::updateCharPosWithUpdate() {
	updateCharPos(0);
	update();
}

void KyraEngine_v2::updateCharAnimFrame(int charId, int *table) {
	static int unkTable1[] = { 0, 0 };
	static const int unkTable2[] = { 17, 0 };
	static const int unkTable3[] = { 10, 0 };
	static const int unkTable4[] = { 24, 0 };
	static const int unkTable5[] = { 19, 0 };
	static const int unkTable6[] = { 21, 0 };
	static const int unkTable7[] = { 31, 0 };
	static const int unkTable8[] = { 26, 0 };

	Character *character = &_mainCharacter;
	++character->animFrame;
	int facing = character->facing;

	if (table) {
		if (table[0] != table[-1] && table[-1] == table[1]) {
			facing = getOppositeFacingDirection(table[-1]);
			table[0] = table[-1];
		}
	}

	if (!facing) {
		++unkTable1[charId];
	} else if (facing == 4) {
		++unkTable1[charId+1];
	} else if (facing == 7 || facing == 1 || facing == 5 || facing == 3) {
		if (facing == 7 || facing == 1) {
			if (unkTable1[charId] > 2)
				facing = 0;
		} else {
			if (unkTable1[charId+1] > 2)
				facing = 4;
		}

		unkTable1[charId] = 0;
		unkTable1[charId+1] = 0;
	}

	if (facing == 0) {
		if (character->animFrame < unkTable8[charId])
			character->animFrame = unkTable8[charId];

		if (character->animFrame > unkTable7[charId])
			character->animFrame = unkTable8[charId];
	} else if (facing == 4) {
		if (character->animFrame < unkTable5[charId])
			character->animFrame = unkTable5[charId];

		if (character->animFrame > unkTable4[charId])
			character->animFrame = unkTable5[charId];
	} else {
		if (character->animFrame > unkTable5[charId])
			character->animFrame = unkTable6[charId];

		if (character->animFrame == unkTable2[charId])
			character->animFrame = unkTable3[charId];

		if (character->animFrame > unkTable2[charId])
			character->animFrame = unkTable3[charId] + 2;
	}

	updateCharacterAnim(charId);
}

int KyraEngine_v2::checkCharCollision(int x, int y) {
	int scale1 = 0, scale2 = 0, scale3 = 0;
	int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	scale1 = getScale(_mainCharacter.x1, _mainCharacter.y1);
	scale2 = (scale1 * 24) >> 8;
	scale3 = (scale1 * 48) >> 8;
	
	x1 = _mainCharacter.x1 - (scale2 >> 1);
	x2 = _mainCharacter.x1 + (scale2 >> 1);
	y1 = _mainCharacter.y1 - scale3;
	y2 = _mainCharacter.y1;
	
	if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
		return 0;
	
	return -1;
}

#pragma mark -

typedef Functor1Mem<ScriptState*, int, KyraEngine_v2> OpcodeV2;
#define Opcode(x) OpcodeV2(this, &KyraEngine_v2::x)
#define OpcodeUnImpl() OpcodeV2(this, 0)
void KyraEngine_v2::setupOpcodeTable() {
	static const OpcodeV2 opcodeTable[] = {
		// 0x00
		Opcode(o2_setCharacterFacingRefresh),
		OpcodeUnImpl(),
		Opcode(o2_defineObject),
		Opcode(o2_refreshCharacter),
		// 0x04
		Opcode(o2_getCharacterX),
		Opcode(o2_getCharacterY),
		Opcode(o2_getCharacterFacing),
		OpcodeUnImpl(),
		// 0x08
		Opcode(o2_setSceneComment),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x0c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x10
		OpcodeUnImpl(),
		Opcode(o2_showChapterMessage),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x14
		Opcode(o2_wsaClose),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o2_displayWsaFrame),
		// 0x18
		Opcode(o2_displayWsaSequentialFrames),
		Opcode(o2_wsaOpen),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x1c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x20
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o2_defineItem),
		// 0x24
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o2_queryGameFlag),
		// 0x28
		Opcode(o2_resetGameFlag),
		Opcode(o2_setGameFlag),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x2c
		OpcodeUnImpl(),
		Opcode(o2_hideMouse),
		Opcode(o2_addSpecialExit),
		OpcodeUnImpl(),
		// 0x30
		Opcode(o2_showMouse),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x34
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x38
		Opcode(o2_dummy),
		OpcodeUnImpl(),
		Opcode(o2_setScaleTableItem),
		Opcode(o2_setDrawLayerTableItem),
		// 0x3c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o2_drawSceneShapeOnPage),
		// 0x40
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o2_dummy),
		OpcodeUnImpl(),
		// 0x44
		OpcodeUnImpl(),
		Opcode(o2_restoreBackBuffer),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x48
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x4c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o2_dummy),
		Opcode(o2_dummy),
		// 0x50
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x54
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x58
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x5c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x60
		Opcode(o2_getRand),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x64
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x68
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x6c
		Opcode(o2_encodeShape),
		Opcode(o2_defineRoomEntrance),
		OpcodeUnImpl(),
		Opcode(o2_setSpecialSceneScriptRunTime),
		// 0x70
		Opcode(o2_defineSceneAnim),
		Opcode(o2_updateSceneAnim),
		Opcode(o2_updateSceneAnim),
		OpcodeUnImpl(),
		// 0x74
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x78
		OpcodeUnImpl(),
		Opcode(o2_defineRoom),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x7c
		OpcodeUnImpl(),
		Opcode(o2_dummy),
		Opcode(o2_dummy),
		OpcodeUnImpl(),
		// 0x80
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x84
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x88
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x8c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o2_setSpecialSceneScriptState),
		// 0x90
		Opcode(o2_clearSpecialSceneScriptState),
		Opcode(o2_querySpecialSceneScriptState),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x94
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o2_wsaClose),
		OpcodeUnImpl(),
		// 0x98
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0x9c
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0xa0
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0xa4
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0xa8
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		// 0xac
		OpcodeUnImpl(),
		OpcodeUnImpl(),
		Opcode(o2_dummy),
		Opcode(o2_dummy),
	};
	
	for (int i = 0; i < ARRAYSIZE(opcodeTable); ++i)
		_opcodes.push_back(&opcodeTable[i]);
}

} // end of namespace Kyra

