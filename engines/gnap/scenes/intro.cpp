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

#include "video/avi_decoder.h"

#include "gnap/gnap.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"
#include "gnap/scenes/intro.h"

namespace Gnap {

SceneIntro::SceneIntro(GnapEngine *vm) : Scene(vm) {
}

int SceneIntro::init() {
	return 0x37C;
}

void SceneIntro::run() {
	const int animIdArr[] = {
		0x356, 0x357, 0x358, 0x35A, 0x35F,
		0x360, 0x361, 0x362, 0x363, 0x364,
		0x365, 0x368, 0x369, 0x36B, 0x378,
		0x36C, 0x36D, 0x36E, 0x36F, 0x370,
		0x371, 0x372, 0x373, 0x374, 0x375,
		0x376, 0x377, 0x378, 0x379, 0x37A,
		0x37B, 0};

	const int backgroundIdArr[] = {
		0x354, 0x355, 0, 1, 3,
		4, 5, 6, 7, 8,
		7, 9, 0xA, 0xB, 0xC,
		0xD, 0xE, 0xF, 0x10, 0x11,
		0x12, 0x13, 0x17, 0x14, 0x19,
		0x1A, 0x14, 0x15, 0x16, 0x14,
		0x19, 0};

	GameSys& gameSys = *_vm->_gameSys;
	int index = 0;
	bool skip = false;

	_vm->hideCursor();
	_vm->_dat->open(1, "musop_n.dat");

	Video::VideoDecoder *videoDecoder = new Video::AVIDecoder();
	if (!videoDecoder->loadFile("hoffman.avi")) {
		delete videoDecoder;
		warning("Unable to open video 'hoffman.avi' - Skipping intro");
		return;
	}
	videoDecoder->start();

	int vidPosX = (800 - videoDecoder->getWidth()) / 2;
	int vidPosY = (600 - videoDecoder->getHeight()) / 2;
	bool skipVideo = false;

	_vm->screenEffect(1, 255, 255, 255);

	while (!_vm->shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();
			if (frame) {
				if (frame->format.bytesPerPixel == 1) {
					_vm->_system->copyRectToScreen(frame->getPixels(), frame->pitch, vidPosX, vidPosY, frame->w, frame->h);
				} else if (frame->format.bytesPerPixel != 4) {
					Graphics::Surface *frame1 = frame->convertTo(_vm->_system->getScreenFormat());
					_vm->_system->copyRectToScreen(frame1->getPixels(), frame1->pitch, vidPosX, vidPosY, frame1->w, frame1->h);
					frame1->free();
					delete frame1;
				} else {
					Graphics::Surface *frame1 = frame->convertTo(_vm->_system->getScreenFormat());

					// The intro AVI is played upside down, it's the only video played in the English version
					for (uint16 y = 0; y < frame1->h / 2; y++) {
						uint32 *ptrFrom = (uint32 *)frame1->getBasePtr(0, y);
						uint32 *ptrTo = (uint32 *)frame1->getBasePtr(0, frame1->h - y - 1);
						// in this else branch, bytesPerPixel equals 4
						for (uint16 x = 0; x < frame1->pitch / 4; x++) {
							uint32 t = *ptrFrom;
							*ptrFrom = *ptrTo;
							*ptrTo = t;
							ptrFrom++;
							ptrTo++;
						}
					}

					_vm->_system->copyRectToScreen(frame1->getPixels(), frame1->pitch, vidPosX, vidPosY, frame1->w, frame1->h);
					frame1->free();
					delete frame1;
				}
				_vm->_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) ||
				event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		_vm->_system->delayMillis(10);
	}

	delete videoDecoder;

	gameSys.drawSpriteToBackground(0, 0, backgroundIdArr[index]);
	gameSys.insertSequence(0x356, 2, 0, 0, kSeqNone, 0, 0, 0);
	gameSys.setAnimation(0x356, 2, 0);

	while (!_vm->_sceneDone) {
		_vm->gameUpdateTick();

		if (gameSys.getAnimationStatus(0) == 2 || skip ) {
			skip = false;
			gameSys.requestClear2(false);
			gameSys.requestClear1();
			if ( index == 11 || index == 1 )
				_vm->screenEffect(0, 0, 0, 0);

			gameSys.setAnimation(0, 0, 0);
			if (++index >= 31)
				_vm->_sceneDone = true;
			else {
				gameSys.insertSequence(animIdArr[index], 2, 0, 0, kSeqNone, 0, 0, 0);
				if (index == 2) {
					_vm->playSound(0x10000, false);
					gameSys.insertSequence(0x359, 2, 0, 0, 0, 0, 0, 0);
				} else if (index == 3)
					gameSys.insertSequence(0x35B, 2, 0, 0, kSeqNone, 0, 0, 0);
				else if (index == 12)
					gameSys.insertSequence(0x36A, 2, 0, 0, kSeqNone, 0, 0, 0);

				gameSys.drawSpriteToBackground(0, 0, backgroundIdArr[index]);
				gameSys.setAnimation(animIdArr[index], 2, 0);

				if (index == 11)
					_vm->stopSound(0x10000);
			}
		}

		if (_vm->isKeyStatus1(Common::KEYCODE_ESCAPE) || _vm->isKeyStatus1(Common::KEYCODE_SPACE) || _vm->isKeyStatus1(Common::KEYCODE_RETURN)) {
			_vm->clearKeyStatus1(Common::KEYCODE_ESCAPE);
			_vm->clearKeyStatus1(Common::KEYCODE_SPACE);
			_vm->clearKeyStatus1(Common::KEYCODE_RETURN);
			if (index == 0) {
				skip = true;
				_vm->stopSound(0x3CF);
			} else if (index == 1)
				skip = true;
			else
				_vm->_sceneDone = true;
		}
	}

	_vm->stopSound(0x10000);

	_vm->_newSceneNum = 1;
	_vm->_newCursorValue = 1;

	_vm->_dat->open(1, "stock_n.dat");
}

} // End of namespace Gnap
