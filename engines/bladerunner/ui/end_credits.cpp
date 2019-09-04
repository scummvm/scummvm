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

#include "common/system.h"
#include "common/rect.h"

#include "audio/mixer.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/font.h"
#include "bladerunner/game_info.h"
#include "bladerunner/mouse.h"
#include "bladerunner/music.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/ui/end_credits.h"

namespace BladeRunner {

EndCredits::EndCredits(BladeRunnerEngine *vm) {
	_vm = vm;
}

EndCredits::~EndCredits() {
}

void EndCredits::show() {
	_vm->_mouse->disable();
	_vm->_mixer->stopAll();
	_vm->_ambientSounds->removeAllNonLoopingSounds(true);
	_vm->_ambientSounds->removeAllLoopingSounds(4);
	_vm->_audioSpeech->stopSpeech();

	_vm->_music->play(_vm->_gameInfo->getMusicTrack(kMusicCredits), 100, 0, 2, -1, 0, 3);

	Font *fontBig = Font::load(_vm, "TAHOMA24.FON", 1, true);
	Font *fontSmall = Font::load(_vm, "TAHOMA18.FON", 1, true);

	TextResource *textResource = new TextResource(_vm);
	textResource->open("ENDCRED");

	int textCount = textResource->getCount();
	int *textPositions = (int *)malloc(textCount * sizeof(int));
	int y = 452;
	bool small = false;

	for (int i = 0; i < textCount; i++) {
		Common::String s = textResource->getText(i);
		if (s.hasPrefix("^")) {
			if (!small) {
				y += 28;
			}
			small = false;
		} else {
			if (small) {
				y += 24;
			} else {
				y += 28;
			}
			small = true;
		}
		if (s.hasPrefix("^")) {
			textPositions[i] = y;
		} else {
			textPositions[i] = y + 2;
		}
	}

	_vm->_vqaIsPlaying = true;
	_vm->_vqaStopIsRequested = false;

	double position = 0.0;
	uint32 timeLast = _vm->_time->currentSystem();

	while (!_vm->_vqaStopIsRequested && !_vm->shouldQuit()) {
		if (position >= textPositions[textCount - 1]) {
			break;
		}

		//soundSystem::tick(SoundSystem);
		_vm->handleEvents();

		if (!_vm->_windowIsActive) {
			timeLast = _vm->_time->currentSystem();
			continue;
		}

		uint32 timeNow = _vm->_time->currentSystem();
		position += (double)(timeNow - timeLast) * 0.05f; // unsigned difference is intentional
		timeLast = timeNow;

		_vm->_surfaceFront.fillRect(Common::Rect(640, 480), 0);

		for (int i = 0; i < textCount; i++) {
			Common::String s = textResource->getText(i);
			Font *font;
			int height;

			if (s.hasPrefix("^")) {
				font = fontBig;
				height = 28;
				s.deleteChar(0);
			} else {
				font = fontSmall;
				height = 24;
			}

			y = textPositions[i] - (int)position;

			if (y < 452 && y + height > 28) {
				int x;

				if (font == fontBig) {
					x = 280;
				} else {
					x = 270 - font->getStringWidth(s);
				}

				font->drawString(&_vm->_surfaceFront, s, x, y, _vm->_surfaceFront.w, 0);
			}
		}

		_vm->_surfaceFront.fillRect(Common::Rect(0, 0, 640, 28), 0);
		_vm->_surfaceFront.fillRect(Common::Rect(0, 452, 640, 480), 0);

		_vm->blitToScreen(_vm->_surfaceFront);
	}

	_vm->_vqaIsPlaying = false;
	_vm->_vqaStopIsRequested = false;

	free(textPositions);
	delete textResource;

	delete fontSmall;
	delete fontBig;

	_vm->_music->stop(0);
	_vm->_mouse->enable();
}

} // End of namespace BladeRunner
