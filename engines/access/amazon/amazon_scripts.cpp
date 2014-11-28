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

#include "common/scummsys.h"
#include "access/access.h"
#include "access/resources.h"
#include "access/amazon/amazon_game.h"
#include "access/amazon/amazon_resources.h"
#include "access/amazon/amazon_scripts.h"

namespace Access {

namespace Amazon {

AmazonScripts::AmazonScripts(AccessEngine *vm) : Scripts(vm) {
	_game = (AmazonEngine *)_vm;

	_xTrack = 0;
	_yTrack = 0;
	_zTrack = 0;
	_xCam = 0;
	_yCam = 0;
	_zCam = 0;
	_CHICKENOUTFLG = false;

	_pNumObj = 0;
	for (int i = 0; i < 32; i++) {
		_pImgNum[i] = 0;
		_pObject[i] = nullptr;
		_pObjX[i] = 0;
		_pObjY[i] = 0;
		_pObjZ[i] = 0;
	}

	for (int i = 0; i < 16; i++) {
		_pObjXl[i] = 0;
		_pObjYl[i] = 0;
	}
}

void AmazonScripts::cLoop() {
	searchForSequence();
	_vm->_images.clear();
	_vm->_buffer2.copyFrom(_vm->_buffer1);
	_vm->_oldRects.clear();
	_vm->_scripts->executeScript();
	_vm->plotList1();
	_vm->copyBlocks();
}

void AmazonScripts::mWhile1() {
	_vm->_screen->setDisplayScan();
	_vm->_screen->fadeOut();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 0);
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_screen->forceFadeIn();
	
	Resource *spriteData = _vm->_files->loadFile(14, 6);
	_vm->_objectsTable[0] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2100;

	do {
		cLoop();
		_sequence = 2100;
	} while (_vm->_flags[52] == 1);

	_vm->_screen->copyFrom(_vm->_buffer1);
	_vm->_buffer1.copyFrom(_vm->_buffer2);

	_game->establish(-1, 14);

	spriteData = _vm->_files->loadFile(14, 7);
	_vm->_objectsTable[1] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_sound->playSound(0);
	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 1);
	_vm->_screen->setPalette();
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2200;

	_vm->_sound->queueSound(0, 14, 15);

	do {
		cLoop();
		_sequence = 2200;
	} while (_vm->_flags[52] == 2);

	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 2);
	_vm->_screen->setPalette();
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->freeCells();

	spriteData = _vm->_files->loadFile(14, 8);
	_vm->_objectsTable[2] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2300;
	_vm->_sound->playSound(0);

	do {
		cLoop();
		_sequence = 2300;
	} while (_vm->_flags[52] == 3);

	_vm->freeCells();
	spriteData = _vm->_files->loadFile(14, 9);
	_vm->_objectsTable[3] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 3);
	_vm->_screen->setPalette();
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2400;

	do {
		cLoop();
		_sequence = 2400;
	} while (_vm->_flags[52] == 4);
}

void AmazonScripts::mWhile2() {
	_vm->_screen->setDisplayScan();
	_vm->_screen->fadeOut();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 0);
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_screen->forceFadeIn();

	Resource *spriteData = _vm->_files->loadFile(14, 6);
	_vm->_objectsTable[0] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2100;

	do {
		cLoop();
		_sequence = 2100;
	} while (_vm->_flags[52] == 1);

	_vm->_screen->fadeOut();
	_vm->freeCells();
	spriteData = _vm->_files->loadFile(14, 9);
	_vm->_objectsTable[3] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 3);
	_vm->_screen->setPalette();
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2400;

	do {
		cLoop();
		_sequence = 2400;
	} while (_vm->_flags[52] == 4);
}

void AmazonScripts::initJWalk2() {
	const int JUNGLE1OBJ[7][4] = {
		{2, 470, 0, 20},
		{0, 290, 0, 50},
		{1, 210, 0, 40},
		{0, 500, 0, 30},
		{1, 550, 0, 20},
		{0, 580, 0, 60},
		{1, 650, 0, 30}
	};
	_vm->_screen->fadeOut();
	_vm->_events->hideCursor();
	_vm->_screen->clearScreen();
	_vm->_buffer2.clearBuffer();
	_vm->_screen->setBufferScan();

	_vm->_screen->_scrollX = _vm->_screen->_scrollY;
	_vm->_screen->_scrollCol = _vm->_screen->_scrollRow;
	_vm->_room->buildScreen();
	_vm->copyBF2Vid();
	_vm->_screen->fadeIn();
	// KEYFL = 0;

	_game->_plane._xCount = 2;
	_vm->_player->_scrollAmount = 5;
	_xTrack = -10;
	_yTrack = _zTrack = 0;
	_xCam = 480;
	_yCam = 0;
	_zCam = 80;

	_game->_timers[24]._timer = 1;
	_game->_timers[24]._initTm = 1;
	++_game->_timers[24]._flag;

	_pNumObj = 7;
	for (int i = 0; i < _pNumObj; i++) {
		_pObject[i] = _vm->_objectsTable[24];
		_pImgNum[i] = JUNGLE1OBJ[i][0];
		_pObjX[i] = JUNGLE1OBJ[i][1];
		_pObjY[i] = JUNGLE1OBJ[i][2];
		_pObjZ[i] = JUNGLE1OBJ[i][3];
		_pObjXl[i] = _pObjYl[i] = 0;
	}

	_jCnt[0] = 0;
	_jCnt[1] = 3;
	_jCnt[2] = 5;

	_jungleX[0] = 50;
	_jungleX[1] = 16;
	_jungleX[2] = 93;
}

void AmazonScripts::jungleMove() {
	const static int jungleY[3] = {27, 30, 29};
	int count = 1;
	int frameOffset = 0;

	if (!_vm->_timers[0]._flag) {
		++_vm->_timers[0]._flag;
		_vm->_screen->_scrollX += _vm->_player->_scrollAmount;

		for (int i = 0; i < 3; ++i) {
			int newJCnt = (_jCnt[i] + 1) % 8;
			_jCnt[i] = newJCnt;
			_jungleX[i] += 5;
		}

		frameOffset = 4;
		count = (_game->_allenFlag != 1) ? 2 : 3;
	}

	for (int i = 0; i < count; ++i) {
		ImageEntry ie;
		ie._flags = IMGFLAG_UNSCALED;
		ie._spritesPtr = _vm->_objectsTable[24];
		ie._frameNumber = _jCnt[i] + frameOffset;
		ie._position = Common::Point(_jungleX[i], jungleY[i]);
		ie._offsetY = jungleY[i];

		_vm->_images.addToList(ie);
		frameOffset += 8;
	}
}

void AmazonScripts::mWhileJWalk2() {
	Screen &screen = *_vm->_screen;

	initJWalk2();

	while (!_vm->shouldQuit() && !_vm->_events->isKeyMousePressed() &&
			(screen._scrollCol + screen._vWindowWidth) != _vm->_room->_playFieldWidth) {
		_vm->_images.clear();
		_vm->_events->_vbCount = 6;
		_pImgNum[0] = _game->_plane._xCount;

		jungleMove();
		while (screen._scrollX >= TILE_WIDTH) {
			screen._scrollX -= TILE_WIDTH;
			++screen._scrollCol;
			_vm->_buffer1.moveBufferLeft();
			_vm->_room->buildColumn(screen._scrollCol + screen._vWindowWidth, screen._vWindowBytesWide);
		}

		if (_game->_plane._xCount == 2)
			++_game->_plane._xCount;
		else
			--_game->_plane._xCount;

		pan();
		scrollJWalk();

		while (!_vm->shouldQuit() && _vm->_events->_vbCount > 0) {
			_vm->_events->pollEventsAndWait();
		}
	}

	_vm->_events->showCursor();
}

void AmazonScripts::doFlyCell() {
	Plane &plane = _game->_plane;
	SpriteResource *sprites = _vm->_objectsTable[15];

	if (plane._pCount <= 40) {
		_vm->_buffer2.plotImage(sprites, 3, Common::Point(70, 74));
	} else if (plane._pCount <= 80) {
		_vm->_buffer2.plotImage(sprites, 6, Common::Point(70, 74));
	} else if (plane._pCount <= 120) {
		_vm->_buffer2.plotImage(sprites, 2, Common::Point(50, 76));
	} else if (plane._pCount <= 160) {
		_vm->_buffer2.plotImage(sprites, 14, Common::Point(63, 78));
	} else if (plane._pCount <= 200) {
		_vm->_buffer2.plotImage(sprites, 5, Common::Point(86, 74));
	} else if (plane._pCount <= 240) {
		_vm->_buffer2.plotImage(sprites, 0, Common::Point(103, 76));
	} else if (plane._pCount <= 280) {
		_vm->_buffer2.plotImage(sprites, 4, Common::Point(119, 77));
	} else {
		_vm->_buffer2.plotImage(sprites, 1, Common::Point(111, 77));
	}

	if (plane._planeCount == 11 || plane._planeCount == 12)
		++plane._position.y;
	else if (plane._planeCount >= 28)
		--plane._position.y;

	_vm->_buffer2.plotImage(sprites, 7, plane._position);
	_vm->_buffer2.plotImage(sprites, 8 + plane._propCount, Common::Point(
		plane._position.x + 99, plane._position.y + 10));
	_vm->_buffer2.plotImage(sprites, 11 + plane._propCount, Common::Point(
		plane._position.x + 104, plane._position.y + 18));

	if (++plane._planeCount >= 30)
		plane._planeCount = 0;
	if (++plane._propCount >= 3)
		plane._propCount = 0;

	++plane._xCount;
	if (plane._xCount == 1)
		++plane._position.x;
	else
		plane._xCount = 0;
}

void AmazonScripts::doFallCell() {
	if (_vm->_scaleI <= 20)
		return;

	SpriteFrame *frame = _vm->_objectsTable[20]->getFrame(_game->_plane._planeCount / 6);
	Common::Rect r(115, 11, 115 + _vm->_screen->_scaleTable1[frame->w], 
		11 + _vm->_screen->_scaleTable1[frame->h]);
	_vm->_buffer2.sPlotF(frame, r);

	_vm->_scaleI -= 3;
	_vm->_scale = _vm->_scaleI;
	_vm->_screen->setScaleTable(_vm->_scale);
	++_game->_plane._xCount;
	if (_game->_plane._xCount == 5)
		return;
	_game->_plane._xCount = 0;
	if (_game->_plane._planeCount == 18)
		_game->_plane._planeCount = 0;
	else
		_game->_plane._planeCount += 6;
}

void AmazonScripts::pan() {
	_zCam += _zTrack;
	_xCam += _xTrack;
	int tx = (_xTrack << 8) / _zCam;
	_yCam += _yTrack;
	int ty = (_yTrack << 8) / _zCam;

	if (_vm->_timers[24]._flag != 1) {
		++_vm->_timers[24]._flag;
		for (int i = 0; i < _pNumObj; i++) {
			_pObjZ[i] += _zTrack;
			_pObjXl[i] += (_pObjZ[i] * tx) & 0xff;
			_pObjX[i] += ((_pObjZ[i] * tx) >> 8) + (_pObjXl[i] >> 8);
			_pObjXl[i] &= 0xff;

			_pObjYl[i] += (_pObjZ[i] * ty) & 0xff;
			_pObjY[i] += ((_pObjZ[i] * ty) >> 8) + (_pObjYl[i] >> 8);
			_pObjYl[i] &= 0xff;
		}
	}

	for (int i = 0; i < _pNumObj; i++) {
		ImageEntry ie;
		ie._flags = IMGFLAG_UNSCALED;
		ie._position = Common::Point(_pObjX[i], _pObjY[i]);
		ie._offsetY = 255;
		ie._spritesPtr = _pObject[i];
		ie._frameNumber = _pImgNum[i];

		_vm->_images.addToList(ie);
	}
}

void AmazonScripts::scrollFly() {
	_vm->copyBF1BF2();
	_vm->_newRects.clear();
	doFlyCell();
	_vm->copyRects();
	_vm->copyBF2Vid();
}

void AmazonScripts::scrollFall() {
	_vm->copyBF1BF2();
	_vm->_newRects.clear();
	doFallCell();
	_vm->copyRects();
	_vm->copyBF2Vid();
}

void AmazonScripts::scrollJWalk() {
	_vm->copyBF1BF2();
	_vm->_newRects.clear();
	_game->plotList();
	_vm->copyRects();
	_vm->copyBF2Vid();
}

void AmazonScripts::mWhileFly() {
	Screen &screen = *_vm->_screen;
	Player &player = *_vm->_player;
	EventsManager &events = *_vm->_events;
	Plane &plane = _game->_plane;

	events.hideCursor();
	screen.clearScreen();
	screen.setBufferScan();
	screen.fadeOut();
	screen._scrollX = 0;

	_vm->_room->buildScreen();
	_vm->copyBF2Vid();
	screen.fadeIn();
	_vm->_oldRects.clear();
	_vm->_newRects.clear();

	// KEYFLG = 0;

	screen._scrollRow = screen._scrollCol = 0;
	screen._scrollX = screen._scrollY = 0;
	player._rawPlayer = Common::Point(0, 0);
	player._scrollAmount = 1;

	plane._pCount = 0;
	plane._planeCount = 0;
	plane._propCount = 0;
	plane._xCount = 0;
	plane._position = Common::Point(20, 29);

	while (!_vm->shouldQuit() && !events.isKeyMousePressed() &&
			((screen._scrollCol + screen._vWindowWidth) != _vm->_room->_playFieldWidth)) {
		events._vbCount = 4;
		screen._scrollX += player._scrollAmount;

		while (screen._scrollX >= TILE_WIDTH) {
			screen._scrollX -= TILE_WIDTH;
			++screen._scrollCol;

			_vm->_buffer1.moveBufferLeft();
			_vm->_room->buildColumn(screen._scrollCol + screen._vWindowWidth, screen._vWindowBytesWide);
		}

		scrollFly();
		++plane._pCount;

		while (!_vm->shouldQuit() && events._vbCount > 0) {
			// To be rewritten when NEWTIMER is done
			events.checkForNextFrameCounter();
			_vm->_sound->playSound(0);

			events.pollEventsAndWait();
		}
	}

	events.showCursor();
}

void AmazonScripts::mWhileFall() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;

	events.hideCursor();
	screen.clearScreen();
	screen.setBufferScan();
	screen.fadeOut();
	screen._scrollX = 0;

	_vm->_room->buildScreen();
	_vm->copyBF2Vid();
	screen.fadeIn();
	_vm->_oldRects.clear();
	_vm->_newRects.clear();

	// KEYFLG = 0;

	screen._scrollRow = screen._scrollCol = 0;
	screen._scrollX = screen._scrollY = 0;
	_vm->_player->_scrollAmount = 3;
	_vm->_scaleI = 255;

	_game->_plane._xCount = 0;
	_game->_plane._planeCount = 0;

	while (!_vm->shouldQuit() && !events.isKeyMousePressed() &&
			(screen._scrollCol + screen._vWindowWidth != _vm->_room->_playFieldWidth)) {
		events._vbCount = 4;
		screen._scrollX += _vm->_player->_scrollAmount;

		while (screen._scrollX >= TILE_WIDTH) {
			screen._scrollX -= TILE_WIDTH;
			++screen._scrollCol;

			_vm->_buffer1.moveBufferLeft();
			_vm->_room->buildColumn(screen._scrollCol + screen._vWindowWidth, screen._vWindowBytesWide);
		}

		scrollFall();

		while (!_vm->shouldQuit() && events._vbCount > 0) {
			events.pollEventsAndWait();
		}
	}

	events.showCursor();
}

void AmazonScripts::mWhileJWalk() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	Player &player = *_vm->_player;

	static const int JUNGLE_OBJ[7][4] = {
		{2, 77, 0, 40},
		{0, 290, 0, 50},
		{1, 210, 0, 70},
		{0, 50, 0, 30},
		{1, 70, 0, 20},
		{0, -280, 0, 60},
		{1, -150, 0, 30},
	};

	screen.fadeOut();
	events.hideCursor();
	screen.clearScreen();
	_vm->_buffer2.clearBuffer();
	screen.setBufferScan();
	screen._scrollX = 0;

	// Build the initial jungle scene and fade it in
	_vm->_room->buildScreen();
	_vm->copyBF2Vid();
	screen.fadeIn();

	// Set up the player to walk horizontally
	player._xFlag = 1;
	player._yFlag = 0;
	player._moveTo.x = 160;
	player._playerMove = true;

	_game->_plane._xCount = 2;
	_xTrack = 10;
	_yTrack = _zTrack = 0;
	_xCam = 480;
	_yCam = 0;
	_zCam = 80;

	TimerEntry *te = &_vm->_timers[24];
	te->_initTm = te->_timer = 1;
	te->_flag++;
	
	_pNumObj = 7;
	for (int i = 0; i < _pNumObj; i++) {
		_pObject[i] = _vm->_objectsTable[24];
		_pImgNum[i] = JUNGLE_OBJ[i][0];
		_pObjX[i] = JUNGLE_OBJ[i][1];
		_pObjY[i] = JUNGLE_OBJ[i][2];
		_pObjZ[i] = JUNGLE_OBJ[i][3];
		_pObjXl[i] = _pObjYl[i] = 0;
	}
	
	while (!_vm->shouldQuit() && !events.isKeyMousePressed() && (player._xFlag != 2)) {
		_vm->_images.clear();
		events._vbCount = 6;

		_pImgNum[0] = _game->_plane._xCount;
		if (_game->_plane._xCount == 2)
			++_game->_plane._xCount;
		else
			--_game->_plane._xCount;

		player.checkMove();
		player.checkScroll();
		pan();
		scrollJWalk();

		while (!_vm->shouldQuit() && events._vbCount > 0) {
			events.pollEventsAndWait();
		}
	}

	_vm->_images.clear();
	events.showCursor();
}

void AmazonScripts::mWhileDoOpen() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;

	screen.setBufferScan();
	events.hideCursor();
	screen.forceFadeOut();
	_game->_skipStart = false;
	if (_vm->_conversation != 2) {
		// Cutscene at start of chapter 1
		screen.setPanel(3);
		_game->startChapter(1);
		_game->establishCenter(0, 1);
	}

	Resource *data = _vm->_files->loadFile(1, 0);
	_vm->_objectsTable[1] = new SpriteResource(_vm, data);
	delete data;

	_vm->_files->_setPaletteFlag = false;
	_vm->_files->loadScreen(1, 2);
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);

	// Load animation data
	_vm->_animation->freeAnimationData();
	Resource *animResource = _vm->_files->loadFile(1, 1);
	_vm->_animation->loadAnimations(animResource);
	delete animResource;

	_xTrack = 8;
	_yTrack = -3;
	_zTrack = 0;
	_xCam = _yCam = 0;
	_zCam = 270;
	_vm->_timers[24]._timer = _vm->_timers[24]._initTm = 1;
	++_vm->_timers[24]._flag;
	_vm->_timers.updateTimers();

	_pNumObj = 10;
	for (int i = 0; i < _pNumObj; i++) {
		_pObject[i] = _vm->_objectsTable[1];
		_pImgNum[i] = OPENING_OBJS[i][0];
		_pObjX[i] = OPENING_OBJS[i][1];
		_pObjY[i] = OPENING_OBJS[i][2];
		_pObjZ[i] = OPENING_OBJS[i][3];
		_pObjXl[i] = _pObjYl[i] = 0;
	}

	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	Animation *anim = _vm->_animation->setAnimation(0);
	_vm->_animation->setAnimTimer(anim);
	anim = _vm->_animation->setAnimation(1);
	_vm->_animation->setAnimTimer(anim);
	_vm->_sound->newMusic(10, 0);
	
	bool startFl = false;
	while (!_vm->shouldQuit()) {
		_vm->_images.clear();
		_vm->_animation->animate(0);
		_vm->_animation->animate(1);
		pan();
		_vm->_buffer2.copyFrom(_vm->_buffer1);
		_vm->_newRects.clear();
		_game->plotList();
		_vm->copyBlocks();
		if (!startFl) {
			startFl = true;
			screen.forceFadeIn();
		}

		events.pollEventsAndWait();

		if (events._leftButton || events._rightButton || events._keypresses.size() > 0) {
			_game->_skipStart = true;
			_vm->_sound->newMusic(10, 1);

			events.debounceLeft();
			events.zeroKeys();
			break;
		}

		if (_xCam > 680) {
			events._vbCount = 125;
			
			while(!_vm->shouldQuit() && !events.isKeyMousePressed() && events._vbCount > 0) {
				events.pollEventsAndWait();
			}
			break;
		}
	}
	
	events.showCursor();
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	
	_vm->freeCells();
	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	_vm->_numAnimTimers = 0;
	_vm->_images.clear();

	if (_vm->_conversation == 2) {
		// Cutscene at end of Chapter 6
		Resource *spriteData = _vm->_files->loadFile(28, 37);
		_vm->_objectsTable[28] = new SpriteResource(_vm, spriteData);
		delete spriteData;
		
		_vm->_animation->freeAnimationData();
		animResource = _vm->_files->loadFile(28, 38);
		_vm->_animation->loadAnimations(animResource);
		delete animResource;
	}
}

void AmazonScripts::scrollRiver() {
	_vm->copyBF1BF2();
	_vm->_newRects.clear();
	_vm->_buffer2.plotImage(_vm->_objectsTable[33], 0, Common::Point(66, 30));
	_vm->plotList();
	_vm->copyRects();
	_vm->copyBF2Vid();
}

void AmazonScripts::mWhileDownRiver() {
	_vm->_events->hideCursor();
	_vm->_screen->setDisplayScan();
	_vm->_screen->clearScreen();
	_vm->_screen->savePalette();

	_vm->_files->loadScreen(95, 4);
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_screen->restorePalette();
	_vm->_screen->setPalette();
	_vm->_screen->setBufferScan();
	_vm->_screen->_scrollX = 0;
	_vm->_room->buildScreen();
	_vm->copyBF2Vid();

	_vm->_player->_scrollAmount = 2;
	_vm->_destIn = &_vm->_buffer2;
	_xTrack = -7;
	_yTrack = _zTrack = 0;
	_xCam = _yCam = 0;
	_zCam = 80;

	_game->_timers[24]._timer = 1;
	_game->_timers[24]._initTm = 1;
	++_game->_timers[24]._flag;
	
	_pNumObj = 14;
	for (int i = 0; i <_pNumObj; i++) {
		_pObject[i] = _vm->_objectsTable[33];
		_pImgNum[i] = DOWNRIVEROBJ[i][0];
		_pObjX[i] = DOWNRIVEROBJ[i][1];
		_pObjY[i] = DOWNRIVEROBJ[i][2];
		_pObjZ[i] = DOWNRIVEROBJ[i][3];
		_pObjXl[i] = _pObjYl[i] = 0;
	}

	_game->_timers[3]._timer = 200;
	_game->_timers[3]._initTm = 200;
	++_game->_timers[3]._flag;
	_game->_timers[4]._timer = 350;
	_game->_timers[4]._initTm = 350;
	++_game->_timers[4]._flag;

	while (!_vm->shouldQuit() && !_vm->_events->isKeyMousePressed() &&
			(_vm->_screen->_scrollCol + _vm->_screen->_vWindowWidth != _vm->_room->_playFieldWidth)) {
		_vm->_images.clear();
		_vm->_events->_vbCount = 6;

		_vm->_screen->_scrollX += _vm->_player->_scrollAmount;
		while (_vm->_screen->_scrollX >= TILE_WIDTH) {
			_vm->_screen->_scrollX -= TILE_WIDTH;
			++_vm->_screen->_scrollCol;
			_vm->_buffer1.moveBufferLeft();
			_vm->_room->buildColumn(_vm->_screen->_scrollCol + _vm->_screen->_vWindowWidth, _vm->_screen->_vWindowBytesWide);
		}

		pan();
		scrollRiver();

		if (!_game->_timers[3]._flag) {
			++_game->_timers[3]._flag;
			_vm->_sound->playSound(1);
		} else if (!_game->_timers[4]._flag) {
			++_game->_timers[4]._flag;
			_vm->_sound->playSound(0);
		}

		while (!_vm->shouldQuit() && _vm->_events->_vbCount > 0) {
			_vm->_events->pollEventsAndWait();
		}
	}

	_vm->_events->showCursor();
}

void AmazonScripts::mWhile(int param1) {
	switch(param1) {
	case 1:
		mWhile1();
		break;
	case 2:
		mWhileFly();
		break;
	case 3:
		mWhileFall();
		break;
	case 4:
		mWhileJWalk();
		break;
	case 5:
		mWhileDoOpen();
		break;
	case 6:
		mWhileDownRiver();
		break;
	case 7:
		mWhile2();
		break;
	case 8:
		mWhileJWalk2();
		break;
	default:
		break;
	}
}

void AmazonScripts::loadBackground(int param1, int param2) {
	_vm->_files->_setPaletteFlag = false;
	_vm->_files->loadScreen(param1, param2);

	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);

	_vm->_screen->forceFadeIn();
}

void AmazonScripts::doCast(int param1) {
	static const int END_OBJ[26][4] = {
		{ 0, 118, 210, 10},
		{ 1,  38, 250, 10},
		{ 2,  38, 280, 10},
		{ 3,  38, 310, 10},
		{ 4,  38, 340, 10},
		{ 5,  38, 370, 10},
		{ 6,  38, 400, 10},
		{ 7,  38, 430, 10},
		{ 8,  38, 460, 10},
		{ 9,  38, 490, 10},
		{10,  38, 520, 10},
		{11,  38, 550, 10},
		{12,  38, 580, 10},
		{13,  38, 610, 10},
		{14,  38, 640, 10},
		{15,  38, 670, 10},
		{16,  38, 700, 10},
		{17,  38, 730, 10},
		{18,  38, 760, 10},
		{19,  38, 790, 10},
		{20,  95, 820, 10},
		{21,  94, 850, 10},
		{22,  96, 880, 10},
		{23, 114, 910, 10},
		{24, 114, 940, 10},
		{25, 110, 970, 10}
	};

	static const int END_OBJ1[4][4] = {
		{0,  40, 1100, 10},
		{2,  11, 1180, 10},
		{1, 154, 1180, 10},
		{3, 103, 1300, 10}
	};

	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();
	_vm->_screen->forceFadeOut();
	_vm->_screen->_clipHeight = 173;
	_vm->_screen->clearScreen();
	_game->_chapter = 16;
	_game->tileScreen();
	_game->updateSummary(param1);
	_vm->_screen->setPanel(3);
	_game->_chapter = 14;

	Resource *spriteData = _vm->_files->loadFile(91, 0);
	_vm->_objectsTable[0] = new SpriteResource(_vm, spriteData);
	delete spriteData;
	spriteData = _vm->_files->loadFile(91, 1);
	_vm->_objectsTable[1] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_files->_setPaletteFlag = false;
	_vm->_files->loadScreen(58, 1);
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);

	_xTrack = 0;
	_yTrack = -6;
	_zTrack = 0;
	_xCam = _yCam = 0;
	_zCam = 60;

	_game->_timers[24]._timer = 1;
	_game->_timers[24]._initTm = 1;
	++_game->_timers[24]._flag;

	_pNumObj = 26;
	for (int i = 0; i < _pNumObj; i++) {
		_pObject[i] = _vm->_objectsTable[0];
		_pImgNum[i] = END_OBJ[i][0];
		_pObjX[i] = END_OBJ[i][1];
		_pObjY[i] = END_OBJ[i][2];
		_pObjZ[i] = END_OBJ[i][3];
		_pObjXl[i] = _pObjYl[i] = 0;
	}

	_pNumObj = 4;
	for (int i = 0; i < _pNumObj; i++) {
		_pObject[26 + i] = _vm->_objectsTable[1];
		_pImgNum[26 + i] = END_OBJ1[i][0];
		_pObjX[26 + i] = END_OBJ1[i][1];
		_pObjY[26 + i] = END_OBJ1[i][2];
		_pObjZ[26 + i] = END_OBJ1[i][3];
		_pObjXl[26 + i] = _pObjYl[26 + i] = 0;
	}

	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	_vm->_numAnimTimers = 0;

	_vm->_sound->newMusic(58, 0);
	_vm->_screen->forceFadeIn();

	while (!_vm->shouldQuit()) {
		_vm->_images.clear();
		pan();
		_vm->_buffer2.copyFrom(_vm->_buffer1);
		_vm->_newRects.clear();
		_game->plotList();
		_vm->copyBlocks();

		_vm->_events->pollEvents();
		if (_vm->_events->isKeyMousePressed())
			break;

		if (_yCam < -7550) {
			_vm->_events->_vbCount = 50;

			while(!_vm->shouldQuit() && !_vm->_events->isKeyMousePressed() && _vm->_events->_vbCount > 0) {
				_vm->_events->pollEventsAndWait();
			}

			while (!_vm->shouldQuit() && !_vm->_sound->checkMidiDone())
				_vm->_events->pollEventsAndWait();

			break;
		}
	}

	_vm->_sound->newMusic(58, 1);
	_vm->_events->showCursor();

	_vm->freeCells();
	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	_vm->_numAnimTimers = 0;
	_vm->_images.clear();
	_vm->_screen->forceFadeOut();

	_vm->quitGame();
	_vm->_events->pollEvents();
}

void AmazonScripts::setInactive() {
	_game->_rawInactiveX = _vm->_player->_rawPlayer.x;
	_game->_rawInactiveY = _vm->_player->_rawPlayer.y;
	_game->_charSegSwitch = false;

	mWhile(_game->_rawInactiveY);
}

void AmazonScripts::plotTorchSpear(int indx, const int *&buf) {
	int idx = indx;

	ImageEntry ie;
	ie._flags = IMGFLAG_UNSCALED;
	ie._spritesPtr = _vm->_objectsTable[62];
	ie._frameNumber = buf[(idx / 2)];
	ie._position = Common::Point(_game->_pitPos.x + buf[(idx / 2) + 1], _game->_pitPos.y + buf[(idx / 2) + 2]);
	ie._offsetY = 255;
	_vm->_images.addToList(ie);
}

void AmazonScripts::plotPit(int indx, const int *&buf) {
	int idx = indx;
	ImageEntry ie;
	ie._flags = IMGFLAG_UNSCALED;
	ie._spritesPtr = _vm->_objectsTable[62];
	ie._frameNumber = buf[(idx / 2)];
	ie._position = Common::Point(_game->_pitPos.x, _game->_pitPos.y);
	ie._offsetY = _game->_pitPos.y;
	_vm->_images.addToList(ie);

	_vm->_player->_rawPlayer = _game->_pitPos;
	if (_vm->_inventory->_inv[76]._value == 1) {
		idx = _game->_torchCel;
		buf = Amazon::TORCH;
		_vm->_timers[14]._flag = 1;
		idx += 6;
		if (buf[idx / 2] == -1)
			idx = 0;
		_game->_torchCel = idx;
		plotTorchSpear(idx, buf);
	} else if (!_game->_stabFl && (_vm->_inventory->_inv[78]._value == 1)) {
		idx = 0;
		buf = Amazon::SPEAR;
		plotTorchSpear(idx, buf);
	}
}

int AmazonScripts::antHandleRight(int indx, const int *&buf) {
	int retval = indx;
	if (_game->_pitDirection == NONE) {
		_game->_pitDirection = UP;
		_game->_pitPos.y = 127;
	}
	retval = _game->_pitCel;
	buf = Amazon::PITWALK;
	if (_game->_pitPos.x < 230) {
		if (retval == 0) {
			retval = 48;
			_game->_pitPos.y = 127;
		}
		retval -= 6;
		_game->_pitPos.x -= buf[(retval / 2) + 1];
		_game->_pitPos.y -= buf[(retval / 2) + 2];
		_game->_pitCel = retval;
	}
	return retval;
}

int AmazonScripts::antHandleLeft(int indx, const int *&buf) {
	int retval = indx;
	if (_game->_pitDirection == UP) {
		_game->_pitDirection = NONE;
		_game->_pitPos.y = 127;
	}
	retval = _game->_pitCel;
	buf = Amazon::PITWALK;
	retval += 6;
	if (buf[retval / 2] == -1) {
		retval = 0;
		_game->_pitPos.y = 127;
	}
	_game->_pitPos.x += buf[(retval / 2) + 1];
	_game->_pitPos.y += buf[(retval / 2) + 2];
	_game->_pitCel = retval;

	return retval;
}

int AmazonScripts::antHandleStab(int indx, const int *&buf) {
	int retval = indx;
	if (_vm->_inventory->_inv[78]._value != 1) {
		if (_game->_stabFl) {
			buf = Amazon::PITSTAB;
			retval = _game->_stabCel;
			if (_game->_timers[13]._flag == 0) {
				_game->_timers[13]._flag = 1;
				retval += 6;
				if (Amazon::PITSTAB[retval] == -1) {
					_game->_stabFl = false;
					_game->_pitCel = 0;
					_game->_pitPos.y = 127;
					retval = 0;
					buf = Amazon::PITWALK;
				} else {
					_game->_pitPos.x += buf[(retval / 2) + 1];
					_game->_pitPos.y += buf[(retval / 2) + 2];
					_game->_pitCel = retval;
				}
			}
		} else {
			_game->_stabFl = true;
			_game->_pitCel = 0;
			retval = 0;
			_game->_stabCel = 0;
			int dist = _game->_pitPos.x - _game->_antPos.x;
			if (_game->_antEatFl && !_game->_antDieFl && (dist <= 80)) {
				_game->_antDieFl = true;
				_game->_antCel = 0;
				_game->_antPos.y = 123;
				_vm->_sound->playSound(1);
			}
		}
	}
	return retval;
}

void AmazonScripts::ANT() {
	_game->_antDirection = NONE;
	if (_game->_aniFlag != 1) {
		_game->_aniFlag = 1;
		_game->_antCel = 0;
		_game->_torchCel = 0;
		_game->_pitCel = 0;

		_game->_timers[15]._timer = 16;
		_game->_timers[15]._initTm = 16;
		_game->_timers[15]._flag = 1;

		_game->_timers[13]._timer = 5;
		_game->_timers[13]._initTm = 5;
		_game->_timers[13]._flag = 1;

		_game->_timers[14]._timer = 10;
		_game->_timers[14]._initTm = 10;
		_game->_timers[14]._flag = 1;

		_game->_antPos = Common::Point(-40, 123);
		_game->_antDieFl = _game->_antEatFl = false;
		_game->_stabFl = false;
		_game->_pitPos = Common::Point(_vm->_player->_rawPlayer.x, 127);
	}

	const int *buf = nullptr;
	if (_game->_antDieFl) {
		buf = Amazon::ANTDIE;
	} else if (_game->_antEatFl) {
		buf = Amazon::ANTEAT;
	} else if (_game->_antPos.x > 120 && _vm->_flags[198] == 1) {
		_game->_antEatFl = true;
		_vm->_flags[235] = 1;
		_game->_antCel = 0;
		buf = Amazon::ANTEAT;
	} else {
		buf = Amazon::ANTWALK;
		if (_vm->_inventory->_inv[76]._value == 1)
			_game->_antDirection = UP;
	}

	int idx = _game->_antCel;
	if (_game->_timers[15]._flag == 0) {
		_game->_timers[15]._flag = 1;
		if (_game->_antDirection == UP) {
			if (_game->_antPos.x > 10) {
				if (idx == 0)
					idx = 36;
				else
					idx -= 6;

				_game->_antPos = Common::Point(buf[(idx / 2) + 1], buf[(idx / 2) + 2]);
				_game->_antCel = idx;
			}
		} else {
			idx += 6;
			if (buf[(idx / 2)] != -1) {
				_game->_antPos = Common::Point(buf[(idx / 2) + 1], buf[(idx / 2) + 2]);
				_game->_antCel = idx;
			} else if (!_game->_antDieFl) {
				idx = 0;
				_game->_antPos = Common::Point(buf[(idx / 2) + 1], buf[(idx / 2) + 2]);
				_game->_antCel = idx;
			} else {
				idx -= 6;
				if (_game->_flags[200] == 0)
					_game->_flags[200] = 1;
			}
		}
	}

	ImageEntry ie;
	ie._flags = IMGFLAG_UNSCALED;
	ie._spritesPtr = _vm->_objectsTable[61];
	ie._frameNumber = buf[(idx / 2)];
	ie._position = Common::Point(_game->_antPos.x, _game->_antPos.y);
	ie._offsetY = _game->_antPos.y - 70;
	_vm->_images.addToList(ie);
	_game->_antCel = idx;

	if (_game->_flags[196] != 1) {
		idx = _game->_pitCel;
		if (_game->_stabFl == 1) {
			idx = antHandleStab(idx, buf);
		} else {
			buf = Amazon::PITWALK;
			if (_game->_timers[13]._flag == 0) {
				_game->_timers[13]._flag = 1;
				_vm->_events->pollEvents();
				if (_vm->_events->_leftButton) {
					Common::Point pt = _vm->_events->calcRawMouse();
					if (pt.x < _game->_pitPos.x)
						idx = antHandleLeft(idx, buf);
					else if (pt.x > _game->_pitPos.x)
						idx = antHandleRight(idx, buf);
				} else {
					buf = Amazon::PITWALK;
					if (_vm->_player->_playerDirection == UP)
						idx = antHandleStab(idx, buf);
					else if (_vm->_player->_playerDirection == LEFT)
						idx = antHandleLeft(idx, buf);
					else if (_vm->_player->_playerDirection == RIGHT)
						idx = antHandleRight(idx, buf);
				}
			}
		}
		plotPit(idx, buf);
	}

	if (!_game->_antDieFl) {
		int dist = _game->_pitPos.x - _game->_antPos.x;
		if ((_game->_antEatFl && (dist <= 45)) || (!_game->_antEatFl && (dist <= 80))) {
			_game->_flags[199] = 1;
			_game->_aniFlag = 0;
		}
	}
}

void AmazonScripts::boatWalls(int param1, int param2) {
	if (param1 == 1)
		_vm->_room->_plotter._walls[42] = Common::Rect(96, 27, 96 + 87, 27 + 42);
	else {
		_vm->_room->_plotter._walls[39].bottom = _vm->_room->_plotter._walls[41].bottom = 106;
		_vm->_room->_plotter._walls[40].left = 94;
	}
}

void AmazonScripts::plotInactive() {
	Player &player = *_vm->_player;
	InactivePlayer &inactive = _game->_inactive;

	if (_game->_charSegSwitch) {
		_game->_currentCharFlag = true;
		SWAP(inactive._altSpritesPtr, player._playerSprites);
		_game->_charSegSwitch = false;
	} else if (_game->_jasMayaFlag != (_game->_currentCharFlag ? 1 : 0)) {
		if (player._playerOff) {
			_game->_jasMayaFlag = (_game->_currentCharFlag ? 1 : 0);
		} else {
			_game->_currentCharFlag = (_game->_jasMayaFlag == 1);
			int tmpX = _game->_rawInactiveX;
			int tmpY = _game->_rawInactiveY;
			_game->_rawInactiveX = player._rawPlayer.x;
			_game->_rawInactiveY = player._rawPlayer.y;
			player._rawPlayer.x = tmpX;
			player._rawPlayer.y = tmpY;
			_game->_inactiveYOff = player._playerOffset.y;
			player.calcManScale();

			SWAP(inactive._altSpritesPtr, player._playerSprites);
			_vm->_room->setWallCodes();
		}
	}

	_game->_flags[155] = 0;
	if (_game->_rawInactiveX >= 152 && _game->_rawInactiveX <= 167 &&
			_game->_rawInactiveY >= 158 && _game->_rawInactiveY <= 173) {
		_game->_flags[155] = 1;
	} else {
		_game->_flags[160] = 1;
		if (!_game->_jasMayaFlag && _game->_rawInactiveX <= 266 && _game->_rawInactiveX < 290
			&& _game->_rawInactiveY >= 70 && _game->_rawInactiveY <= 87) {
			_game->_flags[160] = 1;
		}
	}

	inactive._flags &= ~IMGFLAG_UNSCALED;
	inactive._flags &= ~IMGFLAG_BACKWARDS;
	inactive._position.x = _game->_rawInactiveX;
	inactive._position.y = _game->_rawInactiveY - _game->_inactiveYOff;
	inactive._offsetY = _game->_inactiveYOff;
	inactive._spritesPtr = inactive._altSpritesPtr;

	_vm->_images.addToList(_game->_inactive);
}

void AmazonScripts::setRiverPan() {
	static const int RIVER1OBJ[23][4] = {
		{18,   -77,  0, 30},
		{18,  -325,  0, 20},
		{18,  -450,  0, 15},
		{18, -1250,  0, 25},
		{19,  -130,  0, 20},
		{19,  -410,  0, 15},
		{19,  -710,  0, 25},
		{19, -1510,  0, 20},
		{20,  -350,  0, 30},
		{20,  -695,  0, 25},
		{20,  -990,  0, 20},
		{20, -1300,  0, 25},
		{20, -1600,  0, 30},
		{21,  -370,  0, 20},
		{21,  -650,  0, 30},
		{21, -1215,  0, 40},
		{21, -1815,  0, 35},
		{22,  -380,  0, 25},
		{22,  -720,  0, 35},
		{22, -1020,  0, 30},
		{22, -1170,  0, 25},
		{22, -1770,  0, 35},
		{23,  -500, 63, 20}
	};

	int delta = (_vm->_screen->_scrollCol * 16) + _vm->_screen->_scrollX;

	_xTrack = 9;
	_yTrack = _zTrack = 0;
	_xCam = 160;
	_yCam = 0;
	_zCam = 80;

	_game->_timers[24]._timer = 1;
	_game->_timers[24]._initTm = 1;
	++_game->_timers[24]._flag;

	_pNumObj = 23;
	for (int i = 0; i < _pNumObj; i++) {
		_pObject[i] = _vm->_objectsTable[45];
		_pImgNum[i] = RIVER1OBJ[i][0];
		_pObjX[i] = RIVER1OBJ[i][1] + delta;
		_pObjY[i] = RIVER1OBJ[i][2];
		_pObjZ[i] = RIVER1OBJ[i][3];
		_pObjXl[i] = _pObjYl[i] = 0;
	}
}

void AmazonScripts::initRiver() {
	static const int RIVERVXTBL[3] = {6719, 7039, 8319};

	_vm->_events->centerMousePos();
	_vm->_events->restrictMouse();
	_vm->_screen->setDisplayScan();
	_vm->_screen->clearScreen();
	_vm->_screen->savePalette();
	_vm->_screen->forceFadeOut();

	_vm->_files->_setPaletteFlag = false;
	_vm->_files->loadScreen(95, 4);
	_vm->_buffer2.copyFrom(*_vm->_screen);

	_vm->_screen->restorePalette();
	_vm->_screen->setBufferScan();
	_vm->_destIn = &_vm->_buffer2;
	_vm->_room->roomMenu();

	if (_game->_saveRiver) {
		// Restoring a savegame, so set properties from saved fields
		_vm->_screen->_scrollRow = _vm->_rScrollRow;
		_vm->_screen->_scrollCol = _vm->_rScrollCol;
		_vm->_screen->_scrollX = _vm->_rScrollX;
		_vm->_screen->_scrollY = _vm->_rScrollY;
	} else {
		// Set initial scene state
		_vm->_screen->_scrollRow = 0;
		_vm->_screen->_scrollCol = 140;
		_vm->_screen->_scrollX = 0;
		_vm->_screen->_scrollY = 0;
	}

	_vm->_room->buildScreen();
	_vm->copyBF2Vid();
	_vm->_screen->forceFadeIn();

	if (_game->_saveRiver) {
		// Restore draw rects from savegame
		_vm->_oldRects.resize(_vm->_rOldRectCount);
		_vm->_newRects.resize(_vm->_rNewRectCount);
		// KEYFLG = _vm->_rKeyFlag
	} else {
		// Reset draw rects
		_vm->_oldRects.clear();
		_vm->_newRects.clear();
		// KEYFLG = 0
	}

	_vm->_player->_scrollAmount = 2;
	setRiverPan();
	_game->_timers[3]._timer = 1;
	_game->_timers[3]._initTm = 1;
	++_game->_timers[3]._flag;

	_game->_canoeFrame = 0;
	_game->_mapPtr = (const byte *)MAPTBL[_game->_riverFlag] + 1;
	if (_game->_saveRiver) {
		_game->_mapPtr--;
		_game->_mapPtr += _game->_mapOffset;
	} else {
		_screenVertX = RIVERVXTBL[_game->_riverFlag] - 320;
		_game->_canoeLane = 3;
		_game->_hitCount = 0;
		_game->_hitSafe = 0;
		_game->_canoeYPos = 71;
	}

	_game->_riverIndex = _game->_riverFlag;
	_game->_topList = RIVEROBJECTTBL[_game->_riverIndex];
	updateObstacles();
	riverSetPhysX();
	_game->_canoeDir = 0;
	_game->_deathFlag = 0;
	_game->_deathCount = 0;

	_game->_timers[11]._timer = 1200;
	_game->_timers[11]._initTm = 1200;
	++_game->_timers[11]._flag;
	_game->_timers[12]._timer = 1500;
	_game->_timers[12]._initTm = 1500;
	++_game->_timers[12]._flag;
	
	_game->_maxHits = 2 - _game->_riverFlag;
	_game->_saveRiver = false;
}

void AmazonScripts::resetPositions() {
	riverSetPhysX();
	int val = (_vm->_screen->_scrollCol + 1 - _game->_oldScrollCol) * 16;
	if (val > 256) {
		val &= 0x7F;
		val |= 0x80;
	}

	for (int i = 0; i < _pNumObj; i++)
		_pObjX[i] += val;
}

void AmazonScripts::checkRiverPan() {
	int val = (_vm->_screen->_scrollCol + 20) * 16;

	for (int i = 0; i < _pNumObj; i++) {
		if (_pObjX[i] < val)
			return;
	}

	setRiverPan();
}

bool AmazonScripts::riverJumpTest() {
	if (_vm->_screen->_scrollCol == 120 || _vm->_screen->_scrollCol == 60 || _vm->_screen->_scrollCol == 0) {
		int val = _game->_mapPtr[0];
		++_game->_mapPtr;
		if (val == 0xFF)
			return true;
		_game->_oldScrollCol = _vm->_screen->_scrollCol;

		if (val == 0) {
			_vm->_screen->_scrollCol = 139;
			_vm->_screen->_scrollX = 14;
			_vm->_room->buildScreen();
			resetPositions();
			return false;
		}
	} else if (_vm->_screen->_scrollCol == 105) {
		int val1 = _game->_mapPtr[1];
		int val2 = _game->_mapPtr[2];
		_game->_mapPtr += 3;
		if (_game->_canoeLane < 3) {
			if (val1 != 0) {
				_game->_deathFlag = true;
				_game->_deathCount = 300;
				_game->_deathType = val2;
			}
		} else {
			if (val1 != 1) {
				_game->_deathFlag = true;
				_game->_deathCount = 300;
				_game->_deathType = val2;
			}
			_game->_oldScrollCol = _vm->_screen->_scrollCol;
			_vm->_screen->_scrollCol = 44;
			_vm->_screen->_scrollX = 14;
			_vm->_room->buildScreen();
			resetPositions();
			return false;
		}
	}

	_vm->_screen->_scrollX = 14;
	--_vm->_screen->_scrollCol;
	_vm->_buffer1.moveBufferRight();
	_vm->_room->buildColumn(_vm->_screen->_scrollCol, 0);
	checkRiverPan();
	return false;
}

void AmazonScripts::riverSound() {
	if (_game->_timers[11]._flag == 0) {
		++_game->_timers[11]._flag;
		_vm->_sound->playSound(2);
	}

	if (_game->_timers[12]._flag == 0) {
		++_game->_timers[12]._flag;
		_vm->_sound->playSound(3);
	}

	if ((_xCam >= 1300) && (_xCam <= 1320))
		_vm->_sound->playSound(1);
}

void AmazonScripts::moveCanoe() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	Common::Point pt = events.calcRawMouse();

	// Do an event polling
	_vm->_canSaveLoad = true;
	events.pollEvents();
	_vm->_canSaveLoad = false;
	if (_vm->_room->_function == FN_CLEAR1)
		return;

	if (_game->_canoeDir) {
		// Canoe movement in progress
		moveCanoe2();
	} else {
		if (events._leftButton && pt.y >= 140) {
			if (pt.x < RMOUSE[8][0]) {
				// Disk icon wasn't clicked
				printString(BAR_MESSAGE);
			} else {
				// Clicked on the Disc icon
				_game->_saveRiver = true;
				_game->_rScrollRow = screen._scrollRow;
				_game->_rScrollCol = screen._scrollCol;
				_game->_rScrollX = screen._scrollX;
				_game->_rScrollY = screen._scrollY;
				_game->_mapOffset = _game->_mapPtr - MAPTBL[_game->_riverFlag];

				// Show the ScummVM menu
				_vm->_room->handleCommand(9);

				if (_vm->_room->_function != FN_CLEAR1) {
					_game->_saveRiver = false;
					_vm->_room->buildScreen();
					_vm->copyBF2Vid();
				}
			}
		} else if ((events._leftButton && pt.y <= _game->_canoeYPos) ||
			(!events._leftButton && _vm->_player->_move == UP)) {
			// Move canoe up
			if (_game->_canoeLane > 0) {
				_game->_canoeDir = -1;
				_game->_canoeMoveCount = 0;

				moveCanoe2();
			}
		} else if (events._leftButton || _vm->_player->_move == DOWN) {
			// Move canoe down
			if (_game->_canoeLane < 7) {
				_game->_canoeDir = 1;
				_game->_canoeMoveCount = 0;

				moveCanoe2();
			}
		}
	}
}

void AmazonScripts::moveCanoe2() {
	_game->_canoeYPos += _game->_canoeDir;

	if (++_game->_canoeMoveCount == 5) {
		_game->_canoeYPos += _game->_canoeDir;
		_game->_canoeDir = 0;
	}
}

void AmazonScripts::updateObstacles() {
	RiverStruct *cur;
	for (cur = _game->_topList; cur < RIVEROBJECTTBL[_game->_riverIndex + 1]; ++cur) {
		int val = cur->_field1 + cur->_field3 - 1;
		if (val < _screenVertX)
			break;

		if (cur->_field3 < (_game->_screenVirtX + 319)) {
			_game->_topList = cur;
			_game->_botList = cur;

			while (cur < RIVEROBJECTTBL[_game->_riverIndex + 1]) {
				++cur;
				val = cur->_field1 + cur->_field3 - 1;
				if (val < _screenVertX || (cur->_field3 >= (_game->_screenVirtX + 319)))
					break;

				_game->_botList = cur;
			}

			return;
		}
	}

	cur = _game->_topList;
	cur--;
	_game->_botList = cur;
}

void AmazonScripts::riverSetPhysX() {
	int val = (_vm->_screen->_scrollCol * 16) + _vm->_screen->_scrollX;
	RiverStruct *cur = _game->_topList;
	while (cur <= _game->_botList) {
		cur[0]._field5 = val - (_screenVertX - cur[0]._field3);
		++cur;
	}
}

bool AmazonScripts::checkRiverCollide() {
	if (_game->_hitSafe)
		return false;

	_game->_canoeVXPos = _screenVertX + 170;

	for (RiverStruct *si = _game->_topList; si <= _game->_botList; ++si) {
		if (si[0]._lane < _game->_canoeLane)
			continue;

		if ((si[0]._lane == _game->_canoeLane) || (si[0]._lane == _game->_canoeLane + 1)) {
			if (si[0]._field1 + si[0]._field3 - 1 >= _game->_canoeVXPos) {
				if (_game->_canoeVXPos + 124 >= si[0]._field3) {
					_vm->_sound->playSound(4);
					return true;
				}
			}
		}
	}
	return false;
}

void AmazonScripts::plotRiver() {
	if (_vm->_timers[3]._flag == 0) {
		++_vm->_timers[3]._flag;
		if (_game->_canoeFrame == 12)
			_game->_canoeFrame = 0;
		else
			++_game->_canoeFrame;
	}

	ImageEntry ie;
	ie._flags = IMGFLAG_UNSCALED;
	ie._spritesPtr = _vm->_objectsTable[45];
	ie._frameNumber = _game->_canoeFrame;
	ie._position.x = (_vm->_screen->_scrollCol * 16) + _vm->_screen->_scrollX + 160;
	ie._position.y = _game->_canoeYPos - 41;
	ie._offsetY = 41;
	_vm->_images.addToList(ie);

	RiverStruct *cur = _game->_topList;
	while (cur <= _game->_botList) {
		if (cur[0]._id != -1) {
			ie._flags = IMGFLAG_UNSCALED;
			ie._spritesPtr = _vm->_objectsTable[45];
			ie._frameNumber = 0;
			ie._position.x = cur[0]._field5;
			int val = (cur[0]._lane * 5) + 56;
			ie._position.y = val - cur[0]._field8;
			ie._offsetY = cur[0]._field8;
			_vm->_images.addToList(ie);
		}
		++cur;
	}
}

void AmazonScripts::scrollRiver1() {
	_vm->copyBF1BF2();
	_vm->_newRects.clear();
	plotRiver();
	_vm->plotList();
	_vm->copyRects();
	_vm->copyBF2Vid();
}

void AmazonScripts::RIVER() {
	static const int RIVERDEATH[5] = {22, 23, 24, 25, 26};

	initRiver();
	_vm->_events->showCursor();

	while (!_vm->shouldQuit()) {
		_vm->_events->_vbCount = 4;

//		int bx = _vm->_player->_scrollAmount - _screenVertX;
		if (_vm->_screen->_scrollX == 0) {
			_vm->_sound->midiRepeat();
			if (riverJumpTest()) {
				_CHICKENOUTFLG = false;
				return;
			}
		} else {
			_vm->_screen->_scrollX -= _vm->_player->_scrollAmount;
		}

		if (_CHICKENOUTFLG) {
			_CHICKENOUTFLG = false;
			return;
		}

		_vm->_images.clear();
		_vm->_animation->animate(0);

		riverSound();
		pan();
		moveCanoe();

		if (_vm->_room->_function != FN_CLEAR1) {
			updateObstacles();
			riverSetPhysX();
			bool checkCollide = checkRiverCollide();
			if (_game->_hitSafe != 0)
				_game->_hitSafe -= 2;

			if (checkCollide) {
				cmdDead(RIVERDEATH[0]);
				return;
			}

			if (_game->_deathFlag) {
				_game->_deathCount--;
				if (_game->_deathCount == 0) {
					cmdDead(RIVERDEATH[_game->_deathType]);
					return;
				}
			}

			// Scroll the river
			scrollRiver1();

			// Allow time for new scrolled river position to be shown
			_vm->_canSaveLoad = true;
			while (!_vm->shouldQuit() && _vm->_room->_function == FN_NONE &&
					_vm->_events->_vbCount > 0) {
				_vm->_events->pollEventsAndWait();
			}
			_vm->_canSaveLoad = false;
		}

		if (_vm->_room->_function == FN_CLEAR1) {
			_endFlag = true;
			_returnCode = 0;
			_CHICKENOUTFLG = false;
			break;
		}
	}
}

void AmazonScripts::executeSpecial(int commandIndex, int param1, int param2) {
	switch (commandIndex) {
	case 1:
		_vm->establish(param1, param2);
		break;
	case 2:
		loadBackground(param1, param2);
		break;
	case 3:
		doCast(param1);
		break;
	case 4:
		setInactive();
		break;
	case 6:
		mWhile(param1);
		break;
	case 9:
		_game->_guard.guard();
		break;
	case 10:
		_vm->_sound->newMusic(param1, param2);
		break;
	case 11:
		plotInactive();
		break;
	case 13:
		RIVER();
		break;
	case 14:
		ANT();
		break;
	case 15:
		boatWalls(param1, param2);
		break;
	default:
		warning("Unexpected Special code %d - Skipped", commandIndex);
	}
}

typedef void(AmazonScripts::*AmazonScriptMethodPtr)();

void AmazonScripts::executeCommand(int commandIndex) {
	static const AmazonScriptMethodPtr COMMAND_LIST[] = {
		&AmazonScripts::cmdHelp, &AmazonScripts::cmdCycleBack,
		&AmazonScripts::cmdChapter, &AmazonScripts::cmdSetHelp,
		&AmazonScripts::cmdCenterPanel, &AmazonScripts::cmdMainPanel,
		&AmazonScripts::CMDRETFLASH
	};

	if (commandIndex >= 73)
		(this->*COMMAND_LIST[commandIndex - 73])();
	else
		Scripts::executeCommand(commandIndex);
}

void AmazonScripts::cmdHelp() {
	Common::String helpMessage = readString();

	if (_game->_helpLevel == 0) {
		_game->_timers.saveTimers();
		_game->_useItem = 0;

		if (_game->_noHints) {
			printString(NO_HELP_MESSAGE);
			return;
		} else if (_game->_hintLevel == 0) {
			printString(NO_HINTS_MESSAGE);
			return;
		}
	}

	int level = _game->_hintLevel - 1;
	if (level < _game->_helpLevel)
		_game->_moreHelp = 0;

	_game->drawHelp(helpMessage);

	while(true) {
		while(!_vm->shouldQuit() && !_vm->_events->_leftButton)
			_vm->_events->pollEventsAndWait();

		_vm->_events->debounceLeft();

		static const Common::Rect butn1 = Common::Rect(HELP1COORDS[0][0], HELP1COORDS[0][2], HELP1COORDS[0][1], HELP1COORDS[0][3]);
		static const Common::Rect butn2 = Common::Rect(HELP1COORDS[1][0], HELP1COORDS[1][2], HELP1COORDS[1][1], HELP1COORDS[1][3]);
		const Common::Point pt = _vm->_events->_mousePos;

		int choice = -1;
		if (butn1.contains(pt))
			choice = 0;
		else if (butn2.contains(pt))
			choice = 1;

		if (choice < 0)
			continue;

		if (choice == 1) {
			_game->_helpLevel = 0;
			_game->_moreHelp = 1;
			_game->_useItem = 0;
			_vm->_events->hideCursor();
			_vm->_screen->restoreScreen();
			_vm->_screen->setPanel(0);
			_vm->_buffer2.copyFrom(*_vm->_screen);
			_vm->_screen->restorePalette();
			_vm->_screen->setPalette();
			_vm->_events->showCursor();
			free(_vm->_objectsTable[45]);
			_vm->_objectsTable[45] = nullptr;
			_vm->_timers.restoreTimers();
			break;
		} else {
			if ((_game->_moreHelp == 0) || (choice != 0))
				continue;
			++_game->_helpLevel;
			_game->_useItem = 1;
			break;
		}
	}
	findNull();
}

void AmazonScripts::cmdCycleBack() {
	if (_vm->_startup == -1)
		_vm->_screen->cyclePaletteBackwards();
}
void AmazonScripts::cmdChapter() {
	int chapter = _data->readByte();
	_game->startChapter(chapter);
}

void AmazonScripts::cmdSetHelp() {
	int arrayId = (_data->readUint16LE() && 0xFF) - 1;
	int helpId = _data->readUint16LE() && 0xFF;

	byte *help = _vm->_helpTbl[arrayId];
	help[helpId] = 1;

	if (_vm->_useItem == 0) {
		_sequence = 11000;
		searchForSequence();
	}
}

void AmazonScripts::cmdCenterPanel() {
	if (_vm->_screen->_vesaMode) {
		_vm->_screen->clearScreen();
		_vm->_screen->setPanel(3);
	}
}

void AmazonScripts::cmdMainPanel() {
	if (_vm->_screen->_vesaMode) {
		_vm->_room->init4Quads();
		_vm->_screen->setPanel(0);
	}
}

void AmazonScripts::CMDRETFLASH() { 
	error("TODO CMDRETFLASH"); 
}

} // End of namespace Amazon

} // End of namespace Access
