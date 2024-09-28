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

#include "darkseed/cutscene.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

Cutscene::~Cutscene() {
	delete _titleFont;
}

void Cutscene::play(char cutsceneId) {
	_cutsceneId = cutsceneId;
	_movieStep = 1;
	g_engine->_sound->waitForSpeech();
	g_engine->_cursor.showCursor(false);
	g_engine->fadeOut();
}


void Cutscene::update() {
	switch (_cutsceneId) {
	case 'B' :
		_movieStep = 9999;
		break;
	case 'C' :
		_movieStep = 9999;
		break;
	case 'D' :
		_movieStep = 9999;
		break;
	case 'E' :
		shipLaunchScene();
		break;
	case 'G' :
		_movieStep = 9999;
		break;
	case 'H' :
		_movieStep = 9999;
		break;
	case 'I' :
		introScene();
		break;
	case 'J' :
		embryoInsertedScene();
		break;
	case 'Y' :
		alienBornScene();
		break;
	case 'Z' :
		_movieStep = 9999;
		break;
	}

	if (_movieStep == 9999) {
		if (_cutsceneId == 'E') {
			g_engine->_previousRoomNumber = 38;
			g_engine->changeToRoom(7);
		} else if (_cutsceneId == 'Z') {
			g_engine->restartGame();
		} else if (_cutsceneId == 'I') {
			play('J');
		} else if (_cutsceneId == 'J') {
			delete _titleFont;
			_titleFont = nullptr;
			g_engine->newGame();
		} else if (_cutsceneId == 'Y') {
			play('I');
		} else if (_cutsceneId == 'G') {
			g_engine->_cursor.showCursor(true);
		}
	}
}

static constexpr int CREDITS_DELAY = 25;

bool Cutscene::introScene() {
	switch (_movieStep) {
	case 1:
		g_engine->fadeOut();
		break;
	case 2:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 3: {
		g_engine->_screen->clear();
		_palette.load("art/house.pal");
		if (_titleFont == nullptr) {
			_titleFont = new TitleFont();
		}
		_titleFont->displayString(68, 160, "DEVELOPING NEW WAYS TO AMAZE");
		g_engine->fadeIn(_palette);
	}
	break;
	case 4:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 5:
		g_engine->fadeOut();
		break;
	case 6:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 7:
		g_engine->_screen->clear();
		_titleFont->displayString(222, 160, "CYBERDREAMS");
		g_engine->fadeIn(_palette);
		break;
	case 8:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 9:
		g_engine->fadeOut();
		break;
	case 10:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 11:
		g_engine->_screen->clear();
		_titleFont->displayString(250, 160, "PRESENTS");
		g_engine->fadeIn(_palette);
		break;
	case 12:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 13:
		g_engine->fadeOut();
		break;
	case 14:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 15: {
		g_engine->_screen->clear();
		g_engine->_screen->clearPalette();
		g_engine->_screen->makeAllDirty();
		_palette.load("art/ship.pal", false);
		Img left00Img;
		left00Img.load("art/left00.img");
		left00Img.draw();
		Img left01Img;
		left01Img.load("art/left01.img");
		left01Img.draw();
		Img i001Img;
		i001Img.load("art/i001.img");
		i001Img.draw(1);
		break;
	}
	case 16:
		g_engine->fadeIn(_palette);
		break;
	case 17:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 18:
		_animation.load("art/shipin.anm");
		_animIdx = 0;
		_animCount = 47;
		runAnim();
		break;
	case 19:
		if (stepAnim()) {
			return true;
		}
		break;
	case 20:
		_animIdx = 47;
		_animCount = 29;
		runAnim();
		break;
	case 21:
		if (stepAnim()) {
			return true;
		}
		break;
	case 22:
		_animation.load("art/t2.anm");
		_animIdx = 0;
		_animCount = 50;
		runAnim();
		break;
	case 23:
		if (stepAnim()) {
			return true;
		}
		break;
	case 24: {
		g_engine->_screen->clear();
		g_engine->_screen->makeAllDirty();
		_palette.load("art/house.pal");
		Img tmImg;
		tmImg.load("art/tm.img");
		tmImg.draw();
		Img versionImg;
		versionImg.load("art/version.img");
		versionImg.draw();
		putHouse();
		Img titleImg;
		titleImg.load("art/title.img");
		titleImg.draw(1);
		versionImg.load("art/version.img");
		versionImg.draw();
		registTime();
		break;
	}
	case 25:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 26:
		putHouse();
		registTime();
		g_engine->_screen->makeAllDirty();
		break;
	case 27:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 28:
		putHouse();
		registTime();
		_titleFont->displayString(155, 90, "EXECUTIVE PRODUCERS");
		_titleFont->displayString(200, 130, "PATRICK KETCHUM");
		_titleFont->displayString(236, 160, "ROLF KLUG");
		_titleFont->displayString(236, 190, "JEAN KLUG");
		g_engine->_screen->makeAllDirty();
		break;
	case 29:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 30:
		putHouse();
		registTime();
		_titleFont->displayString(236, 95, "PRODUCERS");
		_titleFont->displayString(200, 135, "HARALD SEELEY");
		_titleFont->displayString(218, 165, "MIKE DAWSON");
		g_engine->_screen->makeAllDirty();
		break;
	case 31:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 32:
		putHouse();
		registTime();
		_titleFont->displayString(245, 95, "DESIGNERS");
		_titleFont->displayString(209, 135, "MIKE CRANFORD");
		_titleFont->displayString(227, 165, "MIKE DAWSON");
		g_engine->_screen->makeAllDirty();
		break;
	case 33:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 34:
		putHouse();
		registTime();
		_titleFont->displayString(227, 95, "PROGRAMMERS");
		_titleFont->displayString(164, 135, "LENNARD FEDDERSEN");
		_titleFont->displayString(227, 165, "JOHN KRAUSE");
		_titleFont->displayString(245, 195, "GARY VICK");
		g_engine->_screen->makeAllDirty();
		break;
	case 35:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 36:
		putHouse();
		registTime();
		_titleFont->displayString(200, 100, "MUSICAL SCORE");
		_titleFont->displayString(200, 140, "GREGORY ALPER");
		g_engine->_screen->makeAllDirty();
		break;
	case 37:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 38:
		putHouse();
		registTime();
		_titleFont->displayString(119, 100, "MUSIC AND SOUND EFFECTS");
		_titleFont->displayString(200, 140, "DAVID A. BEAN");
		g_engine->_screen->makeAllDirty();
		break;
	case 39:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 40:
		putHouse();
		registTime();
		_titleFont->displayString(218, 100, "ART DIRECTOR");
		_titleFont->displayString(236, 140, "BRUMMBAER");
		g_engine->_screen->makeAllDirty();
		break;
	case 41:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 42:
		putHouse();
		registTime();
		_titleFont->displayString(164, 100, "ASST. ART DIRECTOR");
		_titleFont->displayString(191, 140, "PAUL DRZEWIECKI");
		g_engine->_screen->makeAllDirty();
		break;
	case 43:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 44:
		putHouse();
		registTime();
		_titleFont->displayString(200, 100, "DARK WORLD ART");
		_titleFont->displayString(245, 140, "H.R. GIGER");
		g_engine->_screen->makeAllDirty();
		break;
	case 45:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 46:
		putHouse();
		registTime();
		_titleFont->displayString(182, 90, "COMPUTER ARTISTS");
		_titleFont->displayString(227, 130, "JULIA ULANO");
		_titleFont->displayString(191, 160, "JOBY ROME-OTERO");
		_titleFont->displayString(236, 190, "PAUL RYAN");
		g_engine->_screen->makeAllDirty();
		break;
	case 47:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 48:
		putHouse();
		registTime();
		_titleFont->displayString(236, 100, "GAME TEXT");
		_titleFont->displayString(209, 140, "MICHEL HORVAT");
		g_engine->_screen->makeAllDirty();
		break;
	case 49:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		break;
	case 50: // TODO wait for music.
		break;
	case 51:
		g_engine->fadeOut();
		break;
	case 52:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	default:
		_movieStep = 9999;
		return false;
	}
	_movieStep++;
	if (g_engine->_isLeftMouseClicked || g_engine->_isRightMouseClicked) {
		g_engine->zeromousebuttons();
		if (_movieStep < 51) {
			_movieStep = 51;
		}
	}
	return true;
}

bool Cutscene::embryoInsertedScene() {
	switch (_movieStep) {
	case 1:
	case 2:
		break;
	case 3:
		g_engine->_screen->clear();
		g_engine->_screen->makeAllDirty();
		_palette.load("art/ship.pal");
		_animation.load("art/nm1.anm");
		g_engine->_screen->clearPalette();
		break;
	case 4: {
		Img tmImg;
		tmImg.load("art/nmf0.img");
		tmImg.draw();
		Img nmf1Img;
		nmf1Img.load("art/nmf1.img");
		nmf1Img.draw();
		break;
	}
	case 5: {
		Img tmImg;
		tmImg.load("art/nm101.img");
		tmImg.draw(1);
		break;
	}
	case 6:
		// TODO play implant music here.
		registTime();
		g_engine->fadeIn(_palette);
		break;
	case 7:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 8:
		_animIdx = 0;
		_animCount = 39;
		runAnim();
		break;
	case 9:
		if (stepAnim()) {
			if (g_engine->_isLeftMouseClicked || g_engine->_isRightMouseClicked) {
				g_engine->zeromousebuttons();
				if (_movieStep < 16) {
					_movieStep = 16;
				}
			}
			return true;
		}
		registTime();
		break;
	case 10:
		if (waitTime(30)) {
			return true;
		}
		break;
	case 11:
		g_engine->fadeOut();
		break;
	case 12:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 13: {
		g_engine->_screen->clear();
		_palette.load("art/house.pal");
		if (_titleFont == nullptr) {
			_titleFont = new TitleFont();
		}
		_titleFont->displayString(80, 130, "AFTER A HORRIFYING NIGHTMARE");
		_titleFont->displayString(80, 170, "MIKE DAWSON AWAKENS TO THE");
		_titleFont->displayString(80, 210, "FIRST DAY IN HIS NEW HOUSE...");
		g_engine->fadeIn(_palette);
		break;
	}
	case 14:
		if (g_engine->fadeStep()) {
			return true;
		}
		registTime();
		break;
	case 15:
		if (waitTime(30)) {
			return true;
		}
		break;
	case 16:
		g_engine->fadeOut();
		break;
	case 17:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	default:
		_movieStep = 9999;
		return false;
	}
	_movieStep++;
	if (g_engine->_isLeftMouseClicked || g_engine->_isRightMouseClicked) {
		g_engine->zeromousebuttons();
		if (_movieStep < 16) {
			_movieStep = 16;
		}
	}
	return true;
}

bool Cutscene::shipLaunchScene() {
	switch (_movieStep) {
	case 1: {
		_palette.load("art/ship.pal");
		_animation.load("art/left.anm");
		Img left00Img;
		left00Img.load("art/left00.img");
		left00Img.draw();
		Img left01Img;
		left01Img.load("art/left01.img");
		left01Img.draw();
		g_engine->_screen->clearPalette();
		break;
	}
	case 2:
		g_engine->fadeIn(_palette);
		break;
	case 3:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 4:
		// TODO play music 'launch'
		_animIdx = 2;
		_animCount = 49;
		runAnim();
		break;
	case 5:
		if (stepAnim(1)) {
			return true;
		}
		break;
	case 6:
		_animation.load("art/shipin.anm");
		_animIdx = 75;
		_animCount = 29;
		runAnim(-1);
		break;
	case 7:
		if (stepAnim()) {
			return true;
		}
		registTime();
		break;
	case 8:
		if (waitTime(20)) {
			return true;
		}
		break;
	case 9:
		_animIdx = 46;
		_animCount = 47;
		runAnim(-1);
		break;
	case 10:
		if (stepAnim()) {
			return true;
		}
		break;
	case 11:
		_animation.load("art/left2.anm");
		_animIdx = 0;
		_animCount = 23;
		runAnim();
		break;
	case 12:
		if (stepAnim()) {
			return true;
		}
		break;
	case 13:
		g_engine->fadeOut();
		break;
	case 14:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	default:
		_movieStep = 9999;
		return false;
	}
	_movieStep++;
	return true;
}

bool Cutscene::alienBornScene() {
	switch (_movieStep) {
	case 1: {
		_palette.load("art/ship.pal");
		Img left00Img;
		left00Img.load("art/nmf0.img");
		left00Img.draw();
		Img left01Img;
		left01Img.load("art/nmf1.img");
		left01Img.draw();
		Img born1Img;
		born1Img.load("art/born01.img");
		born1Img.draw(1);
		g_engine->_screen->clearPalette();
		break;
	}
	case 2:
		_animation.load("art/born.anm");
		// TODO play alien music here.
		g_engine->fadeIn(_palette);
		break;
	case 3:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 4:
		_animIdx = 0;
		_animCount = 31;
		// TODO speed = 2
		runAnim();
		break;
	case 5:
		if (stepAnim()) {
			return true;
		}
		registTime();
		break;
	case 6:
		if (waitTime(30)) {
			return true;
		}
		break;
	case 7:
		g_engine->fadeOut();
		break;
	case 8:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 9:
		// TODO some logic here. stopSequence.
		break;
	default:
		_movieStep = 9999;
		return false;
	}
	_movieStep++;
	return true;
}

bool Cutscene::stepAnim(int drawMode) {
	if (_animDelayCount == 0) {
		Img animFrame;
		_animation.getImg(_animIdx, animFrame);
		animFrame.draw(drawMode);
		_animIdx += _animDirection;
		_animCount--;
	}

	_animDelayCount++;
	if (_animDelayCount == 6) {
		_animDelayCount = 0;
	}

	if (_animCount > 0) {
		return true;
	}
	return false;
}

void Cutscene::putHouse() {
	Img dollImg;
	dollImg.load("art/bdoll0.img");
	dollImg.draw();
	Img doll1Img;
	doll1Img.load("art/bdoll1.img");
	doll1Img.draw();
	Img houseFrameImg;
	houseFrameImg.load("art/house.img");
	houseFrameImg.draw(1);
}

void Cutscene::registTime() {
	_startTime = g_system->getMillis();
}

bool Cutscene::waitTime(int16 duration) {
	return g_system->getMillis() < _startTime + (duration * 100);
}

void Cutscene::runAnim(int direction) {
	_animDelayCount = 0;
	_animDirection = direction;
}

} // End of namespace Darkseed
