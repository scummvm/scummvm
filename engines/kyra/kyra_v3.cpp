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
#include "kyra/kyra_v3.h"
#include "kyra/screen_v3.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"
#include "kyra/text.h"
#include "kyra/vqa.h"
#include "kyra/gui.h"

#include "common/system.h"
#include "common/config-manager.h"

namespace Kyra {
KyraEngine_v3::KyraEngine_v3(OSystem *system, const GameFlags &flags) : KyraEngine(system, flags) {
	_soundDigital = 0;
	_musicSoundChannel = -1;
	_menuAudioFile = "TITLE1.AUD";
	_curMusicTrack = -1;
	_itemBuffer1 = _itemBuffer2 = 0;
	_scoreFile = 0;
	_cCodeFile = 0;
	_scenesFile = 0;
	_itemFile = 0;
	_gamePlayBuffer = 0;
	_interface = _interfaceCommandLine = 0;
	_costPalBuffer = 0;
	_animObjects = 0;
	_sceneAnims = 0;
	memset(_sceneShapes, 0, sizeof(_sceneShapes));
	memset(_sceneAnimMovie, 0, sizeof(_sceneAnimMovie));
	_gfxBackUpRect = 0;
	_itemList = 0;
	_malcolmShapes = 0;
	_paletteOverlay = 0;
	_sceneList = 0;
	memset(&_mainCharacter, 0, sizeof(_mainCharacter));
	_mainCharacter.sceneId = 9;
	_mainCharacter.dlgIndex = 0;
	_mainCharacter.unk4 = 0x4C;
	_mainCharacter.facing = 5;
	_mainCharacter.walkspeed = 5;
	memset(_mainCharacter.inventory, -1, sizeof(_mainCharacter.inventory));
}

KyraEngine_v3::~KyraEngine_v3() {
	delete _screen;
	delete _soundDigital;

	delete [] _itemBuffer1;
	delete [] _itemBuffer2;
	delete [] _scoreFile;
	delete [] _cCodeFile;
	delete [] _scenesFile;
	delete [] _itemFile;
	delete [] _gamePlayBuffer;
	delete [] _interface;
	delete [] _interfaceCommandLine;
	delete [] _costPalBuffer;
	delete [] _animObjects;
	delete [] _sceneAnims;

	for (uint i = 0; i < ARRAYSIZE(_sceneShapes); ++i)
		delete _sceneShapes[i];

	for (uint i = 0; i < ARRAYSIZE(_sceneAnimMovie); ++i)
		delete _sceneAnimMovie[i];

	delete [] _gfxBackUpRect;
	delete [] _itemList;
	delete [] _paletteOverlay;
	delete [] _sceneList;

	for (ShapeMap::iterator i = _gameShapes.begin(); i != _gameShapes.end(); ++i) {
		delete i->_value;
		i->_value = 0;
	}
	_gameShapes.clear();
}

int KyraEngine_v3::init() {
	_screen = new Screen_v3(this, _system);
	assert(_screen);
	if (!_screen->init())
		error("_screen->init() failed");

	KyraEngine::init();

	_soundDigital = new SoundDigital(this, _mixer);
	assert(_soundDigital);
	if (!_soundDigital->init())
		error("_soundDigital->init() failed");

	_screen->loadFont(Screen::FID_6_FNT, "6.FNT");
	_screen->loadFont(Screen::FID_8_FNT, "8FAT.FNT");
	_screen->loadFont(Screen::FID_BOOKFONT_FNT, "BOOKFONT.FNT");
	_screen->setFont(Screen::FID_6_FNT);
	_screen->setAnimBlockPtr(3500);
	_screen->setScreenDim(0);
	
	_res->loadFileToBuf("PALETTE.COL", _screen->getPalette(0), 768);
	_screen->setScreenPalette(_screen->getPalette(0));

	return 0;
}

int KyraEngine_v3::go() {
	bool running = true;
	preinit();
	_screen->hideMouse();
	initMainMenu();

	_screen->clearPage(0);
	_screen->clearPage(2);

	while (running && !_quitFlag) {
		_screen->_curPage = 0;
		_screen->clearPage(0);

		_screen->setScreenPalette(_screen->getPalette(0));

		// XXX
		playMenuAudioFile();

		_menuAnim->setX(0); _menuAnim->setY(0);
		_menuAnim->setDrawPage(0);

		for (int i = 0; i < 64 && !_quitFlag; ++i) {
			uint32 nextRun = _system->getMillis() + 3 * _tickLength;
			_menuAnim->displayFrame(i, 0);
			_screen->updateScreen();
			delayUntil(nextRun);
		}

		for (int i = 64; i > 29 && !_quitFlag; --i) {
			uint32 nextRun = _system->getMillis() + 3 * _tickLength;
			_menuAnim->displayFrame(i, 0);
			_screen->updateScreen();
			delayUntil(nextRun);
		}

		switch (_menu->handle(3)) {
		case 0:
			uninitMainMenu();

			startup();
			// XXX
			running = false;
			break;

		case 1:
			_soundDigital->beginFadeOut(_musicSoundChannel, 60);
			_screen->fadeToBlack(60);
			playVQA("K3INTRO");
			_screen->hideMouse();
			break;

		case 2:
			//uninitMainMenu();
			//show load dialog
			//running = false;
			break;

		case 3:
			_soundDigital->beginFadeOut(_musicSoundChannel, 60);
			_screen->fadeToBlack();
			_soundDigital->stopSound(_musicSoundChannel);
			_musicSoundChannel = -1;
			uninitMainMenu();
			running = false;
			break;

		default:
			uninitMainMenu();
			quitGame();
			running = false;
			break;
		}
	}

	return 0;
}

void KyraEngine_v3::initMainMenu() {
	_menuAnim = createWSAMovie();
	_menuAnim->open("REVENGE.WSA", 1, _screen->getPalette(0));
	memset(_screen->getPalette(0), 0, 3);

	_menu = new MainMenu(this);
	MainMenu::StaticData data = {
		{ _mainMenuStrings[_lang*4+0], _mainMenuStrings[_lang*4+1], _mainMenuStrings[_lang*4+2], _mainMenuStrings[_lang*4+3] },
		{ 0x01, 0x04, 0x0C, 0x04, 0x00, 0x80, 0xFF, 0x00, 0x01, 0x02, 0x03 },
		{ 0x16, 0x19, 0x1A, 0x16 },
		0x80, 0xFF
	};

	MainMenu::Animation anim;
	anim.anim = _menuAnim;
	anim.startFrame = 29;
	anim.endFrame = 63;
	anim.delay = 2;

	_menu->init(data, anim);
}

void KyraEngine_v3::uninitMainMenu() {
	delete _menuAnim;
	_menuAnim = 0;
	delete _menu;
	_menu = 0;
}

void KyraEngine_v3::playVQA(const char *name) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::playVQA('%s')", name);

	memcpy(_screen->getPalette(1), _screen->getPalette(0), 768);

	VQAMovie vqa(this, _system);

	char filename[20];
	int size = 0;		// TODO: Movie size is 0, 1 or 2.

	snprintf(filename, sizeof(filename), "%s%d.VQA", name, size);

	if (vqa.open(filename)) {
		uint8 pal[768];
		memcpy(pal, _screen->getPalette(0), sizeof(pal));
		if (_screen->_curPage == 0)
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 0, 3);

		_screen->hideMouse();
		_soundDigital->beginFadeOut(_musicSoundChannel, 60);
		_musicSoundChannel = -1;
		_screen->fadeToBlack();
		vqa.setDrawPage(0);
		vqa.play();
		vqa.close();
		_screen->showMouse();

		if (_screen->_curPage == 0)
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 3, 0);
		_screen->setScreenPalette(pal);
	}

	memcpy(_screen->getPalette(0), _screen->getPalette(1), 768);
}

#pragma mark -

void KyraEngine_v3::playMenuAudioFile() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::playMenuAudioFile()");
	if (_soundDigital->isPlaying(_musicSoundChannel))
		return;

	Common::SeekableReadStream *stream = _res->getFileStream(_menuAudioFile);
	if (stream)
		_musicSoundChannel = _soundDigital->playSound(stream, true);
}

void KyraEngine_v3::playMusicTrack(int track, int force) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::playMusicTrack(%d, %d)", track, force);

	// XXX byte_3C87C compare

	if (_musicSoundChannel != -1 && !_soundDigital->isPlaying(_musicSoundChannel))
		force = 1;
	else if (_musicSoundChannel == -1)
		force = 1;

	if (track == _curMusicTrack && !force)
		return;

	stopMusicTrack();

	if (_musicSoundChannel == -1) {
		assert(track < _soundListSize && track >= 0);

		Common::SeekableReadStream *stream = _res->getFileStream(_soundList[track]);
		if (stream)
			_musicSoundChannel = _soundDigital->playSound(stream);
	}

	_musicSoundChannel = track;
}

void KyraEngine_v3::stopMusicTrack() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::stopMusicTrack()");

	if (_musicSoundChannel != -1 && _soundDigital->isPlaying(_musicSoundChannel))
		_soundDigital->stopSound(_musicSoundChannel);

	_curMusicTrack = -1;
	_musicSoundChannel = -1;
}

int KyraEngine_v3::musicUpdate(int forceRestart) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::musicUpdate(%d)", forceRestart);

	static uint32 mTimer = 0;
	static uint16 lock = 0;

	if (ABS<int>(_system->getMillis() - mTimer) > (int)(0x0F * _tickLength))
		mTimer = _system->getMillis();

	if (_system->getMillis() < mTimer && !forceRestart)
		return 1;

	if (!lock) {
		lock = 1;
		if (_musicSoundChannel >= 0) {
			if (!_soundDigital->isPlaying(_musicSoundChannel)) {
				if (_curMusicTrack != -1)
					playMusicTrack(_curMusicTrack, 1);
			}
		}
		lock = 0;
		mTimer = _system->getMillis() + 0x0F * _tickLength;
	}

	return 1;
}

#pragma mark -

void KyraEngine_v3::preinit() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::preinit()");
	
	_unkBuffer1040Bytes = new uint8[1040];
	_itemBuffer1 = new uint8[72];
	_itemBuffer2 = new uint8[144];
	initMouseShapes();
	initItems();

	_screen->setMouseCursor(0, 0, _gameShapes[0]);
}

void KyraEngine_v3::initMouseShapes() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::initMouseShapes()");
	uint8 *data = _res->fileData("MOUSE.SHP", 0);
	assert(data);
	for (int i = 0; i <= 6; ++i)
		_gameShapes[i] = _screen->makeShapeCopy(data, i);
	delete [] data;
}

void KyraEngine_v3::startup() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::startup()");
	musicUpdate(0);

	memset(_flagsTable, 0, sizeof(_flagsTable));
	setGameFlag(0x216);

	_gamePlayBuffer = new uint8[64000];
	musicUpdate(0);
	musicUpdate(0);

	_interface = new uint8[17920];
	_interfaceCommandLine = new uint8[3840];

	_screen->setFont(Screen::FID_6_FNT);

	//XXX
	musicUpdate(0);
	//XXX
	_costPalBuffer = new uint8[864];
	//XXX
	_animObjects = new AnimObj[67];
	_sceneAnims = new SceneAnim[16];

	musicUpdate(0);

	memset(_sceneShapes, 0, sizeof(_sceneShapes));
	_screenBuffer = new uint8[64000];

	musicUpdate(0);
	musicUpdate(0);

	if (!loadLanguageFile("ITEMS.", _itemFile))
		error("couldn't load ITEMS");
	if (!loadLanguageFile("C_CODE.", _cCodeFile))
		error("couldn't load C_CODE");
	if (!loadLanguageFile("SCENES.", _scenesFile))
		error("couldn't load SCENES");

	//XXX

	if ((_actorFileSize = loadLanguageFile("_ACTOR.", _actorFile)) == 0)
		error("couldn't load _ACTOR");

	musicUpdate(0);
	//XXX
	musicUpdate(0);
	openTalkFile(0);
	musicUpdate(0);
	_currentTalkFile = 0;
	openTalkFile(1);
	//XXX
	loadCostPal();
	musicUpdate(0);

	for (int i = 0; i < 16; ++i) {
		_sceneAnims[i].flags = 0;
		_sceneAnimMovie[i] = new WSAMovieV2(this, _screen);
		assert(_sceneAnimMovie[i]);
	}

	_screen->_curPage = 0;

	//XXX

	musicUpdate(0);
	updateMalcolmShapes();
	_gfxBackUpRect = new uint8[_screen->getRectSize(32, 32)];
	_itemList = new Item[50];
	resetItemList();

	loadShadowShape();
	//loadButtonShapes();
	musicUpdate(0);
	loadExtrasShapes();
	musicUpdate(0);
	loadMalcolmShapes(_malcolmShapes);
	musicUpdate(0);
	//initInventoryButtonList(1);
	loadInterfaceShapes();

	musicUpdate(0);
	_res->loadFileToBuf("PALETTE.COL", _screen->getPalette(0), 768);
	_paletteOverlay = new uint8[256];
	_screen->generateOverlay(_screen->getPalette(0), _paletteOverlay, 0xF0, 0x19);

	loadInterface();
	musicUpdate(0);

	clearAnimObjects();

	//XXX

	musicUpdate(0);
	memset(_hiddenItems, -1, sizeof(_hiddenItems));
	
	//resetNewSceneDlgState();

	_sceneList = new SceneDesc[98];
	musicUpdate(0);
	runStartupScript(1, 0);
	//openMoondomtrWsa();
	_soundDigital->beginFadeOut(_musicSoundChannel, 60);
	delayWithTicks(60);
	//XXX enterNewScene(_mainCharacter.sceneId, _mainCharacter.facing, 0, 0, 1);
	musicUpdate(0);
	_screen->showMouse();
	//XXX
}

void KyraEngine_v3::loadCostPal() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::loadCostPal()");
	_costPalBuffer = _res->fileData("_COSTPAL.DAT", 0);
}

void KyraEngine_v3::loadShadowShape() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::loadShadowShape()");
	_screen->loadBitmap("SHADOW.CSH", 3, 3, 0);
	addShapeToPool(_screen->getCPagePtr(3), 421, 0);
}

void KyraEngine_v3::loadExtrasShapes() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::loadExtrasShapes()");
	_screen->loadBitmap("EXTRAS.CSH", 3, 3, 0);
	for (int i = 0; i < 20; ++i)
		addShapeToPool(_screen->getCPagePtr(3), i+433, i);
	addShapeToPool(_screen->getCPagePtr(3), 453, 20);
	addShapeToPool(_screen->getCPagePtr(3), 454, 21);
}

void KyraEngine_v3::loadInterfaceShapes() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::loadInterfaceShapes()");
	_screen->loadBitmap("INTRFACE.CSH", 3, 3, 0);
	for (int i = 422; i <= 432; ++i)
		addShapeToPool(_screen->getCPagePtr(3), i, i-422);
}

void KyraEngine_v3::loadInterface() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::loadInterface()");
	_screen->loadBitmap("INTRFACE.CPS", 3, 3, 0);
	memcpy(_interface, _screen->getCPagePtr(3), 17920);
	memcpy(_interfaceCommandLine, _screen->getCPagePtr(3), 3840);
}

void KyraEngine_v3::initItems() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::initItems()");

	_screen->loadBitmap("ITEMS.CSH", 3, 3, 0);

	for (int i = 248; i <= 319; ++i)
		addShapeToPool(_screen->getCPagePtr(3), i, i-248);

	_screen->loadBitmap("ITEMS2.CSH", 3, 3, 0);

	for (int i = 320; i <= 397; ++i)
		addShapeToPool(_screen->getCPagePtr(3), i, i-320);

	uint32 size = 0;
	uint8 *itemsDat = _res->fileData("_ITEMS.DAT", &size);

	assert(size >= 72+144);

	memcpy(_itemBuffer1, itemsDat   ,  72);
	memcpy(_itemBuffer2, itemsDat+72, 144);

	delete [] itemsDat;

	_screen->_curPage = 0;
}

void KyraEngine_v3::runStartupScript(int script, int unk1) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::runStartupScript(%d, %d)", script, unk1);
	ScriptState state;
	ScriptData data;
	memset(&state, 0, sizeof(state));
	memset(&data, 0, sizeof(data));
	char filename[13];
	strcpy(filename, "_START0X.EMC");
	filename[7] = (script % 10) + '0';

	_scriptInterpreter->loadScript(filename, &data, &_opcodes);
	_scriptInterpreter->initScript(&state, &data);
	_scriptInterpreter->startScript(&state, 0);
	state.regs[6] = unk1;

	while (_scriptInterpreter->validScript(&state))
		_scriptInterpreter->runScript(&state);
}

void KyraEngine_v3::openTalkFile(int file) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::openTalkFile(%d)", file);
	char talkFilename[16];

	if (file == 0) {
		strcpy(talkFilename, "ANYTALK.TLK");
	} else {
		if (_currentTalkFile > 0)
			sprintf(talkFilename, "CH%dTALK.TLK", _currentTalkFile);
		_res->unloadPakFile(talkFilename);
		sprintf(talkFilename, "CH%dTALK.TLK", file);
	}

	_currentTalkFile = file;
	//TODO: support Kyra3 TLK files
	//_res->loadPakFile(talkFilename);
}

#pragma mark -

void KyraEngine_v3::addShapeToPool(const uint8 *data, int realIndex, int shape) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::addShapeToPool(%p, %d, %d)", data, realIndex, shape);
	ShapeMap::iterator iter = _gameShapes.find(realIndex);
	if (iter != _gameShapes.end()) {
		delete [] iter->_value;
		iter->_value = 0;
	}
	_gameShapes[realIndex] = _screen->makeShapeCopy(data, shape);
	assert(_gameShapes[realIndex]);
}

void KyraEngine_v3::loadMalcolmShapes(int newShapes) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::loadMalcolmShapes(%d)", newShapes);
	static const uint8 numberOffset[] = { 3, 3, 4, 4, 3, 3 };
	static const uint8 startShape[] = { 0x32, 0x58, 0x78, 0x98, 0xB8, 0xD8 };
	static const uint8 endShape[] = { 0x57, 0x77, 0x97, 0xB7, 0xD7, 0xF7 };
	static const char * const filenames[] = {
		"MSW##.SHP",
		"MTA##.SHP",
		"MTFL##.SHP",
		"MTFR##.SHP",
		"MTL##.SHP",
		"MTR#.SHP"
	};

	for (int i = 50; i <= 247; ++i) {
		if (i == 87)
			continue;

		ShapeMap::iterator iter = _gameShapes.find(i);
		if (iter != _gameShapes.end()) {
			delete iter->_value;
			iter->_value = 0;
		}
	}

	const char lowNum = (newShapes % 10) + '0';
	const char highNum = (newShapes / 10) + '0';

	for (int i = 0; i < 6; ++i) {
		char filename[16];
		strcpy(filename, filenames[i]);
		filename[numberOffset[i]+0] = highNum;
		filename[numberOffset[i]+1] = lowNum;
		_res->loadFileToBuf(filename, _screenBuffer, 64000);
		for (int j = startShape[i]; j < endShape[i]; ++j) {
			if (j == 87)
				continue;
			addShapeToPool(_screenBuffer, j, j-startShape[i]);
		}
	}

	_malcolmShapes = newShapes;
	updateMalcolmShapes();
}

void KyraEngine_v3::updateMalcolmShapes() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::updateMalcolmShapes()");
	assert(_malcolmShapes >= 0 && _malcolmShapes < _shapeDescsSize);
	_malcolmShapeXOffset = _shapeDescs[_malcolmShapes].xOffset;
	_malcolmShapeYOffset = _shapeDescs[_malcolmShapes].yOffset;
	_animObjects[0].width = _shapeDescs[_malcolmShapes].width;
	_animObjects[0].height = _shapeDescs[_malcolmShapes].height;
}

#pragma mark -

int KyraEngine_v3::getMaxFileSize(const char *file) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::getMaxFileSize(%s)", file);
	int size = 0;

	char buffer[32];

	for (int i = 0; i < _languageExtensionSize; ++i) {
		strncpy(buffer, file, 32);
		size = MAX<uint32>(size, _res->getFileSize(appendLanguage(buffer, i, sizeof(buffer))));
	}

	return size + 20;
}

char *KyraEngine_v3::appendLanguage(char *buf, int lang, int bufSize) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::appendLanguage([%p|'%s'], %d, %d)", (const void*)buf, buf, lang, bufSize);
	assert(lang < _languageExtensionSize);

	int size = strlen(buf) + strlen(_languageExtension[lang]);

	if (size > bufSize) {
		warning("buffer too small to append language extension");
		return 0;
	}

	char *temp = buf + strlen(buf);
	bufSize -= strlen(buf);

	strncat(temp, _languageExtension[lang], bufSize);

	return buf;
}

int KyraEngine_v3::loadLanguageFile(const char *file, uint8 *&buffer) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::loadLanguageFile('%s', %p)", file, (const void*)buffer);

	uint32 size = 0;
	char nBuf[32];
	strncpy(nBuf, file, 32);
	buffer = _res->fileData(appendLanguage(nBuf, _lang, sizeof(nBuf)), &size);

	return buffer ? size : 0 ;
}

Movie *KyraEngine_v3::createWSAMovie() {
	WSAMovieV2 *movie = new WSAMovieV2(this, _screen);
	assert(movie);
	return movie;
}

} // end of namespace Kyra

