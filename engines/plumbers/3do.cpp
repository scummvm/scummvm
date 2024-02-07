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

#include "image/cel_3do.h"

#include "video/3do_decoder.h"

namespace Plumbers {
// TODO(3do):
// * effect when changing between scenes
// * 3do boot logo
// * return to previous scene
// * hide cursor

namespace {
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

void makeMikeDecision(Scene &scene, uint num) {
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

void makeVideo(Scene &scene, const Common::String &videoName, const Common::String &nextScene) {
	scene._bitmapNum = 0;
	scene._startBitmap = 0;
	scene._decisionChoices = 1;
	scene._waveFilename = "";
	scene._style = Scene::STYLE_VIDEO;
	scene._sceneName = videoName;
	scene._decisionBitmap = "";
	scene._choices[0]._sceneName = nextScene;
}

const struct {
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
}  // end of anonymous namespace

PlumbersGame3DO::PlumbersGame3DO(OSystem *syst, const ADGameDescription *gameDesc) :
	PlumbersGame(syst, gameDesc), _ctrlHelpImage(nullptr), _cheatEnabled(false), _cheatFSM(0), _leftShoulderPressed(false),
	_hiLite(-1), _mouseHiLite(-1), _kbdHiLite(-1) {
}

void PlumbersGame3DO::readTables() {
	Common::File file;
	if (!file.open("launchme"))
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
		Common::String shortName = Common::String(buf, 5);
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

void PlumbersGame3DO::loadMikeDecision(const Common::String &dirname, const Common::String &baseFilename, uint num) {
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
		Common::Path nameP(Common::String::format("%s%dP.CEL", baseName.c_str(), i + 1));
		if (!fileP.open(nameP))
			error("unable to load image %s", nameP.toString().c_str());

		_image->loadStream(fileP);
		surf->copyRectToSurface(*_image->getSurface(), p.x, p.y,
					Common::Rect(0, 0, sz.x, sz.y));

		Common::File fileW;
		Common::Path nameW(Common::String::format("%s%dW.CEL", baseName.c_str(), i + 1));
		if (!fileW.open(nameW))
			error("unable to load image %s", nameW.toString().c_str());

		_image->loadStream(fileW);
		surf->copyRectToSurface(*_image->getSurface(), p.x + sz.x, p.y,
					Common::Rect(0, 0, sz.x, sz.y));
	}

	_compositeSurface = surf;

	Common::File fileCtrl;
	if (fileCtrl.open(Common::Path(dirname + "/CONTROLHELP.CEL")))
		_ctrlHelpImage->loadStream(fileCtrl);
}

void PlumbersGame3DO::postSceneBitmaps() {
	if (_scenes[_curSceneIdx]._style == Scene::STYLE_VIDEO) {
		_videoDecoder = new Video::ThreeDOMovieDecoder();
		_curChoice = 0;
		if (!_videoDecoder->loadFile(Common::Path(_scenes[_curSceneIdx]._sceneName))) {
			_actions.push(ChangeScene);
			return;
		}
		_videoDecoder->start();
		return;
	}

	if (_scenes[_curSceneIdx]._decisionChoices == 1) {
		_curChoice = 0;
		_actions.push(ChangeScene);
		return;
	}
	
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

void PlumbersGame3DO::startGraphics() {
	_image = new Image::Cel3DODecoder();
	_ctrlHelpImage = new Image::Cel3DODecoder();
	_screenW = 320;
	_screenH = 240;
	Graphics::PixelFormat pf(2, 5, 5, 5, 1, 10,  5,  0, 15);
	initGraphics(_screenW, _screenH, &pf);
}

void PlumbersGame3DO::blitImage() {
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

	if (_hiLite >= 0 && _leftButtonDownFl && !ctrlHelp) {
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

	blitImageSurface(modded ? &modSurf : surface);
}

void PlumbersGame3DO::skipVideo() {
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

void PlumbersGame3DO::joyUp() {
	int decNum = _scenes[_curSceneIdx]._decisionChoices;
	if (!_leftButtonDownFl)
		return;
	_kbdHiLite = _kbdHiLite < 0 ? 0 : (_kbdHiLite + decNum - 1) % decNum;
	_hiLite = _kbdHiLite;
	updateHiLite();
}

void PlumbersGame3DO::joyDown() {
	if (!_leftButtonDownFl)
		return;
	int decNum = _scenes[_curSceneIdx]._decisionChoices;
	_kbdHiLite = _kbdHiLite < 0 ? 0 : (_kbdHiLite + 1) % decNum;
	_hiLite = _kbdHiLite;
	updateHiLite();
}

void PlumbersGame3DO::joyA() {
	if (_kbdHiLite < 0 || !_leftButtonDownFl)
		return;
	debugC(5, kDebugGeneral, "Accepting enter press with choice = %d", _kbdHiLite);
	_curChoice = _kbdHiLite;
	_totScore += _scenes[_curSceneIdx]._choices[_kbdHiLite]._points;
	_actions.push(ChangeScene);
	_leftButtonDownFl = false;
}

void PlumbersGame3DO::handleEvent(const Common::Event &event) {
	switch (event.type) {
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
			return;
		}
		if (event.joystick.button == Common::JOYSTICK_BUTTON_DPAD_UP ||
		    event.joystick.button == Common::JOYSTICK_BUTTON_DPAD_LEFT) {
			joyUp();
			return;
		}
		if (event.joystick.button == Common::JOYSTICK_BUTTON_DPAD_DOWN ||
		    event.joystick.button == Common::JOYSTICK_BUTTON_DPAD_RIGHT) {
			joyDown();
			return;
		}
		if (event.joystick.button == Common::JOYSTICK_BUTTON_A) {
			joyA();
			return;
		}
		if (event.joystick.button == Common::JOYSTICK_BUTTON_LEFT_SHOULDER) {
			_leftShoulderPressed = true;
			if (_leftButtonDownFl && _ctrlHelpImage)
				_actions.push(Redraw);
			return;
		}
		break;
	case Common::EVENT_JOYBUTTON_UP:
		if (event.joystick.button == Common::JOYSTICK_BUTTON_LEFT_SHOULDER) {
			_leftShoulderPressed = false;
			if (_leftButtonDownFl && _ctrlHelpImage)
				_actions.push(Redraw);
			return;
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
			return;
		}
		if ((event.kbd.keycode == Common::KEYCODE_UP ||
		     event.kbd.keycode == Common::KEYCODE_LEFT)) {
			joyUp();
			return;
		}
		if ((event.kbd.keycode == Common::KEYCODE_DOWN ||
		     event.kbd.keycode == Common::KEYCODE_RIGHT)) {
			joyDown();
			return;
		}
		if (event.kbd.keycode == Common::KEYCODE_RETURN) {
			joyA();
			return;
		}
		if (event.kbd.keycode == Common::KEYCODE_q) {
			_leftShoulderPressed = true;
			if (_leftButtonDownFl && _ctrlHelpImage)
				_actions.push(Redraw);
			return;
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
	PlumbersGame::handleEvent(event);
}

int PlumbersGame3DO::getSceneNumb(const Common::String &sName) {
	debugC(1, kDebugGeneral, "%s : %s", __FUNCTION__, sName.c_str());
	if (sName == "miketest/sc04" && _cheatEnabled)
	        return PlumbersGame::getSceneNumb("miketest/sc04a");

	return PlumbersGame::getSceneNumb(sName);
}

void PlumbersGame3DO::updateHiLite() {
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

void PlumbersGame3DO::preActions() {
	if (_leftButtonDownFl) {
		int nh = getMouseHiLite();
		if (nh != _mouseHiLite) {
			_mouseHiLite = nh;
			_hiLite = _mouseHiLite;
			updateHiLite();
		}
	}
}
}
