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
#include "graphics/paletteman.h"
#include "graphics/scaler/downscaler.h"
#include "graphics/surface.h"

namespace Plumbers {

PlumbersGame::PlumbersGame(OSystem *syst, const ADGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _console(nullptr), _image(nullptr),
		_compositeSurface(nullptr), _videoDecoder(nullptr), _quit(false) {
	_timerInstalled = false;
	_showScoreFl = false;
	_setDurationFl = false;
	_leftButtonDownFl = false;
	_endGameFl = false;
	_curSceneIdx = -1;
	_prvSceneIdx = -1;
	_curBitmapIdx = -1;
	_curChoice = 0;
	_totScene = -1;
	_totScore = 0;
}

PlumbersGame::~PlumbersGame() {
	delete _image;
	//_console is deleted by Engine
}

static const byte MOUSECURSOR_SCI[] = {
	1,1,0,0,0,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,0,0,0,
	1,2,2,1,0,0,0,0,0,0,0,
	1,2,2,2,1,0,0,0,0,0,0,
	1,2,2,2,2,1,0,0,0,0,0,
	1,2,2,2,2,2,1,0,0,0,0,
	1,2,2,2,2,2,2,1,0,0,0,
	1,2,2,2,2,2,2,2,1,0,0,
	1,2,2,2,2,2,2,2,2,1,0,
	1,2,2,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,1,0,0,0,0,
	1,2,1,0,1,2,2,1,0,0,0,
	1,1,0,0,1,2,2,1,0,0,0,
	0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,1,2,2,1,0
};

static const byte MOUSECURSOR_AMIGA[] = {
	1,1,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,
	1,2,2,1,0,0,0,0,
	1,2,2,2,1,0,0,0,
	1,2,2,2,2,1,0,0,
	1,2,2,2,2,2,1,0,
	1,1,1,2,2,1,1,0,
	0,0,0,1,2,1,0,0,
	0,0,0,1,2,2,1,0,
	0,0,0,0,1,2,1,0,
	0,0,0,0,1,2,2,1,
	0,0,0,0,0,1,1,0,
};

static const byte cursorPalette[] = {
	0, 0, 0,           // Black / Transparent
	0x80, 0x80, 0x80,  // Gray
	0xff, 0xff, 0xff   // White
};

void PlumbersGame::handleEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_QUIT:
	case Common::EVENT_RETURN_TO_LAUNCHER:
		_quit = true;
		break;

	case Common::EVENT_LBUTTONDOWN:
		if (_leftButtonDownFl) {
			_curChoice = getMouseHiLite();

			if (_curChoice >= 0 && _curChoice < _scenes[_curSceneIdx]._decisionChoices) {
				debugC(5, kDebugGeneral, "Accepting mouse click with choice = %d", _curChoice);
				_totScore += _scenes[_curSceneIdx]._choices[_curChoice]._points;
				_actions.push(ChangeScene);
				_leftButtonDownFl = false;
			}
		} else if (_console->_allowSkip && _timerInstalled) {
			// Allows to skip speech by skipping wait delay
			onTimer(this);
		}
		break;
	default:
		break;
	}
}

Common::Error PlumbersGame::run() {
	startGraphics();

	_console = new Console();
	setDebugger(_console);

	if (_screenW > 320)
		CursorMan.replaceCursor(MOUSECURSOR_SCI, 11, 16, 0, 0, 0);
	else
		CursorMan.replaceCursor(MOUSECURSOR_AMIGA, 8, 12, 0, 0, 0);
	CursorMan.replaceCursorPalette(cursorPalette, 0, 3);
	CursorMan.showMouse(true);

	readTables();

	_showScoreFl = false;
	_leftButtonDownFl = false;
	_endGameFl = false;
	_totScore = 0;
	_curSceneIdx = _prvSceneIdx = 0;
	_curChoice = 0;
	_actions.clear();
	_actions.push(ShowScene);

	_quit = false;

	while (!_quit && !_endGameFl) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			handleEvent(event);
		}

		preActions();

		while (!_actions.empty()) {
			switch (_actions.pop()) {
			case Redraw:
				drawScreen();
				break;
			case ShowScene:
				showScene();
				break;
			case UpdateScene:
				updateScene();
				break;
			case ChangeScene:
				changeScene();
				break;
			case PlaySound:
				playSound(_scenes[_curSceneIdx]._sceneName + "/" + _scenes[_curSceneIdx]._waveFilename);
				break;
			default:
				break;
			}
		}

		if (_videoDecoder) {
			if (_videoDecoder->endOfVideo()) {
				_actions.push(ChangeScene);
				_videoDecoder->close();
				delete _videoDecoder;
				_videoDecoder = nullptr;
			} else if (_videoDecoder->needsUpdate()) {
				drawScreen();
			}
		}

		g_system->updateScreen();
		g_system->delayMillis(_videoDecoder ? (1000 / 60) : 10);
	}

	g_system->getTimerManager()->removeTimerProc(onTimer);
	stopSound();

	return Common::kNoError;
}

void PlumbersGame::loadImage(const Common::String &name) {
	debugC(1, kDebugGeneral, "%s : %s", __FUNCTION__, name.c_str());
	Common::File file;
	if (!file.open(Common::Path(name)))
		error("unable to load image %s", name.c_str());

	_image->loadStream(file);
	delete _compositeSurface;
	_compositeSurface = nullptr;
}

void PlumbersGame::blitImageSurface(const Graphics::Surface *surface) {
	int w = CLIP<int>(surface->w, 0, _screenW);
	int h = CLIP<int>(surface->h, 0, _screenH);

	int x = (_screenW - w) / 2;
	int y = (_screenH - h) / 2;

	g_system->copyRectToScreen(surface->getPixels(), surface->pitch, x, y, w, h);
}

void PlumbersGame::blitImage() {
	blitImageSurface(_compositeSurface ? _compositeSurface : _image->getSurface());
}

void PlumbersGame::drawScreen() {
	debugC(_videoDecoder ? 10 : 1, kDebugGeneral, "%s : %s", __FUNCTION__, _image ? "YES" : "NO");
	if (_videoDecoder ? _videoDecoder->needsUpdate() : _image || _compositeSurface) {
		if (_setDurationFl) {
			g_system->getTimerManager()->removeTimerProc(onTimer);
			g_system->getTimerManager()->installTimerProc(onTimer, _bitmaps[_curBitmapIdx]._duration * 1000, this, "timer");
			_timerInstalled = true;
			_actions.push(UpdateScene);
		}

		g_system->fillScreen(0);

		blitImage();

		if (_showScoreFl) {
			Graphics::PixelFormat screenFormat = g_system->getScreenFormat();
			Common::String score = Common::String::format("Your Score is: %ld", _totScore);
			const Graphics::Font &font(*FontMan.getFontByUsage(
						       _screenW >= 640 ? Graphics::FontManager::kBigGUIFont : Graphics::FontManager::kGUIFont));
			int scoreTop = _screenH - _screenH / 12;
			int scoreMaxWidth = _screenW >= 640 ? 200 : 150;
			uint scoreColor = screenFormat.bytesPerPixel == 1 ? 0xff : screenFormat.RGBToColor(0xff, 0xff, 0xff);
			Common::Rect rect(10, scoreTop, scoreMaxWidth, scoreTop + font.getFontHeight());
			if (getPlatform() != Common::kPlatform3DO)
				g_system->fillScreen(rect, 0);
			Graphics::Surface *screen = g_system->lockScreen();
			font.drawString(screen, score, rect.left, rect.top, scoreMaxWidth - 10,
					scoreColor, Graphics::kTextAlignCenter);
			g_system->unlockScreen();
			_showScoreFl = false;
		}

		if (_image->getPalette() != nullptr)
			g_system->getPaletteManager()->setPalette(_image->getPalette(), 0, 256);
		g_system->updateScreen();
	}
}

void PlumbersGame::playSound(const Common::String &name) {
	debugC(3, kDebugGeneral, "%s : %s", __FUNCTION__, name.c_str());
	Common::File *file = new Common::File();
	if (!file->open(Common::Path(name)))
		error("unable to load sound %s", name.c_str());

	Audio::AudioStream *stream;
	if (name.hasSuffixIgnoreCase(".aiff"))
		stream = Audio::makeAIFFStream(file, DisposeAfterUse::YES);
	else
		stream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
	stopSound();
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream, -1, Audio::Mixer::kMaxChannelVolume);
}

void PlumbersGame::stopSound() {
	debugC(3, kDebugGeneral, "%s", __FUNCTION__);
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);
}

void PlumbersGame::showScene() {
	debugC(1, kDebugGeneral, "%s : %d", __FUNCTION__, _curSceneIdx);
	_curBitmapIdx = _scenes[_curSceneIdx]._startBitmap - 1;
	updateScene();
	if (_scenes[_curSceneIdx]._waveFilename != "")
		_actions.push(PlaySound);
	_actions.push(Redraw);
}

int PlumbersGame::getMouseHiLite() {
	Common::Point mousePos = g_system->getEventManager()->getMousePos();
	for (int i = 0; i < _scenes[_curSceneIdx]._decisionChoices && i < kMaxChoice; i++) {
		if (_scenes[_curSceneIdx]._choices[i]._region.contains(mousePos))
			return i;
	}

	return -1;
}

void PlumbersGame::updateScene() {
	debugC(2, kDebugGeneral, "%s : %d", __FUNCTION__, _curBitmapIdx);
	_curBitmapIdx++;
	if (_curBitmapIdx >= _scenes[_curSceneIdx]._startBitmap + _scenes[_curSceneIdx]._bitmapNum) {
		postSceneBitmaps();
	} else {
		loadImage(_scenes[_curSceneIdx]._sceneName  + "/" + _bitmaps[_curBitmapIdx]._filename);
		_setDurationFl = true;
	}
}

void PlumbersGame::changeScene() {
	debugC(1, kDebugGeneral, "%s : %d", __FUNCTION__, _curChoice);
	if (_scenes[_curSceneIdx]._choices[_curChoice]._sceneName == "SC-1") {
		_curSceneIdx = _prvSceneIdx;
		_curBitmapIdx = 9999;
		_actions.push(UpdateScene);
		_actions.push(Redraw);
	} else if (_scenes[_curSceneIdx]._choices[_curChoice]._sceneName == "restart") {
		_curSceneIdx = 0;
		_totScore = 0;
		_actions.push(UpdateScene);
		_actions.push(Redraw);
	} else if (_scenes[_curSceneIdx]._choices[_curChoice]._sceneName == "SC32767"
		|| _scenes[_curSceneIdx]._choices[_curChoice]._sceneName == "end") {
		_endGameFl = true;
	} else {
		if (_scenes[_curSceneIdx]._decisionChoices > 1)
			_prvSceneIdx = _curSceneIdx;
		if (_scenes[_curSceneIdx]._choices[_curChoice]._skipScene) {
			_curSceneIdx = getSceneNumb(_scenes[_curSceneIdx]._choices[_curChoice]._sceneName);
			_curBitmapIdx = 9999;
			_actions.push(UpdateScene);
			_actions.push(Redraw);
			g_system->getTimerManager()->removeTimerProc(onTimer);
			_timerInstalled = false;
		} else {
			_curSceneIdx = getSceneNumb(_scenes[_curSceneIdx]._choices[_curChoice]._sceneName);
			_actions.push(ShowScene);
		}
	}
}

void PlumbersGame::processTimer() {
	debugC(7, kDebugGeneral, "%s", __FUNCTION__);
	_timerInstalled = false;
	if (!_endGameFl)
		_actions.push(Redraw);
}

void PlumbersGame::onTimer(void *arg) {
	g_system->getTimerManager()->removeTimerProc(onTimer);
	((PlumbersGame*)arg)->processTimer();
}

void PlumbersGame::initTables() {
	for (uint i = 0; i < ARRAYSIZE(_scenes); i++) {
		_scenes[i]._bitmapNum = 0;
		_scenes[i]._startBitmap = 0;
		_scenes[i]._decisionChoices = 0;
		_scenes[i]._sceneName = "";
		_scenes[i]._waveFilename = "";
		_scenes[i]._decisionBitmap = "";
		_scenes[i]._style = Scene::STYLE_PC;
		for (uint j = 0; j < ARRAYSIZE(_scenes[i]._choices); j++) {
			_scenes[i]._choices[j]._points = 0;
			_scenes[i]._choices[j]._skipScene = 0;
			_scenes[i]._choices[j]._region = Common::Rect(0, 0, 0, 0);
			_scenes[i]._choices[j]._sceneName = "";
		}
	}
	for (uint i = 0; i < ARRAYSIZE(_bitmaps); i++) {
		_bitmaps[i]._duration = 0;
		_bitmaps[i]._filename = "";
	}
}

int PlumbersGame::getSceneNumb(const Common::String &sName) {
	debugC(1, kDebugGeneral, "%s : %s", __FUNCTION__, sName.c_str());
	for (int sCurScene = 0; sCurScene < _totScene; sCurScene++) {
		if (sName == _scenes[sCurScene]._sceneName)
			return sCurScene;
	}
	return 0;
}

} // End of namespace Plumbers
