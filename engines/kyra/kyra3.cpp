/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
#include "kyra/kyra3.h"
#include "kyra/screen.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"
#include "kyra/text.h"
#include "kyra/vqa.h"

#include "common/system.h"
#include "common/config-manager.h"

// TODO: Temporary, to get the mouse cursor mock-up working
#include "graphics/cursorman.h"

namespace Kyra {
KyraEngine_v3::KyraEngine_v3(OSystem *system) : KyraEngine(system) {
	_soundDigital = 0;
	_musicSoundChannel = -1;
	_menuAudioFile = "TITLE1.AUD";
	_curMusicTrack = -1;
	_unkPage1 = _unkPage2 = 0;
	_interfaceCPS1 = _interfaceCPS2 = 0;
	memset(_gameShapes, 0, sizeof(_gameShapes));
	_shapePoolBuffer = 0;
	_itemBuffer1 = _itemBuffer2 = 0;
	_mouseSHPBuf = 0;
	_tableBuffer1 = _tableBuffer2 = 0;
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

	delete [] _shapePoolBuffer;

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

int KyraEngine_v3::setupGameFlags() {
	_game = GI_KYRA3;
	return 0;
}

Movie *KyraEngine_v3::createWSAMovie() {
	return new WSAMovieV2(this);
}

int KyraEngine_v3::init() {
	KyraEngine::init();

	gui_initMainMenu();
	
	_soundDigital = new SoundDigital(this, _mixer);
	assert(_soundDigital);
	if (!_soundDigital->init())
		error("_soundDigital->init() failed");
	
	_screen->loadFont(Screen::FID_6_FNT, "6.FNT");
	_screen->loadFont(Screen::FID_8_FNT, "8FAT.FNT");
	_screen->loadFont(Screen::FID_BOOKFONT_FNT, "BOOKFONT.FNT");
	_screen->setAnimBlockPtr(3500);
	_screen->setScreenDim(0);

	_shapePoolBuffer = new uint8[300000];
	assert(_shapePoolBuffer);
	memset(_shapePoolBuffer, 0, 300000);

	initTableBuffer(_shapePoolBuffer, 300000);

	_itemBuffer1 = new uint8[72];
	_itemBuffer2 = new uint8[144];
	assert(_itemBuffer1 && _itemBuffer2);

	_mouseSHPBuf = _res->fileData("MOUSE.SHP", 0);
	assert(_mouseSHPBuf);

	for (int i = 0; i <= 6; ++i) {
		_gameShapes[i] = _screen->getPtrToShape(_mouseSHPBuf, i);
	}

	initItems();

	_screen->setMouseCursor(0, 0, *_gameShapes);

	return 0;
}

int KyraEngine_v3::go() {
	uint8 *pal = _screen->getPalette(1);
	assert(pal);
	
	_mainMenuLogo = createWSAMovie();
	assert(_mainMenuLogo);
	_mainMenuLogo->open("REVENGE.WSA", 1, pal);
	assert(_mainMenuLogo->opened());
	
	bool running = true;
	while (running && !_quitFlag) {
		_screen->_curPage = 0;
		_screen->clearPage(0);

		pal[0] = pal[1] = pal[2] = 0;
		
		_screen->setScreenPalette(pal);
		
		// XXX
		playMenuAudioFile();
		
		_mainMenuLogo->setX(0); _mainMenuLogo->setY(0);
		_mainMenuLogo->setDrawPage(0);

		for (int i = 0; i < 64 && !_quitFlag; ++i) {
			uint32 nextRun = _system->getMillis() + 3 * _tickLength;
			_mainMenuLogo->displayFrame(i);
			_screen->updateScreen();
			delayUntil(nextRun);
		}

		for (int i = 64; i > 29 && !_quitFlag; --i) {
			uint32 nextRun = _system->getMillis() + 3 * _tickLength;
			_mainMenuLogo->displayFrame(i);
			_screen->updateScreen();
			delayUntil(nextRun);
		}

		switch (gui_handleMainMenu()) {
		case 0:
			delete _mainMenuLogo;
			_mainMenuLogo = 0;
			preinit();
			realInit();
			// XXX
			running = false;
			break;
		
		case 1:
			playVQA("K3INTRO");
			break;
		
		case 2:
			//delete _mainMenuLogo;
			//_mainMenuLogo = 0;
			//show load dialog
			//running = false;
			break;
		
		case 3:
			_soundDigital->beginFadeOut(_musicSoundChannel);
			_screen->fadeToBlack();
			_soundDigital->stopSound(_musicSoundChannel);
			_musicSoundChannel = -1;
			running = false;
			break;
		
		default:
			break;
		}
	}
	delete _mainMenuLogo;

	return 0;
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
		_soundDigital->beginFadeOut(_musicSoundChannel);
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

	Common::File *handle = new Common::File();
	uint32 temp = 0;
	_res->fileHandle(_menuAudioFile, &temp, *handle);
	if (handle->isOpen()) {
		_musicSoundChannel = _soundDigital->playSound(handle, true);
	} else {
		delete handle;
	}
}

void KyraEngine_v3::playMusicTrack(int track, int force) {
	debugC(9, kDebugLevelMain, "KyraEngine::playMusicTrack(%d, %d)", track, force);
	
	// XXX byte_2C87C compare
	
	if (_musicSoundChannel != -1 && !_soundDigital->isPlaying(_musicSoundChannel)) {
		force = 1;
	} else if (_musicSoundChannel == -1) {
		force = 1;
	}
	
	if (track == _curMusicTrack && !force)
		return;
	
	stopMusicTrack();
	
	if (_musicSoundChannel == -1) {
		assert(track < _soundListSize && track >= 0);

		Common::File *handle = new Common::File();
		uint32 temp = 0;
		_res->fileHandle(_soundList[track], &temp, *handle);
		if (handle->isOpen()) {
			_musicSoundChannel = _soundDigital->playSound(handle);
		} else {
			delete handle;
		}
	}
	
	_musicSoundChannel = track;
}

void KyraEngine_v3::stopMusicTrack() {
	if (_musicSoundChannel != -1 && _soundDigital->isPlaying(_musicSoundChannel)) {
		_soundDigital->stopSound(_musicSoundChannel);
	}
	
	_curMusicTrack = -1;
	_musicSoundChannel = -1;
}

int KyraEngine_v3::musicUpdate(int forceRestart) {
	debugC(9, kDebugLevelMain, "KyraEngine::unkUpdate(%d)", forceRestart);
	
	static uint32 timer = 0;
	static uint16 lock = 0;

	if (ABS<int>(_system->getMillis() - timer) > (int)(0x0F * _tickLength)) {
		timer = _system->getMillis();
	}
	
	if (_system->getMillis() < timer && !forceRestart) {
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
		timer = _system->getMillis() + 0x0F * _tickLength;
	}
	
	return 1;
}

#pragma mark -

void KyraEngine_v3::gui_initMainMenu() {
	KyraEngine::gui_initMainMenu();
	_mainMenuFrame = 29;
	_mainMenuFrameAdd = 1;
}

void KyraEngine_v3::gui_updateMainMenuAnimation() {
	uint32 nextRun = 0;
	
	uint32 now = _system->getMillis();
	if (now < nextRun)
		return;

	// yes 2 * _tickLength here not 3 * like in the first draw
	nextRun = now + 2 * _tickLength;
	
	_mainMenuLogo->displayFrame(_mainMenuFrame);
	_screen->updateScreen();
		
	_mainMenuFrame += _mainMenuFrameAdd;
	if (_mainMenuFrame < 29) {
		_mainMenuFrame = 29;
		_mainMenuFrameAdd = 1;
	} else if (_mainMenuFrame > 63) {
		_mainMenuFrame = 64;
		_mainMenuFrameAdd = -1;
	}
		
	// XXX
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

	assert(loadLanguageFile("ITEMS.", _itemList));
	assert(loadLanguageFile("C_CODE.", _cCodeFile));
	assert(loadLanguageFile("SCENES.", _scenesList));

	assert(_unkBuffer5 && _unkBuffer6 && _unkBuffer7 && _costpalData && _unkBuffer9 &&
			_unkWSAPtr && _unkPage2 && _itemList && _cCodeFile && _scenesList);

	musicUpdate(0);
}

#pragma mark -

int KyraEngine_v3::initTableBuffer(uint8 *buf, int size) {
	debugC(9, kDebugLevelMain, "KyraEngine::initTableBuffer(%p, %d)", (void*)buf, size);

	if (!buf || size < 6320)
		return 0;

	if (_tableBuffer2 != _tableBuffer1 && _tableBuffer2 && _tableBuffer1) {
		// no idea if this *should* be called
		memmove(_tableBuffer2, _tableBuffer1, 6320);
	}

	_tableBuffer1 = buf;
	size -= 6320;

	*((uint16*)(_tableBuffer1)) = 0;
	*((uint16*)(_tableBuffer1 + 2)) = 1;
	*((uint16*)(_tableBuffer1 + 4)) = 1;
	*((uint32*)(_tableBuffer1 + 6)) = size >> 4;
	*((uint16*)(_tableBuffer1 + 10)) = 1;
	*((uint32*)(_tableBuffer1 + 16)) = 6320;
	*((uint32*)(_tableBuffer1 + 22)) = size >> 4;

	_tableBuffer2 = buf;

	return 1;
}

void KyraEngine_v3::updateTableBuffer(uint8 *buf) {
	debugC(9, kDebugLevelMain, "KyraEngine::updateTableBuffer(%p)", (void*)buf);

	if (_tableBuffer2 == buf)
		return;

	if (_tableBuffer1 != _tableBuffer2)
		memmove(_tableBuffer2, _tableBuffer1, 6320);

	_tableBuffer2 = _tableBuffer1 = buf;
}

int KyraEngine_v3::addShapeToTable(uint8 *buf, int id, int shapeNum) {
	debugC(9, kDebugLevelMain, "KyraEngine::addShapeToTable(%p, %d, %d)", (void*)buf, id, shapeNum);

	if (!buf)
		return 0;

	uint8 *shapePtr = _screen->getPtrToShape(buf, shapeNum);
	if (!shapePtr)
		return 0;

	int shapeSize = _screen->getShapeSize(shapePtr);

	if (getTableSize(_shapePoolBuffer) < shapeSize) {
		// XXX
		error("[1] unimplemented table handling");
	}

	uint8 *ptr = allocTableSpace(_shapePoolBuffer, shapeSize, id);

	if (!ptr) {
		// XXX
		error("[2] unimplemented table handling");
	}

	if (!ptr) {
		warning("adding shape %d to _shapePoolBuffer not possible, not enough space left\n", id);
		return shapeSize;
	}

	memcpy(ptr, shapePtr, shapeSize);
	return shapeSize;
}

int KyraEngine_v3::getTableSize(uint8 *buf) {
	debugC(9, kDebugLevelMain, "KyraEngine::getTableSize(%p)", (void*)buf);
	updateTableBuffer(buf);

	if (*((uint16*)(_tableBuffer1 + 4)) >= 450)
		return 0;

	return (*((uint32*)(_tableBuffer1 + 6)) << 4);
}

uint8 *KyraEngine_v3::allocTableSpace(uint8 *buf, int size, int id) {
	debugC(9, kDebugLevelMain, "KyraEngine::allocTableSpace(%p, %d, %d)", (void*)buf, size, id);

	if (!buf || !size)
		return 0;

	updateTableBuffer(buf);

	int entries = *(uint16*)(_tableBuffer1 + 4);

	if (entries >= 450)
		return 0;

	size += 0xF;
	size &= 0xFFFFFFF0;

	uint size2 = size >> 4;

	if (*(uint32*)(_tableBuffer1 + 6) < size2)
		return 0;

	int unk1 = *(uint16*)(_tableBuffer1);
	int usedEntry = unk1;
	int ok = 0;

	for (; usedEntry < entries; ++usedEntry) {
		if (size2 <= *(uint32*)(_tableBuffer1 + usedEntry * 14 + 22)) {
			ok = 1;
			break;
		}
	}

	if (!ok)
		return 0;

	ok = 0;
	int unk3 = unk1 - 1;
	while (ok <= unk3) {
		int temp = (ok + unk3) >> 1;

		if (*(uint32*)(_tableBuffer1 + temp * 14 + 12) >= (uint)id) {
			if (*(uint32*)(_tableBuffer1 + temp * 14 + 12) <= (uint)id) {
				return 0;
			} else {
				unk3 = temp - 1;
				continue;
			}
		}

		ok = temp + 1;
	}

	uint8 *buf2 = _tableBuffer1 + usedEntry * 14;

	uint unkValue1 = *(uint32*)(buf2 + 16);
	uint unkValue2 = *(uint32*)(buf2 + 22);

	if (size2 < unkValue2) {
		*(uint32*)(buf2 + 22) = unkValue2 - size2;
		*(uint32*)(buf2 + 16) = unkValue1 + size;
		memcpy(_tableBuffer1 + entries * 14 + 12, _tableBuffer1 + unk1 * 14 + 12, 14);
	} else {
		if (usedEntry > unk1) {
			memcpy(buf2 + 12, _tableBuffer1 + unk1 * 14 + 12, 14);
		}
		int temp = *(uint16*)(_tableBuffer1 + 2) - 1;
		*(uint16*)(_tableBuffer1 + 2) = temp;
		temp = *(uint16*)(_tableBuffer1 + 4) - 1;
		*(uint16*)(_tableBuffer1 + 4) = temp;
	}

	for (int i = unk1; i > ok; --i) {
		memcpy(_tableBuffer1 + i * 14 + 12, _tableBuffer1 + (i-1) * 14 + 12, 14);
	}

	buf2 = _tableBuffer1 + ok * 14;

	*(uint32*)(buf2 + 12) = id;
	*(uint32*)(buf2 + 16) = unkValue1;
	*(uint32*)(buf2 + 20) = (_system->getMillis() / 60) >> 4;
	*(uint32*)(buf2 + 22) = size2;

	int temp = *(uint16*)(_tableBuffer1) + 1;
	*(uint16*)(_tableBuffer1) = temp;
	temp = *(uint16*)(_tableBuffer1 + 4) + 1;
	*(uint16*)(_tableBuffer1 + 4) = temp;

	if (temp > *(uint16*)(_tableBuffer1 + 10)) {
		*(uint16*)(_tableBuffer1 + 10) = temp;
		if (temp > _unkTableValue)
			_unkTableValue = temp;
	}

	temp = *(uint32*)(_tableBuffer1 + 6) - size2;
	*(uint32*)(_tableBuffer1 + 6) = temp;

	return _tableBuffer2 + unkValue1;
}

namespace {
int tableIdCompare(const void *l, const void *r) {
	int lV = *(const uint32*)(l);
	int rV = *(const uint32*)(r);

	return CLIP(lV - rV, -1, 1);
}
}

uint8 *KyraEngine_v3::findIdInTable(uint8 *buf, int id) {
	debugC(9, kDebugLevelMain, "KyraEngine::findIdInTable(%p, %d)", (void*)buf, id);

	updateTableBuffer(buf);

	uint32 idVal = id;
	uint8 *ptr = (uint8*)bsearch(&idVal, _tableBuffer1 + 12, *(uint16*)(_tableBuffer1), 14, &tableIdCompare);

	if (!ptr) {
		return 0;
	}

	return _tableBuffer2 + *(uint32*)(ptr + 4);
}

uint8 *KyraEngine_v3::findShapeInTable(int id) {
	debugC(9, kDebugLevelMain, "KyraEngine::findShapeInTable(%d)", id);

	return findIdInTable(_shapePoolBuffer, id);
}

#pragma mark - items

void KyraEngine_v3::initItems() {
	debugC(9, kDebugLevelMain, "KyraEngine::initItems()");

	_screen->loadBitmap("ITEMS.CSH", 3, 3, 0);

	for (int i = 248; i <= 319; ++i) {
		addShapeToTable(_screen->getPagePtr(3), i, i-248);
	}

	_screen->loadBitmap("ITEMS2.CSH", 3, 3, 0);

	for (int i = 320; i <= 397; ++i) {
		addShapeToTable(_screen->getPagePtr(3), i, i-320);
	}

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

} // end of namespace Kyra
