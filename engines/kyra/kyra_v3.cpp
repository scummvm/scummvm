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
#include "kyra/text_v3.h"
#include "kyra/vqa.h"
#include "kyra/gui.h"
#include "kyra/timer.h"
#include "kyra/debugger.h"
#include "kyra/gui_v3.h"

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
	_mainCharacter.height = 0x4C;
	_mainCharacter.facing = 5;
	_mainCharacter.animFrame = 0x57;
	_mainCharacter.walkspeed = 5;
	memset(_mainCharacter.inventory, -1, sizeof(_mainCharacter.inventory));
	_text = 0;
	_commandLineY = 189;
	_inventoryState = false;
	memset(&_sceneScriptState, 0, sizeof(_sceneScriptState));
	memset(&_sceneScriptData, 0, sizeof(_sceneScriptData));
	_updateCharPosNextUpdate = 0;
	memset(_characterAnimTable, 0, sizeof(_characterAnimTable));
	_overwriteSceneFacing = false;
	_maskPageMinY = _maskPageMaxY = 0;
	_sceneStrings = 0;
	_enterNewSceneLock = 0;
	_mainCharX = _mainCharY = -1;
	_animList = 0;
	_drawNoShapeFlag = false;
	_wsaPlayingVQA = false;
	_lastCharPalLayer = -1;
	_charPalUpdate = false;
	_runFlag = false;
	_unk5 = 0;
	_unkSceneScreenFlag1 = false;
	_noScriptEnter = true;
	_itemInHand = _handItemSet = -1;
	_unk3 = -1;
	_unk4 = 0;
	_loadingState = false;
	_noStartupChat = false;
	_lastProcessedSceneScript = 0;
	_specialSceneScriptRunFlag = false;
	_pathfinderFlag = 0;
	_talkObjectList = 0;
	_chatText = 0;
	_chatObject = -1;
	memset(&_chatScriptState, 0, sizeof(_chatScriptState));
	memset(&_chatScriptData, 0, sizeof(_chatScriptData));
	_voiceSoundChannel = -1;
	_charBackUpWidth2 = _charBackUpHeight2 = -1;
	_charBackUpWidth = _charBackUpHeight = -1;
	_useActorBuffer = false;
	_curStudioSFX = 283;
	_badConscienceShown = false;
	_curChapter = 1;
	_deathHandler = -1;
	_moveFacingTable = 0;
	_unkHandleSceneChangeFlag = false;
	memset(_sceneShapeDescs, 0, sizeof(_sceneShapeDescs));
	_cnvFile = _dlgBuffer = 0;
	_curDlgChapter = _curDlgIndex = _curDlgLang = -1;
	_isStartupDialog = 0;
	_stringBuffer = 0;
	_dialogSceneAnim = _dialogSceneScript = -1;
	memset(&_dialogScriptData, 0, sizeof(_dialogScriptData));
	memset(&_dialogScriptState, 0, sizeof(_dialogScriptState));
	_dialogScriptFuncStart = _dialogScriptFuncProc = _dialogScriptFuncEnd = 0;
	_malcolmsMood = 1;
	_nextIdleAnim = 0;
	_nextIdleType = false;
	_newShapeFlag = -1;
	_newShapeFiledata = 0;
	_inventoryScrollSpeed = -1;
	_invWsa = 0;
	_invWsaFrame = -1;
	_score = 0;
	memset(_scoreFlagTable, 0, sizeof(_scoreFlagTable));
	_debugger = 0;
	_mainButtonData = 0;
	_mainButtonList = 0;
	_mainButtonListInitialized = false;
	_enableInventory = true;
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
		delete [] _sceneShapes[i];

	for (uint i = 0; i < ARRAYSIZE(_sceneAnimMovie); ++i)
		delete _sceneAnimMovie[i];

	delete [] _gfxBackUpRect;
	delete [] _itemList;
	delete [] _paletteOverlay;
	delete [] _sceneList;

	for (ShapeMap::iterator i = _gameShapes.begin(); i != _gameShapes.end(); ++i) {
		delete [] i->_value;
		i->_value = 0;
	}
	_gameShapes.clear();

	_scriptInterpreter->unloadScript(&_sceneScriptData);

	delete [] _sceneStrings;
	delete [] _talkObjectList;
	delete [] _moveFacingTable;

	for (Common::Array<const Opcode*>::iterator i = _opcodesTemporary.begin(); i != _opcodesTemporary.end(); ++i)
		delete *i;
	_opcodesTemporary.clear();

	for (Common::Array<const Opcode*>::iterator i = _opcodesDialog.begin(); i != _opcodesDialog.end(); ++i)
		delete *i;
	_opcodesDialog.clear();

	delete _cnvFile;
	delete _dlgBuffer;
	delete [] _stringBuffer;
	delete [] _newShapeFiledata;
	delete _invWsa;
	delete _debugger;
	delete [] _mainButtonData;
	delete _gui;
}

int KyraEngine_v3::init() {
	_screen = new Screen_v3(this, _system);
	assert(_screen);
	_screen->setResolution();

	KyraEngine::init();
	
	_debugger = new Debugger_v3(this);
	assert(_debugger);

	_soundDigital = new SoundDigital(this, _mixer);
	assert(_soundDigital);
	if (!_soundDigital->init())
		error("_soundDigital->init() failed");
	KyraEngine::_text = _text = new TextDisplayer_v3(this, _screen);
	assert(_text);
	_gui = new GUI_v3(this);
	assert(_gui);

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

			fadeOutMusic(60);
			_screen->fadeToBlack();
			_musicSoundChannel = -1;
			startup();
			runLoop();
			running = false;
			break;

		case 1:
			playVQA("K3INTRO");
			_wsaPlayingVQA = false;
			_screen->hideMouse();
			break;

		case 2:
			//uninitMainMenu();
			//show load dialog
			//running = false;
			break;

		case 3:
		default:
			fadeOutMusic(60);
			_screen->fadeToBlack();
			uninitMainMenu();
			quitGame();
			running = false;
			break;
		}
	}

	return 0;
}

void KyraEngine_v3::initMainMenu() {
	_menuAnim = new WSAMovieV2(this, _screen);
	_menuAnim->open("REVENGE.WSA", 1, _screen->getPalette(0));
	_menuAnim->setX(0);
	_menuAnim->setY(0);
	_menuAnim->setDrawPage(0);
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

	VQAMovie vqa(this, _system);

	char filename[20];
	int size = 0;		// TODO: Movie size is 0, 1 or 2.

	snprintf(filename, sizeof(filename), "%s%d.VQA", name, size);

	if (vqa.open(filename)) {
		_soundDigital->stopAllSounds();

		_screen->hideMouse();
		memcpy(_screen->getPalette(1), _screen->getPalette(0), 768);
		fadeOutMusic(60);
		_screen->fadeToBlack();
		_screen->clearPage(0);

		vqa.setDrawPage(0);
		vqa.play();
		vqa.close();

		_soundDigital->stopAllSounds();
		_screen->showMouse();

		uint8 pal[768];
		// Taken from original, it used '1' here too
		memset(pal, 1, sizeof(pal));
		_screen->setScreenPalette(pal);
		_screen->clearPage(0);
		memcpy(_screen->getPalette(0), _screen->getPalette(1), 768);
		_wsaPlayingVQA = true;
	}
}

#pragma mark -

void KyraEngine_v3::playMenuAudioFile() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::playMenuAudioFile()");
	if (_soundDigital->isPlaying(_musicSoundChannel))
		return;

	_musicSoundChannel = _soundDigital->playSound(_menuAudioFile, 0xFF, Audio::Mixer::kMusicSoundType);
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

		_musicSoundChannel = _soundDigital->playSound(_soundList[track], 0xFF, Audio::Mixer::kMusicSoundType);
	}

	_curMusicTrack = track;
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

void KyraEngine_v3::fadeOutMusic(int ticks) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::fadeOutMusic(%d)", ticks);
	if (_musicSoundChannel >= 0) {
		_fadeOutMusicChannel = _musicSoundChannel;
		_soundDigital->beginFadeOut(_musicSoundChannel, ticks);
		_curMusicTrack = -1;
	}
}

void KyraEngine_v3::playSoundEffect(int item, int volume) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::playSoundEffect(%d, %d)", item, volume);
	if (_sfxFileMap[item*2+0] != 0xFF) {
		char filename[16];
		snprintf(filename, 16, "%s.AUD", _sfxFileList[_sfxFileMap[item*2+0]]);
		uint8 priority = _sfxFileMap[item*2+1];

		_soundDigital->playSound(filename, priority, Audio::Mixer::kSFXSoundType, volume);
	}
}

void KyraEngine_v3::playVoice(int high, int low) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::playVoice(%d, %d)", high, low);
	snd_playVoiceFile(high * 1000 + low);
}

void KyraEngine_v3::snd_playVoiceFile(int file) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::snd_playVoiceFile(%d)", file);
	char filename[16];
	snprintf(filename, 16, "%u.AUD", (uint)file);

	_voiceSoundChannel = _soundDigital->playSound(filename, 0xFE, Audio::Mixer::kSpeechSoundType, 255);
}

bool KyraEngine_v3::snd_voiceIsPlaying() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::snd_voiceIsPlaying()");
	return _soundDigital->isPlaying(_voiceSoundChannel);
}

void KyraEngine_v3::snd_stopVoice() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::snd_stopVoice()");
	if (_voiceSoundChannel != -1)
		_soundDigital->stopSound(_voiceSoundChannel);
}

void KyraEngine_v3::playStudioSFX(const char *str) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::playStudioSFX('%s')", str);
	if (_rnd.getRandomNumberRng(1, 2) != 2)
		return;

	const int strSize = strlen(str) - 1;
	if (str[strSize] != '?' && str[strSize] != '!')
		return;

	playSoundEffect(_curStudioSFX++, 128);

	if (_curStudioSFX > 291)
		_curStudioSFX = 283;
}

#pragma mark -

void KyraEngine_v3::preinit() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::preinit()");
	
	_itemBuffer1 = new int8[72];
	_itemBuffer2 = new int8[144];
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

	_gamePlayBuffer = new uint8[64000];
	musicUpdate(0);
	musicUpdate(0);

	_interface = new uint8[17920];
	_interfaceCommandLine = new uint8[3840];

	_screen->setFont(Screen::FID_6_FNT);

	_stringBuffer = new char[500];	
	//XXX
	musicUpdate(0);
	_moveFacingTable = new int[600];
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
		error("Couldn't load ITEMS");
	if (!loadLanguageFile("SCORE.", _scoreFile))
		error("Couldn't load SCORE");
	if (!loadLanguageFile("C_CODE.", _cCodeFile))
		error("Couldn't load C_CODE");
	if (!loadLanguageFile("SCENES.", _scenesFile))
		error("Couldn't load SCENES");

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

	_talkObjectList = new TalkObject[88];
	memset(_talkObjectList, 0, sizeof(TalkObject)*88);
	for (int i = 0; i < 88; ++i)
		_talkObjectList[i].sceneId = 0xFF;

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
	initMainButtonList(true);
	loadInterfaceShapes();

	musicUpdate(0);
	_res->loadFileToBuf("PALETTE.COL", _screen->getPalette(0), 768);
	_paletteOverlay = new uint8[256];
	_screen->generateOverlay(_screen->getPalette(0), _paletteOverlay, 0xF0, 0x19);

	loadInterface();
	musicUpdate(0);

	clearAnimObjects();

	_scoreMax = 0;
	for (int i = 0; i < _scoreTableSize; ++i) {
		if (_scoreTable[i] > 0)
			_scoreMax += _scoreTable[i];
	}

	musicUpdate(0);
	memset(_hiddenItems, -1, sizeof(_hiddenItems));

	memset(_newSceneDlgState, 0, sizeof(_newSceneDlgState));
	memset(_conversationState, -1, sizeof(_conversationState));

	_sceneList = new SceneDesc[98];
	_sceneListSize = 98;
	
	musicUpdate(0);
	runStartupScript(1, 0);
	_res->exists("MOODOMTR.WSA", true);
	_invWsa = new WSAMovieV2(this, _screen);
	assert(_invWsa);
	_invWsa->open("MOODOMTR.WSA", 1, 0);
	_invWsaFrame = 6;
	_soundDigital->beginFadeOut(_musicSoundChannel, 60);
	delayWithTicks(60);
	enterNewScene(_mainCharacter.sceneId, _mainCharacter.facing, 0, 0, 1);
	_screen->updateScreen();
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

	_scriptInterpreter->unloadScript(&data);
}

void KyraEngine_v3::openTalkFile(int file) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::openTalkFile(%d)", file);
	char talkFilename[16];

	if (file == 0) {
		strcpy(talkFilename, "ANYTALK.TLK");
	} else {
		if (_currentTalkFile > 0) {
			sprintf(talkFilename, "CH%dTALK.TLK", _currentTalkFile);
			_res->unloadPakFile(talkFilename);
		}
		sprintf(talkFilename, "CH%dTALK.TLK", file);
	}

	_currentTalkFile = file;
	_res->loadPakFile(talkFilename);
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

uint8 *KyraEngine_v3::getShapePtr(int shape) const {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::getShapePtr(%d)", shape);
	ShapeMap::iterator iter = _gameShapes.find(shape);
	if (iter == _gameShapes.end())
		return 0;
	return iter->_value;
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
		"MTR##.SHP"
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
		_res->exists(filename, true);
		_res->loadFileToBuf(filename, _screenBuffer, 64000);
		for (int j = startShape[i]; j <= endShape[i]; ++j) {
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

void KyraEngine_v3::moveCharacter(int facing, int x, int y) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::moveCharacter(%d, %d, %d)", facing, x, y);
	x &= ~3;
	y &= ~1;
	_mainCharacter.facing = facing;

	_screen->hideMouse();
	switch (facing) {
	case 0:
		while (_mainCharacter.y1 > y)
			updateCharPosWithUpdate();
		break;

	case 2:
		while (_mainCharacter.x1 < x)
			updateCharPosWithUpdate();
		break;

	case 4:
		while (_mainCharacter.y1 < y)
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

void KyraEngine_v3::updateCharPosWithUpdate() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::updateCharPosWithUpdate()");
	updateCharPos(0, 0);
	update();
}

int KyraEngine_v3::updateCharPos(int *table, int force) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::updateCharPos(%p, %d)", (const void*)table, force);
	if (_updateCharPosNextUpdate > _system->getMillis() && !force)
		return 0;
	_mainCharacter.x1 += _updateCharPosXTable[_mainCharacter.facing];
	_mainCharacter.y1 += _updateCharPosYTable[_mainCharacter.facing];
	updateCharAnimFrame(0, table);
	_updateCharPosNextUpdate = _system->getMillis() + _mainCharacter.walkspeed * _tickLength;
	return 1;
}

void KyraEngine_v3::updateCharAnimFrame(int character, int *table) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::updateCharPos(%d, %p)", character, (const void*)table);
	++_mainCharacter.animFrame;
	int facing = _mainCharacter.facing;

	if (table) {
		if (table[0] != table[-1] && table[1] == table[-1]) {
			facing = getOppositeFacingDirection(table[-1]);
			table[0] = table[-1];
		}
	}

	if (facing) {
		if (facing == 7 || facing == 1) {
			if (_characterAnimTable[0] > 2)
				facing = 0;
			memset(_characterAnimTable, 0, sizeof(_characterAnimTable));
		} else if (facing == 4) {
			++_characterAnimTable[1];
		} else if (facing == 5 || facing == 3) {
			if (_characterAnimTable[1] > 2)
				facing = 4;
			memset(_characterAnimTable, 0, sizeof(_characterAnimTable));
		}
	} else {
		++_characterAnimTable[0];
	}

	switch (facing) {
	case 0:
		if (_mainCharacter.animFrame < 79 || _mainCharacter.animFrame > 86)
			_mainCharacter.animFrame = 79;
		break;

	case 1: case 2: case 3:
		if (_mainCharacter.animFrame < 71 || _mainCharacter.animFrame > 78)
			_mainCharacter.animFrame = 71;
		break;

	case 4:
		if (_mainCharacter.animFrame < 55 || _mainCharacter.animFrame > 62)
			_mainCharacter.animFrame = 55;
		break;

	case 5: case 6: case 7:
		if (_mainCharacter.animFrame < 63 || _mainCharacter.animFrame > 70)
			_mainCharacter.animFrame = 63;
		break;

	default:
		break;
	}

	updateCharacterAnim(0);
}

void KyraEngine_v3::updateCharPal(int unk1) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::updateCharPal(%d)", unk1);
	int layer = _screen->getLayer(_mainCharacter.x1, _mainCharacter.y1) - 1;
	const uint8 *src = _costPalBuffer + _malcolmShapes * 72;
	uint8 *dst = _screen->getPalette(0) + 432;
	const int8 *sceneDatPal = &_sceneDatPalette[layer * 3];

	if (layer != _lastCharPalLayer && unk1) {
		for (int i = 0, j = 0; i < 72; ++i) {
			uint8 col = *dst;
			int8 addCol = *src + *sceneDatPal;
			addCol = MAX<int8>(0, MIN<int8>(addCol, 63));
			addCol = (col - addCol) >> 1;
			*dst -= addCol;
			++dst;
			++src;
			++sceneDatPal;
			++j;
			if (j > 3) {
				sceneDatPal = &_sceneDatPalette[layer * 3];
				j = 0;
			}
		}
		_charPalUpdate = true;
		_screen->setScreenPalette(_screen->getPalette(0));
		_lastCharPalLayer = layer;
	} else if (_charPalUpdate || !unk1) {
		memcpy(dst, src, 72);

		for (int i = 0, j = 0; i < 72; ++i) {
			uint8 col = *dst + *sceneDatPal;
			*dst = MAX<int8>(0, MIN<int8>(col, 63));
			++dst;
			++sceneDatPal;
			++j;
			if (j > 3) {
				sceneDatPal = &_sceneDatPalette[layer * 3];
				j = 0;
			}
		}

		_screen->setScreenPalette(_screen->getPalette(0));
		_charPalUpdate = false;
	}
}

bool KyraEngine_v3::checkCharCollision(int x, int y) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::checkCharCollision(%d, %d)", x, y);

	int scale = getScale(_mainCharacter.x1, _mainCharacter.y1);
	int width = (scale * 37) >> 8;
	int height = (scale * 76) >> 8;

	int x1 = _mainCharacter.x1 - width/2;
	int x2 = _mainCharacter.x1 + width/2;
	int y1 = _mainCharacter.y1 - height;
	int y2 = _mainCharacter.y1;

	if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
		return true;
	return false;
}

#pragma mark -

void KyraEngine_v3::runLoop() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::runLoop()");

	_runFlag = true;
	while (_runFlag && !_quitFlag) {
		//XXX deathHandler
		
		if (_system->getMillis() >= _nextIdleAnim)
			showIdleAnim();

		int inputFlag = checkInput(_mainButtonList, true);
		removeInputTop();

		update();
		_timer->update();

		if (inputFlag == 198 || inputFlag == 199) {
			_unk3 = _handItemSet;
			Common::Point mouse = getMousePos();
			handleInput(mouse.x, mouse.y);
		}

		_system->delayMillis(10);
	}
}

void KyraEngine_v3::handleInput(int x, int y) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::handleInput(%d, %d)", x, y);
	if (_inventoryState)
		return;
	setNextIdleAnimTimer();

	if (_unk5) {
		_unk5 = 0;
		return;
	}

	if (!_screen->isMouseVisible())
		return;

	if (_unk3 == -3) {
		playSoundEffect(0x0D, 0x80);
		return;
	}

	setNextIdleAnimTimer();
	
	int skip = 0;

	if (checkCharCollision(x, y) && _unk3 >= -1 && runSceneScript2()) {
		return;
	} else if (_itemInHand != 27 && pickUpItem(x, y, 1)) {
		return;
	} else if (checkItemCollision(x, y) == -1) {
		resetGameFlag(1);
		skip = runSceneScript1(x, y);

		if (queryGameFlag(1)) {
			resetGameFlag(1);
			return;
		} else if (_unk5) {
			_unk5 = 0;
			return;
		}
	}

	if (_deathHandler >= 0)
		skip = 1;

	if (skip)
		return;

	if (checkCharCollision(x, y)) {
		if (runSceneScript2())
			return;
	} else if (_itemInHand >= 0 && _unk3 >= 0) {
		if (_itemInHand == 27) {
			makeCharFacingMouse();
		} else if (y <= 187) {
			if (_itemInHand == 43)
				removeHandItem();
			else
				dropItem(0, _itemInHand, x, y, 1);
		}
		return;
	} else if (_unk3 == -3) {
		return;
	} else {
		if (y > 187 && _unk3 > -4)
			return;
		if (_unk5) {
			_unk5 = 0;
			return;
		}
	}

	inputSceneChange(x, y, 1, 1);
}

int KyraEngine_v3::inputSceneChange(int x, int y, int unk1, int unk2) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::inputSceneChange(%d, %d, %d, %d)", x, y, unk1, unk2);
	uint16 curScene = _mainCharacter.sceneId;
	_pathfinderFlag = 15;

	if (!_unkHandleSceneChangeFlag) {
		if (_unk3 == -4) {
			if (_sceneList[curScene].exit4 != 0xFFFF) {
				x = 4;
				y = _sceneEnterY4;
				_pathfinderFlag = 7;
			}
		} else if (_unk3 == -6) {
			if (_sceneList[curScene].exit2 != 0xFFFF) {
				x = 316;
				y = _sceneEnterY2;
				_pathfinderFlag = 7;
			}
		} else if (_unk3 == -7) {
			if (_sceneList[curScene].exit1 != 0xFFFF) {
				x = _sceneEnterX1;
				y = _sceneEnterY1 - 2;
				_pathfinderFlag = 14;
			}
		} else if (_unk3 == -5) {
			if (_sceneList[curScene].exit3 != 0xFFFF) {
				x = _sceneEnterX3;
				y = 191;
				_pathfinderFlag = 11;
			}
		}
	}

	if (ABS(_mainCharacter.x1 - x) < 4 && ABS(_mainCharacter.y1 - y) < 2) {
		_pathfinderFlag = 0;
		return 0;
	}

	int x1 = _mainCharacter.x1 & (~3);
	int y1 = _mainCharacter.y1 & (~1);
	x &= ~3;
	y &= ~1;

	int size = findWay(x1, y1, x, y, _moveFacingTable, 600);
	_pathfinderFlag = 0;

	if (!size || size == 0x7D00)
		return 0;

	return trySceneChange(_moveFacingTable, unk1, unk2);
}

void KyraEngine_v3::update() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::update()");
	updateInput();

	musicUpdate(0);
	refreshAnimObjectsIfNeed();
	musicUpdate(0);
	updateMouse();
	updateSpecialSceneScripts();
	updateCommandLine();
	//XXX
	musicUpdate(0);

	_screen->updateScreen();
}

void KyraEngine_v3::updateWithText() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::update()");
	updateInput();

	musicUpdate(0);
	updateMouse();
	//XXX
	updateSpecialSceneScripts();
	updateCommandLine();
	//XXX
	musicUpdate(0);

	restorePage3();
	drawAnimObjects();
	if (textEnabled() && _chatText) {
		int curPage = _screen->_curPage;
		_screen->_curPage = 2;
		objectChatPrintText(_chatText, _chatObject);
		_screen->_curPage = curPage;
	}
	refreshAnimObjects(0);

	_screen->updateScreen();
}

void KyraEngine_v3::updateMouse() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::updateMouse()");
	int shape = 0, offsetX = 0, offsetY = 0;
	Common::Point mouse = getMousePos();
	bool hasItemCollision = checkItemCollision(mouse.x, mouse.y) != -1;

	if (mouse.y > 187) {
		bool setItemCursor = false;
		if (_handItemSet == -6) {
			if (mouse.x < 311)
				setItemCursor = true;
		} else if (_handItemSet == -5) {
			if (mouse.x < _sceneMinX || mouse.x > _sceneMaxX)
				setItemCursor = true;
		} else if (_handItemSet == -4) {
			if (mouse.x > 8)
				setItemCursor = true;
		}

		if (setItemCursor) {
			setItemMouseCursor();
			return;
		}
	}

	if (_inventoryState) {
		if (mouse.y >= 144)
			return;
		hideInventory();
	}

	if (hasItemCollision && _handItemSet < -1 && _itemInHand < 0) {
		_handItemSet = -1;
		_itemInHand = -1;
		_screen->setMouseCursor(0, 0, _gameShapes[0]);
	}

	int type = 0;
	if (mouse.y <= 199) {
		if (mouse.x <= 8) {
			if (_sceneExit4 != 0xFFFF) {
				type = -4;
				shape = 4;
				offsetX = 0;
				offsetY = 0;
			}
		} else if (mouse.x >= 311) {
			if (_sceneExit2 != 0xFFFF) {
				type = -6;
				shape = 2;
				offsetX = 13;
				offsetY = 8;
			}
		} else if (mouse.y >= 171) {
			if (_sceneExit3 != 0xFFFF) {
				if (mouse.x >= _sceneMinX && mouse.x <= _sceneMaxX) {
					type = -5;
					shape = 3;
					offsetX = 8;
					offsetY = 13;
				}
			}
		} else if (mouse.y <= 8) {
			if (_sceneExit1 != 0xFFFF) {
				type = -7;
				shape = 1;
				offsetX = 8;
				offsetY = 0;
			}
		}
	}

	for (int i = 0; i < _specialExitCount; ++i) {
		if (checkSpecialSceneExit(i, mouse.x, mouse.y)) {
			switch (_specialExitTable[20+i]) {
			case 0:
				type = -7;
				shape = 1;
				offsetX = 8;
				offsetY = 0;
				break;

			case 2:
				type = -6;
				shape = 2;
				offsetX = 13;
				offsetY = 8;
				break;

			case 4:
				type = -5;
				shape = 3;
				offsetX = 8;
				offsetY = 13;
				break;

			case 6:
				type = -4;
				shape = 4;
				offsetX = 0;
				offsetY = 8;
				break;

			default:
				break;
			}
		}
	}

	if (type != 0 && type != _handItemSet && !hasItemCollision) {
		_handItemSet = type;
		_screen->setMouseCursor(offsetX, offsetY, _gameShapes[shape]);
	} else if (type == 0 && _handItemSet != _itemInHand && mouse.x > 8 && mouse.x < 311 && mouse.y < 171 && mouse.y > 8) {
		setItemMouseCursor();
	} else if (mouse.y > 187 && _handItemSet > -4 && type == 0 && !_inventoryState) {
		showInventory();
	}
}

void KyraEngine_v3::delay(uint32 millis, bool doUpdate, bool isMainLoop) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::delay(%d, %d, %d)", millis, doUpdate, isMainLoop);
	uint32 endTime = _system->getMillis() + millis;
	while (endTime > _system->getMillis()) {
		if (doUpdate) {
			//XXX
			update();
		}

		_system->delayMillis(10);
	}
}

#pragma mark -

void KyraEngine_v3::updateInput() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::updateInput()");
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
			_quitFlag = true;
			break;

		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == '.' || event.kbd.keycode == Common::KEYCODE_ESCAPE)
				_eventList.push_back(Event(event, true));
			else if (event.kbd.keycode == 'q' && event.kbd.flags == Common::KBD_CTRL)
				_quitFlag = true;
			else
				_eventList.push_back(event);
			break;

		case Common::EVENT_LBUTTONDOWN:
			_eventList.push_back(Event(event, true));
			break;

		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_MOUSEMOVE:
			_eventList.push_back(event);
			break;

		default:
			break;
		}
	}
}

int KyraEngine_v3::checkInput(Button *buttonList, bool mainLoop) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::checkInput(%p, %d)", (const void*)buttonList, mainLoop);
	updateInput();

	int keys = 0;

	while (_eventList.size()) {
		Common::Event event = *_eventList.begin();
		bool breakLoop = false;

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode >= '1' && event.kbd.keycode <= '9' &&
					(event.kbd.flags == Common::KBD_CTRL || event.kbd.flags == Common::KBD_ALT) && mainLoop) {
				const char *saveLoadSlot = getSavegameFilename(9 - (event.kbd.keycode - '0') + 990);

				if (event.kbd.flags == Common::KBD_CTRL) {
					loadGame(saveLoadSlot);
					_eventList.clear();
					breakLoop = true;
				} else {
					char savegameName[14];
					sprintf(savegameName, "Quicksave %d", event.kbd.keycode - '0');
					saveGame(saveLoadSlot, savegameName);
				}
			} else if (event.kbd.flags == Common::KBD_CTRL) {
				if (event.kbd.keycode == 'd')
					_debugger->attach();
			}
			break;

		case Common::EVENT_MOUSEMOVE: {
			Common::Point pos = getMousePos();
			_mouseX = pos.x;
			_mouseY = pos.y;
			_screen->updateScreen();
			} break;

		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP: {
			Common::Point pos = getMousePos();
			_mouseX = pos.x;
			_mouseY = pos.y;
			keys = event.type == Common::EVENT_LBUTTONDOWN ? 199 : (200 | 0x800);
			breakLoop = true;
			} break;

		default:
			break;
		}

		if (_debugger->isAttached())
			_debugger->onFrame();

		if (breakLoop)
			break;

		_eventList.erase(_eventList.begin());
	}

	return _gui->processButtonList(buttonList, keys | 0x8000);
}

void KyraEngine_v3::removeInputTop() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::removeInputTop()");
	if (!_eventList.empty())
		_eventList.erase(_eventList.begin());
}

bool KyraEngine_v3::skipFlag() const {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::skipFlag()");
	for (Common::List<Event>::const_iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
		if (i->causedSkip)
			return true;
	}
	return false;
}

void KyraEngine_v3::resetSkipFlag(bool removeEvent) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::resetSkipFlag(%d)", removeEvent);
	for (Common::List<Event>::iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
		if (i->causedSkip) {
			if (removeEvent)
				_eventList.erase(i);
			else
				i->causedSkip = false;
			return;
		}
	}
}

void KyraEngine_v3::makeCharFacingMouse() {
	debugC(9, kDebugLevelAnimator, "KyraEngine_v3::makeCharFacingMouse()");
	if (_mainCharacter.x1 > _mouseX)
		_mainCharacter.facing = 5;
	else
		_mainCharacter.facing = 3;
	_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	updateCharacterAnim(0);
	refreshAnimObjectsIfNeed();
}

#pragma mark -

int KyraEngine_v3::getDrawLayer(int x, int y) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::getDrawLayer(%d, %d)", x, y);
	int layer = _screen->getLayer(x, y) - 1;
	layer = _sceneDatLayerTable[layer];
	return MAX(0, MIN(layer, 6));
}

int KyraEngine_v3::getScale(int x, int y) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::getScale(%d, %d)", x, y);
	return _scaleTable[_screen->getLayer(x, y) - 1];
}

#pragma mark -

void KyraEngine_v3::backUpGfxRect32x32(int x, int y) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::backUpGfxRect32x32(%d, %d)", x, y);
	_screen->copyRegionToBuffer(_screen->_curPage, x, y, 32, 32, _gfxBackUpRect);
}

void KyraEngine_v3::restoreGfxRect32x32(int x, int y) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::restoreGfxRect32x32(%d, %d)", x, y);
	_screen->copyBlockToPage(_screen->_curPage, x, y, 32, 32, _gfxBackUpRect);
}

#pragma mark -

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

uint8 *KyraEngine_v3::getTableEntry(uint8 *buffer, int id) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::getTableEntry(%p, %d)", (const void*)buffer, id);
	uint16 tableEntries = READ_LE_UINT16(buffer);
	const uint16 *indexTable = (const uint16*)(buffer + 2);
	const uint16 *offsetTable = indexTable + tableEntries;

	int num = 0;
	while (id != READ_LE_UINT16(indexTable)) {
		++indexTable;
		++num;
	}

	return buffer + READ_LE_UINT16(offsetTable + num);
}

void KyraEngine_v3::getTableEntry(Common::SeekableReadStream *stream, int id, char *dst) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::getTableEntry(%p, %d, %p)", (const void*)stream, id, (const void*)dst);
	stream->seek(0, SEEK_SET);
	uint16 tableEntries = stream->readUint16LE();

	int num = 0;
	while (id != stream->readUint16LE())
		++num;

	stream->seek(2+tableEntries*2+num*2, SEEK_SET);
	stream->seek(stream->readUint16LE(), SEEK_SET);
	char c = 0;
	while ((c = stream->readByte()) != 0)
		*dst++ = c;
	*dst = 0;
}

#pragma mark -

bool KyraEngine_v3::talkObjectsInCurScene() {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::talkObjectsInCurScene()");

	for (int i = 0; i < 88; ++i) {
		if (_talkObjectList[i].sceneId == _mainCharacter.sceneId)
			return true;
	}

	return false;
}

#pragma mark -

void KyraEngine_v3::runTemporaryScript(const char *filename, int allowSkip, int resetChar, int newShapes, int shapeUnload) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::runTemporaryScript('%s', %d, %d, %d, %d)", filename, allowSkip, resetChar, newShapes, shapeUnload);
	memset(&_temporaryScriptData, 0, sizeof(_temporaryScriptData));
	memset(&_temporaryScriptState, 0, sizeof(_temporaryScriptState));

	if (!_scriptInterpreter->loadScript(filename, &_temporaryScriptData, &_opcodesTemporary))
		error("Couldn't load temporary script '%s'", filename);

	_scriptInterpreter->initScript(&_temporaryScriptState, &_temporaryScriptData);
	_scriptInterpreter->startScript(&_temporaryScriptState, 0);

	_newShapeFlag = -1;

	if (_newShapeFiledata && newShapes) {
		resetNewShapes(_newShapeCount, _newShapeFiledata);
		_newShapeFiledata = 0;
		_newShapeCount = 0;
	}

	while (_scriptInterpreter->validScript(&_temporaryScriptState))
		_scriptInterpreter->runScript(&_temporaryScriptState);

	uint8 *fileData = 0;

	if (newShapes)
		_newShapeFiledata = _res->fileData(_newShapeFilename, 0);

	fileData = _newShapeFiledata;

	if (!fileData) {
		_scriptInterpreter->unloadScript(&_temporaryScriptData);
		return;
	}

	if (newShapes)
		_newShapeCount = initNewShapes(fileData);

	processNewShapes(allowSkip, resetChar);

	if (shapeUnload) {
		resetNewShapes(_newShapeCount, fileData);
		_newShapeCount = 0;
		_newShapeFiledata = 0;
	}

	_scriptInterpreter->unloadScript(&_temporaryScriptData);
}

#pragma mark - 

bool KyraEngine_v3::updateScore(int scoreId, int strId) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::updateScore(%d, %d)", scoreId, strId);

	int scoreIndex = (scoreId >> 3);
	int scoreBit = scoreId & 7;
	if ((_scoreFlagTable[scoreIndex] & (1 << scoreBit)) != 0)
		return false;

	setNextIdleAnimTimer();
	_scoreFlagTable[scoreIndex] |= (1 << scoreBit);

	_screen->hideMouse();
	strcpy(_stringBuffer, (const char*)getTableEntry(_scoreFile, strId));
	strcat(_stringBuffer, ":        ");

	assert(scoreId < _scoreTableSize);

	int count = _scoreTable[scoreId];
	if (count > 0)
		scoreIncrease(count, _stringBuffer);

	_screen->showMouse();
	setNextIdleAnimTimer();
	return true;
}

void KyraEngine_v3::scoreIncrease(int count, const char *str) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::scoreIncrease(%d, '%s')", count, str);
	int drawOld = 1;
	_screen->hideMouse();

	showMessage(str, 0xFF, 0xF0);
	const int x = getScoreX(str);

	for (int i = 0; i < count; ++i) {
		int oldScore = _score;
		int newScore = ++_score;

		if (newScore > _scoreMax) {
			_score = _scoreMax;
			break;
		}

		drawScoreCounting(oldScore, newScore, drawOld, x);
		if (_inventoryState)
			drawScore(0, 215, 191);
		_screen->updateScreen();
		delay(20, true);
		
		playSoundEffect(0x0E, 0xC8);
		drawOld = 0;
	}

	_screen->showMouse();
}

#pragma mark -

void KyraEngine_v3::changeChapter(int newChapter, int sceneId, int malcolmShapes, int facing) {
	debugC(9, kDebugLevelMain, "KyraEngine_v3::changeChapter(%d, %d, %d, %d)", newChapter, sceneId, malcolmShapes, facing);	
	resetItemList();

	_curChapter = newChapter;
	runStartupScript(newChapter, 0);
	_mainCharacter.dlgIndex = 0;

	_malcolmsMood = 1;
	memset(_newSceneDlgState, 0, sizeof(_newSceneDlgState));

	if (malcolmShapes >= 0)
		loadMalcolmShapes(malcolmShapes);

	enterNewScene(sceneId, facing, 0, 0, 0);
}

#pragma mark -

Movie *KyraEngine_v3::createWSAMovie() {
	WSAMovieV2 *movie = new WSAMovieV2(this, _screen);
	assert(movie);
	return movie;
}

} // end of namespace Kyra

