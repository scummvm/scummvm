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
#include "darkseed/langtext.h"

namespace Darkseed {

Cutscene::~Cutscene() {
	delete _titleFont;
}

void Cutscene::play(char cutsceneId) {
	_cutsceneId = cutsceneId;
	_movieStep = 1;
	g_engine->waitForSpeech();
	g_engine->_cursor.showCursor(false);
	g_engine->fadeOut();
}


void Cutscene::update() {
	switch (_cutsceneId) {
	case 'B' :
		nightmare2Scene();
		break;
	case 'C' :
		nightmare3Scene();
		break;
	case 'D' :
		babyDollScene();
		break;
	case 'E' :
		shipLaunchScene();
		break;
	case 'G' :
		bookScene();
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
	case 'H' :
	case 'Z' :
		_movieStep = 9999;
		break;
	default:
		break;
	}

	if (_movieStep == 9999) {
		g_engine->_sound->stopMusic();

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
		} else if (_cutsceneId == 'B' || _cutsceneId == 'C') {
			g_engine->_room->restorePalette();
			g_engine->_frame.draw();
			g_engine->doCircles();

			if (g_engine->_currentDay == 2) {
				g_engine->_console->printTosText(12);
			} else if (g_engine->_currentDay == 3) {
				g_engine->_console->printTosText(14);
			}
			g_engine->_console->draw();
			g_engine->_screen->update();

			g_engine->waitForSpeech();
			g_engine->_systemTimerCounter = 4;
			g_engine->_cursor.showCursor(true);
			g_engine->_room->loadRoomMusic();
		} else if (_cutsceneId == 'D' || _cutsceneId == 'G') {
			g_engine->_room->restorePalette();
			g_engine->_frame.draw();
			g_engine->_cursor.showCursor(true);
		}
	}
}

static constexpr int CREDITS_DELAY = 25;

bool Cutscene::introScene() {
	auto lang = g_engine->getLanguage();
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
		if (lang == Common::FR_FRA || lang == Common::DE_DEU) {
			displayTitleText({
				{0, 0, ""},
				{0, 0, ""},
				{41, 145, "DEVELOPPE DE NOUVELLES METHODE"},
				{68, 145, "DIE ENTWICKLUNG NEUER, SIE IN"}
			});
			displayTitleText({
				{0, 0, ""},
				{0, 0, ""},
				{142, 175, "POUR VOUS SURPRENDRE"},
				{60, 175, "ERSTAUNEN VERSETZENDER WEGE..."}
			});
		} else {
			displayTitleText({
				{68, 160, "DEVELOPING NEW WAYS TO AMAZE"},
				{0, 160, "CREANDO NUEVAS FORMAS DE DIVERTIRSE"},
				{0, 0, ""},
				{0, 0, ""}
			});
		}
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
		displayTitleText({
			{250, 160, "PRESENTS"},
			{250, 160, "PRESENTA"},
			{250, 160, "PRESENTE"},
			{232, 160, "PRAESENTIERT"}
		});
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
		g_engine->_sound->playMusic(StartMusicId::kCredits);
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
		if (lang == Common::KO_KOR) {
			_animation.load("art/ht2.anm");
			_animCount = 45;
		} else {
			_animation.load("art/t2.anm");
			_animCount = 50;
		}
		_animIdx = 0;
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
		titleImg.load(lang == Common::KO_KOR ? "art/htitle.img" : "art/title.img");
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
		if (lang == Common::DE_DEU) {
			_titleFont->displayString(185, 80, "VERANTWORTLICHE");
			_titleFont->displayString(218, 110, "PRODUZENTEN");

			_titleFont->displayString(200, 150, "PATRICK KETCHUM");
			_titleFont->displayString(236, 180, "ROLF KLUG");
			_titleFont->displayString(236, 210, "JEAN KLUG");
		} else {
			displayTitleText({
				{155, 90, "EXECUTIVE PRODUCERS"},
				{137, 90, "PRODUCTORES EJECUTIVOS"},
				{119, 90, "DIRECTEURS DE PRODUCTION"},
				{0, 0, ""}
			});

			_titleFont->displayString(200, 130, "PATRICK KETCHUM");
			_titleFont->displayString(236, 160, "ROLF KLUG");
			_titleFont->displayString(236, 190, "JEAN KLUG");
		}
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
		displayTitleText({
			{236, 95, "PRODUCERS"},
			{218, 95, "PRODUCTORES"},
			{218, 95, "PRODUCTEURS"},
			{218, 95, "PRODUZENTEN"}
		});
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
		displayTitleText({
			{245, 95, "DESIGNERS"},
			{227, 95, "DISENADORES"},
			{218, 95, "DESSINATEURS"},
			{254, 95, "DESIGNER"}
		});
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
		displayTitleText({
			{227, 95, "PROGRAMMERS"},
			{203, 95, "PROGRAMADORES"},
			{218, 95, "PROGRAMMEURS"},
			{206, 95, "PROGRAMMIERER"}
		});
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
		displayTitleText({
			{200, 100, "MUSICAL SCORE"},
			{206, 100, "BANDA SONORA"},
			{200, 100, "THEME MUSICAL"},
			{214, 100, "MUSIKSTUECKE"}
		});
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
		displayTitleText({
			{119, 100, "MUSIC AND SOUND EFFECTS"},
			{89, 100, "MUSICA Y EFECTOS DE SONIDO"},
			{92, 100, "MUSIQUE ET EFFETS SONORES"},
			{128, 100, "MUSIK-UND SOUNDEFFEKTE"}
		});
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
		if (lang == Common::DE_DEU) {
			_titleFont->displayString(239, 100, "KUNST-UND");
			_titleFont->displayString(167, 130, "ASSISTENZ DER KUNST-UND");
			_titleFont->displayString(236, 170, "BRUMMBAER");
		} else {
			displayTitleText({
				{218, 100, "ART DIRECTOR"},
				{176, 100, "DIRECTOR ARTISTICO"},
				{146, 100, "DIRECTEUR ARTISTIQUE"},
				{0, 100, ""}
			});
			_titleFont->displayString(236, 140, "BRUMMBAER");
		}
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
		if (lang == Common::EN_ANY || lang == Common::KO_KOR) {
			_titleFont->displayString(164, 100, "ASST. ART DIRECTOR");
			_titleFont->displayString(191, 140, "PAUL DRZEWIECKI");
		} else {
			displayTitleText({
				{0, 0, ""},
				{206, 100, "AYUDANTE DEL"},
				{218, 100, "ASSISTANT DU"},
				{119, 100, "ASSISTENZ DER KUNST-UND"}
			});
			displayTitleText({
				{0, 0, ""},
				{176, 130, "DIRECTOR ARTISTICO"},
				{150, 130, "DIRECTEUR ARTISTIQUE"},
				{167, 130, "ANIMATIONSDIREKTION"}
			});
			_titleFont->displayString(191, 170, "PAUL DRZEWIECKI");
		}
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
		displayTitleText({
			{200, 100, "DARK WORLD ART"},
			{104, 100, "GRAFICOS DEL MUNDO OSCURO"},
			{137, 100, "L'ART DU MONDE OBSCUR"},
			{188, 100, "DARK-WORLD-KUNST"}
		});
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
		if (lang == Common::FR_FRA) {
			_titleFont->displayString(182, 75, "LES ARTISTES SUR");
			_titleFont->displayString(236, 105, "ORDINATEUR");

			_titleFont->displayString(227, 145, "JULIA ULANO");
			_titleFont->displayString(191, 175, "JOBY ROME-OTERO");
			_titleFont->displayString(236, 205, "PAUL RYAN");
		} else {
			displayTitleText({
				{182, 90, "COMPUTER ARTISTS"},
				{242, 90, "GRAFISTAS"},
				{0, 0, ""},
				{107, 90, "COMPUTERGRAFIK-KUENSTLER"}
			});
			_titleFont->displayString(227, 130, "JULIA ULANO");
			_titleFont->displayString(191, 160, "JOBY ROME-OTERO");
			_titleFont->displayString(236, 190, "PAUL RYAN");
		}
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
		displayTitleText({
			{236, 100, "GAME TEXT"},
			{194, 100, "TEXTO DEL JUEGO"},
			{173, 100, "LE TEXTE DU JEUX"},
			{281, 100, "TEXTE"}
		});
		_titleFont->displayString(209, 140, "MICHEL HORVAT");
		g_engine->_screen->makeAllDirty();
		break;
	case 49:
		if (waitTime(CREDITS_DELAY)) {
			return true;
		}
		if (lang == Common::KO_KOR) {
			putHouse();
			Img distrib;
			distrib.load("art/distrib.img");
			distrib.draw(1);
			g_engine->_screen->makeAllDirty();
			registTime();
		}
		break;
	case 50:
		if (lang == Common::KO_KOR) {
			if (waitTime(CREDITS_DELAY * 2)) {
				return true;
			}
		}
		if (g_engine->_sound->isPlayingMusic()) {
			if (g_engine->_isLeftMouseClicked || g_engine->_isRightMouseClicked) {
				g_engine->zeroMouseButtons();
				_movieStep = 51;
			}
			return true;
		}
		break;
	case 51:
		g_engine->_sound->stopMusic();
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
		g_engine->zeroMouseButtons();
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
		g_engine->_sound->playMusic(StartMusicId::kImplant);
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
				g_engine->zeroMouseButtons();
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
		switch (g_engine->getLanguage()) {
		case Common::ES_ESP :
			_titleFont->displayString(80, 130, "TRAS UNA HORRIBLE PESADILLA");
			_titleFont->displayString(80, 170, "MIKE DAWSON SE DESPIERTA EL");
			_titleFont->displayString(80, 210, "PRIMER DIA EN SU NUEVA CASA...");
			break;
		case Common::FR_FRA :
			_titleFont->displayString(0, 110, "APRES UN CAUCHEMAAR EPOUVANTABLE");
			_titleFont->displayString(0, 150, "MIKE DAWSON SE REVEILLE POUR SA");
			_titleFont->displayString(0, 190, "PREMIERE JOURNEE DANS LA NOUVELLE");
			_titleFont->displayString(0, 230, "MAISON...");
			break;
		case Common::DE_DEU :
			_titleFont->displayString(44, 130, "NACH EINEM SCHRECKENERREGENDEN");
			_titleFont->displayString(44, 170, "ALPTRAUM MIKE DAWSON ERWACHT AM");
			_titleFont->displayString(44, 210, "ERSTEM TAG IN SEINEN NEUEM HAUS...");
			break;
		default :
			_titleFont->displayString(80, 130, "AFTER A HORRIFYING NIGHTMARE");
			_titleFont->displayString(80, 170, "MIKE DAWSON AWAKENS TO THE");
			_titleFont->displayString(80, 210, "FIRST DAY IN HIS NEW HOUSE...");
			break;
		}
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
		g_engine->_sound->stopMusic();
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
		g_engine->zeroMouseButtons();
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
		g_engine->_sound->playMusic(StartMusicId::kLaunch);
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
		g_engine->_sound->playMusic(StartMusicId::kAlien);
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

bool Cutscene::babyDollScene() {
	switch (_movieStep) {
	case 1: {
		freeMorph();
		_morph = new Morph({73, 46, 472, 240});
		_palette.load("art/norm.pal");
		Img left00Img;
		left00Img.load("art/bdoll0.img");
		left00Img.draw();
		Img left01Img;
		left01Img.load("art/bdoll1.img");
		left01Img.draw();
		_morph->loadSrcFromScreen();

		Img born1Img;
		born1Img.load("art/embryo.img");
		born1Img.draw(1);
		_morph->loadDestFromScreen();

		g_engine->_screen->clear();
		left00Img.draw();
		left01Img.draw();
		g_engine->_screen->clearPalette();
		break;
	}
	case 2:
		break;
	case 3:
		g_engine->_sound->playMusic(StartMusicId::kDoll);
		g_engine->fadeIn(_palette);
		break;
	case 4:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 5:
		_morph->start(MorphDirection::Forward);
		registTime();
		break;
	case 6:
		if (waitTime(50)) {
			return true;
		}
		break;
	case 7: {
		if (_morph->morphStep()) {
			return true;
		}
		registTime();
		break;
	}
	case 8:
		if (waitTime(40)) {
			return true;
		}
		_morph->start(MorphDirection::Backward);
		break;
	case 9:
		if (_morph->morphStep()) {
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
		freeMorph();
		break;
	default:
		_movieStep = 9999;
		return false;
	}
	_movieStep++;
	return true;
}

bool Cutscene::bookScene() {
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
		freeMorph();
		_morph = new Morph({136, 41, 423, 239});
		_palette.load("art/norm.pal");
		Img left00Img;
		left00Img.load("art/bdoll0.img");
		left00Img.draw();
		Img left01Img;
		left01Img.load("art/bdoll1.img");
		left01Img.draw();
		Img book1Img;
		book1Img.load("art/book1.img");
		book1Img.draw(1);
		_morph->loadSrcFromScreen();

		Img book2Img;
		book2Img.load("art/book2.img");
		book2Img.draw(1);
		_morph->loadDestFromScreen();

		g_engine->_screen->clear();
		left00Img.draw();
		left01Img.draw();
		book1Img.draw(1);
		g_engine->_screen->clearPalette();
		break;
	}
	case 4:
		break;
	case 5:
		g_engine->fadeIn(_palette);
		break;
	case 6:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 7:
		g_engine->_sound->playMusic(StartMusicId::kBook);
		registTime();
		break;
	case 8:
		if (waitTime(70)) {
			return true;
		}
		_morph->start(MorphDirection::Forward);
		break;
	case 9:
		if (_morph->morphStep()) {
			return true;
		}
		registTime();
		break;
	case 10:
		if (waitTime(70)) {
			return true;
		}
		break;
	case 11:
		registTime();
		_morph->start(MorphDirection::Backward);
		break;
	case 12:
		if (_morph->morphStep()) {
			return true;
		}
		registTime();
		break;
	case 13:
		if (waitTime(40)) {
			return true;
		}
		break;
	case 14:
		freeMorph();
		g_engine->fadeOut();
		break;
	case 15:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 16:
		g_engine->_sound->killAllSound();
		break;
	default:
		_movieStep = 9999;
		return false;
	}
	_movieStep++;
	return true;
}

bool Cutscene::nightmare2Scene() {
	switch (_movieStep) {
	case 1: {
		freeMorph();
		_morph = new Morph({264, 85, 515, 267});
		_palette.load("art/ship.pal");
		Img left00Img;
		left00Img.load("art/nmf0.img");
		left00Img.draw();
		Img left01Img;
		left01Img.load("art/nmf1.img");
		left01Img.draw();
		_morph->loadSrcFromScreen();

		Img book2Img;
		book2Img.load("art/dmik.img");
		book2Img.draw(1);
		_morph->loadDestFromScreen();

		g_engine->_screen->clear();
		left00Img.draw();
		left01Img.draw();
		g_engine->_screen->clearPalette();
		break;
	}
	case 2:
		break;
	case 3:
		g_engine->_sound->playMusic(StartMusicId::kNight2);
		break;
	case 4:
		g_engine->fadeIn(_palette);
		break;
	case 5:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 6:
		registTime();
		break;
	case 7:
		if (waitTime(20)) {
			return true;
		}
		_morph->start(MorphDirection::Forward);
		break;
	case 8:
		if (_morph->morphStep()) {
			return true;
		}
		registTime();
		break;
	case 9:
		if (waitTime(10)) {
			return true;
		}
		_morph->start(MorphDirection::Backward);
		break;
	case 10:
		if (_morph->morphStep()) {
			return true;
		}
		registTime();
		break;
	case 11:
		if (waitTime(40)) {
			return true;
		}
		_morph->start(MorphDirection::Forward);
		break;
	case 12:
		if (_morph->morphStep()) {
			return true;
		}
		registTime();
		break;
	case 13:
		if (waitTime(10)) {
			return true;
		}
		_morph->start(MorphDirection::Backward);
		break;
	case 14:
		if (_morph->morphStep()) {
			return true;
		}
		registTime();
		break;
	case 15:
		if (waitTime(40)) {
			return true;
		}
		break;
	case 16:
		freeMorph();
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
	return true;
}

bool Cutscene::nightmare3Scene() {
	switch (_movieStep) {
	case 1: {
		_palette.load("art/ship.pal");
		Img left00Img;
		left00Img.load("art/nmf0.img");
		left00Img.draw();
		Img left01Img;
		left01Img.load("art/nmf1.img");
		left01Img.draw();

		Img book2Img;
		book2Img.load("art/nm3a.img");
		book2Img.draw(1);

		g_engine->_screen->clearPalette();
		break;
	}
	case 2: {
		_animation.load("art/valves.anm", -9340);
		g_engine->_sound->playMusic(StartMusicId::kNight3);
		g_engine->fadeIn(_palette);
		break;
	}
	case 3:
		if (g_engine->fadeStep()) {
			return true;
		}
		break;
	case 4:
		_animIdx = 0;
		_animCount = 24;
		runAnim();
		break;
	case 5:
		if (stepValveAnim(false)) {
			return true;
		}
		registTime();
		break;
	case 6:
		_faceIdx = 0;
		_animCount = 34;
		break;
	case 7:
		if (stepValveAnim(true)) {
			return true;
		}
		break;
	case 8:
		_animCount = 34;
		break;
	case 9:
		if (stepValveAnim(false)) {
			return true;
		}
		registTime();
		break;
	case 10:
		if (waitTime(10)) {
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
	default:
		g_engine->_sound->stopMusic();
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

bool Cutscene::stepValveAnim(bool doFaceAnim) {
	if (_animDelayCount == 0) {
		Img animLeftFrame;
		_animation.getImg(_valvesIdx, animLeftFrame);
		animLeftFrame.draw(1);

		Img animRightFrame;
		_animation.getImg(_valvesIdx + 6, animRightFrame);
		animRightFrame.draw(1);

		if (doFaceAnim && (_animCount % 2) == 0) {
			Img faceFrame;
			Common::Path facePath = Common::Path("art").join(Common::String::format("f%02d.img", _faceIdx + 2));
			faceFrame.load(facePath);
			faceFrame.draw(0, faceFrame.getWidth() - 6);
			_faceIdx++;
		}

		_valvesIdx++;
		if (_valvesIdx > 5) {
			_valvesIdx = 0;
		}
		_animCount--;
	}

	_animDelayCount++;
	if (_animDelayCount == 12) {
		_animDelayCount = 0;
	}

	return _animCount > 0;
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

bool Cutscene::waitTime(int16 duration) const {
	return g_system->getMillis() < _startTime + (duration * 100);
}

void Cutscene::runAnim(int direction) {
	_animDelayCount = 0;
	_animDirection = direction;
}

void Cutscene::freeMorph() {
	delete _morph;
	_morph = nullptr;
}

void Cutscene::displayTitleText(const I18NTextWithPosition &text) {
	const TextWithPosition &textWithPosition = getI18NTextWithPosition(text);
	_titleFont->displayString(textWithPosition.x, textWithPosition.y, textWithPosition.text);
}

} // End of namespace Darkseed
