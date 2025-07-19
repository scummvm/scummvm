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

#include "common/scummsys.h"
#include "mm/xeen/worldofxeen/worldofxeen_menu.h"
#include "mm/xeen/worldofxeen/worldofxeen.h"
#include "mm/xeen/dialogs/credits_screen.h"
#include "mm/xeen/dialogs/dialogs_difficulty.h"
#include "mm/xeen/resources.h"

namespace MM {
namespace Xeen {
namespace WorldOfXeen {

#ifdef USE_TTS

static const uint8 kOtherOptionsMaxButtonCount = 5;

static const char *worldMainMenuButtons[] = {
	"Start a New Game\nLoad a Saved Game\nCredits\nOther Options\n",	// English
	"Neues Spiel\nSpielstand laden\nCredits\nWeitere Optionen\n",		// German
	"Nouvelle Partie\nCharger une Partie\nCredits\nOptions\n",			// French
	"Partida nueva\nCargar una partida\nCr\44ditos\nOtras opciones\n"	// Spanish
	// No Russian or Chinese versions exist
};

static const char *worldOtherOptionsButtons[] = {
	"View Darkside Intro\nView Clouds Intro\nView Darkside End\nView Clouds End\nView World End\n",			// English
	"Darkside-Intro\nClouds-Intro\nDarkside-Ende\nClouds-Ende\nWorld-Ende\n",								// German
	"Intro de Darkside\nIntro des Nuages\nFin de Darkside\nFin des Nuages\nFin de World\n",					// French	
	"Intro de Darkside\nIntro de Clouds\nVer Final de Darkside\nVer Final de Clouds\nVer Final de World\n",	// Spanish
	"",																										// Russian (no version exists)
	// Chinese
	"\xb6\xc2\xb7\x74\xbb\xe2\xa5\x44\xa4\xb6\xd2\xd0\n"	// 黑暗領主介紹
	"\xb6\xb3\xa4\xb6\xd2\xd0\n"							// 雲介紹
	"\xb6\xc2\xb7\x74\xbb\xe2\xa5\x44\xa5\xbd\xa7\xc0\n"	// 黑暗領主末尾
	"\xb6\xb3\xa5\xbd\xa7\xc0\n"							// 雲末尾
	"\xa5\x40\xac\xc9\n"									// 世界末尾
};

static const char *cloudsMainMenuButtons[] = {
	"Start a New Game\nLoad a Saved Game\nCredits\nView Endgame\n",	// English
	"Neues Spiel\nSpielstand laden\nCredits\nEndspiel ansehen\n",	// German
	"Nouvelle Partie\nCharger une Partie\nCr\202dits\nVoir la fin",	// French
	"",																// Spanish (no version exists)
	// Russian
	"\x8d\xa0\xe7\xa0\xe2\xec \xad\xae\xa2\xe3\xee \xa8\xa3\xe0\xe3\n"	// Начать новую игру
	"\x87\xa0\xa3\xe0\xe3\xa7\xa8\xe2\xec \xa8\xa3\xe0\xe3\n"			// Загрузить игру
	"\x91\xae\xa7\xa4\xa0\xe2\xa5\xab\xa8\n"							// Создатели
	"\x94\xa8\xad\xa0\xab\xec\xad\xeb\xa9 \xe0\xae\xab\xa8\xaa\n",		// Финальный ролик
	// Chinese
	"\xb6\x7d\xa9\x6c\xb7\x73\xb9\x43\xc0\xb8\n"						// 開始新遊戲
	"\xb8\xfc\xa4\x4a\xb6\x69\xab\xd7\xc0\xc9\n"						// 載入進度檔
	"\xb9\x43\xc0\xb8\xbb\x73\xa7\x40\xb8\x73\n"						// 遊戲製作群
	"\xac\x64\xac\xdd\xb4\xdd\xa7\xbd\n"								// 查看殘局
};

static const char *darksideMainMenuButtons[] = {
	"Start\nLoad\nView Credits\nOther Options\n",	// English
	"Start\nLaden\nCredits\nWeitere Optionen\n",	// German
	"Nouveau\nPartie\nCredits\nOptions\n",			// French
	"",												// Spanish (no version exists)
	"",												// Russian (no version exists)
	// Chinese
	"\xb6\x7d\xa9\x6c\xb7\x73\xb9\x43\xc0\xb8\n"	// 開始新遊戲
	"\xb8\xfc\xa4\x4a\xc2\xc2\xc0\xc9\n"			// 載入舊檔
	"\xf7\x54\xac\xdd\xb3\x5d\xad\x70\xb8\x73\n"	// 覾看設計群
	"\xa8\xe4\xa5\x4c\xbf\xef\xb6\xb5\n"			// 其他選項
};

#endif

enum CloudsMainMenuButtonTTSTextIndex {
	kCloudsNew = 0,
	kCloudsLoad = 1,
	kCloudsCredits = 2,
	kCloudsEndgame = 3
};

enum DarksideWorldMainMenuButtonTTSTextIndex {
	kDarksideWorldStart = 0,
	kDarksideWorldLoad = 1,
	kDarksideWorldCredits = 2,
	kDarksideWorldOther = 3
};

enum OtherOptionsButtonTTSTextIndex {
	kDarksideIntro = 0,
	kCloudsIntro = 1,
	kDarksideEnd = 2,
	kCloudsEnd = 3,
	kWorldEnd = 4
};

void MainMenuContainer::show() {
	MainMenuContainer *menu;

	switch (g_vm->getGameID()) {
	case GType_Clouds:
		menu = new CloudsMainMenuContainer();
		break;
	case GType_DarkSide:
		menu = new DarkSideMainMenuContainer();
		break;
	case GType_WorldOfXeen:
		if (g_vm->getIsCD())
			menu = new WorldOfXeenCDMainMenuContainer();
		else
			menu = new WorldOfXeenMainMenuContainer();
		break;
	default:
		error("Invalid game");
	}

	menu->execute();
	delete menu;
}

MainMenuContainer::MainMenuContainer(const char *spritesName1, const char *spritesName2, const char *spritesName3) :
		_animateCtr(0), _dialog(nullptr) {
	g_vm->_files->setGameCc(g_vm->getGameID() == GType_Clouds ? 0 : 1);

	_backgroundSprites.resize(1 + (spritesName2 ? 1 : 0) + (spritesName3 ? 1 : 0));
	_backgroundSprites[0].load(spritesName1);
	if (spritesName2)
		_backgroundSprites[1].load(spritesName2);
	if (spritesName3)
		_backgroundSprites[2].load(spritesName3);

	_frameCount = 0;
	for (uint idx = 0; idx < _backgroundSprites.size(); ++idx)
		_frameCount += _backgroundSprites[idx].size();
}

MainMenuContainer::~MainMenuContainer() {
	delete _dialog;
	g_vm->_windows->closeAll();
	g_vm->_sound->stopAllAudio();
	g_vm->_events->clearEvents();
}

void MainMenuContainer::draw() {
	g_vm->_screen->restoreBackground();
	_animateCtr = (_animateCtr + 1) % _frameCount;

	// Draw the next background frame
	uint frameNum = _animateCtr;
	for (uint idx = 0; idx < _backgroundSprites.size(); ++idx) {
		if (frameNum < _backgroundSprites[idx].size()) {
			_backgroundSprites[idx].draw(0, frameNum);
			return;
		} else {
			frameNum -= _backgroundSprites[idx].size();
		}
	}
}

void MainMenuContainer::execute() {
	EventsManager &events = *g_vm->_events;
	Screen &screen = *g_vm->_screen;
	bool showFlag = false;

	// Show the cursor
	events.clearEvents();
	events.setCursor(0);
	events.showCursor();

	screen.doScroll(true, false);

	if (_dialog) {
		_dialog->_ttsVoiceText = true;
	}

	while (!g_vm->shouldExit() && g_vm->_gameMode == GMODE_NONE) {
		// Draw the menu
		draw();
		if (_dialog) {
			_dialog->draw();
#ifdef USE_TTS
			_dialog->checkHoverOverButton();
#endif
			_dialog->_ttsVoiceText = false;
		}

		// Fade/scroll in screen if first frame showing screen
		if (!showFlag) {
			display();
			screen.doScroll(false, true);
			showFlag = true;
		}

		// Check for events
		events.updateGameCounter();

		if (events.wait(4, true)) {
			if (_dialog) {
				// There's a dialog active, so let it handle the event
				_dialog->handleEvents();

				// If dialog was removed as a result of the event, flag screen for re-showing,
				// such as returning to main menu from the Credits screen
				if (!_dialog)
					showFlag = false;
			} else {
				// No active dialog. If Escape pressed, exit game entirely. Otherwise,
				// open up the main menu dialog
				PendingEvent pe;
				if (events.getEvent(pe) && pe._keyState.keycode == Common::KEYCODE_ESCAPE)
					g_vm->_gameMode = GMODE_QUIT;

				events.clearEvents();
				showMenuDialog();
			}
		}
	}
}

/*------------------------------------------------------------------------*/

CloudsMainMenuContainer::CloudsMainMenuContainer() : MainMenuContainer("intro.vga") {
}

void CloudsMainMenuContainer::display() {
	FileManager &files = *g_vm->_files;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	sound._musicSide = 0;
	files.setGameCc(0);

	screen.loadPalette("mm4.pal");
	screen.loadBackground("intro.raw");
	screen.saveBackground();

	if (!sound.isMusicPlaying())
		sound.playSong("inn.m");
}

void CloudsMainMenuContainer::showMenuDialog() {
	setOwner(new CloudsMenuDialog(this));
}

/*------------------------------------------------------------------------*/

DarkSideMainMenuContainer::DarkSideMainMenuContainer() : MainMenuContainer("title2a.int") {
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;
	screen.loadPalette("dark.pal");
	screen.fadeIn(0x81);

	sound._musicSide = 1;
	sound.playSong("newbrigh.m");

	_background.load("title2.int");
}

void DarkSideMainMenuContainer::display() {
	FileManager &files = *g_vm->_files;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	_background.draw(0, 0, Common::Point(0, 0));
	_background.draw(0, 1, Common::Point(160, 0));

	sound._musicSide = 1;
	files.setGameCc(1);

	screen.loadPalette("dark.pal");
	screen.saveBackground();

	if (!sound.isMusicPlaying())
		sound.playSong("newbrigh.m");
}

void DarkSideMainMenuContainer::showMenuDialog() {
	setOwner(new DarkSideMenuDialog(this));
}

/*------------------------------------------------------------------------*/

WorldOfXeenMainMenuContainer::WorldOfXeenMainMenuContainer() : MainMenuContainer("world.int") {
}

void WorldOfXeenMainMenuContainer::display() {
	FileManager &files = *g_vm->_files;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	sound._musicSide = 1;
	files.setGameCc(1);

	screen.loadPalette("dark.pal");
	screen.loadBackground("world.raw");
	screen.saveBackground();

	if (!sound.isMusicPlaying())
		sound.playSong("newbrigh.m");
}

void WorldOfXeenMainMenuContainer::showMenuDialog() {
	setOwner(new WorldMenuDialog(this));
}

/*------------------------------------------------------------------------*/

WorldOfXeenCDMainMenuContainer::WorldOfXeenCDMainMenuContainer() : MainMenuContainer("world0.int", "world1.int", "world2.int") {
}

void WorldOfXeenCDMainMenuContainer::display() {
	FileManager &files = *g_vm->_files;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	sound._musicSide = 1;
	files.setGameCc(1);

	screen.loadPalette("dark.pal");
	screen.loadBackground("world.raw");
	screen.saveBackground();

	if (!sound.isMusicPlaying())
		sound.playSong("newbrigh.m");
}

void WorldOfXeenCDMainMenuContainer::showMenuDialog() {
	setOwner(new WorldMenuDialog(this));
}

/*------------------------------------------------------------------------*/

bool MainMenuDialog::handleEvents() {
	FileManager &files = *g_vm->_files;
	checkEvents(g_vm);
	int difficulty;

	if (Res.KeyConstants.CloudsOfXeenMenu.KEY_START_NEW_GAME == _buttonValue) {
		// Start new game
		difficulty = DifficultyDialog::show(g_vm);
		if (difficulty == -1)
			return true;

		// Load a new game state and set the difficulty
		g_vm->_saves->newGame();
		g_vm->_party->_difficulty = (Difficulty)difficulty;
		g_vm->_gameMode = GMODE_PLAY_GAME;
	} else if (Res.KeyConstants.CloudsOfXeenMenu.KEY_LOAD_GAME == _buttonValue) {
		// Load existing game
		int ccNum = files._ccNum;
		g_vm->_saves->newGame();
		if (!g_vm->_saves->loadGame()) {
			files.setGameCc(ccNum);
			return true;
		}

		g_vm->_gameMode = GMODE_PLAY_GAME;
	} else if (Res.KeyConstants.CloudsOfXeenMenu.KEY_SHOW_CREDITS == _buttonValue) {
		// Show credits
		CreditsScreen::show(g_vm);
	} else if (Common::KEYCODE_ESCAPE == _buttonValue) {
		// Exit dialog (returning to just the animated background)
	} else {
		return false;
	}

	// If this point is reached, delete the dialog itself, which will return the main menu
	// to it's default "No dialog showing" state
	delete this;
	return true;
}

/*------------------------------------------------------------------------*/

CloudsMenuDialog::CloudsMenuDialog(MainMenuContainer *owner) : MainMenuDialog(owner) {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.setBounds(Common::Rect(72, 25, 248, g_vm->_gameWon[0] ? 175 : 150));
	w.open();

	loadButtons();
#ifdef USE_TTS
	setButtonTexts(cloudsMainMenuButtons[_vm->_ttsLanguage]);
#endif
}

CloudsMenuDialog::~CloudsMenuDialog() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.close();
}

void CloudsMenuDialog::loadButtons() {
	_buttonSprites.load("start.icn");
	addButton(Common::Rect(93, 53, 227, 73), Res.KeyConstants.CloudsOfXeenMenu.KEY_START_NEW_GAME, &_buttonSprites, kCloudsNew);
	addButton(Common::Rect(93, 78, 227, 98), Res.KeyConstants.CloudsOfXeenMenu.KEY_LOAD_GAME, &_buttonSprites, kCloudsLoad);
	addButton(Common::Rect(93, 103, 227, 123), Res.KeyConstants.CloudsOfXeenMenu.KEY_SHOW_CREDITS, &_buttonSprites, kCloudsCredits);
	if (g_vm->_gameWon[0])
		addButton(Common::Rect(93, 128, 227, 148), Res.KeyConstants.CloudsOfXeenMenu.KEY_VIEW_ENDGAME, &_buttonSprites, kCloudsEndgame);
}

void CloudsMenuDialog::draw() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];

	w.frame();
	w.writeString(Common::String::format(Res.OPTIONS_MENU, Res.GAME_NAMES[0], g_vm->_gameWon[0] ? 117 : 92, 1992), _ttsVoiceText);
	drawButtons(&w);
}

bool CloudsMenuDialog::handleEvents() {
	if (MainMenuDialog::handleEvents())
		return true;

	if (Res.KeyConstants.CloudsOfXeenMenu.KEY_VIEW_ENDGAME == _buttonValue) {
		if (g_vm->_gameWon[0]) {
			// Close the window
			delete this;

			// Show clouds ending
			WOX_VM._sound->stopAllAudio();
			WOX_VM.showCloudsEnding(g_vm->_finalScore);
			return true;
		}
	}

	return false;
}

/*------------------------------------------------------------------------*/

DarkSideMenuDialog::DarkSideMenuDialog(MainMenuContainer *owner) : MainMenuDialog(owner), _firstDraw(true) {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.setBounds(Common::Rect(72, 25, 248, 150));
	w.open();

	loadButtons();
#ifdef USE_TTS
	setButtonTexts(darksideMainMenuButtons[_vm->_ttsLanguage]);
#endif
}

DarkSideMenuDialog::~DarkSideMenuDialog() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.close();
}

void DarkSideMenuDialog::loadButtons() {
	addButton(Common::Rect(124, 87, 177, 97), Common::KEYCODE_s, nullptr, kDarksideWorldStart);
	addButton(Common::Rect(126, 98, 173, 108), Common::KEYCODE_l, nullptr, kDarksideWorldLoad);
	addButton(Common::Rect(91, 110, 209, 120), Common::KEYCODE_c, nullptr, kDarksideWorldCredits);
	addButton(Common::Rect(85, 121, 216, 131), Common::KEYCODE_o, nullptr, kDarksideWorldOther);
}

void DarkSideMenuDialog::draw() {
	Screen &screen = *g_vm->_screen;
	EventsManager &events = *g_vm->_events;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;

	if (!_firstDraw)
		return;

	SpriteResource kludgeSprites("kludge.int");
	SpriteResource title2Sprites[8] = {
		SpriteResource("title2b.int"), SpriteResource("title2c.int"),
		SpriteResource("title2d.int"), SpriteResource("title2e.int"),
		SpriteResource("title2f.int"), SpriteResource("title2g.int"),
		SpriteResource("title2h.int"), SpriteResource("title2i.int"),
	};

	screen.loadBackground("title2b.raw");
	kludgeSprites.draw(0, 0, Common::Point(85, 86));
	screen.saveBackground();
	sound.playSound("elect.voc");

	for (int i = 0; i < 30 && !g_vm->shouldExit(); ++i) {
		events.updateGameCounter();
		screen.restoreBackground();
		title2Sprites[i / 4].draw(0, i % 4);
		windows[0].update();

		if (i == 19)
			sound.stopSound();

		if (events.wait(2))
			break;
	}

	events.clearEvents();
	sound.stopSound();

	screen.restoreBackground();
	windows[0].update();
	_firstDraw = false;
}

bool DarkSideMenuDialog::handleEvents() {
	if (MainMenuDialog::handleEvents())
		return true;

	switch (_buttonValue) {
	case Common::KEYCODE_o: {
		// Show other options dialog
		// Remove this dialog
		MainMenuContainer *owner = _owner;
		delete this;

		// Set the new options dialog
		owner->setOwner(new OtherOptionsDialog(owner));
		return true;
	}

	default:
		break;
	}

	return false;
}

/*------------------------------------------------------------------------*/

WorldMenuDialog::WorldMenuDialog(MainMenuContainer *owner) : MainMenuDialog(owner) {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.setBounds(Common::Rect(72, 25, 248, 175));
	w.open();

	loadButtons();
#ifdef USE_TTS
	setButtonTexts(worldMainMenuButtons[_vm->_ttsLanguage]);
#endif
}

WorldMenuDialog::~WorldMenuDialog() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.close();
}

void WorldMenuDialog::loadButtons() {
	_buttonSprites.load("start.icn");
	addButton(Common::Rect(93, 53, 227, 73), Common::KEYCODE_s, &_buttonSprites, kDarksideWorldStart);
	addButton(Common::Rect(93, 78, 227, 98), Common::KEYCODE_l, &_buttonSprites, kDarksideWorldLoad);
	addButton(Common::Rect(93, 103, 227, 123), Common::KEYCODE_c, &_buttonSprites, kDarksideWorldCredits);
	addButton(Common::Rect(93, 128, 227, 148), Common::KEYCODE_o, &_buttonSprites, kDarksideWorldOther);
}

void WorldMenuDialog::draw() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];

	w.frame();
	w.writeString(Common::String::format(Res.OPTIONS_MENU, Res.GAME_NAMES[2], 117, 1993), _ttsVoiceText);
	drawButtons(&w);
}

bool WorldMenuDialog::handleEvents() {
	if (MainMenuDialog::handleEvents())
		return true;

	switch (_buttonValue) {
	case Common::KEYCODE_o: {
		// Show other options dialog
		// Remove this dialog
		MainMenuContainer *owner = _owner;
		delete this;

		// Set the new options dialog
		owner->setOwner(new OtherOptionsDialog(owner));
		return true;
	}

	default:
		break;
	}

	return false;
}

/*------------------------------------------------------------------------*/

OtherOptionsDialog::OtherOptionsDialog(MainMenuContainer *owner) : MenuContainerDialog(owner) {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];

	int height = (g_vm->getGameID() == GType_WorldOfXeen ? 25 : 0)
		+ (g_vm->getGameID() == GType_WorldOfXeen && g_vm->_gameWon[0] ? 25 : 0)
		+ (g_vm->_gameWon[1] ? 25 : 0)
		+ (g_vm->_gameWon[2] ? 25 : 0)
		+ 75;

	w.setBounds(Common::Rect(72, 25, 248, 25 + height));
	w.open();

	loadButtons();
}

OtherOptionsDialog::~OtherOptionsDialog() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.close();
}

void OtherOptionsDialog::loadButtons() {
	_buttonSprites.load("special.icn");
	Common::Rect r(93, 53, 227, 73);

	// View Darkside Intro
	addButton(r, Common::KEYCODE_d, &_buttonSprites, kDarksideIntro);
	r.translate(0, 25);

	// View Clouds Intro
	if (g_vm->getGameID() == GType_WorldOfXeen) {
		addButton(r, Common::KEYCODE_c, &_buttonSprites, kCloudsIntro);
		r.translate(0, 25);
	} else {
		addButton(Common::Rect(), Common::KEYCODE_INVALID, nullptr, kCloudsIntro);
	}

	// View Darkside End
	if (g_vm->_gameWon[1]) {
		addButton(r, Common::KEYCODE_e, &_buttonSprites, kDarksideEnd);
		r.translate(0, 25);
	} else {
		addButton(Common::Rect(), Common::KEYCODE_INVALID, nullptr, kDarksideEnd);
	}

	// View Clouds End
	if (g_vm->_gameWon[0]) {
		addButton(r, Common::KEYCODE_v, &_buttonSprites, kCloudsEnd);
		r.translate(0, 25);
	} else {
		addButton(Common::Rect(), Common::KEYCODE_INVALID, nullptr, kCloudsEnd);
	}

	// View World End
	if (g_vm->_gameWon[2]) {
		addButton(r, Common::KEYCODE_w, &_buttonSprites, kWorldEnd);
	} else {
		addButton(Common::Rect(), Common::KEYCODE_INVALID, nullptr, kWorldEnd);
	}

#ifdef USE_TTS
	uint index = 0;
	addNextTextToButtons(worldOtherOptionsButtons[_vm->_ttsLanguage], index, kOtherOptionsMaxButtonCount);
#endif
}

void OtherOptionsDialog::draw() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];

	w.frame();
	w.writeString(Common::String::format(Res.OPTIONS_MENU,
		Res.GAME_NAMES[g_vm->getGameID() == GType_WorldOfXeen ? 2 : 1],
		w.getBounds().height() - 33, 1993), _ttsVoiceText);
	drawButtons(&w);
}

bool OtherOptionsDialog::handleEvents() {
	Sound &sound = *g_vm->_sound;
	checkEvents(g_vm);

	switch (_buttonValue) {
	case Common::KEYCODE_d:
		delete this;
		sound.stopAllAudio();
		WOX_VM._events->hideCursor();
		WOX_VM.showDarkSideIntro(false);
		WOX_VM._events->showCursor();
		break;

	case Common::KEYCODE_c:
		if (g_vm->getGameID() == GType_WorldOfXeen) {
			delete this;
			sound.stopAllAudio();
			WOX_VM._events->hideCursor();
			WOX_VM.showCloudsIntro();
			WOX_VM._events->showCursor();
		}
		break;

	case Common::KEYCODE_e:
		if (g_vm->_gameWon[1]) {
			delete this;
			sound.stopAllAudio();
			WOX_VM._events->hideCursor();
			WOX_VM.showDarkSideEnding(g_vm->_finalScore);
			WOX_VM._events->showCursor();
		}
		break;

	case Common::KEYCODE_v:
		if (g_vm->_gameWon[0]) {
			delete this;
			sound.stopAllAudio();
			WOX_VM._events->hideCursor();
			WOX_VM.showCloudsEnding(g_vm->_finalScore);
			WOX_VM._events->showCursor();
		}
		break;

	case Common::KEYCODE_w:
		if (g_vm->_gameWon[2]) {
			delete this;
			sound.stopAllAudio();
			WOX_VM._events->hideCursor();
			WOX_VM.showWorldOfXeenEnding(NON_GOOBER, g_vm->_finalScore);
			WOX_VM._events->showCursor();
		}
		break;

	case Common::KEYCODE_ESCAPE:
		// Exit dialog
		delete this;
		break;

	default:
		return false;
	}

	return true;
}

} // End of namespace WorldOfXeen
} // End of namespace Xeen
} // End of namespace MM
