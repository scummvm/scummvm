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

#include "common/config-manager.h"
#include "common/error.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "graphics/palette.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "common/system.h"
#include "engines/util.h"
#include "common/debug.h"

#include "mario/mario.h"

namespace Mario {

MarioGame::MarioGame(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_image = nullptr;
}

MarioGame::~MarioGame() {
	if (_image)
		delete _image;
}

Common::Error MarioGame::run() {
	initGraphics(640, 480, true);
	g_system->showMouse(true);

	readTables("game.bin");

	_showScoreFl = false;
	_leftButtonDownFl = false;
	_endGameFl = false;
	_totScore = 0;
	_curSceneIdx = _prvSceneIdx = 0;
	_curChoice = 0;
	_actions.clear();
	_actions.push(ShowScene);

	bool quit = false;

	while (!quit && !_endGameFl) {
		g_system->delayMillis(100);

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
				quit = true;
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (_leftButtonDownFl) {
					Common::Point mousePos = g_system->getEventManager()->getMousePos();
					for (_curChoice = 0; _curChoice < _scenes[_curSceneIdx]._decisionChoices; _curChoice++) {
						if (_scenes[_curSceneIdx]._choices[_curChoice]._region.contains(mousePos))
							break;
					}
					if (_curChoice < kMaxChoice) {
						debug("Accepting mouse click at %d : %d , choice = %d", mousePos.x, mousePos.y, _curChoice);
						_totScore += _scenes[_curSceneIdx]._choices[_curChoice]._points;
						_actions.push(ChangeScene);
						_leftButtonDownFl = false;
					}
				}
				break;
			default:
				break;
			}
		}

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
				playSound();
				break;
			}
		}

		g_system->updateScreen();	//TODO: keep mouse cursor alive
	}

	g_system->getTimerManager()->removeTimerProc(onTimer);
	stopSound();

	return Common::kNoError;
}

void MarioGame::loadImage(const Common::String &dirname, const Common::String &filename) {
	Common::String name = dirname + "/" + filename;
	debug("%s : %s", __FUNCTION__, name.c_str());
	Common::File *file = new Common::File();
	if (!file->open(name))
		error("unable to load image %s", name.c_str());

	if (_image)
		delete _image;

	_image = new Image::BitmapDecoder();
	_image->loadStream(*file);
	file->close();
	delete file;
}

void MarioGame::drawScreen() {
	debug("%s : %s", __FUNCTION__, _image ? "YES" : "NO");
	if (_image) {
		if (_setDurationFl) {
			g_system->getTimerManager()->removeTimerProc(onTimer);
			g_system->getTimerManager()->installTimerProc(onTimer, _bitmaps[_curBitmapIdx]._duration * 100 * 1000, this, "timer");
			_actions.push(UpdateScene);
		}

		Graphics::Surface *screen = g_system->lockScreen();
		screen->fillRect(Common::Rect(0, 0, g_system->getWidth(), g_system->getHeight()), 0);

		const Graphics::Surface *surface = _image->getSurface();

		int w = CLIP<int>(surface->w, 0, 640);
		int h = CLIP<int>(surface->h, 0, 480);

		int x = (640 - w) / 2;
		int y = (480 - h) / 2;

		screen->copyRectToSurface(*surface, x, y, Common::Rect(0, 0, w, h));

		if (_showScoreFl) {
			Common::String score = Common::String::format("Your Score is: %ld", _totScore);
			const Graphics::Font &font(*FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont));
			Common::Rect rect(10, 440, 200, 440 + font.getFontHeight());
			screen->fillRect(rect, 0);
			font.drawString(screen, score, rect.left, rect.top, 190, 255, Graphics::kTextAlignCenter);
			_showScoreFl = false;
		}

		g_system->unlockScreen();
		g_system->getPaletteManager()->setPalette(_image->getPalette(), 0, 256);
		g_system->updateScreen();
	}
}

void MarioGame::playSound() {
	Common::String name = _scenes[_curSceneIdx]._sceneName + "/" + _scenes[_curSceneIdx]._waveFilename;
	debug("%s : %s", __FUNCTION__, name.c_str());
	Common::File *file = new Common::File();
	if (!file->open(name))
		error("unable to load sound %s", name.c_str());

	Audio::RewindableAudioStream *audioStream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
	Audio::AudioStream *stream = audioStream;
	stopSound();
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream, -1, Audio::Mixer::kMaxChannelVolume);
}

void MarioGame::stopSound() {
	debug("%s", __FUNCTION__);
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);
}

void MarioGame::showScene() {
	debug("%s : %d", __FUNCTION__, _curSceneIdx);
	_curBitmapIdx = _scenes[_curSceneIdx]._startBitmap;
	loadImage(_scenes[_curSceneIdx]._sceneName, _bitmaps[_curBitmapIdx]._filename);
	_actions.push(Redraw);
	_setDurationFl = true;
	_actions.push(PlaySound);
}

void MarioGame::updateScene() {
	debug("%s : %d", __FUNCTION__, _curBitmapIdx);
	_curBitmapIdx++;
	if (_curBitmapIdx >= _scenes[_curSceneIdx]._startBitmap + _scenes[_curSceneIdx]._bitmapNum) {
		if (_scenes[_curSceneIdx]._decisionChoices == 1) {
			_curChoice = 0;
			_actions.push(ChangeScene);
		} else {
			_showScoreFl = true;
			_leftButtonDownFl = true;
			_setDurationFl = false;
			loadImage(_scenes[_curSceneIdx]._sceneName, _scenes[_curSceneIdx]._decisionBitmap);
		}
	} else {
		loadImage(_scenes[_curSceneIdx]._sceneName, _bitmaps[_curBitmapIdx]._filename);
		_setDurationFl = true;
	}
}

void MarioGame::changeScene() {
	debug("%s : %d", __FUNCTION__, _curChoice);
	if (_scenes[_curSceneIdx]._choices[_curChoice]._sceneIdx == -1) {
		_curSceneIdx = _prvSceneIdx;
		_curBitmapIdx = 9999;
		_actions.push(UpdateScene);
		_actions.push(Redraw);
	} else if (_scenes[_curSceneIdx]._choices[_curChoice]._sceneIdx == 32767) {
		_endGameFl = true;
	} else {
		if (_scenes[_curSceneIdx]._decisionChoices > 1)
			_prvSceneIdx = _curSceneIdx;
		if (_scenes[_curSceneIdx]._choices[_curChoice]._skipScene) {
			_curSceneIdx = getSceneNumb(_scenes[_curSceneIdx]._choices[_curChoice]._sceneIdx);
			_curBitmapIdx = 9999;
			_actions.push(UpdateScene);
			_actions.push(Redraw);
			g_system->getTimerManager()->removeTimerProc(onTimer);
		} else {
			_curSceneIdx = getSceneNumb(_scenes[_curSceneIdx]._choices[_curChoice]._sceneIdx);
			_actions.push(ShowScene);
		}
	}
}

void MarioGame::processTimer() {
	debug("%s", __FUNCTION__);
	if (!_endGameFl)
		_actions.push(Redraw);
}

void MarioGame::onTimer(void *arg) {
	g_system->getTimerManager()->removeTimerProc(onTimer);
	((MarioGame*)arg)->processTimer();
}

void MarioGame::initTables() {
	memset(_scenes, 0, sizeof(_scenes));
	memset(_bitmaps, 0, sizeof(_bitmaps));
}

void MarioGame::readTables(const Common::String &fileName) {
	Common::File file;
	if (!file.open(fileName))
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

		for (int j = 0; j < kMaxChoice; j++) {
			_scenes[i]._choices[j]._points = file.readSint32LE();
			_scenes[i]._choices[j]._sceneIdx = file.readSint16LE();
			_scenes[i]._choices[j]._skipScene = file.readSint16LE();
			int left = file.readSint16LE();
			int top = file.readSint16LE();
			int right = file.readSint16LE();
			int bottom = file.readSint16LE();
			_scenes[i]._choices[j]._region = Common::Rect(left, top, right, bottom);
		}
	}

	for (int i = 0; i < kMaxBitmaps; i++) {
		_bitmaps[i]._duration = file.readSint16LE();
		file.read(buf, kMaxName);
		_bitmaps[i]._filename = Common::String(buf);
	}
	file.close();
}

int MarioGame::getSceneNumb(int sNo) {
	debug("%s : %d", __FUNCTION__, sNo);
	Common::String testString = Common::String::format("SC%02d", sNo);

	for (int sCurScene = 0; sCurScene < _totScene; sCurScene++) {
		if (testString == _scenes[sCurScene]._sceneName)
			return sCurScene;
	}
	return 0;
}

} // End of namespace Mario
