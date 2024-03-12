/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "plumbers/plumbers.h"
#include "plumbers/console.h"

#include "audio/decoders/aiff.h"
#include "audio/decoders/wave.h"
#include "audio/audiostream.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/system.h"
#include "common/timer.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"
#include "graphics/scaler/downscaler.h"

#include "image/bmp.h"

namespace Plumbers {
PlumbersGameWindows::PlumbersGameWindows(OSystem *syst, const ADGameDescription *gameDesc) : PlumbersGame(syst, gameDesc), _halfSize(false) {
}

void PlumbersGameWindows::loadImage(const Common::String &name) {
	PlumbersGame::loadImage(name);

	if (_halfSize) {
		_compositeSurface = new Graphics::Surface();
		const Graphics::Surface *inSurf = _image->getSurface();
		_compositeSurface->create(_screenW, _screenH, inSurf->format);
		Graphics::downscaleSurfaceByHalf(_compositeSurface, inSurf, _image->getPalette());
	}
}

void PlumbersGameWindows::startGraphics() {
	_image = new Image::BitmapDecoder();

	Graphics::ModeWithFormatList modes = {
		// First try for a 640x480 mode
		Graphics::ModeWithFormat(640, 480),
		// System doesn't support it, so fall back on 320x240 mode
		Graphics::ModeWithFormat(320, 240),
	};

	int modeIdx = initGraphicsAny(modes);

	if (modeIdx == 0) {
		_screenW = 640;
		_screenH = 480;
	} else {
		_halfSize = true;
		_screenW = 320;
		_screenH = 240;
	}
}

void PlumbersGameWindows::readTables() {
	Common::File file;
	if (!file.open("game.bin"))
		error("sReadTables(): Error reading BIN file");

	initTables();

	_totScore = file.readSint32LE();
	file.skip(10);
	_totScene = file.readSint16LE();
	file.skip(6);

	char buf[kMaxName];
	for (int i = 0; i < kMaxScene; i++) {
		_scenes[i]._bitmapNum = file.readSint16LE();
		_scenes[i]._startBitmap = file.readSint16LE();
		_scenes[i]._decisionChoices = file.readSint16LE();
		file.read(buf, kMaxName);
		_scenes[i]._sceneName = Common::String(buf);
		file.read(buf, kMaxName);
		_scenes[i]._waveFilename = Common::String(buf);
		file.read(buf, kMaxName);
		_scenes[i]._decisionBitmap = Common::String(buf);
		_scenes[i]._style = Scene::STYLE_PC;

		for (int j = 0; j < kMaxChoice; j++) {
			_scenes[i]._choices[j]._points = file.readSint32LE();
			_scenes[i]._choices[j]._sceneName = Common::String::format("SC%02d", file.readSint16LE());
			_scenes[i]._choices[j]._skipScene = file.readSint16LE();
			int left = file.readSint16LE();
			int top = file.readSint16LE();
			int right = file.readSint16LE();
			int bottom = file.readSint16LE();
			if (_halfSize)
				_scenes[i]._choices[j]._region = Common::Rect(left / 2, top / 2, right / 2, bottom / 2);
			else
				_scenes[i]._choices[j]._region = Common::Rect(left, top, right, bottom);
		}
	}

	for (int i = 0; i < kMaxBitmaps; i++) {
		_bitmaps[i]._duration = file.readSint16LE() * 100;
		file.read(buf, kMaxName);
		_bitmaps[i]._filename = Common::String(buf);
	}
}

void PlumbersGameWindows::postSceneBitmaps() {
	if (_scenes[_curSceneIdx]._decisionChoices == 1) {
		_curChoice = 0;
		_actions.push(ChangeScene);
		return;
	}

	_showScoreFl = true;
	_leftButtonDownFl = true;
	_setDurationFl = false;
	loadImage(_scenes[_curSceneIdx]._sceneName + "/" + _scenes[_curSceneIdx]._decisionBitmap);
}
}  // End of namespace Plumbers
