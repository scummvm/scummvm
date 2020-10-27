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
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "image/bmp.h"
#include "image/cel_3do.h"

#include "video/3do_decoder.h"

namespace Plumbers {

static const Common::KeyCode cheatKbd[] = {
	Common::KEYCODE_UP,
	Common::KEYCODE_DOWN,
	Common::KEYCODE_RIGHT,
	Common::KEYCODE_LEFT,
	Common::KEYCODE_DOWN,
	Common::KEYCODE_RIGHT,
	Common::KEYCODE_RETURN
};

static const Common::JoystickButton cheatJoy[] = {
	Common::JOYSTICK_BUTTON_DPAD_UP,
	Common::JOYSTICK_BUTTON_DPAD_DOWN,
	Common::JOYSTICK_BUTTON_DPAD_RIGHT,
	Common::JOYSTICK_BUTTON_DPAD_LEFT,
	Common::JOYSTICK_BUTTON_DPAD_DOWN,
	Common::JOYSTICK_BUTTON_DPAD_RIGHT,
	Common::JOYSTICK_BUTTON_X
};

PlumbersGame::PlumbersGame(OSystem *syst, const ADGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _console(0), _image(0) {
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
	_cheatEnabled = false;
	_cheatFSM = 0;

	DebugMan.addDebugChannel(kDebugGeneral, "general", "General debug level");
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

static const byte cursorPalette[] = {
	0, 0, 0,           // Black / Transparent
	0x80, 0x80, 0x80,  // Gray
	0xff, 0xff, 0xff   // White
};

// TODO(3do):
// * effect when changing between scenes
// * 3do boot logo
// * return to previous scene
// * hide cursor

void PlumbersGame::updateHiLite() {
	_actions.push(Redraw);
	if (_hiLite < 0)
		return;
	if (_scenes[_curSceneIdx]._style == Scene::STYLE_DECISION_MIKE) {
		playSound(Common::String::format("%s/%s%dS.Aiff",
						 _scenes[_curSceneIdx]._sceneName.c_str(),
						 _scenes[_curSceneIdx]._decisionBitmap.c_str(), _hiLite + 1));
	} else if (_scenes[_curSceneIdx]._style == Scene::STYLE_DECISION_TUN) {
		playSound(Common::String::format("%s%c.aiff", _scenes[_curSceneIdx]._sceneName.c_str(), _hiLite + 'a'));
	}
}

void PlumbersGame::joyUp() {
	int decNum = _scenes[_curSceneIdx]._decisionChoices;
	if (!_leftButtonDownFl || !_hiLiteEnabled)
		return;
	_kbdHiLite = _kbdHiLite < 0 ? 0 : (_kbdHiLite + decNum - 1) % decNum;
	_hiLite = _kbdHiLite;
	updateHiLite();
}

void PlumbersGame::joyDown() {
	if (!_leftButtonDownFl || !_hiLiteEnabled)
		return;
	int decNum = _scenes[_curSceneIdx]._decisionChoices;
	_kbdHiLite = _kbdHiLite < 0 ? 0 : (_kbdHiLite + 1) % decNum;
	_hiLite = _kbdHiLite;
	updateHiLite();
}

void PlumbersGame::joyA() {
	if (_kbdHiLite < 0 || !_leftButtonDownFl || !_hiLiteEnabled)
		return;
	debugC(5, kDebugGeneral, "Accepting enter press with choice = %d", _kbdHiLite);
	_curChoice = _kbdHiLite;
	_totScore += _scenes[_curSceneIdx]._choices[_kbdHiLite]._points;
	_actions.push(ChangeScene);
	_leftButtonDownFl = false;
}

void PlumbersGame::skipVideo() {
	if (_scenes[_curSceneIdx]._sceneName == "janp1weaver"
	    || _scenes[_curSceneIdx]._sceneName == "janp2weaver") {
		// Skip janp2weaver and janp3weaver
		_curSceneIdx = getSceneNumb("titleweaver");
		_actions.push(ShowScene);
	} else {
		_actions.push(ChangeScene);
	}
	_videoDecoder->close();
	delete _videoDecoder;
	_videoDecoder = nullptr;
}


Common::Error PlumbersGame::run() {
	if (getPlatform() == Common::kPlatform3DO) {
		_image = new Image::Cel3DODecoder();
		_ctrlHelpImage = new Image::Cel3DODecoder();
		_screenW = 320;
		_screenH = 240;
		Graphics::PixelFormat pf(2, 5, 5, 5, 1, 10,  5,  0, 15);
		initGraphics(_screenW, _screenH, &pf);
	} else {
		_image = new Image::BitmapDecoder();
		_screenW = 640;
		_screenH = 480;
		initGraphics(_screenW, _screenH);
	}

	_console = new Console();
	setDebugger(_console);

	CursorMan.replaceCursor(MOUSECURSOR_SCI, 11, 16, 0, 0, 0);
	CursorMan.replaceCursorPalette(cursorPalette, 0, 3);
	CursorMan.showMouse(true);

	if (getPlatform() == Common::kPlatform3DO)
		readTables3DO("launchme");
	else
		readTablesPC("game.bin");

	_showScoreFl = false;
	_leftButtonDownFl = false;
	_endGameFl = false;
	// PC uses a palette, so we don't do highlighting.
	// Original does the same
	_hiLiteEnabled = getPlatform() == Common::kPlatform3DO;
	_totScore = 0;
	_curSceneIdx = _prvSceneIdx = 0;
	_curChoice = 0;
	_kbdHiLite = -1;
	_mouseHiLite = -1;
	_hiLite = -1;
	_mouseHiLite = -1;
	_kbdHiLite = -1;
	_leftShoulderPressed = false;
	_actions.clear();
	_actions.push(ShowScene);

	bool quit = false;

	while (!quit && !_endGameFl) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				quit = true;
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
			case Common::EVENT_JOYBUTTON_DOWN:
				if (_videoDecoder) {
					if (_cheatFSM < ARRAYSIZE(cheatJoy) && event.joystick.button == cheatJoy[_cheatFSM]) {
						_cheatFSM++;
						if (_cheatFSM == ARRAYSIZE(cheatJoy)) {
							debugC(1, kDebugGeneral, "Cheat enabled");
							_cheatEnabled = true;
						}
					} else if (event.joystick.button == cheatJoy[0])
						_cheatFSM = 1;
					else
						_cheatFSM = 0;
				}
				if (_videoDecoder && (event.joystick.button == Common::JOYSTICK_BUTTON_A ||
						      event.joystick.button == Common::JOYSTICK_BUTTON_B ||
						      event.joystick.button == Common::JOYSTICK_BUTTON_X)) {
					skipVideo();
				} else if (event.joystick.button == Common::JOYSTICK_BUTTON_DPAD_UP ||
					   event.joystick.button == Common::JOYSTICK_BUTTON_DPAD_LEFT) {
					joyUp();
				} else if (event.joystick.button == Common::JOYSTICK_BUTTON_DPAD_DOWN ||
					   event.joystick.button == Common::JOYSTICK_BUTTON_DPAD_RIGHT) {
					joyDown();
				} else if (event.joystick.button == Common::JOYSTICK_BUTTON_A) {
					joyA();
				} else if (event.joystick.button == Common::JOYSTICK_BUTTON_LEFT_SHOULDER) {
					_leftShoulderPressed = true;
					if (_leftButtonDownFl && _ctrlHelpImage)
						_actions.push(Redraw);
				}
				break;
			case Common::EVENT_JOYBUTTON_UP:
				if (event.joystick.button == Common::JOYSTICK_BUTTON_LEFT_SHOULDER) {
					_leftShoulderPressed = false;
					if (_leftButtonDownFl && _ctrlHelpImage)
						_actions.push(Redraw);
				}
				break;
			case Common::EVENT_KEYDOWN:
				if (_videoDecoder) {
					if (_cheatFSM < ARRAYSIZE(cheatKbd) && event.kbd.keycode == cheatKbd[_cheatFSM]) {
						_cheatFSM++;
						if (_cheatFSM == ARRAYSIZE(cheatKbd)) {
							debugC(1, kDebugGeneral, "Cheat enabled");
							_cheatEnabled = true;
						}
					} else if (event.kbd.keycode == cheatKbd[0])
						_cheatFSM = 1;
					else
						_cheatFSM = 0;
				}
				if (event.kbd.keycode == Common::KEYCODE_SPACE && _videoDecoder) {
					skipVideo();
				} else if ((event.kbd.keycode == Common::KEYCODE_UP ||
					    event.kbd.keycode == Common::KEYCODE_LEFT)) {
					joyUp();
				} else if ((event.kbd.keycode == Common::KEYCODE_DOWN ||
					    event.kbd.keycode == Common::KEYCODE_RIGHT)) {
					joyDown();
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN) {
					joyA();
				} else if (event.kbd.keycode == Common::KEYCODE_q) {
					_leftShoulderPressed = true;
					if (_leftButtonDownFl && _ctrlHelpImage)
						_actions.push(Redraw);
				}
				break;
			case Common::EVENT_KEYUP:
				if (event.kbd.keycode == Common::KEYCODE_q) {
					_leftShoulderPressed = false;
					if (_leftButtonDownFl && _ctrlHelpImage)
						_actions.push(Redraw);
				}
				break;
			default:
				break;
			}
		}

		if (_leftButtonDownFl) {
			int nh = getMouseHiLite();
			if (nh != _mouseHiLite) {
				_mouseHiLite = nh;
				_hiLite = _mouseHiLite;
				updateHiLite();
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
	if (!file.open(name))
		error("unable to load image %s", name.c_str());

	_image->loadStream(file);
	delete _compositeSurface;
	_compositeSurface = nullptr;
}

// TODO: discover correct offsets
Common::Point getMikeStart(uint num, uint total) {
	if (total == 2)
		return Common::Point(140 * num + 10, 120 * num + 10);
	return Common::Point(60 * num + 20, 70 * num + 20);
}

// TODO: discover correct offsets
Common::Point getMikeSize(uint total) {
	if (total == 2)
		return Common::Point(80, 100);
	return Common::Point(80, 60);
}

void PlumbersGame::loadMikeDecision(const Common::String &dirname, const Common::String &baseFilename, uint num) {
	Common::String baseName = dirname + "/" + baseFilename;
	debugC(1, kDebugGeneral, "%s : %s", __FUNCTION__, baseName.c_str());
	Graphics::Surface *surf = new Graphics::Surface();
	surf->create(_screenW, _screenH, Graphics::PixelFormat(2, 5, 5, 5, 1, 10,  5,  0, 15));

	delete _compositeSurface;
	_compositeSurface = nullptr;

	for (uint i = 0; i < num; i++) {
		Common::Point p = getMikeStart(i, num);
		Common::Point sz = getMikeSize(num);
		Common::File fileP;
		Common::String nameP = Common::String::format("%s%dP.CEL", baseName.c_str(), i + 1);
		if (!fileP.open(nameP))
			error("unable to load image %s", nameP.c_str());

		_image->loadStream(fileP);
		surf->copyRectToSurface(*_image->getSurface(), p.x, p.y,
					Common::Rect(0, 0, sz.x, sz.y));

		Common::File fileW;
		Common::String nameW = Common::String::format("%s%dW.CEL", baseName.c_str(), i + 1);
		if (!fileW.open(nameW))
			error("unable to load image %s", nameW.c_str());
		
		_image->loadStream(fileW);
		surf->copyRectToSurface(*_image->getSurface(), p.x + sz.x, p.y,
					Common::Rect(0, 0, sz.x, sz.y));
	}

	_compositeSurface = surf;

	Common::File fileCtrl;
	if (fileCtrl.open(dirname + "/CONTROLHELP.CEL"))
		_ctrlHelpImage->loadStream(fileCtrl);
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

		Graphics::Surface *screen = g_system->lockScreen();
		screen->fillRect(Common::Rect(0, 0, g_system->getWidth(), g_system->getHeight()), 0);

		const Graphics::Surface *surface;
		bool ctrlHelp = false;
		if (_leftShoulderPressed && _leftButtonDownFl && _ctrlHelpImage) {
			surface = _ctrlHelpImage->getSurface();
			ctrlHelp = true;
		} else if (_videoDecoder)
			surface = _videoDecoder->decodeNextFrame();
		else if (_compositeSurface)
			surface = _compositeSurface;
		else
			surface = _image->getSurface();

		Graphics::Surface modSurf;
		bool modded = false;

		if (_hiLiteEnabled && _hiLite >= 0 && _leftButtonDownFl && !ctrlHelp) {
			Graphics::PixelFormat pf(2, 5, 5, 5, 1, 10,  5,  0, 15);
			modSurf.create(surface->w, surface->h, pf);
			modSurf.copyRectToSurface(*surface, 0, 0, Common::Rect(0, 0, surface->w, surface->h));
			const Common::Rect rec = _scenes[_curSceneIdx]._choices[_hiLite]._region;
			
			for (int y = rec.top; y <= rec.bottom; y++) {
				uint16 *p = (uint16 *) modSurf.getPixels() + modSurf.w * y + rec.left;
				for (int x = rec.left; x < rec.right; x++, p++) {
					uint r, g, b;
					r = (*p >> 10) & 0x1f;
					g = (*p >> 5) & 0x1f;
					b = (*p >> 0) & 0x1f;
					// TODO: figure out the correct multipliers
					r = MIN<int>(3 * r / 2, 0x1f);
					g = MIN<int>(3 * g / 2, 0x1f);
					b = MIN<int>(3 * b / 2, 0x1f);
					*p = (*p & 0x8000) | (r << 10) | (g << 5) | (b);
				}
			}
			modded = true;
		}

		int w = CLIP<int>(surface->w, 0, _screenW);
		int h = CLIP<int>(surface->h, 0, _screenH);

		int x = (_screenW - w) / 2;
		int y = (_screenH - h) / 2;

		screen->copyRectToSurface(modded ? modSurf : *surface, x, y, Common::Rect(0, 0, w, h));

		if (_showScoreFl) {
			Common::String score = Common::String::format("Your Score is: %ld", _totScore);
			const Graphics::Font &font(*FontMan.getFontByUsage(
						       _screenW >= 640 ? Graphics::FontManager::kBigGUIFont : Graphics::FontManager::kGUIFont));
			int scoreTop = _screenH - _screenH / 12;
			int scoreMaxWidth = _screenW >= 640 ? 200 : 150;
			uint scoreColor = screen->format.bytesPerPixel == 1 ? 0xff : screen->format.RGBToColor(0xff, 0xff, 0xff);
			Common::Rect rect(10, scoreTop, scoreMaxWidth, scoreTop + font.getFontHeight());
			if (getPlatform() != Common::kPlatform3DO)
				screen->fillRect(rect, 0);
			font.drawString(screen, score, rect.left, rect.top, scoreMaxWidth - 10,
					scoreColor, Graphics::kTextAlignCenter);
			_showScoreFl = false;
		}

		g_system->unlockScreen();
		if (_image->getPalette() != nullptr)
			g_system->getPaletteManager()->setPalette(_image->getPalette(), 0, 256);
		g_system->updateScreen();
	}
}

void PlumbersGame::playSound(const Common::String &name) {
	debugC(3, kDebugGeneral, "%s : %s", __FUNCTION__, name.c_str());
	Common::File *file = new Common::File();
	if (!file->open(name))
		error("unable to load sound %s", name.c_str());

	Audio::AudioStream *stream;
	if (name.hasSuffix(".aiff") || name.hasSuffix(".Aiff"))
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
		if (_scenes[_curSceneIdx]._style == Scene::STYLE_VIDEO) {
			_videoDecoder = new Video::ThreeDOMovieDecoder();
			_curChoice = 0;
			if (!_videoDecoder->loadFile(_scenes[_curSceneIdx]._sceneName)) {
				_actions.push(ChangeScene);
				return;
			}
			_videoDecoder->start();
		} else if (_scenes[_curSceneIdx]._decisionChoices == 1) {
			_curChoice = 0;
			_actions.push(ChangeScene);
		} else {
			_showScoreFl = true;
			_leftButtonDownFl = true;
			_setDurationFl = false;
			if (_scenes[_curSceneIdx]._style == Scene::STYLE_DECISION_MIKE) {
				loadMikeDecision(_scenes[_curSceneIdx]._sceneName, _scenes[_curSceneIdx]._decisionBitmap,
					_scenes[_curSceneIdx]._decisionChoices);
				_hiLite = 0;
				_kbdHiLite = 0;
				updateHiLite();
			} else if (_scenes[_curSceneIdx]._style == Scene::STYLE_DECISION_TUN) {
				loadImage(_scenes[_curSceneIdx]._sceneName + ".cel");
				_hiLite = 0;
				_kbdHiLite = 0;
				updateHiLite();
				Common::File fileCtrl;
				if (fileCtrl.open("tuntest/dec/controlhelp.cel"))
					_ctrlHelpImage->loadStream(fileCtrl);
			} else {
				loadImage(_scenes[_curSceneIdx]._sceneName + "/" + _scenes[_curSceneIdx]._decisionBitmap);
				_hiLite = -1;
				_kbdHiLite = -1;
			}

			_mouseHiLite = getMouseHiLite();
		}
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

void PlumbersGame::readTablesPC(const Common::String &fileName) {
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
		_scenes[i]._style = Scene::STYLE_PC;

		for (int j = 0; j < kMaxChoice; j++) {
			_scenes[i]._choices[j]._points = file.readSint32LE();
			_scenes[i]._choices[j]._sceneName = Common::String::format("SC%02d", file.readSint16LE());
			_scenes[i]._choices[j]._skipScene = file.readSint16LE();
			int left = file.readSint16LE();
			int top = file.readSint16LE();
			int right = file.readSint16LE();
			int bottom = file.readSint16LE();
			_scenes[i]._choices[j]._region = Common::Rect(left, top, right, bottom);
		}
	}

	for (int i = 0; i < kMaxBitmaps; i++) {
		_bitmaps[i]._duration = file.readSint16LE() * 100;
		file.read(buf, kMaxName);
		_bitmaps[i]._filename = Common::String(buf);
	}
}

static void makeMikeDecision(Scene &scene, uint num) {
	scene._bitmapNum = 0;
	scene._startBitmap = 0;
	scene._decisionChoices = num;
	scene._waveFilename = "";
	scene._style = Scene::STYLE_DECISION_MIKE;

	Common::Point sz = getMikeSize(num);

	for (uint i = 0; i < num; i++) {
		Common::Point ms = getMikeStart(i, num);
		scene._choices[i]._region = Common::Rect(ms.x, ms.y, ms.x + 2 * sz.x, ms.y + sz.y);
	}
}

static void makeVideo(Scene &scene, const Common::String &videoName, const Common::String &nextScene) {
	scene._bitmapNum = 0;
	scene._startBitmap = 0;
	scene._decisionChoices = 1;
	scene._waveFilename = "";
	scene._style = Scene::STYLE_VIDEO;
	scene._sceneName = videoName;
	scene._decisionBitmap = "";
	scene._choices[0]._sceneName = nextScene;
}

static const struct {
	const char *from;
	struct {
		const char *scene;
		int points;
	} to[kMaxChoice];
} tungraph[] = {
	{
		"dec13",
		{
			{"sc15", -10000},
			{"sc17", -10000},
			{"sc20", -50000}
		}
	},
	{
		"dec16",
		{
			{"dec13", -20000},
			{"restart", 0}
		}
	},
	{

		"dec18",
		{
			{"dec13", -30000},
			{"restart", 0}
		}
	},
	{
		"dec20",
		{
			{"sc21", -90000},
			{"sc22", -90000}
		}
	},
	{
		"dec22",
		{
			{"dec20", -90000},
			{"restart", 0}
		}
	},
	{
		"dec23",
		{
			{"sc24", 50000},
			{"sc28", -50000}
		}
	},
	{
		"dec25",
		{
			{"sc26", -75000},
			{"sc27", -90000}
		}
	},
	{
		"dec26",
		{
			{"dec25", -75000},
			{"restart", 0}
		}
	},
	{
		"dec28",
		{
			{"dec23", -75000},
			{"restart", 0}
		}
	},
	{
		"dec29",
		{
			{"sc30", -20000},
			{"sc31", 90000}
		}
	},
	{
		"dec30",
		{
			{"sc32", 0},
			{"restart", 0},
			{"end", 0}
		}
	},
	{
		"dec31",
		{
			{"dec29", -20000},
			{"end", 0}
		}

	}
};

void PlumbersGame::readTables3DO(const Common::String &fileName) {
	Common::File file;
	if (!file.open(fileName))
		error("sReadTables(): Error reading launchme file");

	initTables();

	file.seek(0x1ec08);

	Common::HashMap<Common::String, int> imgCounter, firstImg;

	uint bitmapCtr = 0;

	for (; bitmapCtr < 287; bitmapCtr++) {
		char buf[16];
		file.read(buf, 16);
		_bitmaps[bitmapCtr]._filename = Common::String(buf);
		_bitmaps[bitmapCtr]._duration = (file.readSint32BE() * 1000) / 60;
		Common::String scene = Common::String(buf).substr(0, 4);
		scene.toLowercase();
		imgCounter[scene]++;
		if (!firstImg.contains(scene))
			firstImg[scene] = bitmapCtr;
	}

	file.seek(0x205d0);

	for (; bitmapCtr < 704; bitmapCtr++) {
		char buf[16];
		file.read(buf, 16);
		_bitmaps[bitmapCtr]._filename = Common::String(buf);
		_bitmaps[bitmapCtr]._duration = (file.readSint32BE() * 1000) / 60;
		Common::String scene = Common::String(buf).substr(0, 4);
		scene.toLowercase();
		imgCounter[scene]++;
		if (!firstImg.contains(scene))
			firstImg[scene] = bitmapCtr;
	}
	
	uint scPtr = 0;
	makeVideo(_scenes[scPtr++], "kirinweaver", "janp1weaver");
	makeVideo(_scenes[scPtr++], "janp1weaver", "janp2weaver");
	makeVideo(_scenes[scPtr++], "janp2weaver", "janp3weaver");
	makeVideo(_scenes[scPtr++], "janp3weaver", "titleweaver");
	makeVideo(_scenes[scPtr++], "titleweaver", "miketest/sc00");

	makeMikeDecision(_scenes[scPtr], 2);
	_scenes[scPtr]._sceneName = "miketest/sc00";
	_scenes[scPtr]._decisionBitmap = "DEC00";
	_scenes[scPtr]._choices[0]._sceneName = "miketest/sc01";
	_scenes[scPtr++]._choices[1]._sceneName = "miketest/sc07a";
	
	for (uint scNo = 1; scNo <= 13; scNo++, scPtr++) {
		Common::String imgScene = scNo == 5 ?
			"sc44" : Common::String::format("sc%02d", scNo);
		_scenes[scPtr]._bitmapNum = imgCounter[imgScene];
		_scenes[scPtr]._startBitmap = firstImg[imgScene];
		_scenes[scPtr]._sceneName = scNo == 5 ? "miketest/sc04a" : Common::String::format("miketest/sc%02d", scNo);
		_scenes[scPtr]._waveFilename = Common::String::format("DIA%02d.aiff", scNo == 5 ? 4 : scNo);
		_scenes[scPtr]._style = Scene::STYLE_PC;
		_scenes[scPtr]._decisionChoices = 1;
		switch(scNo) {
		case 4:
		case 5:
			_scenes[scPtr]._choices[0]._sceneName = "miketest/sc06";
			break;
		case 11:
			_scenes[scPtr]._choices[0]._sceneName = "miketest/sc13";
			break;
		case 13:
			_scenes[scPtr]._choices[0]._sceneName = "tuntest/dec/dec13";
			break;
		case 7:
		case 8:
		case 12:
			_scenes[scPtr]._choices[0]._sceneName = Common::String::format("miketest/sc%02da", scNo);
			break;
		default:
			_scenes[scPtr]._choices[0]._sceneName = Common::String::format("miketest/sc%02d", scNo + 1);
			break;
		}
	}

	makeMikeDecision(_scenes[scPtr], 3);
	_scenes[scPtr]._sceneName = "miketest/sc07a";
	_scenes[scPtr]._decisionBitmap = "DEC07";
	_scenes[scPtr]._choices[0]._sceneName = "miketest/sc08";
	_scenes[scPtr]._choices[0]._points = -10000;
	_scenes[scPtr]._choices[1]._sceneName = "miketest/sc11";
	_scenes[scPtr]._choices[1]._points = 10000;
	_scenes[scPtr]._choices[2]._sceneName = "miketest/sc12";
	_scenes[scPtr]._choices[2]._points = -20000;
	scPtr++;

	makeMikeDecision(_scenes[scPtr], 2);
	_scenes[scPtr]._sceneName = "miketest/sc08a";
	_scenes[scPtr]._decisionBitmap = "DEC08";
	_scenes[scPtr]._choices[0]._sceneName = "miketest/sc09";
	_scenes[scPtr]._choices[0]._points = 0;
	_scenes[scPtr]._choices[1]._sceneName = "miketest/sc09";
	_scenes[scPtr]._choices[1]._points = 10000;
	scPtr++;

	makeMikeDecision(_scenes[scPtr], 2);
	_scenes[scPtr]._sceneName = "miketest/sc12a";
	_scenes[scPtr]._decisionBitmap = "DEC12";
	_scenes[scPtr]._choices[0]._sceneName = "miketest/sc07a";
	_scenes[scPtr]._choices[0]._points = 0;
	_scenes[scPtr]._choices[1]._sceneName = "restart";
	_scenes[scPtr]._choices[1]._points = 0;
	scPtr++;
	
	for (uint scNo = 15; scNo <= 32; scNo++) {
		// there is no sc19
		if (scNo == 19)
			continue;
		Common::String imgScene = Common::String::format("sc%02d", scNo);
		_scenes[scPtr]._bitmapNum = imgCounter[imgScene];
		_scenes[scPtr]._startBitmap = firstImg[imgScene];
		_scenes[scPtr]._sceneName = Common::String::format("tuntest/sc%02d", scNo);
		_scenes[scPtr]._waveFilename = Common::String::format("sc%02d.aiff", scNo);
		_scenes[scPtr]._style = Scene::STYLE_PC;
		_scenes[scPtr]._decisionChoices = 1;
		if (scNo == 32)
			_scenes[scPtr]._choices[0]._sceneName = "end";
		else if (scNo == 16 || scNo == 18 || scNo == 20 || scNo == 22 || scNo == 23 || scNo == 25
		    || scNo == 26 || scNo == 28 || scNo == 29 || scNo == 30 || scNo == 31)
			_scenes[scPtr]._choices[0]._sceneName = Common::String::format("tuntest/dec/dec%02d", scNo);
		else
			_scenes[scPtr]._choices[0]._sceneName = Common::String::format("tuntest/sc%02d", scNo + 1);
		scPtr++;
	}

	file.seek(0x20290);

	for (int i = 0; i < 26; i++) {
		char buf[16];
		file.read(buf, 16);
		uint32 x = file.readUint32BE();
		uint32 y = file.readUint32BE();
		uint32 w = file.readUint32BE();
		uint32 h = file.readUint32BE();
		Common::String shortName = Common::String(buf).substr(0, 5);
		Common::String sceneName = "tuntest/dec/" + shortName;
		if (i == 0 || _scenes[scPtr - 1]._sceneName != sceneName) {
			_scenes[scPtr]._bitmapNum = 0;
			_scenes[scPtr]._startBitmap = 0;
			_scenes[scPtr]._decisionChoices = 0;
			_scenes[scPtr]._waveFilename = "";
			_scenes[scPtr]._style = Scene::STYLE_DECISION_TUN;
			_scenes[scPtr]._sceneName = sceneName;
			_scenes[scPtr]._decisionBitmap = shortName;
			scPtr++;
		}

		Scene &scene = _scenes[scPtr - 1];
		assert(scene._decisionChoices < kMaxChoice);
		scene._choices[scene._decisionChoices]._region = Common::Rect(x, y, x + w, y + h);
		for (uint j = 0 ; j < ARRAYSIZE(tungraph); j++) {
			if (shortName == tungraph[j].from) {
				Common::String target = tungraph[j].to[scene._decisionChoices].scene;
				if (target[0] == 's')
					scene._choices[scene._decisionChoices]._sceneName = "tuntest/" + target;
				else
					scene._choices[scene._decisionChoices]._sceneName = "tuntest/dec/" + target;
				scene._choices[scene._decisionChoices]._points = tungraph[j].to[scene._decisionChoices].points;
				break;
			}
		}
		scene._decisionChoices++;
	}

	_totScene = scPtr;
}

int PlumbersGame::getSceneNumb(const Common::String &sName) {
	debugC(1, kDebugGeneral, "%s : %s", __FUNCTION__, sName.c_str());
	if (sName == "miketest/sc04" && _cheatEnabled)
	        return getSceneNumb("miketest/sc04a");

	for (int sCurScene = 0; sCurScene < _totScene; sCurScene++) {
		if (sName == _scenes[sCurScene]._sceneName)
			return sCurScene;
	}
	return 0;
}

} // End of namespace Plumbers
