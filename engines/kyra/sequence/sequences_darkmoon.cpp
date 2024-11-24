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

#ifdef ENABLE_EOB

#include "kyra/engine/darkmoon.h"
#include "kyra/graphics/screen_eob.h"
#include "kyra/resource/resource.h"
#include "kyra/sound/sound.h"

#include "common/system.h"

namespace Kyra {

class DarkmoonSequenceHelper {
friend class DarkMoonEngine;
public:
	enum Mode {
		kIntro,
		kFinale
	};

	DarkmoonSequenceHelper(OSystem *system, DarkMoonEngine *vm, Screen_EoB *screen, Mode mode);
	~DarkmoonSequenceHelper();

	void loadScene(int index, int pageNum, bool ignorePalette = false);
	void animCommand(int index, int del = -1);
	void setPlatformAnimIndexOffset(int offset);

	void printText(int index, int color);
	void fadeText();

	void update(int srcPage);

	void setPalette(int index);
	void fadePalette(int index, int del);
	void copyPalette(int srcIndex, int destIndex);

	int hScroll(bool restart = false);

	void initDelayedPaletteFade(int palIndex, int rate);
	bool processDelayedPaletteFade();

	void delay(uint32 ticks);
	void waitForSongNotifier(int index, bool introUpdateAnim = false);
	void updateAmigaSound();

private:
	void init(Mode mode);
	void setPaletteWithoutTextColor(int index);
	void printStringIntern(const char *str, int x, int y, int col);

	OSystem *_system;
	DarkMoonEngine *_vm;
	Screen_EoB *_screen;

	struct Config {
		Config(const char *const *str, const char *const *cpsfiles, const char *vocPat, const uint8 **cpsdata, const char *const *pal, const DarkMoonShapeDef **shp, const DarkMoonAnimCommand **anim, bool loadScenePalette, bool paletteFading, bool animCmdRestorePalette, bool shapeBackgroundFading, int animPalOffset, int animType1ShapeDim, bool animCmd5SetPalette, int animCmd5ExtraPage) : strings(str), voicePattern(vocPat), cpsFiles(cpsfiles), cpsData(cpsdata), palFiles(pal), shapeDefs(shp), animData(anim), loadScenePal(loadScenePalette), palFading(paletteFading), animCmdRestorePal(animCmdRestorePalette), shpBackgroundFading(shapeBackgroundFading), animPalOffs(animPalOffset), animCmd1ShapeFrame(animType1ShapeDim), animCmd5SetPal(animCmd5SetPalette), animCmd5AltPage(animCmd5ExtraPage) {}
		const char *const *strings;
		const char *voicePattern;
		const char *const *cpsFiles;
		const uint8 **cpsData;
		const char *const *palFiles;
		const DarkMoonShapeDef **shapeDefs;
		const DarkMoonAnimCommand **animData;
		bool loadScenePal;
		bool palFading;
		bool animCmdRestorePal;
		bool shpBackgroundFading;
		int animPalOffs;
		int animCmd1ShapeFrame;
		bool animCmd5SetPal;
		int animCmd5AltPage;
	};

	const Config *_config;

	Palette *_palettes[13];
	uint8 *_fadingTables[7];
	byte *_t1cps;

	const uint8 **_shapes;

	uint32 _fadePalTimer;
	int _fadePalRate;
	int _fadePalIndex;

	uint8 _sndNextTrack;
	uint16 _sndNextTrackMarker;
	const uint16 *_sndMarkersFMTowns;

	uint32 _hScrollStartTimeStamp;
	uint32 _hScrollResumeTimeStamp;
	int _hScrollState;

	uint8 _textColor[3];
	int16 _shadowColor;

	int _platformAnimOffset;

	Screen::FontId _prevFont;

	static const char *const _palFilesIntroVGA[];
	static const char *const _palFilesIntroEGA[];
	static const char *const _palFilesFinaleVGA[];
	static const char *const _palFilesFinaleEGA[];
	static const char *const _palFilesFinaleAmiga[];
};

int DarkMoonEngine::mainMenu() {
	int menuChoice = _menuChoiceInit;
	_menuChoiceInit = 0;

	_sound->selectAudioResourceSet(kMusicIntro);
	_sound->loadSoundFile(0);

	Screen::FontId of = _screen->_currentFont;
	int op = 0;
	Common::SeekableReadStream *s = 0;

	while (menuChoice >= 0 && !shouldQuit()) {
		switch (menuChoice) {
		case 0: {
			if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformAmiga) {
				_screen->loadPalette("MENU.PAL", _screen->getPalette(0));
				_screen->setScreenPalette(_screen->getPalette(0));
				_screen->loadEoBBitmap("MENU", 0, 3, 3, 2);
			} else {
				s = _res->createReadStream("XENU.CPS");
				if (s) {
					s->read(_screen->getPalette(0).getData(), 768);
					_screen->loadFileDataToPage(s, 3, 64000);
					delete s;
				} else {
					_screen->loadBitmap("MENU.CPS", 3, 3, &_screen->getPalette(0));
				}

				if (_configRenderMode == Common::kRenderEGA)
					_screen->loadPalette("MENU.EGA", _screen->getPalette(0));
			}

			_screen->setScreenPalette(_screen->getPalette(0));
			_screen->convertPage(3, 2, 0);

			of = _screen->setFont(Screen::FID_6_FNT);
			op = _screen->setCurPage(2);
			Common::String versionString = "ScummVM " + _versionString;
			_screen->printText(versionString.c_str(), (_flags.lang == Common::ZH_TWN ? 306 : 267) - versionString.size() * 6, _flags.platform == Common::kPlatformFMTowns ? 152 : 160, _flags.platform == Common::kPlatformAmiga ? 18 : 13, _flags.lang == Common::ZH_TWN ? 231 : 0);
			_screen->setFont(of);
			_screen->_curPage = op;
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->shadeRect(78, 99, 249, 141, 4);
			_screen->showMouse();
			_screen->updateScreen();
			_allowImport = true;
			menuChoice = mainMenuLoop();
			_allowImport = false;
		} break;

		case 1:
			// load game in progress
			menuChoice = -1;
			break;

		case 2:
			// create new party
			menuChoice = -2;
			break;

		case 3:
			// transfer party
			menuChoice = -3;
			break;

		case 4:
			// play intro
			seq_playIntro();
			menuChoice = 0;
			break;

		case 5:
			// quit
			menuChoice = -5;
			break;

		default:
			break;
		}
	}

	return shouldQuit() ? -5 : menuChoice;
}

int DarkMoonEngine::mainMenuLoop() {
	int sel = -1;

	do {
		_screen->setScreenDim(6);
		_gui->simpleMenu_setup(6, 0, _mainMenuStrings, -1, 0, 0, _configRenderMode == Common::kRenderCGA ? 1 : guiSettings()->colors.guiColorWhite, guiSettings()->colors.guiColorLightRed, guiSettings()->colors.guiColorBlack);
		_screen->updateScreen();

		while (sel == -1 && !shouldQuit())
			sel = _gui->simpleMenu_process(6, _mainMenuStrings, 0, -1, 0);
	} while ((sel < 0 || sel > 5) && !shouldQuit());

	if (_flags.platform == Common::kPlatformFMTowns && sel == 2) {
		townsUtilitiesMenu();
		sel = -1;
	}

	return sel + 1;
}

void DarkMoonEngine::townsUtilitiesMenu() {
	_screen->copyRegion(78, 99, 78, 99, 172, 43, 2, 0, Screen::CR_NO_P_CHECK);
	int sel = -1;
	do {
		_gui->simpleMenu_setup(8, 0, _utilMenuStrings, -1, 0, 0, _configRenderMode == Common::kRenderCGA ? 1 : guiSettings()->colors.guiColorWhite, guiSettings()->colors.guiColorLightRed, guiSettings()->colors.guiColorBlack);
		_screen->updateScreen();
		while (sel == -1 && !shouldQuit())
			sel = _gui->simpleMenu_process(8, _utilMenuStrings, 0, -1, 0);
		if (sel == 0) {
			_config2431 ^= true;
			sel = -1;
		}
	} while ((sel < 0 || sel > 1) && !shouldQuit());
}

void DarkMoonEngine::seq_playIntro() {
	DarkmoonSequenceHelper sq(_system, this, _screen, DarkmoonSequenceHelper::kIntro);

	_screen->setCurPage(0);
	_screen->clearCurPage();

	snd_stopSound();

	sq.loadScene(4, 2);

	uint8 textColor1 = 16;
	uint8 textColor2 = 15;
	int songCurPos = 0;

	if (_flags.platform == Common::kPlatformPC98)
		sq.loadScene(13, 2);

	if (_flags.platform == Common::kPlatformAmiga) {
		textColor1 = textColor2 = 31;
		sq.loadScene(13, 2);
		sq.loadScene(14, 2);
		sq.loadScene(15, 2);
	} else if (_configRenderMode == Common::kRenderEGA) {
		textColor1 = 15;
	}

	sq.loadScene(0, 2);
	sq.delay(1);

	// PC-98 --- SFX 0

	if (!skipFlag() && !shouldQuit())
		snd_playSong(_flags.platform == Common::kPlatformPC98 ? 54 : 12);

	_screen->copyRegion(0, 0, 8, 8, 304, 128, 2, 0, Screen::CR_NO_P_CHECK);
	sq.setPalette(9);
	sq.fadePalette(0, 3);

	_screen->setCurPage(2);
	_screen->setClearScreenDim(17);
	_screen->setCurPage(0);

	removeInputTop();
	sq.delay(18);

	sq.animCommand(3);
	if (_flags.lang == Common::ZH_TWN)
		sq.printText(20, guiSettings()->colors.guiColorYellow);
	sq.delay(18);
	sq.animCommand(6, 18);
	sq.animCommand(0);

	sq.waitForSongNotifier(++songCurPos);

	sq.animCommand(_flags.platform == Common::kPlatformPC98 ? (_configRenderMode == Common::kRenderEGA ? 43 : 42) : (_configRenderMode == Common::kRenderEGA ? 12 : 11));
	sq.animCommand(7, 6);
	sq.animCommand(2, 6);
	if (_flags.lang == Common::ZH_TWN)
		sq.fadeText();

	sq.waitForSongNotifier(++songCurPos);

	if (_flags.platform == Common::kPlatformPC98) {
		sq.animCommand(_configRenderMode == Common::kRenderEGA ? 37 : 36);
		sq.animCommand(7, 6);
		sq.animCommand(2, 6);
		sq.waitForSongNotifier(++songCurPos);
		sq.animCommand(_configRenderMode == Common::kRenderEGA ? 45 : 44);
		sq.animCommand(7, 6);
		sq.animCommand(2, 6);
		sq.waitForSongNotifier(++songCurPos);
	}

	sq.animCommand(_flags.platform == Common::kPlatformAmiga ? 37 : (_flags.platform == Common::kPlatformPC98 ? (_configRenderMode == Common::kRenderEGA ? 47 : 46) : (_configRenderMode == Common::kRenderEGA ? 39 : 38)));
	sq.animCommand(3);
	sq.animCommand(8);
	sq.animCommand(1, 10);
	sq.animCommand(0, 6);
	sq.animCommand(2);

	sq.waitForSongNotifier(++songCurPos);

	_screen->setClearScreenDim(17);
	_screen->setCurPage(2);
	_screen->setClearScreenDim(17);
	_screen->setCurPage(0);

	sq.animCommand(_flags.platform == Common::kPlatformAmiga ? 38 : (_flags.platform == Common::kPlatformPC98 ? (_configRenderMode == Common::kRenderEGA ? 39 : 38) : (_configRenderMode == Common::kRenderEGA ? 41 : 40)));
	sq.animCommand(7, 18);

	if (_flags.platform == Common::kPlatformAmiga)
		sq.fadeText();

	sq.printText(0, textColor1);    // You were settling...
	sq.animCommand(7, 90);
	sq.fadeText();

	sq.printText(1, textColor1);    // Then a note was slipped to you
	sq.animCommand(8);
	sq.animCommand(2, 72);
	sq.fadeText();

	sq.printText(2, textColor1);    // It was from your friend Khelben Blackstaff...
	sq.animCommand(2);
	sq.animCommand(6, 36);
	sq.animCommand(3);
	sq.fadeText();

	sq.printText(3, textColor1);    // The message was urgent.

	if (!skipFlag() && !shouldQuit() && _flags.platform == Common::kPlatformPC98)
		snd_playSong(55);

	sq.loadScene(1, 2);
	sq.waitForSongNotifier(++songCurPos);
	uint32 endtime = _system->getMillis();

	// intro horizontal scroll
	if (!skipFlag() && !shouldQuit()) {
		for (int i = sq.hScroll(true); i != 279; i = sq.hScroll()) {
			endtime += 18;
			if (_flags.platform == Common::kPlatformAmiga) {
				if (i == 4 || i == 24 || i == 36)
					sq.animCommand(39);
			} else if (i == 96) {
				sq.animCommand(_flags.platform == Common::kPlatformPC98 ? 40 : 42);
			} else if (i == 200) {
				snd_playSoundEffect(11);
			}
			delayUntil(endtime);
		}
	}

	_screen->copyRegion(8, 8, 0, 0, 304, 128, 0, 2, Screen::CR_NO_P_CHECK);
	sq.animCommand(4);
	sq.fadeText();
	sq.delay(10);

	sq.loadScene(2, 2);
	sq.update(2);
	sq.delay(10);

	sq.printText(4, textColor1);    // What could Khelben want?
	sq.delay(25);

	sq.loadScene(3, 2);
	sq.delay(54);
	sq.animCommand(_flags.platform == Common::kPlatformAmiga ? 12 : (_flags.platform == Common::kPlatformPC98 ? 11 : 13));
	_screen->copyRegion(104, 16, 96, 8, 120, 100, 0, 2, Screen::CR_NO_P_CHECK);
	sq.fadeText();

	if (_flags.platform == Common::kPlatformAmiga)
		sq.animCommand(9);

	sq.printText(5, textColor2);    // Welcome, please come in
	sq.animCommand(10);
	sq.animCommand(10);
	sq.animCommand(9);
	sq.animCommand(9);
	sq.fadeText();

	sq.printText(6, textColor2);    // Khelben awaits you in his study
	for (int i = 0; i < 3; i++)
		sq.animCommand(10);
	sq.animCommand(9);

	if (_flags.platform == Common::kPlatformAmiga)
		sq.setPlatformAnimIndexOffset(-1);
	else if (_flags.platform == Common::kPlatformPC98)
		sq.setPlatformAnimIndexOffset(-2);

	sq.animCommand(14);

	if (_flags.platform == Common::kPlatformAmiga)
		_sound->beginFadeOut();

	sq.loadScene(5, 2);

	if (!skipFlag() && !shouldQuit()) {
		if (_flags.platform == Common::kPlatformAmiga) {
			_screen->fadeToBlack(5);
			_screen->clearCurPage();
			_screen->fadeFromBlack(1);
			sq.fadeText();
			snd_playSong(14);
		} else {
			sq.waitForSongNotifier(++songCurPos);
			sq.fadeText();
			_screen->clearCurPage();
			_screen->updateScreen();
		}
	}

	for (int i = 0; i < 6; i++)
		sq.animCommand(15);

	if (_configRenderMode == Common::kRenderEGA && !skipFlag() && !shouldQuit()) {
		_screen->loadPalette("INTRO.EGA", _screen->getPalette(0));
		_screen->setScreenPalette(_screen->getPalette(0));
	}

	sq.loadScene(6, 2);
	sq.loadScene(7, 2);
	_screen->clearCurPage();
	sq.update(2);

	if (_flags.platform == Common::kPlatformAmiga && !skipFlag() && !shouldQuit())
		snd_playSong(15);

	sq.animCommand(16);

	if (!skipFlag() && !shouldQuit() && _flags.platform == Common::kPlatformPC98)
		snd_playSong(56);

	sq.printText(7, textColor2);    // Thank you for coming so quickly
	sq.animCommand(16);
	sq.animCommand(17);
	for (int i = 0; i < 3; i++)
		sq.animCommand(16);
	sq.fadeText();
	sq.animCommand(16);

	sq.loadScene(8, 2, true);
	sq.update(2);
	sq.animCommand(32);
	sq.printText(8, textColor2);    // I am troubled my friend
	sq.animCommand(33);
	sq.animCommand(33);
	for (int i = 0; i < 4; i++)
		sq.animCommand(32);
	sq.fadeText();

	sq.printText(9, textColor2);    // Ancient evil stirs in the Temple Darkmoon
	sq.animCommand(33);
	sq.animCommand(_flags.platform == Common::kPlatformAmiga ? 41 : 43);
	sq.animCommand(33);
	for (int i = 0; i < 3; i++)
		sq.animCommand(32);
	sq.fadeText();

	sq.printText(10, textColor2);   // I fear for the safety of our city
	for (int i = 0; i < 4; i++)
		sq.animCommand(33);
	sq.animCommand(32);
	sq.animCommand(32);

	sq.fadeText();
	sq.loadScene(9, 2);

	sq.waitForSongNotifier(++songCurPos);

	sq.update(2);
	sq.animCommand(34);

	sq.printText(11, textColor2);   // I need your help
	for (int i = 0; i < 3; i++)
		sq.animCommand(34);
	sq.animCommand(35);
	for (int i = 0; i < 4; i++)
		sq.animCommand(34);
	sq.fadeText();

	sq.loadScene(12, 2);
	sq.update(2);
	sq.loadScene(6, 2, true);
	sq.animCommand(18);

	sq.printText(12, textColor2);   // Three nights ago I sent forth a scout
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(22);
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(18);
	sq.fadeText();

	sq.printText(13, textColor2);   // She has not yet returned
	sq.animCommand(20);
	sq.animCommand(19);
	sq.animCommand(23);
	sq.animCommand(24);
	sq.animCommand(20);
	sq.animCommand(19);
	sq.animCommand(17);
	sq.animCommand(18);
	sq.fadeText();

	sq.printText(14, textColor2);   // I fear for her safety
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(20);
	sq.animCommand(18);
	sq.animCommand(25);
	sq.animCommand(18);
	sq.animCommand(18);
	sq.fadeText();
	sq.animCommand(18);
	sq.animCommand(18);

	sq.printText(15, textColor2);   // Take this coin
	sq.animCommand(28);
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(18);
	sq.animCommand(18);
	sq.fadeText();

	sq.loadScene(10, 2);

	if (_flags.platform == Common::kPlatformAmiga)
		_screen->fadeToBlack(10);

	_screen->clearCurPage();
	if (_flags.platform == Common::kPlatformAmiga)
		sq.setPalette(0);
	_screen->updateScreen();

	sq.animCommand(37, 18);
	sq.animCommand(36, 36);

	sq.loadScene(12, 2);
	_screen->clearCurPage();
	sq.update(2);

	sq.loadScene(11, 2, true);
	sq.printText(16, textColor2);   // I will use it to contact you
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(20);
	sq.animCommand(18);
	sq.animCommand(18);
	sq.fadeText();

	sq.printText(17, textColor2);   // You must act quickly
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(19);
	sq.animCommand(18);
	sq.animCommand(18);
	sq.fadeText();
	sq.animCommand(18);

	sq.printText(18, textColor2);   // I will teleport you near Darkmoon
	sq.animCommand(20);
	sq.animCommand(27);
	sq.animCommand(20);
	sq.animCommand(19);
	sq.animCommand(18);
	sq.animCommand(18);
	sq.fadeText();
	sq.animCommand(18);

	sq.printText(19, textColor2);   // May luck be with you my friend
	sq.animCommand(19);
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(18);

	if (!skipFlag() && !shouldQuit() && _flags.platform == Common::kPlatformPC98)
		snd_playSong(57);

	sq.fadeText();
	sq.animCommand(29);

	sq.waitForSongNotifier(++songCurPos);

	sq.animCommand(30);
	sq.animCommand(31);

	sq.waitForSongNotifier(++songCurPos, true);

	if (_flags.platform == Common::kPlatformAmiga && !skipFlag() && !shouldQuit()) {
		static const uint8 magicHandsCol[] = { 0x15, 0x1D, 0x3A, 0x32, 0x32, 0x3F };
		snd_fadeOut();
		_screen->getPalette(0).copy(magicHandsCol, 0, 1, 31);
		_screen->fadePalette(_screen->getPalette(0), 32);
		_screen->getPalette(0).copy(magicHandsCol, 1, 1, 31);
		_screen->fadePalette(_screen->getPalette(0), 32);
	}

	if (skipFlag() || shouldQuit())
		snd_fadeOut();
	else {
		_screen->setScreenDim(17);
		_screen->clearCurDim();
		snd_playSoundEffect(_flags.platform == Common::kPlatformPC98 ? 12 : 14);

		if (_configRenderMode != Common::kRenderEGA)
			sq.fadePalette(10, 1);
		_screen->setClearScreenDim(18);
		sq.delay(6);
		if (_configRenderMode != Common::kRenderEGA)
			sq.fadePalette(9, 1);
		_screen->clearCurPage();
	}
	sq.fadePalette(9, 10);
}

void DarkMoonEngine::seq_playFinale() {
	_screen->fadeToBlack();
	_screen->clearCurPage();
	_screen->clearPage(2);

	DarkmoonSequenceHelper sq(_system, this, _screen, DarkmoonSequenceHelper::kFinale);

	_screen->setCurPage(0);

	_sound->loadSoundFile(0);
	snd_stopSound();
	sq.delay(3);
	_screen->updateScreen();

	uint8 textColor1 = 10;
	uint8 textColor2 = 15;

	if (_flags.platform == Common::kPlatformAmiga) {
		textColor1 = 29;
		textColor2 = 31;
	} else if (_configRenderMode == Common::kRenderEGA) {
		textColor1 = 15;
	}

	sq.loadScene(0, 2);
	sq.delay(18);

	if (!skipFlag() && !shouldQuit() && _flags.platform != Common::kPlatformAmiga)
		snd_playSong(_flags.platform == Common::kPlatformPC98 ? 52 : 1);
	sq.update(2);

	sq.loadScene(1, 2);

	sq.animCommand(0);
	sq.animCommand(0);
	for (int i = 0; i < 3; i++)
		sq.animCommand(2);
	sq.animCommand(1);
	sq.animCommand(2);
	sq.animCommand(2);

	sq.printText(0, textColor1);            // Finally, Dran has been defeated
	for (int i = 0; i < 7; i++)
		sq.animCommand(2);
	sq.fadeText();
	sq.animCommand(2);

	sq.waitForSongNotifier(1);

	sq.printText(1, textColor1);            // Suddenly, your friend Khelben appears
	sq.animCommand(4);
	for (int i = 0; i < 3; i++)
		sq.animCommand(2);
	sq.fadeText();

	sq.printText(2, textColor2);            // Greetings, my victorious friends
	for (int i = 0; i < 4; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();
	sq.animCommand(6);

	sq.printText(3, textColor2);            // You have defeated Dran
	for (int i = 0; i < 5; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();

	sq.printText(4, textColor2);            // I did not know Dran was a dragon
	for (int i = 0; i < 4; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();

	sq.printText(5, textColor2);            // He must have been over 300 years old
	for (int i = 0; i < 4; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();

	sq.printText(6, textColor2);            // His power is gone
	for (int i = 0; i < 3; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();

	sq.printText(7, textColor2);            // But Darkmoon is still a source of great evil
	for (int i = 0; i < 4; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();

	sq.printText(8, textColor2);            // And many of his minions remain
	for (int i = 0; i < 4; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();

	sq.loadScene(2, 2);
	sq.update(2);
	sq.loadScene(3, 2);
	_screen->copyRegion(8, 8, 0, 0, 304, 128, 0, 2, Screen::CR_NO_P_CHECK);

	sq.printText(9, textColor2);            // Now we must leave this place
	sq.animCommand(7);
	sq.animCommand(8);
	sq.animCommand(7);
	sq.animCommand(7, 36);
	sq.fadeText();

	sq.printText(10, textColor2);           // So my forces can destroy it..
	for (int i = 0; i < 3; i++)
		sq.animCommand(7);
	sq.animCommand(8);
	sq.animCommand(7);
	sq.animCommand(7, 36);
	sq.animCommand(8, 18);
	sq.fadeText();

	sq.printText(11, textColor2);           // Follow me
	sq.animCommand(7, 18);
	sq.animCommand(9, 18);
	sq.animCommand(8, 18);
	sq.fadeText();

	sq.loadScene(7, 2);

	sq.copyPalette(3, 0);

	sq.loadScene(4, 2);

	sq.waitForSongNotifier(2);

	_screen->clearCurPage();
	sq.update(2);

	sq.loadScene(8, 2);
	sq.loadScene(6, 6);
	sq.delay(10);

	sq.printText(12, textColor1);           // Powerful mages stand ready for the final assault...
	sq.delay(90);
	sq.fadeText();

	sq.waitForSongNotifier(3);

	int sfxOffset = (_flags.platform == Common::kPlatformPC98) ? -1 : 0;

	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(7 + sfxOffset);
	sq.delay(8);

	sq.animCommand(10);
	sq.animCommand(13);
	if (_flags.platform != Common::kPlatformAmiga)
		sq.initDelayedPaletteFade(4, 1);

	sq.animCommand(14);
	sq.animCommand(13);
	sq.animCommand(14);
	sq.animCommand(14);
	sq.animCommand(13);
	if (_flags.platform != Common::kPlatformAmiga)
		sq.initDelayedPaletteFade(2, 1);

	sq.animCommand(15);
	sq.animCommand(14);
	sq.animCommand(13);
	sq.animCommand(15);
	sq.animCommand(15);
	sq.animCommand(11);

	sq.printText(13, textColor1);           // The temple's evil is very strong
	sq.delay(72);
	sq.fadeText();

	sq.printText(14, textColor1);           // It must not be allowed...
	sq.delay(72);
	sq.fadeText();

	sq.waitForSongNotifier(4);

	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(7 + sfxOffset);
	sq.delay(8);

	sq.animCommand(10);
	if (_flags.platform != Common::kPlatformAmiga)
		sq.initDelayedPaletteFade(5, 1);
	sq.animCommand(13);
	sq.animCommand(14);
	sq.animCommand(13);
	sq.animCommand(14);
	sq.animCommand(13);
	sq.animCommand(13);
	sq.animCommand(14);
	sq.animCommand(14);
	sq.animCommand(13);
	sq.animCommand(12);
	if (_flags.platform == Common::kPlatformAmiga)
		sq.fadePalette(2, 3);
	for (int i = 0; i < 4; i++)
		sq.animCommand(16);
	if (_flags.platform == Common::kPlatformAmiga)
		sq.fadePalette(4, 3);
	sq.animCommand(17);
	sq.animCommand(18);

	sq.printText(15, textColor1);           // The temple ceases to exist
	if (_flags.platform != Common::kPlatformAmiga) {
		sq.initDelayedPaletteFade(6, 1);
	} else if (skipFlag()) {
		_screen->fadeToBlack();
	} else {
		_screen->fadePalette(_screen->getPalette(5), 127);
		sq.copyPalette(5, 0);
	}
	sq.delay(36);

	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(11 + sfxOffset);

	sq.delay(54);
	sq.fadeText();
	sq.loadScene(12, 2);

	sq.waitForSongNotifier(5);

	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(6 + sfxOffset);

	if (_flags.platform == Common::kPlatformAmiga)
		sq.copyPalette(6, 0);

	if (!skipFlag() && !shouldQuit()) {
		if (_configRenderMode != Common::kRenderEGA)
			sq.setPaletteWithoutTextColor(0);
		_screen->crossFadeRegion(0, 0, 8, 8, 304, 128, 2, 0);
	}
	sq.delay(18);

	sq.printText(16, textColor2);           // My friends, our work is done
	sq.animCommand(20);
	sq.animCommand(19);
	sq.animCommand(19, 36);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(12 + sfxOffset);
	sq.fadeText();

	sq.printText(17, textColor2);           // Thank you
	sq.animCommand(19);
	sq.animCommand(20, 36);
	sq.fadeText();

	sq.printText(18, textColor2);           // You have earned my deepest respect
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(11 + sfxOffset);
	sq.animCommand(20);
	sq.animCommand(19);
	sq.animCommand(19);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(11 + sfxOffset);
	sq.delay(36);
	sq.fadeText();

	sq.printText(19, textColor2);           // We will remember you always
	sq.animCommand(19);
	sq.animCommand(19, 18);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(11 + sfxOffset);
	sq.animCommand(20, 18);
	sq.fadeText();

	sq.delay(28);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(12 + sfxOffset);
	sq.delay(3);

	sq.loadScene(5, 2);
	if (skipFlag() || shouldQuit()) {
		_screen->copyRegion(0, 0, 8, 8, 304, 128, 2, 0, Screen::CR_NO_P_CHECK);
	} else {
		sq.updateAmigaSound();
		snd_playSoundEffect(6 + sfxOffset);
		if (_configRenderMode != Common::kRenderEGA)
			sq.setPaletteWithoutTextColor(0);
		_screen->crossFadeRegion(0, 0, 8, 8, 304, 128, 2, 0);
	}

	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(12 + sfxOffset);
	sq.delay(5);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(11 + sfxOffset);
	sq.delay(11);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(12 + sfxOffset);
	sq.delay(7);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(11 + sfxOffset);
	sq.delay(12);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(12 + sfxOffset);
	sq.updateAmigaSound();

	removeInputTop();
	resetSkipFlag(true);

	// The Chinese version has neither the credits player nor the data files for it.
	if (_flags.lang != Common::ZH_TWN) {
		sq.loadScene(10, 2);
		sq.loadScene(9, 2);
	}

	if (_flags.platform == Common::kPlatformAmiga) {
		sq.setPalette(7);
		sq.delay(3);
	} else {
		snd_stopSound();
		sq.delay(3);
		_sound->loadSoundFile(1);
	}

	sq.delay(18);
	if (!skipFlag() && !shouldQuit() && _flags.platform != Common::kPlatformAmiga)
		snd_playSong(_flags.platform == Common::kPlatformFMTowns ? 16 : (_flags.platform == Common::kPlatformPC98 ? 52 : 1));

	if (_flags.lang != Common::ZH_TWN) {
		int temp = 0;

		static const char *const tryFiles[2] = {
			"CREDITS.TXT",
			"CREDITS4.CPS"
		};

		const uint8 *creditsFileData = 0;
		for (int i = 0; i < ARRAYSIZE(tryFiles) && !creditsFileData; ++i)
			creditsFileData = _res->fileData(tryFiles[i], 0);

		const uint8 *creditsData = creditsFileData ? creditsFileData : _staticres->loadRawData(kEoB2CreditsData, temp);

		seq_playCredits(&sq, creditsData, 18, 2, 6, 2);

		delete[] creditsFileData;
	}

	sq.delay(90);

	removeInputTop();
	resetSkipFlag(true);

	if (_configRenderMode != Common::kRenderEGA) {
		if (_flags.platform != Common::kPlatformAmiga)
			sq.setPalette(11);
		sq.fadePalette(9, 10);
	}

	_screen->clearCurPage();
	sq.loadScene(11, 2);

	static const uint8 finPortraitPos[] = { 0x50, 0x50, 0xD0, 0x50, 0x50, 0x90, 0xD0, 0x90, 0x90, 0x50, 0x90, 0x90 };

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;
		if (i > 3)
			_screen->drawShape(2, sq._shapes[6 + i], finPortraitPos[i << 1] - 16, finPortraitPos[(i << 1) + 1] - 16, 0);
		_screen->drawShape(2, _characters[i].faceShape, finPortraitPos[i << 1], finPortraitPos[(i << 1) + 1], 0);
	}

	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);

	if (_flags.platform == Common::kPlatformFMTowns)
		sq.copyPalette(12, 0);

	if (_flags.platform != Common::kPlatformAmiga)
		sq.setPalette(9);
	sq.fadePalette(0, 18);

	while (!skipFlag() && !shouldQuit()) {
		sq.updateAmigaSound();
		delay(_tickLength);
	}

	snd_stopSound();
	removeInputTop();
	resetSkipFlag(true);

	sq.fadePalette(9, 10);
}

void DarkMoonEngine::seq_playCredits(DarkmoonSequenceHelper *sq, const uint8 *data, int sd, int backupPage, int tempPage, int speed) {
	if (!data)
		return;

	_screen->setFont(Screen::FID_8_FNT);
	_screen->setScreenDim(sd);

	const ScreenDim *dm = _screen->_curDim;
	const uint8 col1 = _flags.platform == Common::kPlatformAmiga ? 19 : 12;
	const uint8 col2 = _flags.platform == Common::kPlatformAmiga ? 29 : 240;

	_screen->copyRegion(dm->sx << 3, dm->sy, dm->sx << 3, dm->sy, dm->w << 3, dm->h, 0, backupPage, Screen::CR_NO_P_CHECK);

	struct CreditsDataItem {
		int16 x;
		int16 y;
		const void *data;
		char *str;
		uint8 crlf;
		uint8 size;
		uint8 dataType;
	} items[36];
	memset(items, 0, sizeof(items));

	const char *pos = (const char *)data;
	uint32 end = _system->getMillis();
	uint32 cur = 0;
	int i = 0;

	do {
		for (bool loop = true; loop;) {
			sq->processDelayedPaletteFade();
			cur = _system->getMillis();
			if (end <= cur)
				break;
			delay(MIN<uint32>(_tickLength, end - cur));
		}

		end = _system->getMillis() + ((speed * _tickLength) >> 1);

		for (; i < 35 && *pos; i++) {
			int16 nextY = i ? items[i].y + items[i].size + (items[i].size >> 2) : dm->h;

			const char *posOld = pos;
			pos = strchr(pos, 0x0D);
			if (!pos)
				pos = strchr(posOld, 0x00);

			items[i + 1].crlf = *pos++;

			if (*posOld == 2) {
				const uint8 *shp = sq->_shapes[(*++posOld) - 1];
				items[i + 1].data = shp;
				items[i + 1].size = shp[1];
				items[i + 1].x = (dm->w - shp[2]) << 2;
				items[i + 1].dataType = 1;
				delete[] items[i + 1].str;
				items[i + 1].str = 0;

			} else {
				if (*posOld == 1) {
					posOld++;
					items[i + 1].size = 6;
				} else {
					items[i + 1].size = _screen->getFontWidth();
				}

				items[i + 1].dataType = 0;

				int l = pos - posOld;
				if (items[i + 1].crlf != 0x0D)
					l++;

				delete[] items[i + 1].str;
				items[i + 1].str = new char[l];
				memcpy(items[i + 1].str, posOld, l);
				items[i + 1].str[l - 1] = 0;
				items[i + 1].data = 0;
				items[i + 1].x = (((dm->w << 3) - (strlen(items[i + 1].str) * items[i + 1].size)) >> 1) + 1;
			}

			items[i + 1].y = nextY;
		}

		_screen->copyRegion(dm->sx << 3, dm->sy, dm->sx << 3, dm->sy, dm->w << 3, dm->h, backupPage, tempPage, Screen::CR_NO_P_CHECK);
		sq->updateAmigaSound();

		for (int h = 0; h < i; h++) {
			if (items[h + 1].y < dm->h) {
				if (items[h + 1].dataType == 1) {
					_screen->drawShape(tempPage, (const uint8 *)items[h + 1].data, items[h + 1].x, items[h + 1].y, sd);
				} else {
					_screen->setCurPage(tempPage);

					if (items[h + 1].size == 6)
						_screen->setFont(Screen::FID_6_FNT);

					_screen->printText(items[h + 1].str, (dm->sx << 3) + items[h + 1].x - 1, dm->sy + items[h + 1].y + 1, col1, 0);
					_screen->printText(items[h + 1].str, (dm->sx << 3) + items[h + 1].x, dm->sy + items[h + 1].y, col2, 0);

					if (items[h + 1].size == 6)
						_screen->setFont(Screen::FID_8_FNT);

					_screen->setCurPage(0);
				}
			}

			items[h + 1].y -= MAX<int>(1, speed >> 1);
		}

		_screen->copyRegion(dm->sx << 3, dm->sy, dm->sx << 3, dm->sy, dm->w << 3, dm->h, tempPage, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		if (-items[1].size > items[1].y) {
			delete[] items[1].str;
			--i;
			for (int t = 1; t <= i; t++)
				memcpy(&items[t], &items[t + 1], sizeof(CreditsDataItem));
			items[i + 1].str = 0;
		}

		if (i < 35 && ((items[i].y + items[i].size) < (dm->sy + dm->h))) {
			resetSkipFlag(true);
			break;
		}

		sq->processDelayedPaletteFade();
	} while (!skipFlag() && i && !shouldQuit());

	for (i = 0; i < 35; i++)
		delete[] items[i].str;
}

DarkmoonSequenceHelper::DarkmoonSequenceHelper(OSystem *system, DarkMoonEngine *vm, Screen_EoB *screen, DarkmoonSequenceHelper::Mode mode) : _system(system), _vm(vm), _screen(screen), _fadePalIndex(0), _t1cps(nullptr) {
	init(mode);
}

DarkmoonSequenceHelper::~DarkmoonSequenceHelper() {
	if (_vm->_flags.platform != Common::kPlatformAmiga) {
		for (int i = 4; _config->palFiles[i]; i++)
			delete _palettes[i];
		for (int i = 9; i < 13; ++i)
			delete _palettes[i];
	}

	for (int i = 0; i < 7; i++)
		delete[] _fadingTables[i];

	for (int i = 0; i < 54; i++)
		delete[] _shapes[i];
	delete[] _shapes;

	delete[] _t1cps;

	delete[] _config->animData;
	delete[] _config->shapeDefs;
	delete[] _config->cpsData;
	delete _config;

	_vm->_sound->voiceStop(&_vm->_speechHandle);

	_screen->enableHiColorMode(true);
	_screen->clearCurPage();
	_screen->setFont(_prevFont);
	_screen->updateScreen();

	_system->delayMillis(150);
	_vm->resetSkipFlag(true);
	_vm->_allowSkip = false;
}

void DarkmoonSequenceHelper::loadScene(int index, int pageNum, bool ignorePalette) {
	Common::String file;
	Common::SeekableReadStream *s = 0;
	uint32 chunkID = 0;
	bool isRawData = false;

	if (_config->cpsFiles) {
		file = _config->cpsFiles[index];
		s = _vm->resource()->createReadStream(Common::Path(file));
	}

	if (s) {
		chunkID = s->readUint32LE();
		s->seek(0);
	}

	if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
		// Tolerance for differences up to 2 bytes is needed in some cases.
		if (s && ((((int32)(chunkID & 0xFFFF) + 5) & ~3) != (((s->size()) + 3) & ~3)))
			isRawData = true;
	} else if (file.firstChar() == 'X' && _vm->gameFlags().lang == Common::DE_DEU) {
		isRawData = true;
	}

	if (_config->cpsData[index]) {
		_screen->decodeSHP(_config->cpsData[index], pageNum);
	} else if (s && chunkID == MKTAG('F', 'O', 'R', 'M')) {
		// The original code also handles files with FORM chunks and ILBM and PBM sub chunks.
		// Up until now I haven't found any need for these (Amiga versions included).
		// We error out here theoretically, but this should never happen.
		error("DarkmoonSequenceHelper::loadScene(): CPS file loading failure in scene %d - unhandled FORM chunk encountered", index);

	} else if (s && !isRawData) {
		delete s;
		_screen->loadBitmap(_config->cpsFiles[index], pageNum | 1, pageNum | 1, ignorePalette ? 0 : _palettes[0]);

	} else if (s && _vm->gameFlags().platform == Common::kPlatformAmiga) {
		delete s;
		_screen->loadSpecialAmigaCPS(_config->cpsFiles[index], pageNum | 1, true);

	} else {
		if (!s) {
			file.setChar('X', 0);
			s = _vm->resource()->createReadStream(Common::Path(file));
		}

		if (!s)
			error("DarkmoonSequenceHelper::loadScene(): CPS file loading failure in scene %d", index);

		if (_config->loadScenePal)
			s->read(_palettes[0]->getData(), 768);
		else
			s->seek(768);
		_screen->loadFileDataToPage(s, 3, 64000);
		delete s;
	}

	int cp = _screen->setCurPage(pageNum);

	if (_config->shapeDefs[index]) {
		for (const DarkMoonShapeDef *df = _config->shapeDefs[index]; df->w; df++) {
			uint16 shapeIndex = (df->index < 0) ? df->index * -1 : df->index;
			if (_shapes[shapeIndex])
				delete[] _shapes[shapeIndex];
			_shapes[shapeIndex] = _screen->encodeShape(df->x, df->y, df->w, df->h, (df->index >> 8) != 0);
		}
	}

	_screen->setCurPage(cp);

	if (_vm->_configRenderMode == Common::kRenderEGA)
		setPalette(0);

	_screen->convertPage(pageNum | 1, pageNum, 0);

	if ((pageNum == 0 || pageNum == 1) && !_vm->skipFlag() && !_vm->shouldQuit())
		_screen->updateScreen();
}

void DarkmoonSequenceHelper::animCommand(int index, int del) {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return;

	index += _platformAnimOffset;
	uint32 end = 0;

	for (const DarkMoonAnimCommand *s = _config->animData[index]; s->command != 0xFF && !_vm->skipFlag() && !_vm->shouldQuit(); s++) {
		updateAmigaSound();

		int palIndex = s->pal + _config->animPalOffs;

		int x = s->x1;
		if (x >= Screen::SCREEN_W)
			x >>= 1;
		int y = s->y1;
		int x2 = 0;
		uint16 shapeW = 0;
		uint16 shapeH = 0;

		switch (s->command) {
		case 0:
			// flash palette
			if (_vm->_configRenderMode != Common::kRenderEGA && s->pal)
				setPaletteWithoutTextColor(palIndex);
			delay(s->delay);
			if (_vm->_configRenderMode != Common::kRenderEGA && _config->animCmdRestorePal && s->pal)
				setPaletteWithoutTextColor(0);
			break;

		case 1:
			// draw shape, then restore background
			shapeW = _shapes[s->obj][2];
			shapeH = _shapes[s->obj][3];

			if (_config->animCmd1ShapeFrame == 18) {
				_screen->setScreenDim(18);
				x -= (_screen->_curDim->sx << 3);
				y -= _screen->_curDim->sy;
				if (x < 0)
					shapeW -= ((-x >> 3) + 1);
				else
					x2 = x;
			}

			_screen->drawShape(0, _shapes[s->obj], x, y, _config->animCmd1ShapeFrame);

			if (_vm->_configRenderMode != Common::kRenderEGA && s->pal)
				setPaletteWithoutTextColor(palIndex);
			else
				_screen->updateScreen();

			delay(s->delay);

			if (_config->animCmd1ShapeFrame == 0) {
				if (_vm->_configRenderMode != Common::kRenderEGA && s->pal)
					setPaletteWithoutTextColor(0);
				_screen->copyRegion(x - 8, y - 8, x, y, (shapeW + 1) << 3, shapeH, 2, 0, Screen::CR_NO_P_CHECK);
			} else {
				_screen->copyRegion(x2, y, x2 + (_screen->_curDim->sx << 3), y + _screen->_curDim->sy, (shapeW + 1) << 3, shapeH, 2, 0, Screen::CR_NO_P_CHECK);
			}

			_screen->updateScreen();
			break;

		case 2:
			// draw shape
			_screen->drawShape(_screen->_curPage, _shapes[s->obj], x, y, 0);

			if (_vm->_configRenderMode != Common::kRenderEGA && s->pal)
				setPaletteWithoutTextColor(palIndex);
			else if (!_screen->_curPage)
				_screen->updateScreen();

			delay(s->delay);

			if (_vm->_configRenderMode != Common::kRenderEGA && _config->animCmdRestorePal && s->pal)
				setPaletteWithoutTextColor(0);
			break;

		case 3:
		case 4:
		case 101: { // ScummVM extension
			int isT1 = s->command == 101;
			int shapeWidth = 0, shapeHeight = 0;
			// fade shape in or out or restore background
			if (!_config->shpBackgroundFading)
				break;

			if (isT1 && !_t1cps) {
				_t1cps = new byte[64000];
				memset(_t1cps, 0, 4);
				Common::ScopedPtr<Common::SeekableReadStream> srcStream(_vm->resource()->createReadStream("T1.CPS"));
				if (srcStream)
					Screen_EoB::eob2ChineseLZUncompress(_t1cps, 64000, srcStream.get());
			}
			if (isT1) {
				shapeWidth = READ_LE_UINT16(_t1cps);
				shapeHeight = READ_LE_UINT16(_t1cps + 2);
			} else {
				shapeWidth = (_shapes[s->obj][2] + 1) << 3;
				shapeHeight = _shapes[s->obj][3];
			}

			if (_vm->_configRenderMode == Common::kRenderEGA) {
				if (palIndex && isT1)
					_screen->drawT1Shape(0, _t1cps, s->x1, y, 0);
				else if (palIndex)
					_screen->drawShape(0, _shapes[s->obj], s->x1, y, 0);
				else
					_screen->copyRegion(s->x1 - 8, s->y1 - 8, s->x1, s->y1, shapeWidth, shapeHeight, 2, 0, Screen::CR_NO_P_CHECK);
				_screen->updateScreen();
				delay(s->delay);
			} else if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
				end = _system->getMillis() + s->delay * _vm->tickLength();

				if (--palIndex) {
					uint8 obj = (palIndex - 1) * 10 + s->obj;
					_screen->copyRegion(s->x1 - 8, s->y1 - 8, 0, 0, (_shapes[obj][2] + 1) << 3, _shapes[obj][3], 2, 4, Screen::CR_NO_P_CHECK);
					_screen->drawShape(4, _shapes[obj], s->x1 & 7, 0, 0);
					_screen->copyRegion(0, 0, s->x1, s->y1, (_shapes[obj][2] + 1) << 3, _shapes[obj][3], 4, 0, Screen::CR_NO_P_CHECK);
				} else {
					_screen->copyRegion(s->x1 - 8, s->y1 - 8, s->x1, s->y1, shapeWidth, shapeHeight, 2, 0, Screen::CR_NO_P_CHECK);
				}
				_screen->updateScreen();

				_vm->delayUntil(end);
			} else {
				_screen->enableShapeBackgroundFading(true);
				_screen->setShapeFadingLevel(1);

				end = _system->getMillis() + s->delay * _vm->tickLength();

				if (palIndex) {
					_screen->setFadeTable(_fadingTables[palIndex - 1]);

					_screen->copyRegion(s->x1 - 8, s->y1 - 8, 0, 0, shapeWidth, shapeHeight, 2, 4, Screen::CR_NO_P_CHECK);
					if (isT1)
						_screen->drawT1Shape(4, _t1cps, 0, 0, 0);
					else
						_screen->drawShape(4, _shapes[s->obj], s->x1 & 7, 0, 0);
					_screen->copyRegion(0, 0, s->x1, s->y1, shapeWidth, shapeHeight, 4, 0, Screen::CR_NO_P_CHECK);
				} else {
					_screen->copyRegion(s->x1 - 8, s->y1 - 8, s->x1, s->y1, shapeWidth, shapeHeight, 2, 0, Screen::CR_NO_P_CHECK);
				}
				_screen->updateScreen();

				_vm->delayUntil(end);
				_screen->enableShapeBackgroundFading(false);
				_screen->setShapeFadingLevel(0);
			}
			break;
		}

		case 5:
			// copy region
			if (_config->animCmd5SetPal && s->pal)
				setPaletteWithoutTextColor(palIndex);

			_screen->copyRegion(s->x2 << 3, s->y2, s->x1, s->y1, s->w << 3, s->h, s->obj ? _config->animCmd5AltPage : 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
			delay(s->delay);
			break;

		case 6:
			// play sound effect
			if (s->obj != 0xFF)
				_vm->snd_playSoundEffect(s->obj);
			break;

		case 7:
			// restore background (only used in EGA mode)
			delay(s->delay);
			_screen->copyRegion(s->x1 - 8, s->y1 - 8, s->x1, s->y1, (_shapes[s->obj][2] + 1) << 3, _shapes[s->obj][3], 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
			break;

		default:
			error("DarkmoonSequenceHelper::animCommand(): Unknown animation opcode encountered.");
			break;
		}
	}

	if (del > 0)
		delay(del);
}

void DarkmoonSequenceHelper::setPlatformAnimIndexOffset(int offset) {
	_platformAnimOffset = offset;
}

void DarkmoonSequenceHelper::printText(int index, int color) {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return;

	_screen->setClearScreenDim(17);

	if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
		memset(_textColor, 58, 3);
		_palettes[0]->copy(_textColor, 0, 1, 31);
		color = 31;
	} else if (_vm->_configRenderMode != Common::kRenderEGA) {
		_palettes[0]->copy(*_palettes[0], color, 1, 255);
		setPalette(0);
		color = 255;
	}

	Common::String str = _config->strings[index];

	if (_config->voicePattern) {
		Common::String file(Common::String::format(_config->voicePattern, index + 1));
		if (_vm->_sound->isVoicePresent(file.c_str()))
			_vm->_sound->voicePlay(file.c_str(), &_vm->_speechHandle);
	}

	const ScreenDim *dm = _screen->_curDim;
	int fontHeight = (_vm->gameFlags().platform == Common::kPlatformPC98) ? (_screen->getFontHeight() << 1) : (_screen->getFontHeight() + 1);
	int xAlignFactor = (_vm->gameFlags().platform == Common::kPlatformPC98) ? 2 : 1;
	const char *linebrkChars = (_vm->gameFlags().lang == Common::ZH_TWN) ? "\r " : "\r";

	for (int yOffs = 0; !str.empty(); yOffs += fontHeight) {
		uint linebrk = str.findFirstOf(linebrkChars);
		Common::String str2 = (linebrk != Common::String::npos) ? str.substr(0, linebrk) : str;
		printStringIntern(str2.c_str(), (dm->sx * xAlignFactor + ((dm->w * xAlignFactor - str2.size()) >> 1)) << (4 - xAlignFactor), dm->sy + yOffs, color);
		str = (linebrk != Common::String::npos) ? str.substr(linebrk + 1) : "";
	}

	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		_screen->fadePalette(*_palettes[0], 20);
	else
		_screen->updateScreen();
}

void DarkmoonSequenceHelper::fadeText() {
	uint8 col = _vm->gameFlags().platform == Common::kPlatformAmiga ? 31 : 255;

	if (_vm->skipFlag() || _vm->shouldQuit()) {
		_screen->clearCurDim();
		_screen->setPaletteIndex(col, 0, 0, 0);
		return;
	}

	if (_vm->_configRenderMode != Common::kRenderEGA)
		_screen->fadeTextColor(_palettes[0], col, 8);

	memset(_textColor, 0, 3);
	_screen->setClearScreenDim(17);

	// The Chinese version uses a shadow color for the font which does not get faded.
	// We clear the shadow as quick as possible after the fading, so it will look less weird.
	if (_vm->gameFlags().lang == Common::ZH_TWN)
		_screen->updateScreen();
}

void DarkmoonSequenceHelper::update(int srcPage) {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return;

	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		_screen->fadeToBlack(5);

	_screen->copyRegion(0, 0, 8, 8, 304, 128, srcPage, 0, Screen::CR_NO_P_CHECK);

	if (_vm->_configRenderMode != Common::kRenderEGA)
		setPaletteWithoutTextColor(0);

	_screen->updateScreen();
}

void DarkmoonSequenceHelper::setPalette(int index) {
	_screen->setScreenPalette(*_palettes[index]);
}

void DarkmoonSequenceHelper::fadePalette(int index, int del) {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return;
	if (_vm->_configRenderMode == Common::kRenderEGA) {
		setPalette(index);
		_screen->updateScreen();
	} else {
		_screen->fadePalette(*_palettes[index], del * _vm->tickLength());
	}
}

void DarkmoonSequenceHelper::copyPalette(int srcIndex, int destIndex) {
	_palettes[destIndex]->copy(*_palettes[srcIndex]);
}

int DarkmoonSequenceHelper::hScroll(bool restart) {
	if (restart) {
		_hScrollStartTimeStamp = _system->getMillis();
		_hScrollState = -1;
	} else if (!_hScrollStartTimeStamp) {
		return 0;
	}

	uint32 ct = _system->getMillis();
	int state = (ct - _hScrollStartTimeStamp) / 18;
	if (state < 0 || state > 279) {
		_hScrollStartTimeStamp += (ct - _hScrollResumeTimeStamp);
		state = (ct - _hScrollStartTimeStamp) / 18;
		if (state < 0 || state > 279)
			state = 279;
	}

	_hScrollResumeTimeStamp = ct;

	if (state != _hScrollState) {
		_screen->copyRegion(9, 8, 8, 8, 303, 128, 0, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(state, 0, 311, 8, 1, 128, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}

	_hScrollState = state;

	if (state == 279) {
		_hScrollStartTimeStamp = 0;
		_hScrollState = -1;
	}

	return state;
}

void DarkmoonSequenceHelper::initDelayedPaletteFade(int palIndex, int rate) {
	_palettes[11]->copy(*_palettes[0]);

	_fadePalIndex = palIndex;
	_fadePalRate = rate;
	_fadePalTimer = _system->getMillis() + 2 * _vm->_tickLength;
}

bool DarkmoonSequenceHelper::processDelayedPaletteFade() {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return true;

	if (_vm->_configRenderMode == Common::kRenderEGA || !_fadePalRate || (_system->getMillis() <= _fadePalTimer))
		return false;

	if (_screen->delayedFadePalStep(_palettes[_fadePalIndex], _palettes[0], _fadePalRate)) {
		setPaletteWithoutTextColor(0);
		_fadePalTimer = _system->getMillis() + 3 * _vm->_tickLength;
	} else {
		_fadePalRate = 0;
	}

	return false;
}

void DarkmoonSequenceHelper::delay(uint32 ticks) {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return;

	uint32 end = _system->getMillis() + ticks * _vm->_tickLength;

	if (_config->palFading) {
		do {
			if (processDelayedPaletteFade())
				break;
			_vm->updateInput();
		} while (end > _system->getMillis());
		processDelayedPaletteFade();

	} else {
		for (uint32 ct = 0; ct < end; ) {
			if (ct + 18 <= end)
				hScroll();
			ct = _system->getMillis();
			_vm->delay(MIN<uint32>(9, end - ct));
		}
	}
}

void DarkmoonSequenceHelper::waitForSongNotifier(int index, bool introUpdateAnim) {
	if (_vm->gameFlags().platform == Common::kPlatformFMTowns)
		index = _sndMarkersFMTowns[index - 1];
	else if (_vm->sound()->getMusicType() != Sound::kAdLib && _vm->gameFlags().platform != Common::kPlatformPC98)
		return;

	int seq = 0;

	while (_vm->sound()->musicEnabled() && _vm->sound()->checkTrigger() < index && !(_vm->skipFlag() || _vm->shouldQuit())) {
		if (introUpdateAnim) {
			animCommand(30 | seq);
			seq ^= 1;
		}

		if (_config->palFading)
			processDelayedPaletteFade();

		_vm->updateInput();
	}
}

void DarkmoonSequenceHelper::updateAmigaSound() {
	if (_vm->gameFlags().platform != Common::kPlatformAmiga || !_vm->sound()->musicEnabled())
		return;

	int ct = _vm->sound()->checkTrigger();
	if (ct < _sndNextTrackMarker)
		return;

	_vm->snd_playSong(_sndNextTrack++);
	if (_sndNextTrack == 4)
		_sndNextTrack = 1;

	static const uint16 interval[4] = { 0, 1015, 4461, 1770 };
	_sndNextTrackMarker = interval[_sndNextTrack];
}

void DarkmoonSequenceHelper::init(DarkmoonSequenceHelper::Mode mode) {
	assert(mode == kIntro || mode == kFinale);

	static const uint16 soundMarkersFMTowns[2][8] = {
		{  229,  447,  670, 1380, 2037, 3000, 4475, 4825 },
		{  475, 2030, 2200, 2752, 3475,    0,    0,    0 }
	};

	int size = 0;
	_platformAnimOffset = 0;
	_sndNextTrack = 1;
	_sndNextTrackMarker = 0;
	_sndMarkersFMTowns = soundMarkersFMTowns[mode];
	_hScrollStartTimeStamp = _hScrollResumeTimeStamp = 0;
	_hScrollState = _shadowColor = -1;

	if (mode == kIntro) {
		_config = new Config(
			_vm->staticres()->loadStrings(kEoB2IntroStrings, size),
			_vm->staticres()->loadStrings(kEoB2IntroCPSFiles, size),
			_vm->_flags.isTalkie ? "EOB%d" : nullptr,
			new const uint8*[16],
			_vm->_flags.platform == Common::kPlatformAmiga ? 0 : (_vm->_configRenderMode == Common::kRenderEGA ? _palFilesIntroEGA : _palFilesIntroVGA),
			new const DarkMoonShapeDef*[16],
			new const DarkMoonAnimCommand*[48],
			false,
			false,
			true,
			true,
			_vm->_flags.platform == Common::kPlatformAmiga ? 1 : 0,
			0,
			false,
			2
		);

		if (_vm->_flags.lang == Common::ZH_TWN)
			_shadowColor = _vm->guiSettings()->colors.guiColorBrown;

		for (int i = 0; i < 48; i++)
			_config->animData[i] = _vm->staticres()->loadEoB2SeqData(kEoB2IntroAnimData00 + i, size);

		for (int i = 0; i < 16; i++)
			_config->cpsData[i] = _vm->staticres()->loadRawData(kEoB2IntroCpsDataStreet1 + i, size);

		memset(_config->shapeDefs, 0, 16 * sizeof(DarkMoonShapeDef*));
		_config->shapeDefs[0] = _vm->staticres()->loadEoB2ShapeData(kEoB2IntroShapes00, size);
		_config->shapeDefs[1] = _vm->staticres()->loadEoB2ShapeData(kEoB2IntroShapes01, size);
		_config->shapeDefs[4] = _vm->staticres()->loadEoB2ShapeData(kEoB2IntroShapes04, size);
		_config->shapeDefs[7] = _vm->staticres()->loadEoB2ShapeData(kEoB2IntroShapes07, size);
		_config->shapeDefs[13] = _vm->staticres()->loadEoB2ShapeData(kEoB2IntroShapes13, size);
		_config->shapeDefs[14] = _vm->staticres()->loadEoB2ShapeData(kEoB2IntroShapes14, size);
		_config->shapeDefs[15] = _vm->staticres()->loadEoB2ShapeData(kEoB2IntroShapes15, size);

	} else {
		_config = new Config(
			_vm->staticres()->loadStrings(kEoB2FinaleStrings, size),
			_vm->staticres()->loadStrings(kEoB2FinaleCPSFiles, size),
			_vm->_flags.isTalkie ? "EOBF%d" : nullptr,
			new const uint8*[13],
			_vm->_flags.platform == Common::kPlatformAmiga ? _palFilesFinaleAmiga : (_vm->_configRenderMode == Common::kRenderEGA ? _palFilesFinaleEGA : _palFilesFinaleVGA),
			new const DarkMoonShapeDef*[13],
			new const DarkMoonAnimCommand *[21],
			true,
			true,
			false,
			false,
			_vm->_flags.platform == Common::kPlatformAmiga ? 2 : 1,
			18,
			true,
			6
		);

		if (_vm->_flags.lang == Common::ZH_TWN)
			_shadowColor = _vm->guiSettings()->colors.fill;

		for (int i = 0; i < 21; i++)
			_config->animData[i] = _vm->staticres()->loadEoB2SeqData(kEoB2FinaleAnimData00 + i, size);

		for (int i = 0; i < 13; i++)
			_config->cpsData[i] = _vm->staticres()->loadRawData(kEoB2FinaleCpsDataDragon1 + i, size);

		memset(_config->shapeDefs, 0, 13 * sizeof(DarkMoonShapeDef*));
		_config->shapeDefs[0] = _vm->staticres()->loadEoB2ShapeData(kEoB2FinaleShapes00, size);
		_config->shapeDefs[3] = _vm->staticres()->loadEoB2ShapeData(kEoB2FinaleShapes03, size);
		_config->shapeDefs[7] = _vm->staticres()->loadEoB2ShapeData(kEoB2FinaleShapes07, size);
		_config->shapeDefs[9] = _vm->staticres()->loadEoB2ShapeData(kEoB2FinaleShapes09, size);
		_config->shapeDefs[10] = _vm->staticres()->loadEoB2ShapeData(kEoB2FinaleShapes10, size);
	}

	_screen->enableHiColorMode(false);
	_screen->disableDualPaletteMode();
	int numColors = 256;

	if (_vm->_flags.platform == Common::kPlatformAmiga) {
		static const int8 palIndex[13] = { -1, -1, 3, 2, 4, 5, 6, 7, -1, -1, -1, -1, -1 };
		for (int i = 0; i < 13; ++i)
			_palettes[i] = &_screen->getPalette(i);
		Common::SeekableReadStream *s = _config->palFiles ? _vm->resource()->createReadStream(_config->palFiles[0]) : 0;
		numColors = 32;
		for (int i = 0; i < 13; ++i) {
			if (s && palIndex[i] != -1)
				_palettes[palIndex[i]]->loadAmigaPalette(*s, 0, 32);
		}
		delete s;
	} else {
		for (int i = 0; _config->palFiles[i]; i++) {
			if (i < 4)
				_palettes[i] = &_screen->getPalette(i);
			else
				_palettes[i] = new Palette(256);
			_screen->loadPalette(_config->palFiles[i], *_palettes[i]);
		}

		for (int i = 9; i < 13; ++i)
			_palettes[i] = new Palette(256);
	}

	_palettes[9]->fill(0, numColors, 0);
	_palettes[10]->fill(0, numColors, 63);
	_palettes[11]->fill(0, numColors, 0);


	if (_vm->gameFlags().platform == Common::kPlatformFMTowns)
		_screen->loadPalette("PALETTE.COL", *_palettes[12]);

	for (int i = 0; i < 7; i++)
		_fadingTables[i] = 0;

	uint8 *fadeData = (_vm->_configRenderMode != Common::kRenderCGA && _vm->_configRenderMode != Common::kRenderEGA) ? _vm->resource()->fileData("FADING.DAT", 0) : 0;

	if (fadeData) {
		for (int i = 0; i < 7; i++) {
			_fadingTables[i] = new uint8[256];
			memcpy(_fadingTables[i], fadeData + (i << 8), 256);
		}
	} else {
		if (_vm->_flags.platform != Common::kPlatformAmiga && _vm->_configRenderMode != Common::kRenderCGA && _vm->_configRenderMode != Common::kRenderEGA) {
			uint8 *pal = _vm->resource()->fileData("PALETTE1.PAL", 0);
			for (int i = 0; i < 7; i++)
				_screen->createFadeTable(pal, _fadingTables[i], 18, (i + 1) * 36);
			delete[] pal;
		}
	}

	delete[] fadeData;

	_shapes = new const uint8*[54];
	memset(_shapes, 0, 54 * sizeof(uint8*));

	_fadePalTimer = 0;
	_fadePalRate = 0;

	memset(_textColor, 0, 3);

	_screen->setScreenPalette(*_palettes[0]);
	_prevFont = _screen->setFont(_vm->gameFlags().lang == Common::Language::ZH_TWN ? Screen::FID_CHINESE_FNT :
		_vm->gameFlags().platform == Common::kPlatformFMTowns ? Screen::FID_SJIS_LARGE_FNT : (_vm->gameFlags().platform == Common::kPlatformPC98 ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT));
	_screen->hideMouse();

	_vm->delay(150);
	_vm->_eventList.clear();
	_vm->_allowSkip = true;
}

void DarkmoonSequenceHelper::setPaletteWithoutTextColor(int index) {
	if (_vm->_configRenderMode == Common::kRenderEGA || _vm->skipFlag() || _vm->shouldQuit())
		return;

	int numCol = (_vm->gameFlags().platform == Common::kPlatformAmiga) ? 31 : 255;

	if (_vm->gameFlags().platform != Common::kPlatformAmiga) {
		if (!memcmp(_palettes[11]->getData(), _palettes[index]->getData(), numCol * 3))
			return;
	}

	_palettes[11]->copy(*_palettes[index], 0, numCol);
	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		_palettes[11]->copy(_textColor, 0, 1, numCol);
	else
		_palettes[11]->copy(*_palettes[0], numCol, 1, numCol);
	setPalette(11);

	if (_hScrollState == -1) {
		_screen->updateScreen();
		_system->delayMillis(10);
	}
}

void DarkmoonSequenceHelper::printStringIntern(const char *str, int x, int y, int col) {
	if (_shadowColor != -1)
		_screen->printShadedText(str, x, y, col, 0, _shadowColor);
	else
		_screen->printText(str, x, y, col, _screen->_curDim->col2);
}

const char *const DarkmoonSequenceHelper::_palFilesIntroVGA[] = {
	"PALETTE1.PAL",
	"PALETTE3.PAL",
	"PALETTE2.PAL",
	"PALETTE4.PAL",
	0
};

const char *const DarkmoonSequenceHelper::_palFilesIntroEGA[] = {
	"PALETTE0.PAL",
	"PALETTE3.PAL",
	"PALETTE2.PAL",
	"PALETTE4.PAL",
	0
};

const char *const DarkmoonSequenceHelper::_palFilesFinaleVGA[] = {
	"FINALE_0.PAL",
	"FINALE_0.PAL",
	"FINALE_1.PAL",
	"FINALE_2.PAL",
	"FINALE_3.PAL",
	"FINALE_4.PAL",
	"FINALE_5.PAL",
	"FINALE_6.PAL",
	"FINALE_7.PAL",
	0
};

const char *const DarkmoonSequenceHelper::_palFilesFinaleEGA[] = {
	"FINALE_6.PAL",
	"FINALE_0.PAL",
	"FINALE_1.PAL",
	"FINALE_7.PAL",
	"FINALE_3.PAL",
	"FINALE_4.PAL",
	"FINALE_5.PAL",
	"FINALE_0.PAL",
	"FINALE_0.PAL",
	0
};

const char *const DarkmoonSequenceHelper::_palFilesFinaleAmiga[] = {
	"FINALE.PAL",
	0
};

void DarkMoonEngine::seq_nightmare() {
	Screen::FontId of = _screen->setFont(Screen::FID_6_FNT);
	_screen->clearCurDimOvl(0);
	_screen->copyRegion(0, 0, 0, 120, 176, 24, 12, 2, Screen::CR_NO_P_CHECK);
	initDialogueSequence();
	gui_drawDialogueBox();

	_txt->printDialogueText(99, 0);
	snd_playSoundEffect(54);

	static const uint8 seqX[] = { 0, 20, 0, 20 };
	static const uint8 seqY[] = { 0, 0, 96, 96 };
	static const uint8 seqDelay[] = { 12, 7, 7, 12 };

	for (const int8 *i = _dreamSteps; *i != -1; ++i) {
		drawSequenceBitmap("DREAM", 0, seqX[*i], seqY[*i], 0);
		delay(seqDelay[*i] * _tickLength);
	}

	_txt->printDialogueText(20, _okStrings[0]);

	restoreAfterDialogueSequence();

	_screen->setFont(of);
}

void DarkMoonEngine::seq_kheldran() {
	Screen::FontId of = _screen->setFont(Screen::FID_6_FNT);

	initDialogueSequence();
	gui_drawDialogueBox();

	static const char file[] = "KHELDRAN";
	_screen->set16bitShadingLevel(4);
	_txt->printDialogueText(_kheldranStrings[0]);
	drawSequenceBitmap(file, 0, 0, 0, 0);
	_txt->printDialogueText(20, _moreStrings[0]);
	snd_playSoundEffect(56);
	drawSequenceBitmap(file, 0, 20, 0, 0);
	delay(10 * _tickLength);
	drawSequenceBitmap(file, 0, 0, 96, 0);
	delay(10 * _tickLength);
	drawSequenceBitmap(file, 0, 20, 96, 0);
	delay(7 * _tickLength);
	_txt->printDialogueText(76, _okStrings[0]);

	restoreAfterDialogueSequence();

	_screen->setFont(of);
}

void DarkMoonEngine::seq_dranDragonTransformation() {
	Screen::FontId of = _screen->setFont(Screen::FID_6_FNT);

	initDialogueSequence();
	gui_drawDialogueBox();

	static const char file[] = "DRANX";
	drawSequenceBitmap(file, 0, 0, 0, 0);
	_txt->printDialogueText(120, _moreStrings[0]);
	snd_playSoundEffect(56);
	drawSequenceBitmap(file, 0, 20, 0, 0);
	delay(7 * _tickLength);
	drawSequenceBitmap(file, 0, 0, 96, 0);
	delay(7 * _tickLength);
	drawSequenceBitmap(file, 0, 20, 96, 0);
	delay(18 * _tickLength);

	restoreAfterDialogueSequence();

	_screen->setFont(of);
}

} // End of namespace Kyra

#endif // ENABLE_EOB
