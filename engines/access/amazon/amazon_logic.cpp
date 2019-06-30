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

#include "common/scummsys.h"
#include "access/access.h"
#include "access/resources.h"
#include "access/screen.h"
#include "access/amazon/amazon_game.h"
#include "access/amazon/amazon_logic.h"
#include "access/amazon/amazon_resources.h"

namespace Access {

namespace Amazon {

PannedScene::PannedScene(AmazonEngine *vm) : AmazonManager(vm) {
	for (int i = 0; i < PAN_SIZE; ++i) {
		_pan[i]._pObject = nullptr;
		_pan[i]._pImgNum = 0;
		_pan[i]._pObjX = _pan[i]._pObjY = _pan[i]._pObjZ = 0;
		_pan[i]._pObjXl = _pan[i]._pObjYl = 0;
	}

	_xCount = 0;
	_xTrack = _yTrack = _zTrack = 0;
	_xCam = _yCam = _zCam = 0;
	_pNumObj = 0;
}

void PannedScene::pan() {
	_zCam += _zTrack;
	_xCam += _xTrack;
	int tx = (_xTrack << 8) / _zCam;
	_yCam += _yTrack;
	int ty = (_yTrack << 8) / _zCam;

	if (_vm->_timers[24]._flag != 1) {
		++_vm->_timers[24]._flag;
		for (int i = 0; i < _pNumObj; i++) {
			_pan[i]._pObjZ += _zTrack;
			_pan[i]._pObjXl += (_pan[i]._pObjZ * tx) & 0xff;
			_pan[i]._pObjX += ((_pan[i]._pObjZ * tx) >> 8) + (_pan[i]._pObjXl >> 8);
			_pan[i]._pObjXl &= 0xff;

			_pan[i]._pObjYl += (_pan[i]._pObjZ * ty) & 0xff;
			_pan[i]._pObjY += ((_pan[i]._pObjZ * ty) >> 8) + (_pan[i]._pObjYl >> 8);
			_pan[i]._pObjYl &= 0xff;
		}
	}

	for (int i = 0; i < _pNumObj; i++) {
		ImageEntry ie;
		ie._flags = IMGFLAG_UNSCALED;
		ie._position = Common::Point(_pan[i]._pObjX, _pan[i]._pObjY);
		ie._offsetY = 255;
		ie._spritesPtr = _pan[i]._pObject;
		ie._frameNumber = _pan[i]._pImgNum;

		_vm->_images.addToList(ie);
	}
}

/*------------------------------------------------------------------------*/

CampScene::CampScene(AmazonEngine *vm) : PannedScene(vm) {
	_skipStart = false;
}

void CampScene::mWhileDoOpen() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;

	screen.setDisplayScan();
	events.hideCursor();
	screen.forceFadeOut();
	_skipStart = false;
	if (_vm->_conversation != 2) {
		// Cutscene at start of chapter 1
		screen.setPanel(3);
		_vm->startChapter(1);
		_vm->establishCenter(0, 1);
	}

	Resource *data = _vm->_files->loadFile(1, 0);
	_vm->_objectsTable[1] = new SpriteResource(_vm, data);
	delete data;

	_vm->_files->_setPaletteFlag = false;
	_vm->_files->loadScreen(1, 2);
	_vm->_buffer2.blitFrom(*_vm->_screen);
	_vm->_buffer1.blitFrom(*_vm->_screen);

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
		_pan[i]._pObject = _vm->_objectsTable[1];
		_pan[i]._pImgNum = OPENING_OBJS[i][0];
		_pan[i]._pObjX = OPENING_OBJS[i][1];
		_pan[i]._pObjY = OPENING_OBJS[i][2];
		_pan[i]._pObjZ = OPENING_OBJS[i][3];
		_pan[i]._pObjXl = _pan[i]._pObjYl = 0;
	}

	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	Animation *anim = _vm->_animation->setAnimation(0);
	_vm->_animation->setAnimTimer(anim);
	anim = _vm->_animation->setAnimation(1);
	_vm->_animation->setAnimTimer(anim);
	_vm->_midi->newMusic(10, 0);

	bool startFl = false;
	while (!_vm->shouldQuit()) {
		_vm->_images.clear();
		_vm->_animation->animate(0);
		_vm->_animation->animate(1);
		pan();
		_vm->_buffer2.copyFrom(_vm->_buffer1);
		_vm->_newRects.clear();
		_vm->plotList();
		_vm->copyBlocks();
		if (!startFl) {
			startFl = true;
			screen.forceFadeIn();
		}

		events.pollEventsAndWait();

		if (_vm->_events->isKeyMousePressed()) {
			_skipStart = true;
			_vm->_midi->newMusic(10, 1);
			break;
		}

		if (_xCam > 680) {
			events._vbCount = 125;

			while (!_vm->shouldQuit() && !events.isKeyMousePressed() && events._vbCount > 0) {
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

	if (_vm->isCD()) {
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
	} else {
		_vm->freeCells();
		_vm->_oldRects.clear();
		_vm->_newRects.clear();
		_vm->_numAnimTimers = 0;
		_vm->_images.clear();
	}
}

/*------------------------------------------------------------------------*/

Opening::Opening(AmazonEngine *vm) : CampScene(vm) {
	_pCount = 0;
}

void Opening::doIntroduction() {
	Screen &screen = *_vm->_screen;

	screen.setInitialPalettte();
	_vm->_events->setCursor(CURSOR_ARROW);
	_vm->_events->showCursor();
	screen.setPanel(0);
	screen.setPalette();

	_vm->_events->setCursor(CURSOR_ARROW);
	_vm->_events->showCursor();
	screen.setPanel(3);
	doTitle();

	if (_vm->shouldQuit() || _skipStart || _vm->isDemo())
		return;

	screen.setPanel(3);
	mWhileDoOpen();

	if (_vm->shouldQuit() || _skipStart)
		return;

	doTent();
}

void Opening::doCredit() {
	if (_pCount < 15)
		return;

	if (_pCount <= 75)
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], _vm->isDemo()? 24 : 0, Common::Point(90, 35));
	else if (_pCount <= 210)
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 1, Common::Point(65, 35));
	else if (_pCount <= 272)
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 2, Common::Point(96, 45));
	else if (_pCount <= 334)
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 3, Common::Point(68, 54));
	else if (_pCount <= 396)
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 4, Common::Point(103, 54));
	else if (_pCount <= 458) {
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 5, Common::Point(8, 5));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 12, Common::Point(88, 55));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 6, Common::Point(194, 98));
	} else if (_pCount <= 520) {
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 7, Common::Point(32, 13));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 8, Common::Point(162, 80));
	} else if (_pCount <= 580) {
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 9, Common::Point(18, 15));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 10, Common::Point(164, 81));
	} else
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 11, Common::Point(106, 55));
}

void Opening::doCreditDemo() {
	if (_pCount < 15)
		return;

	if (_pCount <= 75)
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], _vm->isDemo()? 24 : 0, Common::Point(90, 35));
	else if (_pCount <= 210)
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 25, Common::Point(82, 35));
	else if (_pCount <= 272) {
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 23, Common::Point(77, 20));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 4, Common::Point(50, 35));
	} else if (_pCount <= 334) {
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 16, Common::Point(200, 70));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 4, Common::Point(170, 85));
	} else if (_pCount <= 396) {
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 15, Common::Point(65, 15));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 2, Common::Point(30, 30));
	} else if (_pCount <= 458) {
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 19, Common::Point(123, 40));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 10, Common::Point(115, 55));
	} else if (_pCount <= 520) {
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 18, Common::Point(50, 15));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 9, Common::Point(40, 30));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 0, Common::Point(40, 55));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 20, Common::Point(198, 95));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 3, Common::Point(160, 110));
	} else if (_pCount <= 580) {
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 21, Common::Point(40, 10));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 6, Common::Point(20, 25));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 22, Common::Point(145, 50));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 7, Common::Point(125, 65));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 12, Common::Point(207, 90));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 5, Common::Point(200, 105));
	} else {
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 11, Common::Point(125, 30));
		_vm->_buffer2.plotImage(_vm->_objectsTable[0], 4, Common::Point(115, 45));
	}
}

void Opening::scrollTitle() {
	_vm->copyBF1BF2();
	_vm->_newRects.clear();
	if (_vm->isDemo())
		doCreditDemo();
	else
		doCredit();
	_vm->copyRects();
	_vm->copyBF2Vid();
}

void Opening::doTitle() {
	Screen &screen = *_vm->_screen;

	screen.setDisplayScan();

	screen.forceFadeOut();
	_vm->_events->hideCursor();

	if (!_vm->isDemo()) {
		_vm->_sound->loadSoundTable(0, 98, 30);
		_vm->_sound->loadSoundTable(1, 98, 8);

		_vm->_files->_setPaletteFlag = false;
		_vm->_files->loadScreen(0, 3);

		_vm->_buffer2.copyFrom(*_vm->_screen);
		_vm->_buffer1.copyFrom(*_vm->_screen);
		screen.forceFadeIn();

		_vm->_sound->playSound(1, true);

		Resource *spriteData = _vm->_files->loadFile(0, 2);
		_vm->_objectsTable[0] = new SpriteResource(_vm, spriteData);
		delete spriteData;

		_vm->_files->_setPaletteFlag = false;
		_vm->_files->loadScreen(0, 4);

		_vm->_buffer2.copyFrom(*_vm->_screen);
		_vm->_buffer1.copyFrom(*_vm->_screen);

		const int COUNTDOWN[6] = { 2, 0x80, 1, 0x7d, 0, 0x87 };
		for (_pCount = 0; _pCount < 3 && !_vm->shouldQuit(); ++_pCount) {
			_vm->_buffer2.blitFrom(_vm->_buffer1);
			int id = COUNTDOWN[_pCount * 2];
			int xp = COUNTDOWN[_pCount * 2 + 1];
			_vm->_buffer2.plotImage(_vm->_objectsTable[0], id, Common::Point(xp, 71));
			_vm->_buffer2.copyTo(_vm->_screen);

			_vm->_events->_vbCount = 70;
			while (!_vm->shouldQuit() && _vm->_events->_vbCount > 0 && !_skipStart) {
				_vm->_events->pollEventsAndWait();
				if (_vm->_events->_rightButton)
					_skipStart = true;
			}
		}
		if (_vm->shouldQuit())
			return;

		_vm->_sound->stopSound();
		_vm->_sound->checkSoundQueue(); // HACK: Clear sound 1 from the queue
		_vm->_sound->playSound(0);
		screen.forceFadeOut();
		_vm->_events->_vbCount = 100;
		while (!_vm->shouldQuit() && _vm->_events->_vbCount > 0)
			_vm->_events->pollEventsAndWait();
		if (_vm->shouldQuit())
			return;

		_vm->_sound->freeSounds();
		delete _vm->_objectsTable[0];
		_vm->_objectsTable[0] = nullptr;

		_vm->_files->_setPaletteFlag = false;
		_vm->_files->loadScreen(0, 5);
		_vm->_buffer2.blitFrom(*_vm->_screen);
		_vm->_buffer1.blitFrom(*_vm->_screen);
		screen.forceFadeIn();
		_vm->_midi->newMusic(1, 0);
		_vm->_events->_vbCount = 950;
		while (!_vm->shouldQuit() && (_vm->_events->_vbCount > 0) && !_vm->_events->isKeyMousePressed()) {
			_vm->_events->pollEventsAndWait();
		}

		if (_vm->_events->_rightButton) {
			_skipStart = true;
			_vm->_room->clearRoom();
			_vm->_events->showCursor();
			return;
		}

		_vm->_midi->newMusic(1, 1);
		_vm->_midi->setLoop(false);
		_vm->_events->zeroKeys();
	}

	_vm->_buffer1.create(_vm->_screen->w + TILE_WIDTH, _vm->_screen->h);
	_vm->_room->loadRoom(0);
	screen.clearScreen();
	screen.setBufferScan();
	_vm->_scrollRow = _vm->_scrollCol = 0;
	_vm->_scrollX = _vm->_scrollY = 0;
	_vm->_player->_rawPlayer = Common::Point(0, 0);
	screen.forceFadeOut();
	_vm->_room->buildScreen();
	_vm->copyBF2Vid();
	screen.forceFadeIn();
	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	_vm->_events->clearEvents();
	_vm->_player->_scrollAmount = 1;
	_pCount = 0;

	while (!_vm->shouldQuit()) {
		if (_vm->_events->isKeyMousePressed()) {
			if (_vm->_events->_rightButton)
				_skipStart = true;
			_vm->_room->clearRoom();
			_vm->_events->showCursor();
			return;
		}

		_vm->_events->_vbCount = 4;
		if (_vm->_scrollCol + screen._vWindowWidth != _vm->_room->_playFieldWidth) {
			_vm->_scrollX += _vm->_player->_scrollAmount;

			while (_vm->_scrollX >= TILE_WIDTH) {
				_vm->_scrollX -= TILE_WIDTH;
				++_vm->_scrollCol;

				_vm->_buffer1.moveBufferLeft();
				_vm->_room->buildColumn(_vm->_scrollCol + screen._vWindowWidth, screen._vWindowBytesWide);
			}
			scrollTitle();
			++_pCount;

			while (!_vm->shouldQuit() && (_vm->_events->_vbCount > 0)) {
				_vm->_events->pollEventsAndWait();
			}
			continue;
		}

		_vm->_events->_vbCount = 120;
		while (!_vm->shouldQuit() && (_vm->_events->_vbCount > 0))
			_vm->_events->pollEventsAndWait();

		while (!_vm->shouldQuit()) {
			_pCount = 0;
			_vm->_events->_vbCount = 3;
			if (_vm->_scrollRow + screen._vWindowHeight >= _vm->_room->_playFieldHeight) {
				_vm->_room->clearRoom();
				_vm->_events->showCursor();
				return;
			}

			_vm->_scrollY = _vm->_scrollY + _vm->_player->_scrollAmount;

			while (_vm->_scrollY >= TILE_HEIGHT && !_vm->shouldQuit()) {
				_vm->_scrollY -= TILE_HEIGHT;
				++_vm->_scrollRow;
				_vm->_buffer1.moveBufferUp();

				// WORKAROUND: the original was using screen._vWindowBytesWide * screen._vWindowLinesTall
				_vm->_room->buildRow(_vm->_scrollRow + screen._vWindowHeight, screen._vWindowLinesTall);

				if (_vm->_scrollRow + screen._vWindowHeight >= _vm->_room->_playFieldHeight) {
					_vm->_room->clearRoom();
					_vm->_events->showCursor();
					return;
				}
			}
			scrollTitle();
			while (!_vm->shouldQuit() && (_vm->_events->_vbCount > 0))
				_vm->_events->pollEventsAndWait();
		}
	}
}

void Opening::doTent() {
	int step = 0;
	_vm->_screen->setDisplayScan();
	_vm->_screen->forceFadeOut();
	_vm->_events->hideCursor();
	_vm->_sound->loadSoundTable(0, 98, 39);
	_vm->_sound->loadSoundTable(1, 98, 14);
	_vm->_sound->loadSoundTable(2, 98, 15);
	_vm->_sound->loadSoundTable(3, 98, 16);
	_vm->_sound->loadSoundTable(4, 98, 31, 2);
	_vm->_sound->loadSoundTable(5, 98, 52, 2);
	_vm->_sound->playSound(0);

	_vm->_files->_setPaletteFlag = false;
	_vm->_files->loadScreen(2, 0);
	_vm->_buffer2.blitFrom(*_vm->_screen);
	_vm->_buffer1.blitFrom(*_vm->_screen);
	_vm->_screen->forceFadeIn();

	_vm->_video->setVideo(_vm->_screen, Common::Point(126, 73), FileIdent(2, 1), 10);
	int previousFrame = -1;
	while (!_vm->shouldQuit() && !_vm->_video->_videoEnd) {
		_vm->_video->playVideo();
		if (previousFrame != _vm->_video->_videoFrame) {
			previousFrame = _vm->_video->_videoFrame;

			if ((_vm->_video->_videoFrame == 32) || (_vm->_video->_videoFrame == 34))
				_vm->_sound->playSound(4);
			else if (_vm->_video->_videoFrame == 36) {
				if (step != 2) {
					_vm->_sound->playSound(2);
					step = 2;
				}
			} else if (_vm->_video->_videoFrame == 18) {
				if (step != 1) {
					_vm->_midi->newMusic(73, 1);
					_vm->_midi->newMusic(11, 0);
					step = 1;
					_vm->_sound->playSound(1);
				}
			}
		}
		_vm->_events->pollEventsAndWait();
	}

	_vm->_sound->playSound(5);
	_vm->_video->setVideo(_vm->_screen, Common::Point(43, 11), FileIdent(2, 2), 10);
	previousFrame = -1;
	while (!_vm->shouldQuit() && !_vm->_video->_videoEnd) {
		_vm->_video->playVideo();
		if (previousFrame != _vm->_video->_videoFrame) {
			previousFrame = _vm->_video->_videoFrame;
			if (_vm->_video->_videoFrame == 26) {
				_vm->_sound->playSound(5);
			} else if (_vm->_video->_videoFrame == 15) {
				if (step !=3) {
					_vm->_sound->playSound(3);
					step = 3;
				}
			}
		}
		_vm->_events->pollEventsAndWait();
	}

	_vm->_events->_vbCount = 200;
	while (!_vm->shouldQuit() && _vm->_events->_vbCount > 0)
		_vm->_events->pollEventsAndWait();

	_vm->_events->showCursor();
	_vm->_midi->newMusic(11, 1);
	_vm->_sound->_soundTable.clear();

	_vm->establishCenter(0, 4);
}

/*------------------------------------------------------------------------*/

Plane::Plane(AmazonEngine *vm) : PannedScene(vm) {
	_pCount = 0;
	_planeCount = 0;
	_propCount = 0;
}


void Plane::doFlyCell() {
	SpriteResource *sprites = _vm->_objectsTable[15];

	if (_pCount <= 40) {
		_vm->_buffer2.plotImage(sprites, 3, Common::Point(70, 74));
	} else if (_pCount <= 80) {
		_vm->_buffer2.plotImage(sprites, 6, Common::Point(70, 74));
	} else if (_pCount <= 120) {
		_vm->_buffer2.plotImage(sprites, 2, Common::Point(50, 76));
	} else if (_pCount <= 160) {
		_vm->_buffer2.plotImage(sprites, 14, Common::Point(63, 78));
	} else if (_pCount <= 200) {
		_vm->_buffer2.plotImage(sprites, 5, Common::Point(86, 74));
	} else if (_pCount <= 240) {
		_vm->_buffer2.plotImage(sprites, 0, Common::Point(103, 76));
	} else if (_pCount <= 280) {
		_vm->_buffer2.plotImage(sprites, 4, Common::Point(119, 77));
	} else {
		_vm->_buffer2.plotImage(sprites, 1, Common::Point(111, 77));
	}

	if (_planeCount == 11 || _planeCount == 12)
		++_position.y;
	else if (_planeCount >= 28)
		--_position.y;

	_vm->_buffer2.plotImage(sprites, 7, _position);
	_vm->_buffer2.plotImage(sprites, 8 + _propCount, Common::Point(
		_position.x + 99, _position.y + 10));
	_vm->_buffer2.plotImage(sprites, 11 + _propCount, Common::Point(
		_position.x + 104, _position.y + 18));

	if (++_planeCount >= 30)
		_planeCount = 0;
	if (++_propCount >= 3)
		_propCount = 0;

	++_xCount;
	if (_xCount == 1)
		++_position.x;
	else
		_xCount = 0;
}

void Plane::doFallCell() {
	if (_vm->_scaleI <= 20)
		return;

	SpriteFrame *frame = _vm->_objectsTable[20]->getFrame(_planeCount / 6);
	Common::Rect r(115, 11, 115 + _vm->_screen->_scaleTable1[frame->w],
		11 + _vm->_screen->_scaleTable1[frame->h]);
	_vm->_buffer2.sPlotF(frame, r);

	_vm->_scaleI -= 3;
	_vm->_scale = _vm->_scaleI;
	_vm->_screen->setScaleTable(_vm->_scale);
	++_xCount;
	if (_xCount == 5)
		return;
	_xCount = 0;
	if (_planeCount == 18)
		_planeCount = 0;
	else
		_planeCount += 6;
}

void Plane::scrollFly() {
	_vm->copyBF1BF2();
	_vm->_newRects.clear();
	doFlyCell();
	_vm->copyRects();
	_vm->copyBF2Vid();
}

void Plane::scrollFall() {
	_vm->copyBF1BF2();
	_vm->_newRects.clear();
	doFallCell();
	_vm->copyRects();
	_vm->copyBF2Vid();
}

void Plane::mWhileFly() {
	Screen &screen = *_vm->_screen;
	Player &player = *_vm->_player;
	EventsManager &events = *_vm->_events;

	events.hideCursor();
	screen.clearScreen();
	screen.setBufferScan();
	screen.fadeOut();
	_vm->_scrollX = 0;

	_vm->_room->buildScreen();
	_vm->copyBF2Vid();
	screen.fadeIn();
	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	_vm->_events->clearEvents();

	_vm->_scrollRow = _vm->_scrollCol = 0;
	_vm->_scrollX = _vm->_scrollY = 0;
	player._rawPlayer = Common::Point(0, 0);
	player._scrollAmount = 1;

	_pCount = 0;
	_planeCount = 0;
	_propCount = 0;
	_xCount = 0;
	_position = Common::Point(20, 29);

	while (!_vm->shouldQuit() && !events.isKeyMousePressed() &&
		((_vm->_scrollCol + screen._vWindowWidth) != _vm->_room->_playFieldWidth)) {
		events._vbCount = 4;
		_vm->_scrollX += player._scrollAmount;

		while (_vm->_scrollX >= TILE_WIDTH) {
			_vm->_scrollX -= TILE_WIDTH;
			++_vm->_scrollCol;

			_vm->_buffer1.moveBufferLeft();
			_vm->_room->buildColumn(_vm->_scrollCol + screen._vWindowWidth, screen._vWindowBytesWide);
		}

		scrollFly();
		++_pCount;

		while (!_vm->shouldQuit() && events._vbCount > 0) {
			_vm->_sound->playSound(0);
			events.pollEventsAndWait();
		}
	}

	events.showCursor();
}

void Plane::mWhileFall() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;

	events.hideCursor();
	screen.clearScreen();
	screen.setBufferScan();
	screen.fadeOut();
	_vm->_scrollX = 0;

	_vm->_room->buildScreen();
	_vm->copyBF2Vid();
	screen.fadeIn();
	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	_vm->_events->clearEvents();

	_vm->_scrollRow = _vm->_scrollCol = 0;
	_vm->_scrollX = _vm->_scrollY = 0;
	_vm->_player->_scrollAmount = 3;
	_vm->_scaleI = 255;

	_xCount = 0;
	_planeCount = 0;

	while (!_vm->shouldQuit() && !events.isKeyMousePressed() &&
		(_vm->_scrollCol + screen._vWindowWidth != _vm->_room->_playFieldWidth)) {
		events._vbCount = 4;
		_vm->_scrollX += _vm->_player->_scrollAmount;

		while (_vm->_scrollX >= TILE_WIDTH) {
			_vm->_scrollX -= TILE_WIDTH;
			++_vm->_scrollCol;

			_vm->_buffer1.moveBufferLeft();
			_vm->_room->buildColumn(_vm->_scrollCol + screen._vWindowWidth, screen._vWindowBytesWide);
		}

		scrollFall();

		while (!_vm->shouldQuit() && events._vbCount > 0) {
			events.pollEventsAndWait();
		}
	}

	events.showCursor();
}

/*------------------------------------------------------------------------*/

Jungle::Jungle(AmazonEngine *vm) : CampScene(vm) {
	for (int i = 0; i < JUNGLE_SIZE; ++i) {
		_jCnt[i] = _jungleX[i] = -1;
	}
}

void Jungle::jungleMove() {
	const static int jungleY[3] = { 27, 30, 29 };
	int count = 1;
	int frameOffset = 0;

	if (!_vm->_timers[0]._flag) {
		++_vm->_timers[0]._flag;
		_vm->_scrollX += _vm->_player->_scrollAmount;

		for (int i = 0; i < 3; ++i) {
			int newJCnt = (_jCnt[i] + 1) % 8;
			_jCnt[i] = newJCnt;
			_jungleX[i] += 5;
		}

		frameOffset = 4;
		count = (_vm->_allenFlag != 1) ? 2 : 3;
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

void Jungle::initJWalk2() {
	const int JUNGLE1OBJ[7][4] = {
		{ 2, 470, 0, 20 },
		{ 0, 290, 0, 50 },
		{ 1, 210, 0, 40 },
		{ 0, 500, 0, 30 },
		{ 1, 550, 0, 20 },
		{ 0, 580, 0, 60 },
		{ 1, 650, 0, 30 }
	};

	Screen &screen = *_vm->_screen;
	screen.fadeOut();
	_vm->_events->hideCursor();
	screen.clearScreen();
	_vm->_buffer2.clearBuffer();
	screen.setBufferScan();

	_vm->_scrollX = _vm->_scrollY;
	_vm->_scrollCol = _vm->_scrollRow;
	_vm->_room->buildScreen();
	_vm->copyBF2Vid();
	screen.fadeIn();
	_vm->_events->clearEvents();

	_xCount = 2;
	_vm->_player->_scrollAmount = 5;
	_xTrack = -10;
	_yTrack = _zTrack = 0;
	_xCam = 480;
	_yCam = 0;
	_zCam = 80;

	_vm->_timers[24]._timer = 1;
	_vm->_timers[24]._initTm = 1;
	++_vm->_timers[24]._flag;

	_pNumObj = 7;
	for (int i = 0; i < _pNumObj; i++) {
		_pan[i]._pObject = _vm->_objectsTable[24];
		_pan[i]._pImgNum = JUNGLE1OBJ[i][0];
		_pan[i]._pObjX = JUNGLE1OBJ[i][1];
		_pan[i]._pObjY = JUNGLE1OBJ[i][2];
		_pan[i]._pObjZ = JUNGLE1OBJ[i][3];
		_pan[i]._pObjXl = _pan[i]._pObjYl = 0;
	}

	_jCnt[0] = 0;
	_jCnt[1] = 3;
	_jCnt[2] = 5;

	_jungleX[0] = 50;
	_jungleX[1] = 16;
	_jungleX[2] = 93;
}

void Jungle::mWhileJWalk() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	Player &player = *_vm->_player;

	static const int JUNGLE_OBJ[7][4] = {
		{ 2, 77, 0, 40 },
		{ 0, 290, 0, 50 },
		{ 1, 210, 0, 70 },
		{ 0, 50, 0, 30 },
		{ 1, 70, 0, 20 },
		{ 0, -280, 0, 60 },
		{ 1, -150, 0, 30 },
	};

	screen.fadeOut();
	events.hideCursor();
	screen.clearScreen();
	_vm->_buffer2.clearBuffer();
	screen.setBufferScan();
	_vm->_scrollX = 0;

	// Build the initial jungle scene and fade it in
	_vm->_room->buildScreen();
	_vm->copyBF2Vid();
	screen.fadeIn();

	// Set up the player to walk horizontally
	player._xFlag = 1;
	player._yFlag = 0;
	player._moveTo.x = 160;
	player._playerMove = true;

	_xCount = 2;
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
		_pan[i]._pObject = _vm->_objectsTable[24];
		_pan[i]._pImgNum = JUNGLE_OBJ[i][0];
		_pan[i]._pObjX = JUNGLE_OBJ[i][1];
		_pan[i]._pObjY = JUNGLE_OBJ[i][2];
		_pan[i]._pObjZ = JUNGLE_OBJ[i][3];
		_pan[i]._pObjXl = _pan[i]._pObjYl = 0;
	}

	while (!_vm->shouldQuit() && !events.isKeyMousePressed() && (player._xFlag != 2)) {
		_vm->_images.clear();
		events._vbCount = 6;

		_pan[0]._pImgNum = _xCount;
		if (_xCount == 2)
			++_xCount;
		else
			--_xCount;

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

void Jungle::mWhileJWalk2() {
	Screen &screen = *_vm->_screen;

	initJWalk2();

	while (!_vm->shouldQuit() && !_vm->_events->isKeyMousePressed() &&
		(_vm->_scrollCol + screen._vWindowWidth) != _vm->_room->_playFieldWidth) {
		_vm->_images.clear();
		_vm->_events->_vbCount = 6;
		_pan[0]._pImgNum = _xCount;

		jungleMove();
		while (_vm->_scrollX >= TILE_WIDTH) {
			_vm->_scrollX -= TILE_WIDTH;
			++_vm->_scrollCol;
			_vm->_buffer1.moveBufferLeft();
			_vm->_room->buildColumn(_vm->_scrollCol + screen._vWindowWidth, screen._vWindowBytesWide);
		}

		if (_xCount == 2)
			++_xCount;
		else
			--_xCount;

		pan();
		scrollJWalk();

		while (!_vm->shouldQuit() && _vm->_events->_vbCount > 0) {
			_vm->_events->pollEventsAndWait();
		}
	}

	_vm->_events->showCursor();
}

void Jungle::scrollJWalk() {
	_vm->copyBF1BF2();
	_vm->_newRects.clear();
	_vm->plotList();
	_vm->copyRects();
	_vm->copyBF2Vid();
}

/*------------------------------------------------------------------------*/

Guard::Guard(AmazonEngine *vm) : PannedScene(vm) {
	_guardCel = 0;
	_gCode1 = _gCode2 = 0;
	_xMid = _yMid = 0;
}

void Guard::setVerticalCode() {
	Screen &screen = *_vm->_screen;

	_gCode1 = 0;
	_gCode2 = 0;
	if (_topLeft.x < screen._orgX1)
		_gCode1 |= 8;
	else if (_topLeft.x == screen._orgX1) {
		_gCode1 |= 8;
		_gCode1 |= 2;
	} else
		_gCode1 |= 2;

	if (_bottomRight.x < screen._orgX1)
		_gCode2 |= 8;
	else if (_bottomRight.x == screen._orgX1) {
		_gCode2 |= 8;
		_gCode2 |= 2;
	} else
		_gCode2 |= 2;

	if (_topLeft.y < screen._orgY1)
		_gCode1 |= 4;
	else if (_topLeft.y > screen._orgY2)
		_gCode1 |= 1;

	if (_bottomRight.y < screen._orgY1)
		_gCode2 |= 4;
	else if (_bottomRight.y > screen._orgY2)
		_gCode2 |= 1;
}

void Guard::setHorizontalCode() {
	Screen &screen = *_vm->_screen;

	_gCode1 = 0;
	_gCode2 = 0;

	if (_topLeft.y < screen._orgY1)
		_gCode1 |= 4;
	else if (_topLeft.x == screen._orgX1) {
		_gCode1 |= 4;
		_gCode1 |= 1;
	} else
		_gCode1 |= 1;

	if (_bottomRight.y < screen._orgY1)
		_gCode2 |= 4;
	else if (_bottomRight.x == screen._orgX1) {
		_gCode2 |= 4;
		_gCode2 |= 1;
	} else
		_gCode2 |= 1;

	if (_topLeft.x < screen._orgX1)
		_gCode1 |= 8;
	else if (_topLeft.x > screen._orgX2)
		_gCode1 |= 2;

	if (_bottomRight.x < screen._orgX1)
		_gCode2 |= 8;
	else if (_bottomRight.x > screen._orgX2)
		_gCode2 |= 2;
}

void Guard::chkVLine() {
	if (_position.x > _vm->_player->_rawPlayer.x) {
		_topLeft = _vm->_player->_rawPlayer;
		_bottomRight = _position;
	} else {
		_topLeft = _position;
		_bottomRight = _vm->_player->_rawPlayer;
	}

	if (_vm->_screen->_orgY1 > _vm->_screen->_orgY2)
		SWAP(_vm->_screen->_orgY1, _vm->_screen->_orgY2);

	for (;;) {
		setVerticalCode();
		int code = _gCode1 | _gCode2;
		if (code == 10) {
			_vm->_guardFind = 0;
			return;
		}

		int code2 = _gCode1 & _gCode2;
		code2 &= 5;
		if (((code & 10) == 8) || ((code & 10) == 2) || (code2 != 0))
			return;

		int midX = (_topLeft.x + _bottomRight.x) / 2;
		int midY = (_topLeft.y + _bottomRight.y) / 2;

		if (midX < _vm->_screen->_orgX1) {
			if ((midX == _topLeft.x) && (midY == _topLeft.y))
				return;

			_topLeft.x = midX;
			_topLeft.y = midY;
		} else {
			if ((midX == _bottomRight.x) && (midY == _bottomRight.y))
				return;

			_bottomRight.x = midX;
			_bottomRight.y = midY;
		}
	}
}

void Guard::chkHLine() {
	if (_position.y > _vm->_player->_rawPlayer.y) {
		_topLeft = _vm->_player->_rawPlayer;
		_bottomRight = _position;
	} else {
		_topLeft = _position;
		_bottomRight = _vm->_player->_rawPlayer;
	}

	if (_vm->_screen->_orgX1 > _vm->_screen->_orgX2)
		SWAP(_vm->_screen->_orgX1, _vm->_screen->_orgX2);

	while (true) {
		setHorizontalCode();
		int code = _gCode1 | _gCode2;
		if (code == 5) {
			_vm->_guardFind = 0;
			return;
		}

		int code2 = _gCode1 & _gCode2;
		code2 &= 10;
		if (((code & 5) == 4) || ((code & 5) == 1) || (code2 != 0))
			return;

		int midX = (_topLeft.x + _bottomRight.x) / 2;
		int midY = (_topLeft.y + _bottomRight.y) / 2;

		if (midY < _vm->_screen->_orgY1) {
			if ((midX == _topLeft.x) && (midY == _topLeft.y))
				return;

			_topLeft.x = midX;
			_topLeft.y = midY;
		} else {
			if ((midX == _bottomRight.x) && (midY == _bottomRight.y))
				return;

			_bottomRight.x = midX;
			_bottomRight.y = midY;
		}
	}
}

void Guard::guardSee() {
	Screen &screen = *_vm->_screen;
	int tmpY = (_vm->_scrollRow << 4) + _vm->_scrollY;
	_vm->_flags[140] = 0;
	if (tmpY > _position.y)
		return;

	tmpY += screen._vWindowLinesTall;
	tmpY -= 11;

	if (tmpY < _position.y)
		return;

	_vm->_guardFind = 1;
	_vm->_flags[140] = 1;

	for (uint16 idx = 0; idx < _vm->_room->_plotter._walls.size(); idx++) {
		screen._orgX1 = _vm->_room->_plotter._walls[idx].left;
		screen._orgY1 = _vm->_room->_plotter._walls[idx].top;
		screen._orgX2 = _vm->_room->_plotter._walls[idx].right;
		screen._orgY2 = _vm->_room->_plotter._walls[idx].bottom;
		if (screen._orgX1 == screen._orgX2) {
			chkVLine();
			if (_vm->_guardFind == 0)
				return;
		} else if (screen._orgY1 == screen._orgY2) {
			chkHLine();
			if (_vm->_guardFind == 0)
				return;
		}
	}
}

void Guard::setGuardFrame() {
	ImageEntry ie;
	ie._flags = IMGFLAG_UNSCALED;

	if (_vm->_guardLocation == 4)
		ie._flags |= IMGFLAG_BACKWARDS;
	ie._spritesPtr = _vm->_objectsTable[37];
	ie._frameNumber = _guardCel;
	ie._position = _position;
	ie._offsetY = 10;
	_vm->_images.addToList(ie);
}

void Guard::doGuard() {
	// Skip the code dealing with the guard on the boat (chapter 8)
	// if the cheat mode is activated
	if (_vm->_cheatFl)
		return;

	if (_vm->_timers[8]._flag) {
		setGuardFrame();
		return;
	}

	++_vm->_timers[8]._flag;
	++_guardCel;
	int curCel = _guardCel;

	switch (_vm->_guardLocation) {
	case 1:
		// Guard walking down
		if (curCel <= 8 || curCel > 13)
			_guardCel = curCel = 8;

		_position.y += _vm->_player->_walkOffDown[curCel - 8];
		guardSee();
		if (_position.y >= 272) {
			_position.y = 272;
			_vm->_guardLocation = 2;
		}
		break;
	case 2:
		// Guard walking left
		if (curCel <= 43 || curCel > 48)
			_guardCel = curCel = 43;

		_position.x -= _vm->_player->_walkOffLeft[curCel - 43];
		guardSee();
		if (_position.x <= 56) {
			_position.x = 56;
			_vm->_guardLocation = 3;
		}
		break;
	case 3:
		// Guard walking up
		if (curCel <= 0 || curCel > 5)
			_guardCel = curCel = 0;

		_position.y -= _vm->_player->_walkOffUp[curCel];
		guardSee();
		if (_position.y <= 89) {
			_position.y = 89;
			_vm->_guardLocation = 4;
			if (_vm->_flags[121] == 1)
				_vm->_guardLocation = 5;
		}
		break;
	default:
		// Guard walking right
		if (curCel <= 43 || curCel > 48)
			_guardCel = curCel = 43;

		_position.x += _vm->_player->_walkOffRight[curCel - 43];
		guardSee();
		if (_position.x >= 127) {
			_position.x = 127;
			_vm->_guardLocation = 1;
		}
		break;
	}

	setGuardFrame();
}

void Guard::setPosition(const Common::Point &pt) {
	_position = pt;
}

/*------------------------------------------------------------------------*/

Cast::Cast(AmazonEngine *vm) : PannedScene(vm) {
}

void Cast::doCast(int param1) {
	Screen &screen = *_vm->_screen;

	_vm->_buffer1.create(_vm->_screen->w, _vm->_screen->h);
	_vm->_buffer2.create(_vm->_screen->w, _vm->_screen->h);

	screen.setDisplayScan();
	_vm->_events->hideCursor();
	screen.forceFadeOut();
	screen._clipHeight = 173;
	screen.clearScreen();
	_vm->_chapter = 16;
	_vm->tileScreen();
	_vm->updateSummary(param1);
	screen.setPanel(3);
	_vm->_chapter = 14;

	Resource *spriteData = _vm->_files->loadFile(91, 0);
	_vm->_objectsTable[0] = new SpriteResource(_vm, spriteData);
	delete spriteData;
	spriteData = _vm->_files->loadFile(91, 1);
	_vm->_objectsTable[1] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_files->_setPaletteFlag = false;
	_vm->_files->loadScreen(58, 1);
	_vm->_buffer2.blitFrom(*_vm->_screen);
	_vm->_buffer1.blitFrom(*_vm->_screen);

	_xTrack = 0;
	_yTrack = -6;
	_zTrack = 0;
	_xCam = _yCam = 0;
	_zCam = 60;

	_vm->_timers[24]._timer = 1;
	_vm->_timers[24]._initTm = 1;
	++_vm->_timers[24]._flag;

	_pNumObj = 26;
	for (int i = 0; i < _pNumObj; i++) {
		_pan[i]._pObject = _vm->_objectsTable[0];
		_pan[i]._pImgNum = CAST_END_OBJ[i][0];
		_pan[i]._pObjX = CAST_END_OBJ[i][1];
		_pan[i]._pObjY = CAST_END_OBJ[i][2];
		_pan[i]._pObjZ = CAST_END_OBJ[i][3];
		_pan[i]._pObjXl = _pan[i]._pObjYl = 0;
	}

	_pNumObj += 4;
	for (int i = 0; i < 4; i++) {
		_pan[26 + i]._pObject = _vm->_objectsTable[1];
		_pan[26 + i]._pImgNum = CAST_END_OBJ1[i][0];
		_pan[26 + i]._pObjX = CAST_END_OBJ1[i][1];
		_pan[26 + i]._pObjY = CAST_END_OBJ1[i][2];
		_pan[26 + i]._pObjZ = CAST_END_OBJ1[i][3];
		_pan[26 + i]._pObjXl = _pan[26 + i]._pObjYl = 0;
	}

	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	_vm->_numAnimTimers = 0;

	_vm->_midi->newMusic(58, 0);
	screen.forceFadeIn();

	while (!_vm->shouldQuit()) {
		_vm->_images.clear();
		pan();
		_vm->_buffer2.blitFrom(_vm->_buffer1);
		_vm->_newRects.clear();
		_vm->plotList();
		_vm->copyBlocks();

		for (int idx = 0; idx < 5 && !_vm->shouldQuit() &&
				!_vm->_events->isKeyMousePressed(); ++idx)
			_vm->_events->pollEventsAndWait();

		if (_vm->_events->isKeyMousePressed())
			break;

		if (_yCam < -7550) {
			while (!_vm->shouldQuit() && !_vm->_midi->checkMidiDone())
				_vm->_events->pollEventsAndWait();
			break;
		}
	}

	_vm->_midi->newMusic(58, 1);
	_vm->_events->showCursor();

	_vm->freeCells();
	_vm->_oldRects.clear();
	_vm->_newRects.clear();
	_vm->_numAnimTimers = 0;
	_vm->_images.clear();
	screen.forceFadeOut();

	_vm->quitGame();
	_vm->_events->pollEvents();
}

/*------------------------------------------------------------------------*/

River::River(AmazonEngine *vm) : PannedScene(vm) {
	_chickenOutFl = false;
	_rScrollRow = 0;
	_rScrollCol = 0;
	_rScrollX = 0;
	_rScrollY = 0;
	_mapOffset = 0;
	_screenVertX = 0;
	_saveRiver = false;
	_deathFlag = false;
	_deathCount = 0;
	_oldScrollCol = 0;
	_maxHits = 0;
	_mapPtr = nullptr;
	_canoeMoveCount = 0;
	_canoeVXPos = 0;
	_canoeFrame = 0;
	_canoeDir = 0;
	_canoeLane = 0;
	_canoeYPos = 0;
	_hitCount = 0;
	_riverIndex = 0;
	_topList = _botList = nullptr;
	_deathType = 0;
	_hitSafe = 0;
}

void River::setRiverPan() {
	int delta = (_vm->_scrollCol * 16) + _vm->_scrollX;

	_xTrack = 9;
	_yTrack = _zTrack = 0;
	_xCam = 160;
	_yCam = 0;
	_zCam = 80;

	_vm->_timers[24]._timer = 1;
	_vm->_timers[24]._initTm = 1;
	++_vm->_timers[24]._flag;

	_pNumObj = 23;
	for (int i = 0; i < _pNumObj; i++) {
		_pan[i]._pObject = _vm->_objectsTable[45];
		_pan[i]._pImgNum = RIVER1OBJ[i][0];
		_pan[i]._pObjX = RIVER1OBJ[i][1] + delta;
		_pan[i]._pObjY = RIVER1OBJ[i][2];
		_pan[i]._pObjZ = RIVER1OBJ[i][3];
		_pan[i]._pObjXl = _pan[i]._pObjYl = 0;
	}
}

void River::initRiver() {
	static const int RIVERVXTBL[3] = { 6719, 7039, 8319 };
	Screen &screen = *_vm->_screen;

	_vm->_events->centerMousePos();
	_vm->_events->restrictMouse();
	screen.setDisplayScan();
	screen.clearScreen();
	screen.savePalette();
	screen.forceFadeOut();

	_vm->_files->_setPaletteFlag = false;
	_vm->_files->loadScreen(95, 4);
	_vm->_buffer2.blitFrom(*_vm->_screen);

	screen.restorePalette();
	screen.setBufferScan();
	_vm->_destIn = &_vm->_buffer2;
	_vm->_room->roomMenu();

	if (_saveRiver) {
		// Restoring a savegame, so set properties from saved fields
		_vm->_scrollRow = _rScrollRow;
		_vm->_scrollCol = _rScrollCol;
		_vm->_scrollX = _rScrollX;
		_vm->_scrollY = _rScrollY;
	} else {
		// Set initial scene state
		_vm->_scrollRow = 0;
		_vm->_scrollCol = 140;
		_vm->_scrollX = 0;
		_vm->_scrollY = 0;
	}

	_vm->_room->buildScreen();
	_vm->copyBF2Vid();
	screen.forceFadeIn();

	if (!_saveRiver) {
		// Reset draw rects
		_vm->_oldRects.clear();
		_vm->_newRects.clear();
		_vm->_events->clearEvents();

	}

	_vm->_player->_scrollAmount = 2;
	setRiverPan();
	_vm->_timers[3]._timer = 1;
	_vm->_timers[3]._initTm = 1;
	++_vm->_timers[3]._flag;

	_canoeFrame = 0;
	_mapPtr = (const byte *)MAPTBL[_vm->_riverFlag] + 1;
	if (_saveRiver) {
		_mapPtr--;
		_mapPtr += _mapOffset;
	} else {
		_screenVertX = RIVERVXTBL[_vm->_riverFlag] - 320;
		_canoeLane = 3;
		_hitCount = 0;
		_hitSafe = 0;
		_canoeYPos = 71;
	}

	_riverIndex = _vm->_riverFlag;
	_topList = RIVER_OBJECTS[_riverIndex][RIVER_START];
	updateObstacles();
	riverSetPhysX();
	_canoeDir = 0;
	_deathFlag = false;
	_deathCount = 0;

	_vm->_timers[11]._timer = 1200;
	_vm->_timers[11]._initTm = 1200;
	++_vm->_timers[11]._flag;
	_vm->_timers[12]._timer = 1500;
	_vm->_timers[12]._initTm = 1500;
	++_vm->_timers[12]._flag;

	_maxHits = 2 - _vm->_riverFlag;
	_saveRiver = false;

	// Set font colors for drawing using font2
	Font::_fontColors[0] = 0;
	Font::_fontColors[1] = 33;
	Font::_fontColors[2] = 34;
	Font::_fontColors[3] = 35;
}

void River::resetPositions() {
	riverSetPhysX();
	int val = (_vm->_scrollCol + 1 - _oldScrollCol) * 16;
	if (val < 0) {
		val |= 0x80;
	}

	for (int i = 0; i < _pNumObj; i++)
		_pan[i]._pObjX += val;
}

void River::checkRiverPan() {
	int val = _vm->_scrollCol * 16 + 320;

	for (int i = 0; i < _pNumObj; i++) {
		if (_pan[i]._pObjX < val)
			return;
	}

	setRiverPan();
}

bool River::riverJumpTest() {
	if (_vm->_scrollCol == 120 || _vm->_scrollCol == 60 || _vm->_scrollCol == 0) {
		int val = *++_mapPtr;
		if (val == 0xFF)
			return true;

		_oldScrollCol = _vm->_scrollCol;

		if (val == 0) {
			_vm->_scrollCol = 139;
			_vm->_scrollX = 14;
			_vm->_room->buildScreen();
			resetPositions();
			return false;
		}
	} else if (_vm->_scrollCol == 105) {
		int val1 = _mapPtr[1];
		int val2 = _mapPtr[2];
		_mapPtr += 3;
		if (_canoeLane < 3) {
			if (val1 != 0) {
				_deathFlag = true;
				_deathCount = 300;
				_deathType = val2;
			}
		} else {
			if (val1 != 1) {
				_deathFlag = true;
				_deathCount = 300;
				_deathType = val2;
			}
			_oldScrollCol = _vm->_scrollCol;
			_vm->_scrollCol = 44;
			_vm->_scrollX = 14;
			_vm->_room->buildScreen();
			resetPositions();
			return false;
		}
	}

	_vm->_scrollX = 14;
	--_vm->_scrollCol;
	_vm->_buffer1.moveBufferRight();
	_vm->_room->buildColumn(_vm->_scrollCol, 0);
	checkRiverPan();
	return false;
}

void River::riverSound() {
	if (_vm->_timers[11]._flag == 0) {
		++_vm->_timers[11]._flag;
		_vm->_sound->playSound(2);
	}

	if (_vm->_timers[12]._flag == 0) {
		++_vm->_timers[12]._flag;
		_vm->_sound->playSound(3);
	}

	if ((_xCam >= 1300) && (_xCam <= 1320))
		_vm->_sound->playSound(1);
}

void River::moveCanoe() {
	EventsManager &events = *_vm->_events;
	Common::Point pt = events.calcRawMouse();
	Common::Point mousePos = events.getMousePos();

	// Do an event polling
	_vm->_canSaveLoad = true;
	events.pollEvents();
	_vm->_canSaveLoad = false;
	if (_vm->_room->_function == FN_CLEAR1)
		return;

	if (_canoeDir) {
		// Canoe movement in progress
		moveCanoe2();
	} else {
		if (events._leftButton && pt.y >= 140) {
			if (pt.x < _vm->_room->_rMouse[8][0]) {
				// Disk icon wasn't clicked
				_vm->_scripts->printString(AMRES.BAR_MESSAGE);
			} else {
				// Clicked on the Disc icon. Show the ScummVM menu
				_vm->_room->handleCommand(9);

				if (_vm->_room->_function != FN_CLEAR1) {
					_vm->_room->buildScreen();
					_vm->copyBF2Vid();
				}
			}
		} else if (events._leftButton && mousePos.x < 35 && mousePos.y < 12) {
			// Clicked on the Skip button. So chicken out
			_chickenOutFl = true;
		}  else if ((events._leftButton && pt.y <= _canoeYPos) ||
			(!events._leftButton && _vm->_player->_move == UP)) {
			// Move canoe up
			if (_canoeLane > 0) {
				_canoeDir = -1;
				_canoeMoveCount = 0;

				moveCanoe2();
			}
		} else if (events._leftButton || _vm->_player->_move == DOWN) {
			// Move canoe down
			if (_canoeLane < 7) {
				_canoeDir = 1;
				_canoeMoveCount = 0;

				moveCanoe2();
			}
		}
	}
}

void River::moveCanoe2() {
	_canoeYPos += _canoeDir;

	if (++_canoeMoveCount == 5) {
		_canoeLane += _canoeDir;
		_canoeDir = 0;
	}
}

void River::updateObstacles() {
	RiverStruct *cur;
	for (cur = _topList; cur < RIVER_OBJECTS[_riverIndex][RIVER_END]; ++cur) {
		int val = cur->_riverX + cur->_width - 1;
		if (val < _screenVertX)
			// Obstacle is not yet on-screen
			break;

		if (cur->_riverX < (_screenVertX + 319)) {
			// Object is now on-screen. So set _topList/_botList to the range
			// of river obstacles that are currently visible
			_topList = cur;
			_botList = cur;

			while (cur < RIVER_OBJECTS[_riverIndex][RIVER_END]) {
				++cur;
				val = cur->_riverX + cur->_width - 1;
				if (val < _screenVertX || (cur->_riverX >= (_screenVertX + 319)))
					break;

				_botList = cur;
			}

			return;
		}
	}

	cur = _topList;
	cur--;
	_botList = cur;
}

void River::riverSetPhysX() {
	int xAmt = (_vm->_scrollCol * 16) + _vm->_scrollX;

	for (RiverStruct *cur = _topList; cur <= _botList; ++cur) {
		cur->_xp = xAmt - (_screenVertX - cur->_riverX);
	}
}

bool River::checkRiverCollide() {
	if (_hitSafe)
		return false;

	_canoeVXPos = _screenVertX + 170;

	for (RiverStruct *cur = _topList; cur <= _botList; ++cur) {
		if (cur->_lane < _canoeLane)
			continue;

		if ((cur->_lane == _canoeLane) || (cur->_lane == _canoeLane + 1)) {
			if ((cur->_riverX + cur->_width - 1) >= _canoeVXPos &&
					cur->_riverX < (_canoeVXPos + 124)) {
				_vm->_sound->playSound(4);
				return true;
			}
		}
	}

	return false;
}

void River::plotRiver() {
	// Handle cycling through the canoe rowing frames
	if (_vm->_timers[3]._flag == 0) {
		++_vm->_timers[3]._flag;

		if (_canoeFrame++ == 12)
			_canoeFrame = 0;
	}

	// Draw the canoe
	ImageEntry ie;
	ie._flags = IMGFLAG_UNSCALED;
	ie._spritesPtr = _vm->_objectsTable[45];
	ie._frameNumber = _canoeFrame;
	ie._position.x = (_vm->_scrollCol * 16) + _vm->_scrollX + 160;
	ie._position.y = _canoeYPos - 41;
	ie._offsetY = 41;
	_vm->_images.addToList(ie);

	// Draw any on-screen obstacles
	for (RiverStruct *cur = _topList; cur <= _botList; ++cur) {
		if (cur->_id != -1) {
			ie._flags = IMGFLAG_UNSCALED;
			ie._spritesPtr = _vm->_objectsTable[45];
			ie._frameNumber = cur->_id;
			ie._position.x = cur->_xp;
			ie._position.y = (cur->_lane * 5) + 56 - cur->_offsetY;
			ie._offsetY = cur->_offsetY;
			_vm->_images.addToList(ie);
		}
	}

	// Draw the text for skipping the river
	Font &font2 = *_vm->_fonts._font2;
	font2.drawString(_vm->_screen, "SKIP", Common::Point(5, 5));
}

void River::mWhileDownRiver() {
	Screen &screen = *_vm->_screen;
	_vm->_events->hideCursor();

	screen.setDisplayScan();
	screen.clearScreen();
	screen.savePalette();
	if (!_vm->isDemo())
		_vm->_files->loadScreen(95, 4);
	_vm->_buffer2.blitFrom(*_vm->_screen);
	screen.restorePalette();
	screen.setPalette();
	screen.setBufferScan();

	_vm->_scrollX = 0;
	_vm->_room->buildScreen();
	_vm->copyBF2Vid();

	_vm->_player->_scrollAmount = 2;
	_vm->_destIn = &_vm->_buffer2;
	_xTrack = -7;
	_yTrack = _zTrack = 0;
	_xCam = _yCam = 0;
	_zCam = 80;

	_vm->_timers[24]._timer = 1;
	_vm->_timers[24]._initTm = 1;
	++_vm->_timers[24]._flag;

	_pNumObj = 14;
	for (int i = 0; i <_pNumObj; i++) {
		_pan[i]._pObject = _vm->_objectsTable[33];
		_pan[i]._pImgNum = DOWNRIVEROBJ[i][0];
		_pan[i]._pObjX = DOWNRIVEROBJ[i][1];
		_pan[i]._pObjY = DOWNRIVEROBJ[i][2];
		_pan[i]._pObjZ = DOWNRIVEROBJ[i][3];
		_pan[i]._pObjXl = _pan[i]._pObjYl = 0;
	}

	_vm->_timers[3]._timer = 200;
	_vm->_timers[3]._initTm = 200;
	++_vm->_timers[3]._flag;
	_vm->_timers[4]._timer = 350;
	_vm->_timers[4]._initTm = 350;
	++_vm->_timers[4]._flag;

	while (!_vm->shouldQuit() && !_vm->_events->isKeyMousePressed() &&
		(_vm->_scrollCol + screen._vWindowWidth != _vm->_room->_playFieldWidth)) {
		_vm->_images.clear();
		_vm->_events->_vbCount = 6;

		_vm->_scrollX += _vm->_player->_scrollAmount;
		while (_vm->_scrollX >= TILE_WIDTH) {
			_vm->_scrollX -= TILE_WIDTH;
			++_vm->_scrollCol;
			_vm->_buffer1.moveBufferLeft();
			_vm->_room->buildColumn(_vm->_scrollCol + screen._vWindowWidth, screen._vWindowBytesWide);
		}

		pan();
		scrollRiver();

		if (!_vm->_timers[3]._flag) {
			++_vm->_timers[3]._flag;
			_vm->_sound->playSound(1);
		} else if (!_vm->_timers[4]._flag) {
			++_vm->_timers[4]._flag;
			_vm->_sound->playSound(0);
		}

		while (!_vm->shouldQuit() && _vm->_events->_vbCount > 0) {
			_vm->_events->pollEventsAndWait();
		}
	}

	_vm->_events->showCursor();
}

void River::scrollRiver() {
	_vm->copyBF1BF2();
	_vm->_newRects.clear();
	_vm->_buffer2.plotImage(_vm->_objectsTable[33], 0, Common::Point(66, 30));
	_vm->plotList();
	_vm->copyRects();
	_vm->copyBF2Vid();
}

void River::scrollRiver1() {
	_vm->copyBF1BF2();
	_vm->_newRects.clear();
	plotRiver();
	_vm->plotList();
	_vm->copyRects();
	_vm->copyBF2Vid();
}

void River::doRiver() {
	static const int RIVERDEATH[5] = { 22, 23, 24, 25, 26 };

	initRiver();
	_vm->_events->showCursor();

	while (!_vm->shouldQuit()) {
		_vm->_events->_vbCount = 4;

		// Move the river position
		_screenVertX -= _vm->_player->_scrollAmount;

		if (_vm->_scrollX == 0) {
			_vm->_midi->midiRepeat();
			if (riverJumpTest()) {
				_chickenOutFl = false;
				return;
			}
		} else {
			_vm->_scrollX -= _vm->_player->_scrollAmount;
		}

		if (_chickenOutFl) {
			_chickenOutFl = false;
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
			if (_hitSafe != 0)
				_hitSafe -= 2;

			if (checkCollide) {
				_vm->dead(RIVERDEATH[0]);
				return;
			}

			if (_deathFlag) {
				if (--_deathCount == 0) {
					_vm->dead(RIVERDEATH[_deathType]);
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
			_vm->_scripts->_endFlag = true;
			_vm->_scripts->_returnCode = 0;
			_chickenOutFl = false;
			break;
		}
	}
}

void River::synchronize(Common::Serializer &s) {
	if (_vm->_player->_roomNumber == 45) {
		if (s.isSaving()) {
			// Set river properties to be saved out
			_rScrollRow = _vm->_scrollRow;
			_rScrollCol = _vm->_scrollCol;
			_rScrollX = _vm->_scrollX;
			_rScrollY = _vm->_scrollY;
			_mapOffset = _mapPtr - MAPTBL[_vm->_riverFlag];
		}

		s.syncAsSint16LE(_canoeLane);
		s.syncAsSint16LE(_canoeYPos);
		s.syncAsSint16LE(_hitCount);
		s.syncAsSint16LE(_riverIndex);
		s.syncAsSint16LE(_hitSafe);
		s.syncAsUint16LE(_rScrollRow);
		s.syncAsUint16LE(_rScrollCol);
		s.syncAsSint16LE(_rScrollX);
		s.syncAsSint16LE(_rScrollY);
		s.syncAsUint16LE(_mapOffset);
		s.syncAsUint16LE(_screenVertX);

		_saveRiver = s.isLoading();
	}
}

/*------------------------------------------------------------------------*/

Ant::Ant(AmazonEngine *vm) : AmazonManager(vm) {
	_antDirection = ANT_RIGHT;
	_pitDirection = ANT_RIGHT;
	_antCel = 0;
	_torchCel = 0;
	_pitCel = 0;
	_stabCel = 0;
	_antPos = Common::Point(0, 0);
	_antDieFl = _antEatFl = false;
	_stabFl = false;
	_pitPos = Common::Point(0, 0);
}

void Ant::plotTorchSpear(int indx, const int *&buf) {
	int idx = indx;

	ImageEntry ie;
	ie._flags = IMGFLAG_UNSCALED;
	ie._spritesPtr = _vm->_objectsTable[62];
	ie._frameNumber = buf[(idx / 2)];
	ie._position = Common::Point(_pitPos.x + buf[(idx / 2) + 1], _pitPos.y + buf[(idx / 2) + 2]);
	ie._offsetY = 255;
	_vm->_images.addToList(ie);
}

void Ant::plotPit(int indx, const int *&buf) {
	int idx = indx;
	ImageEntry ie;
	ie._flags = IMGFLAG_UNSCALED;
	ie._spritesPtr = _vm->_objectsTable[62];
	ie._frameNumber = buf[(idx / 2)];
	ie._position = Common::Point(_pitPos.x, _pitPos.y);
	ie._offsetY = _pitPos.y;
	_vm->_images.addToList(ie);

	_vm->_player->_rawPlayer = _pitPos;
	if (_vm->_inventory->_inv[INV_TORCH]._value == ITEM_IN_INVENTORY) {
		// Player has torch
		idx = _torchCel;
		buf = Amazon::TORCH;
		_vm->_timers[14]._flag = 1;
		idx += 6;
		if (buf[idx / 2] == -1)
			idx = 0;
		_torchCel = idx;
		plotTorchSpear(idx, buf);
	} else if (!_stabFl && (_vm->_inventory->_inv[INV_KNIFE_SPEAR]._value == ITEM_IN_INVENTORY)) {
		// Player has spear
		idx = 0;
		buf = Amazon::SPEAR;
		plotTorchSpear(idx, buf);
	}
}

int Ant::antHandleRight(int indx, const int *&buf) {
	int retval = indx;
	if (_pitDirection == ANT_RIGHT) {
		_pitDirection = ANT_LEFT;
		_pitPos.y = 127;
	}
	retval = _pitCel;
	buf = Amazon::PITWALK;
	if (_pitPos.x < 230) {
		if (retval == 0) {
			retval = 48;
			_pitPos.y = 127;
		}
		retval -= 6;
		_pitPos.x -= buf[(retval / 2) + 1];
		_pitPos.y -= buf[(retval / 2) + 2];
		_pitCel = retval;
	}
	return retval;
}

int Ant::antHandleLeft(int indx, const int *&buf) {
	int retval = indx;
	if (_pitDirection == ANT_LEFT) {
		_pitDirection = ANT_RIGHT;
		_pitPos.y = 127;
	}
	retval = _pitCel;
	buf = Amazon::PITWALK;
	retval += 6;
	if (buf[retval / 2] == -1) {
		retval = 0;
		_pitPos.y = 127;
	}
	_pitPos.x += buf[(retval / 2) + 1];
	_pitPos.y += buf[(retval / 2) + 2];
	_pitCel = retval;

	return retval;
}

int Ant::antHandleStab(int indx, const int *&buf) {
	int retval = indx;
	if (_vm->_inventory->_inv[INV_KNIFE_SPEAR]._value == ITEM_IN_INVENTORY) {
		if (_stabFl) {
			buf = Amazon::PITSTAB;
			retval = _stabCel;
			if (_vm->_timers[13]._flag == 0) {
				_vm->_timers[13]._flag = 1;
				retval += 6;
				_stabCel = retval;

				if (buf[retval] == -1) {
					_stabFl = false;
					_pitCel = 0;
					_pitPos.y = 127;
					retval = 0;
					buf = Amazon::PITWALK;
				} else {
					_pitPos.x += buf[(retval / 2) + 1];
					_pitPos.y += buf[(retval / 2) + 2];
					_pitCel = retval;
				}
			}
		} else {
			_stabFl = true;
			_pitCel = 0;
			retval = 0;
			_stabCel = 0;
			int dist = _pitPos.x - _antPos.x;
			if (_antEatFl && !_antDieFl && (dist <= 80)) {
				_antDieFl = true;
				_antCel = 0;
				_antPos.y = 123;
				_vm->_sound->playSound(1);
			}
		}
	}

	return retval;
}

void Ant::doAnt() {
	_antDirection = ANT_RIGHT;
	if (_vm->_aniFlag != 1) {
		_vm->_aniFlag = 1;
		_antCel = 0;
		_torchCel = 0;
		_pitCel = 0;

		_vm->_timers[15]._timer = 16;
		_vm->_timers[15]._initTm = 16;
		_vm->_timers[15]._flag = 1;

		_vm->_timers[13]._timer = 5;
		_vm->_timers[13]._initTm = 5;
		_vm->_timers[13]._flag = 1;

		_vm->_timers[14]._timer = 10;
		_vm->_timers[14]._initTm = 10;
		_vm->_timers[14]._flag = 1;

		_antPos = Common::Point(-40, 123);
		_antDieFl = _antEatFl = false;
		_stabFl = false;
		_pitPos = Common::Point(_vm->_player->_rawPlayer.x, 127);
	}

	const int *buf = nullptr;
	if (_antDieFl) {
		buf = Amazon::ANTDIE;
	} else if (_antEatFl) {
		buf = Amazon::ANTEAT;
	} else if (_antPos.x > 120 && _vm->_flags[198] == 1) {
		_antEatFl = true;
		_vm->_flags[235] = 1;
		_antCel = 0;
		buf = Amazon::ANTEAT;
	} else {
		buf = Amazon::ANTWALK;
		if (_vm->_inventory->_inv[INV_TORCH]._value == ITEM_IN_INVENTORY)
			// Player has burning torch, which scares the Ant
			_antDirection = ANT_LEFT;
	}

	int idx = _antCel;
	if (_vm->_timers[15]._flag == 0) {
		_vm->_timers[15]._flag = 1;
		if (_antDirection == ANT_LEFT) {
			if (_antPos.x > 10) {
				if (idx == 0)
					idx = 36;
				else
					idx -= 6;

				_antPos -= Common::Point(buf[(idx / 2) + 1], buf[(idx / 2) + 2]);
				_antCel = idx;
			}
		} else {
			idx += 6;
			if (buf[(idx / 2)] != -1) {
				_antPos += Common::Point(buf[(idx / 2) + 1], buf[(idx / 2) + 2]);
				_antCel = idx;
			} else if (!_antDieFl) {
				idx = 0;
				_antPos += Common::Point(buf[(idx / 2) + 1], buf[(idx / 2) + 2]);
				_antCel = idx;
			} else {
				idx -= 6;
				if (_vm->_flags[200] == 0)
					_vm->_flags[200] = 1;
			}
		}
	}

	ImageEntry ie;
	ie._flags = IMGFLAG_UNSCALED;
	ie._spritesPtr = _vm->_objectsTable[61];
	ie._frameNumber = buf[(idx / 2)];
	ie._position = Common::Point(_antPos.x, _antPos.y);
	ie._offsetY = _antPos.y - 70;
	_vm->_images.addToList(ie);
	_antCel = idx;

	if (_vm->_flags[196] != 1) {
		idx = _pitCel;
		if (_stabFl) {
			idx = antHandleStab(idx, buf);
		} else {
			buf = Amazon::PITWALK;
			if (_vm->_timers[13]._flag == 0) {
				_vm->_timers[13]._flag = 1;
				_vm->_events->pollEvents();
				if (_vm->_events->_leftButton) {
					// Handle moving the player whilst the mouse button is held down
					Common::Point pt = _vm->_events->calcRawMouse();
					if (pt.x < _pitPos.x)
						idx = antHandleLeft(idx, buf);
					else if (pt.x > _pitPos.x)
						idx = antHandleRight(idx, buf);
				} else {
					// Handle movement based on keyboard keys
					buf = Amazon::PITWALK;
					if (_vm->_player->_move == UP)
						idx = antHandleStab(idx, buf);
					else if (_vm->_player->_move == LEFT)
						idx = antHandleLeft(idx, buf);
					else if (_vm->_player->_move == RIGHT)
						idx = antHandleRight(idx, buf);
				}
			}
		}
		plotPit(idx, buf);
	}

	if (!_antDieFl) {
		int dist = _pitPos.x - _antPos.x;
		if ((_antEatFl && (dist <= 45)) || (!_antEatFl && (dist <= 80))) {
			_vm->_flags[199] = 1;
			_vm->_aniFlag = 0;
		}
	}
}

void Ant::synchronize(Common::Serializer &s) {
	if (_vm->_player->_roomNumber == 61) {
		s.syncAsByte(_antDirection);
		s.syncAsByte(_pitDirection);
		s.syncAsSint16LE(_antCel);
		s.syncAsSint16LE(_torchCel);
		s.syncAsSint16LE(_pitCel);
		s.syncAsSint16LE(_stabCel);
		s.syncAsSint16LE(_antPos.x);
		s.syncAsSint16LE(_antPos.y);
		s.syncAsSint16LE(_pitPos.x);
		s.syncAsSint16LE(_pitPos.y);
		s.syncAsByte(_antDieFl);
		s.syncAsByte(_antEatFl);
		s.syncAsByte(_stabFl);
	}
}


} // End of namespace Amazon

} // End of namespace Access
