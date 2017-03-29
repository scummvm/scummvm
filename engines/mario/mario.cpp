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
#include "common/system.h"

#include "engines/util.h"

#include "mario/mario.h"

#include "common/debug.h"

namespace Mario {

MarioGame::MarioGame(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_image = nullptr;

	xadj = 0;
	yadj = 0;

#if 0
	rect.left = xadj + 10;
	rect.top = yadj + 440;
	rect.right = xadj + 200;
	rect.bottom = yadj + 460;
#endif
}

MarioGame::~MarioGame() {
	if (_image)
		delete _image;
}

Common::Error MarioGame::run() {
	initGraphics(640, 480, false);
	g_system->showMouse(true);

	sReadTables("game.bin");

	bShowScore = false;
	bLButtonDown = false;
	bEnd = false;
	Game.lTotScore = 0;
	sCurScene = 0;
	sPrvScene = 0;
	sCurChoice = 0;
	_actions.clear();
	_actions.push(ShowScene);

	bool quit = false;

	while (!quit && !bEnd) {
		g_system->delayMillis(100);

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
				quit = true;
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (bLButtonDown) {
					Common::Point mousePos = g_system->getEventManager()->getMousePos();
					for (sCurChoice = 0; sCurChoice < aScene[sCurScene].sNoDecisionChoices; sCurChoice++) {
						if (aScene[sCurScene].aChoice[sCurChoice].aRegion.contains(mousePos))
							break;
					}
					if (sCurChoice < kMaxChoice) {
						debug("Accepting mouse click at %d : %d , choice = %d", mousePos.x, mousePos.y, sCurChoice);
						Game.lTotScore += aScene[sCurScene].aChoice[sCurChoice].lPoints;
						_actions.push(ChangeScene);
						bLButtonDown = false;
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

bool MarioGame::loadImage(char *dirname, char *filename) {
	if (_image) {
		delete _image;
		_image = nullptr;
	}

	const Common::String name = Common::String::format("%s/%s", dirname, filename);
	debug("%s : %s", __FUNCTION__, name.c_str());
	Common::File *file = new Common::File();
	if (!file->open(name)) {
		delete file;
		error("unable to load image %s", name.c_str());
		return false;
	}

	_image = new Image::BitmapDecoder();
	_image->loadStream(*file);
	file->close();
	delete file;
	return true;
}

void MarioGame::drawScreen() {
	debug("%s : %s", __FUNCTION__, _image ? "YES" : "NO");
	if (_image) {
		if (bSetDuration == true) {
			g_system->getTimerManager()->removeTimerProc(onTimer);
			g_system->getTimerManager()->installTimerProc(onTimer, aBitmaps[sCurBitmap].sBitmapDuration * 100 * 1000, this, "timer");
			_actions.push(UpdateScene);
		}

		const Graphics::Surface *surface = _image->getSurface();

		//TODO sometimes a picture is not 640x480 (SC01/0119.BMP), crop/strectch/center it?
		int w = CLIP<int>(surface->w, 0, 640);
		int h = CLIP<int>(surface->h, 0, 480);

		g_system->copyRectToScreen(surface->getPixels(), surface->pitch, 0, 0, w, h);
		g_system->getPaletteManager()->setPalette(_image->getPalette(), 0, 256);
		g_system->updateScreen();
#if 0
		//TODO this should be drawn on top of picture
		if (bShowScore == true)
		{
			char	szBuffer[24];
			HBRUSH	hBrush;
			HRGN	hrgn;
			hrgn = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
			SelectObject(hdc, hrgn);

			hBrush = GetStockObject(WHITE_BRUSH);
			FillRgn(hdc, hrgn, hBrush);
			DeleteObject(hBrush);

			hBrush = CreateSolidBrush(RGB(255, 0, 0));
			FrameRgn(hdc, hrgn, hBrush, 2, 2);
			DeleteObject(hrgn);
			DeleteObject(hBrush);

			DrawText(hdc, szBuffer, wsprintf(szBuffer, "Your Score is: %ld", Game.lTotScore), &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

			bShowScore = FALSE;
		}
#endif
	}
}

void MarioGame::playSound() {
	const Common::String name = Common::String::format("%s/%s", aScene[sCurScene].cSceneName, aScene[sCurScene].cWaveFileName);
	debug("%s : %s", __FUNCTION__, name.c_str());
	Common::File *file = new Common::File();
	if (!file->open(name)) {
		delete file;
		error("unable to load sound %s", name.c_str());
	}

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
	debug("%s : %d", __FUNCTION__, sCurScene);
	sCurBitmap = aScene[sCurScene].sStartBitmap;
	loadImage(aScene[sCurScene].cSceneName, aBitmaps[sCurBitmap].cBitmapFileName);
	_actions.push(Redraw);
	bSetDuration = true;
	_actions.push(PlaySound);
}

void MarioGame::updateScene() {
	debug("%s : %d", __FUNCTION__, sCurBitmap);
	sCurBitmap++;
	if (sCurBitmap >= aScene[sCurScene].sStartBitmap + aScene[sCurScene].sNoBitmaps) {
		if (aScene[sCurScene].sNoDecisionChoices == 1) {
			sCurChoice = 0;
			_actions.push(ChangeScene);
		} else {
			bShowScore = true;
			bLButtonDown = true;
			bSetDuration = false;
			loadImage(aScene[sCurScene].cSceneName, aScene[sCurScene].cDecisionBitmap);
		}
	} else {
		loadImage(aScene[sCurScene].cSceneName, aBitmaps[sCurBitmap].cBitmapFileName);
		bSetDuration = true;
	}
}

void MarioGame::changeScene() {
	debug("%s : %d", __FUNCTION__, sCurChoice);
	if (aScene[sCurScene].aChoice[sCurChoice].sGoTo == -1) {
		sCurScene = sPrvScene;
		sCurBitmap = 9999;
		_actions.push(UpdateScene);
		_actions.push(Redraw);
	} else if (aScene[sCurScene].aChoice[sCurChoice].sGoTo == 32767) {
		bEnd = true;
	} else {
		if (aScene[sCurScene].sNoDecisionChoices > 1)
			sPrvScene = sCurScene;
		if (aScene[sCurScene].aChoice[sCurChoice].bSkipScene) {
			sCurScene = sGetSceneNumb(aScene[sCurScene].aChoice[sCurChoice].sGoTo);
			sCurBitmap = 9999;
			_actions.push(UpdateScene);
			_actions.push(Redraw);
			g_system->getTimerManager()->removeTimerProc(onTimer);
		} else {
			sCurScene = sGetSceneNumb(aScene[sCurScene].aChoice[sCurChoice].sGoTo);
			_actions.push(ShowScene);
		}
	}
}

void MarioGame::processTimer() {
	debug("%s", __FUNCTION__);
	if (!bEnd)
		_actions.push(Redraw);
	//else
	//	PostMessage(wnd, WM_DESTROY, 0, 0);
}

void MarioGame::onTimer(void *arg) {
	g_system->getTimerManager()->removeTimerProc(onTimer);
	((MarioGame*)arg)->processTimer();
}

void MarioGame::sInitTables() {
	memset(&Game, 0, sizeof(Game));
	memset(aScene, 0, sizeof(aScene));
	memset(aBitmaps, 0, sizeof(aBitmaps));
}

void MarioGame::sReadTables(char *cFileName) {
	Common::File file;
	if (!file.open(cFileName))
		error("sReadTables(): Error reading BIN file");

	sInitTables();

	Game.lTotScore = file.readSint32LE();
	Game.sPrevScene = file.readSint16LE();
	Game.sCurrScene = file.readSint16LE();
	Game.sCurrSceneStep = file.readSint16LE();
	Game.sLkhdScene = file.readSint16LE();
	Game.sLkhdBitmapNo = file.readSint16LE();
	Game.sTotScene = file.readSint16LE();
	Game.sTotBitmap = file.readSint16LE();
	Game.bEightBit = file.readSint16LE();
	Game.bIsaDecision = file.readSint16LE();

	for (int i = 0; i < kMaxScene; i++) {
		aScene[i].sNoBitmaps = file.readSint16LE();
		aScene[i].sStartBitmap = file.readSint16LE();
		aScene[i].sNoDecisionChoices = file.readSint16LE();
		file.read(aScene[i].cSceneName, kMaxName);
		file.read(aScene[i].cWaveFileName, kMaxName);
		file.read(aScene[i].cDecisionBitmap, kMaxName);
		for (int j = 0; j < kMaxChoice; j++) {
			aScene[i].aChoice[j].lPoints = file.readSint32LE();
			aScene[i].aChoice[j].sGoTo = file.readSint16LE();
			aScene[i].aChoice[j].bSkipScene = file.readSint16LE();
			int left = file.readSint16LE();
			int top = file.readSint16LE();
			int right = file.readSint16LE();
			int bottom = file.readSint16LE();
			aScene[i].aChoice[j].aRegion = Common::Rect(left, top, right, bottom);
		}
	}

	for (int i = 0; i < kMaxBitmaps; i++) {
		aBitmaps[i].sBitmapDuration = file.readSint16LE();
		file.read(aBitmaps[i].cBitmapFileName, kMaxName);
	}
	file.close();
}

int MarioGame::sGetSceneNumb(int sNo) {
	debug("%s : %d", __FUNCTION__, sNo);
	char cTestString[kMaxName];

	sprintf(cTestString, "SC%02d", sNo);
	for (int sCurScene = 0; sCurScene < Game.sTotScene; sCurScene++) {
		if (!strcmp(cTestString, aScene[sCurScene].cSceneName))
			return sCurScene;
	}
	return 0;
}

} // End of namespace Mario
