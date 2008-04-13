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
	_unkPage1 = _unkPage2 = 0;
	_interfaceCPS1 = _interfaceCPS2 = 0;
	memset(_gameShapes, 0, sizeof(_gameShapes));
	_itemBuffer1 = _itemBuffer2 = 0;
	_mouseSHPBuf = 0;
	_unkBuffer5 = _unkBuffer6 = _unkBuffer7 = _unkBuffer9 = 0;
	_costpalData = 0;
	_unkWSAPtr = 0;
	memset(_unkShapeTable, 0, sizeof(_unkShapeTable));
	_scoreFile = 0;
	_cCodeFile = 0;
	_scenesList = 0;
}

KyraEngine_v3::~KyraEngine_v3() {
	delete _soundDigital;

	delete [] _unkPage1;
	delete [] _unkPage2;
	delete [] _interfaceCPS1;
	delete [] _interfaceCPS2;

	delete [] _itemBuffer1;
	delete [] _itemBuffer2;

	delete [] _mouseSHPBuf;

	delete [] _unkBuffer5;
	delete [] _unkBuffer6;
	delete [] _unkBuffer7;
	delete [] _unkBuffer9;

	delete [] _costpalData;
	delete [] _unkWSAPtr;

	delete [] _scoreFile;
	delete [] _cCodeFile;
	delete [] _scenesList;
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
	_screen->setAnimBlockPtr(3500);
	_screen->setScreenDim(0);

	_itemBuffer1 = new uint8[72];
	_itemBuffer2 = new uint8[144];
	assert(_itemBuffer1 && _itemBuffer2);

	_mouseSHPBuf = _res->fileData("MOUSE.SHP", 0);
	assert(_mouseSHPBuf);

	for (int i = 0; i <= 6; ++i)
		_gameShapes[i] = _screen->getPtrToShape(_mouseSHPBuf, i);

	initItems();

	_screen->setMouseCursor(0, 0, *_gameShapes);

	return 0;
}

int KyraEngine_v3::go() {
	bool running = true;
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

			preinit();
			realInit();
			// XXX
			running = false;
			break;

		case 1:
			memcpy(_screen->getPalette(1), _screen->getPalette(0), 768);
			playVQA("K3INTRO");
			memcpy(_screen->getPalette(0), _screen->getPalette(1), 768);
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
	debugC(9, kDebugLevelMain, "KyraEngine::playVQA('%s')", name);
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
}

#pragma mark -

void KyraEngine_v3::playMenuAudioFile() {
	debugC(9, kDebugLevelMain, "KyraEngine::playMenuAudioFile()");
	if (_soundDigital->isPlaying(_musicSoundChannel))
		return;

	Common::SeekableReadStream *stream = _res->getFileStream(_menuAudioFile);
	if (stream)
		_musicSoundChannel = _soundDigital->playSound(stream, true);
}

void KyraEngine_v3::playMusicTrack(int track, int force) {
	debugC(9, kDebugLevelMain, "KyraEngine::playMusicTrack(%d, %d)", track, force);

	// XXX byte_2C87C compare

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
	if (_musicSoundChannel != -1 && _soundDigital->isPlaying(_musicSoundChannel))
		_soundDigital->stopSound(_musicSoundChannel);

	_curMusicTrack = -1;
	_musicSoundChannel = -1;
}

int KyraEngine_v3::musicUpdate(int forceRestart) {
	debugC(9, kDebugLevelMain, "KyraEngine::unkUpdate(%d)", forceRestart);

	static uint32 mTimer = 0;
	static uint16 lock = 0;

	if (ABS<int>(_system->getMillis() - mTimer) > (int)(0x0F * _tickLength)) {
		mTimer = _system->getMillis();
	}

	if (_system->getMillis() < mTimer && !forceRestart) {
		return 1;
	}

	if (!lock) {
		lock = 1;
		if (_musicSoundChannel >= 0) {
			// XXX sub_1C262 (sound specific. it seems to close some sound resource files in special cases)
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
	debugC(9, kDebugLevelMain, "KyraEngine::preinit()");

	musicUpdate(0);

	// XXX snd_allocateSoundBuffer?
	memset(_flagsTable, 0, sizeof(_flagsTable));

	// XXX
	setGameFlag(0x216);

	_unkPage1 = new uint8[64000];
	assert(_unkPage1);

	musicUpdate(0);
	musicUpdate(0);

	_interfaceCPS1 = new uint8[17920];
	_interfaceCPS2 = new uint8[3840];
	assert(_interfaceCPS1 && _interfaceCPS2);

	_screen->setFont(Screen::FID_6_FNT);
}

void KyraEngine_v3::realInit() {
	debugC(9, kDebugLevelMain, "KyraEngine::realInit()");

	// XXX sound specific stuff

	_unkBuffer5 = new uint8[500];
	_unkBuffer6 = new uint8[200];
	_unkBuffer7 = new uint8[600];
	_costpalData = new uint8[864];
	_unkBuffer9 = new uint8[3618];
	_unkWSAPtr = new uint8[624];

	musicUpdate(0);

	_unkPage2 = new uint8[64000];

	musicUpdate(0);
	musicUpdate(0);

	if (!loadLanguageFile("ITEMS.", _itemList))
		error("couldn't load ITEMS");
	if (!loadLanguageFile("C_CODE.", _cCodeFile))
		error("couldn't load C_CODE");
	if (!loadLanguageFile("SCENES.", _scenesList))
		error("couldn't load SCENES");

	assert(_unkBuffer5 && _unkBuffer6 && _unkBuffer7 && _costpalData && _unkBuffer9 &&
			_unkWSAPtr && _unkPage2 && _itemList && _cCodeFile && _scenesList);

	musicUpdate(0);
}

#pragma mark - items

void KyraEngine_v3::initItems() {
	debugC(9, kDebugLevelMain, "KyraEngine::initItems()");

	_screen->loadBitmap("ITEMS.CSH", 3, 3, 0);

	//for (int i = 248; i <= 319; ++i)
	//	addShapeToTable(_screen->getCPagePtr(3), i, i-248);

	_screen->loadBitmap("ITEMS2.CSH", 3, 3, 0);

	//for (int i = 320; i <= 397; ++i)
	//	addShapeToTable(_screen->getCPagePtr(3), i, i-320);

	uint32 size = 0;
	uint8 *itemsDat = _res->fileData("_ITEMS.DAT", &size);

	assert(size >= 72+144);

	memcpy(_itemBuffer1, itemsDat   ,  72);
	memcpy(_itemBuffer2, itemsDat+72, 144);

	delete [] itemsDat;

	_screen->_curPage = 0;
}

#pragma mark -

int KyraEngine_v3::getMaxFileSize(const char *file) {
	debugC(9, kDebugLevelMain, "KyraEngine::getMaxFileSize(%s)", file);
	int size = 0;

	char buffer[32];

	for (int i = 0; i < _languageExtensionSize; ++i) {
		strncpy(buffer, file, 32);
		size = MAX<uint32>(size, _res->getFileSize(appendLanguage(buffer, i, sizeof(buffer))));
	}

	return size + 20;
}

char *KyraEngine_v3::appendLanguage(char *buf, int lang, int bufSize) {
	debugC(9, kDebugLevelMain, "KyraEngine::appendLanguage([%p|'%s'], %d, %d)", (const void*)buf, buf, lang, bufSize);
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

bool KyraEngine_v3::loadLanguageFile(const char *file, uint8 *&buffer) {
	debugC(9, kDebugLevelMain, "KyraEngine::loadLanguageFile('%s', %p)", file, (const void*)buffer);

	uint32 size = 0;
	char nBuf[32];
	strncpy(nBuf, file, 32);
	buffer = _res->fileData(appendLanguage(nBuf, _lang, sizeof(nBuf)), &size);

	return size != 0 && buffer != 0;
}

Movie *KyraEngine_v3::createWSAMovie() {
	WSAMovieV2 *movie = new WSAMovieV2(this, _screen);
	assert(movie);
	return movie;
}

} // end of namespace Kyra

