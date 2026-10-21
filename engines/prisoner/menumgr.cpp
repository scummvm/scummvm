#include "common/config-manager.h"

#include "engines/dialogs.h"

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"
#include "prisoner/midi.h"

#include "menumgr.h"


namespace Prisoner {

static const _PakMenuDirectoryEntry _kPrisonerPanelData[] = {
	/* Menu data from .exe */
	/*
	S_PANEL.PUK 0, 255 255
	S_PANEL.PUK 1 156 156
	S_PANEL.PUK 2 156 156
	S_PANEL.PUK 3 156 156
	S_PANEL.PUK 4 156 156
	S_PANEL.PUK 5 156 156
	S_PANEL.PUK 6 156 156
	S_PANEL.PUK 7 126 126
	S_PANEL.PUK 8 255 255
	S_PANEL.PUK 9 255 255
	S_PANEL.PUK 10 255 255
	S_PANEL.PUK 11 156 156
	S_PANEL.PUK 12 155 153
	S_PANEL.PUK 13 89 87
	*/

	{SAVE_PANEL, "S_PANEL", 0, 255, 255},
	{OPTIONS_PANEL, "S_PANEL", 1, 156, 156},
	{MAIN_PANEL, "S_PANEL", 2, 156, 156},
	{VOLOPT_PANEL, "S_PANEL", 3, 156, 156},
	{DIVOPT_PANEL, "S_PANEL", 4, 156, 156},
	{PAUSE_PANEL, "S_PANEL", 5, 156, 156},
	{SNDCFG_PANEL, "S_PANEL", 6, 156, 156},
	{LOAD_PANEL, "S_PANEL", 7, 126, 126},
	{ASKER1_PANEL, "S_PANEL", 8, 255, 255},
	{ASKER2_PANEL, "S_PANEL", 9, 255, 255},
	{ASKER3_PANEL, "S_PANEL", 10, 255, 255},
	{OPTGFX_PANEL, "S_PANEL", 11, 156, 156},
	{FORLOOKINV_PANEL, "S_PANEL", 12, 155, 153},
	{FORIDM_PANEL, "S_PANEL", 13, 89, 87},
	{PANEL_SIZE_END, NULL, 0, 0, 0}};

enum MAIN_MENU_OPTIONS {
	RET_GAME = 0,
	NEW_GAME,
	LOAD_GAME,
	SAVE_GAME,
	OPTIONS,
	EXIT,
	MAIN_MENU_SIZE,
};

static const _MenuOffset _mainMenuOffsets[] = {
	{111, 119, 402, 140},
	{111, 144, 402, 165},
	{111, 177, 402, 198},
	{111, 202, 402, 223},
	{111, 235, 402, 256},
	{111, 260, 402, 281},
	{98, 13, 413, 39}};

static const _MenuOffset _dialogOffsets[] = {
	{ 98,  13, 418,  38}, // title
	{ 91, 301, 184, 328}, // option 1 (left)
	{206, 301, 297, 328}, // option 2 (center)
	{320, 301, 412, 328}, // option 3 (right)
	{ 93,  85, 418, 108},
	{ 94, 119, 419, 142}, // text line 1
	{ 94, 143, 419, 166}, // ??
	{ 94, 177, 419, 200}, // ??
	{ 94, 201, 419, 224}, // ??
	{ 94, 235, 419, 258}, // ??
};

// The menu panels are drawn at this screen position
static const int16 kMenuPanelX = 67;
static const int16 kMenuPanelY = 77;

enum OptionsMenuItem {
	kOptionsMenuSound = 0,
	kOptionsMenuTextMusic,
	kOptionsMenuDisplay,
	kOptionsMenuSaveSettings,
	kOptionsMenuBack,
	kOptionsMenuCount
};

// From runOptionsMenu_addClickBoxes, the last entry is the title
static const _MenuOffset kOptionsMenuOffsets[] = {
	{111,  73, 403,  94},
	{111, 132, 403, 153},
	{111, 189, 403, 210},
	{111, 247, 403, 268},
	{111, 306, 403, 327},
	{ 98,  13, 413,  38}
};

// Display settings screen (runOptionsMenu_sub_34B17), click box tags from runOptionsMenu_sub_3432C
enum DisplayMenuBox {
	kDisplayBoxTitle = 0,
	kDisplayBoxScrollingLabel,
	kDisplayBoxStandbyLabel,
	kDisplayBoxSpeedLabel,
	kDisplayBoxUnused,
	kDisplayBoxScrollingToggle,
	kDisplayBoxStandbyMinus,
	kDisplayBoxStandbyValue,
	kDisplayBoxStandbyPlus,
	kDisplayBoxSpeedFirst,			// 11 slider cells, one per speed
	kDisplayBoxSpeedLast = kDisplayBoxSpeedFirst + 10,
	kDisplayBoxOk,
	kDisplayBoxCancel,
	kDisplayBoxSpeedSlowest,
	kDisplayBoxSpeedNormal,
	kDisplayBoxSpeedFastest,
	kDisplayBoxCount
};

static const _MenuOffset kDisplayMenuOffsets[] = {
	{ 98,  12, 413,  38},
	{109,  72, 323,  96},
	{109, 130, 323, 154},
	{109, 188, 323, 212},
	{ 95, 241,  99, 245},
	{369,  58, 414, 111},
	{356, 130, 376, 154},
	{376, 130, 400, 154},
	{400, 130, 419, 154},
	{165, 260, 179, 281},
	{180, 260, 194, 281},
	{195, 260, 209, 281},
	{210, 260, 224, 281},
	{225, 260, 239, 281},
	{240, 260, 254, 281},
	{255, 260, 269, 281},
	{270, 260, 284, 281},
	{285, 260, 299, 281},
	{300, 260, 314, 281},
	{315, 260, 329, 281},
	{ 90, 301, 181, 329},
	{320, 301, 411, 329},
	{163, 239, 181, 256},
	{238, 239, 256, 256},
	{313, 239, 338, 256}
};

// Animation speed percentages for the speed slider, slowest first (word_61634)
static const int16 kAnimationSpeeds[] = {400, 300, 250, 200, 150, 100, 84, 67, 50, 25, 0};
static const int16 kAnimationSpeedCount = ARRAYSIZE(kAnimationSpeeds);
static const int16 kMaxStandbyMinutes = 99;

// Rows of the display settings screen. The OK and Cancel buttons can't be reached with the
// keyboard in the original; they are added here as a last row (Left/Right switch between them).
enum {
	kDisplayRowScrolling = 0,
	kDisplayRowStandby,
	kDisplayRowSpeed,
	kDisplayRowOk,
	kDisplayRowCancel
};

// Focused button of the yes/no dialog (not in the original, which only takes Enter/Escape)
enum {
	kDialogFocusOk = 0,
	kDialogFocusCancel
};

static int8 findMenuItemAtPos(const _MenuOffset *offsets, int8 count, int16 x, int16 y) {
	for (int8 i = 0; i < count; i++) {
		if (x >= offsets[i].x1 && x <= offsets[i].x2 && y >= offsets[i].y1 && y <= offsets[i].y2)
			return i;
	}
	return -1;
}

// MENU SAMPLES
// TEST_MUSIC_MIDI=SOUNDTST.PUK 0
// TEST_MUSIC_SMP=SOUNDTST.PUK  1
// TEST_VOICE=F_M06R01.PUK      21
// TEST_SAMPLE=SOUNDTST.PUK     2

static const byte _byte_19184[] = {
	0x0E, 0x11, 0x16, 0x07, 0x0B, 0x12, 0x18, 0x06, 0x1A, 0x0A, 0x1C,
	0x12, 0x04, 0x0D, 0x0D, 0x74, 0x6E, 0x1C, 0x1A, 0x1B, 0x0A, 0x10,
	0x07, 0x07, 0x7A, 0x7E, 0x05, 0x1E, 0x6B, 0x10, 0x3D, 0x07, 0x5E,
	0x11, 0x16, 0x07, 0x0B, 0x12, 0x18, 0x06, 0x1A, 0x0A, 0x1C, 0x0C,
	0x1E, 0x1D, 0x6D, 0x6E, 0x1C, 0x1A, 0x1B, 0x0A, 0x10, 0x07, 0x07,
	0x7A, 0x7E, 0x05, 0x1E, 0x6B, 0x00, 0x11, 0x3C, 0x07, 0x5E, 0x11,
	0x16, 0x07, 0x0B, 0x09, 0x19, 0x06, 0x0A, 0x06, 0x78, 0x7B, 0x19,
	0x12, 0x7D, 0x06, 0x64, 0x62, 0x01, 0x1F, 0x7E, 0x05, 0x1E, 0x6B,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x03, 0x3C, 0x07, 0x5E, 0x11,
	0x16, 0x07, 0x0B, 0x0C, 0x12, 0x0C, 0x1D, 0x1C, 0x09, 0x78, 0x6E,
	0x1C, 0x1A, 0x1B, 0x0A, 0x10, 0x07, 0x07, 0x7A, 0x7E, 0x05, 0x1E,
	0x6B, 0x00, 0x00, 0x00, 0x00, 0x12, 0x3F, 0x07
};

static const byte testfile_data[] = {
	0x0E, 0x11, 0x16, 0x07, 0x0B, 0x12, 0x18, 0x06, 0x1A, 0x0A, 0x1C,
	0x12, 0x04, 0x0D, 0x0D, 0x74, 0x6E, 0x1C, 0x1A, 0x1B, 0x0A, 0x10,
	0x07, 0x07, 0x7A, 0x7E, 0x05, 0x1E, 0x6B, 0x10, 0x3D, 0x07, 0x5E,
	0x11, 0x16, 0x07, 0x0B, 0x12, 0x18, 0x06, 0x1A, 0x0A, 0x1C, 0x0C,
	0x1E, 0x1D, 0x6D, 0x6E, 0x1C, 0x1A, 0x1B, 0x0A, 0x10, 0x07, 0x07,
	0x7A, 0x7E, 0x05, 0x1E, 0x6B, 0x00, 0x11, 0x3C, 0x07, 0x5E, 0x11,
	0x16, 0x07, 0x0B, 0x09, 0x19, 0x06, 0x0A, 0x06, 0x78, 0x7B, 0x19,
	0x12, 0x7D, 0x06, 0x64, 0x62, 0x01, 0x1F, 0x7E, 0x05, 0x1E, 0x6B,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x03, 0x3C, 0x07, 0x5E, 0x11,
	0x16, 0x07, 0x0B, 0x0C, 0x12, 0x0C, 0x1D, 0x1C, 0x09, 0x78, 0x6E,
	0x1C, 0x1A, 0x1B, 0x0A, 0x10, 0x07, 0x07, 0x7A, 0x7E, 0x05, 0x1E,
	0x6B, 0x00, 0x00, 0x00, 0x00, 0x12, 0x3F, 0x07
};

Common::StringArray MainMenuIds = {"M_RETG", "M_NEWG", "M_LOADG", "M_SAVEG", "M_OPT", "EXIT_G", "MAINM"};
// The back entry reads "Return to game" (M_RETG) when the options were opened from the game
static const char *const kOptionsMenuTextIds[] = {"M_SND", "M_DVOPT", "M_GFX", "BSAVOPT", "M_BACK", "M_OPT"};


extern void decryptBuffer(byte *buf, uint32 size);


void PrisonerEngine::loadDialogData() {

}

void PrisonerEngine::displayDialog(
	DIALOG_TYPE type,
	Common::String &titleStrId,
	Common::String &actionStrId,
	DialogAction actionCallback
	) {
	_isDialogMenuShowing = true;
	_dialogMenuFocus = kDialogFocusOk;
	_dialogActionCallback = actionCallback;
	Common::StringArray DialogIds = {"", "OK", "CANCEL", "IGNORE"};
	Common::String title = getGlobalText(titleStrId);
	Common::String action = getGlobalText(actionStrId);
	Common::String option = getGlobalText(DialogIds[type]);

	_dialogStrings.clear();
	_dialogStrings.push_back(title);
	_dialogStrings.push_back(getGlobalText(DialogIds[1])); // OK is always on the left
	_dialogStrings.push_back("");           // middle position
	_dialogStrings.push_back(option);		// right position
	_dialogStrings.push_back("");			// first action text
	_dialogStrings.push_back("");			
	_dialogStrings.push_back(action);

	// TODO: Load dialog panel with 1 button ASKER1 and 3 buttons ASKER3
	_dialogPanel = _res->get<AnimationResource>(_menuPanelResourceCacheSlots[ASKER2_PANEL]);
}

void PrisonerEngine::loadMenuPanels() {
	const _PakMenuDirectoryEntry *xentry = _kPrisonerPanelData;
	uint8 i = 0;
	while (xentry->pakName) {
		Common::String pakName = xentry->pakName;
		_menuPanelResourceCacheSlots[i++] = _res->load<AnimationResource>(pakName, xentry->pakSlot, 11);
		xentry++;
	}



	//uint16 s = sizeof(_byte_19184);
	//byte *bytes = (byte *)malloc(s + 1);
	//memcpy(bytes, _byte_19184, s);
	//decryptBuffer(bytes, s);
	//bytes[s] = '\0';
	//debug("%c", _byte_19184);

	runMainMenu_initMessages();
	runMainMenu_addClickBoxes(67, 77);
	_currMenuIndex = _inGame ? 0 : 1;
	_selectedMenuIndex = MAIN_PANEL;
	_menuMouseCursorActive = 1;
	_menuMouseCursor = 13;
}

void PrisonerEngine::loadOnscreenMenuText() {
}

void PrisonerEngine::runMainMenu_initMessages() {
	for (Common::StringArray::iterator id = MainMenuIds.begin(); id != MainMenuIds.end(); id++) {
		_menuItems.push_back(getGlobalText(*id));
	}

	updateActiveMenuItems();
}

void PrisonerEngine::updateActiveMenuItems() {
	_currMenuIndex = _inGame ? 0 : 1;
	_activeMenuItems.clear();

	for (uint8 i = 0; i < _menuItems.size(); i++) {
		bool active = true;
		if (!_inGame && (i == 0 || i == 3)) {
			active = false;
		}

		_activeMenuItems.push_back(active);
	}
}

void PrisonerEngine::runMainMenu_addClickBoxes(int16 x, int16 y) {
	for (uint8 i = 0; i < _menuItems.size(); i++) {
		if (_activeMenuItems[i]) {
			addClickBox(
				x + _mainMenuOffsets[i].x1,
				y + _mainMenuOffsets[i].y1,
				x + _mainMenuOffsets[i].x2,
				y + _mainMenuOffsets[i].y2,
				i);
		}
	}
	// TODO: Remove after debugging
	// addClickBox(x + 111, y + 119, x + 402, y + 140, 0);
	// addClickBox(x + 111, y + 144, x + 402, y + 165, 1);
	// addClickBox(x + 111, y + 177, x + 402, y + 198, 2);
	// addClickBox(x + 111, y + 202, x + 402, y + 223, 3);
	// addClickBox(x + 111, y + 235, x + 402, y + 256, 4);
	// addClickBox(x + 111, y + 260, x + 402, y + 281, 5);
	// addClickBox(x + 98, y + 13, x + 413, y + 39, 6);
}

void PrisonerEngine::drawClickBoxLabels() {
}

void PrisonerEngine::handleMainMenuInput() {
	Common::KeyCode keyState;
	uint16 buttonState;

	if (_selectedMenuIndex == OPTIONS_PANEL) {
		handleOptionsMenuInput();
		return;
	} else if (_selectedMenuIndex == OPTGFX_PANEL) {
		handleDisplaySettingsInput();
		return;
	}

	getInputStatus(keyState, buttonState);

	// Hit-test the item rectangles directly; the click box indices don't match the
	// menu items once inactive items are left out
	int8 itemIndex = findMenuItemAtPos(_mainMenuOffsets, MAIN_MENU_SIZE, _mouseX - kMenuPanelX, _mouseY - kMenuPanelY);
	if ((buttonState & kLeftButton) && itemIndex != -1 && _activeMenuItems[itemIndex]) {
		inpMouseSetWaitRelease(true);
		_currMenuIndex = itemIndex;
		doMenuAction(itemIndex);
	}

	if (keyState == Common::KEYCODE_DOWN) {
		do {
			_currMenuIndex = (_currMenuIndex + 1) % MAIN_MENU_SIZE;
		} while (!_activeMenuItems[_currMenuIndex]);
		inpKeybSetWaitRelease(true);
	} else if (keyState == Common::KEYCODE_UP) {
		do {
			_currMenuIndex = (_currMenuIndex - 1) % MAIN_MENU_SIZE;
			if (_currMenuIndex < 0) {
				_currMenuIndex += MAIN_MENU_SIZE;
			}
		} while (!_activeMenuItems[_currMenuIndex]);
		inpKeybSetWaitRelease(true);
	} else if (keyState == Common::KEYCODE_RETURN) {
		doMenuAction(_currMenuIndex);
	}

	updateMouseCursor();
}

void PrisonerEngine::handleDialogMenuInput() {
	Common::KeyCode keyState;
	uint16 buttonState;

	getInputStatus(keyState, buttonState);

	if (buttonState & kLeftButton) {
		inpMouseSetWaitRelease(true);
		// Hit-test the left and right buttons directly; the old click boxes belong to the main menu
		int8 buttonIndex = findMenuItemAtPos(&_dialogOffsets[1], 3, _mouseX - kMenuPanelX, _mouseY - kMenuPanelY);
		if (buttonIndex == 0) {
			_dialogMenuFocus = kDialogFocusOk;
			closeDialogMenu(true);
		} else if (buttonIndex == 2) {
			_dialogMenuFocus = kDialogFocusCancel;
			closeDialogMenu(false);
		}
	} else if (keyState != Common::KEYCODE_INVALID) {
		inpKeybSetWaitRelease(true);
		switch (keyState) {
		case Common::KEYCODE_LEFT:
			_dialogMenuFocus = kDialogFocusOk;
			break;
		case Common::KEYCODE_RIGHT:
			_dialogMenuFocus = kDialogFocusCancel;
			break;
		case Common::KEYCODE_TAB:
			_dialogMenuFocus = _dialogMenuFocus == kDialogFocusOk ? kDialogFocusCancel : kDialogFocusOk;
			break;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_SPACE:
			closeDialogMenu(_dialogMenuFocus == kDialogFocusOk);
			break;
		case Common::KEYCODE_ESCAPE:
			closeDialogMenu(false);
			break;
		default:
			break;
		}
	}

	updateMouseCursor();
}

void PrisonerEngine::closeDialogMenu(bool confirmed) {
	_isDialogMenuShowing = false;
	if (confirmed)
		(this->*_dialogActionCallback)();
}

void PrisonerEngine::dialogActionNewGame() {
	_inGame = true;

	if (isPaused()) {
		_pauseToken.clear();
	}

	_mainMenuRequested = false;
	_newModuleIndex = -1;
	leaveScene();
	_newModuleIndex = 2;
	_newSceneIndex = 39;

	//enterScene(2, -1);
	//checkForSceneChange();
}

void PrisonerEngine::dialogActionExit() {
	_mainLoopDone = true;
}

void PrisonerEngine::doMenuAction(uint8 clickBoxIndex) {
	Common::String titleStr("T_WARN");
	Common::String newGameActionStr("DO_NEW");
	Common::String exitActionStr("DO_QUIT");

	switch (clickBoxIndex) {
	case RET_GAME:
		resumeGame();
		break;
	case NEW_GAME:
		inpKeybSetWaitRelease(true);
		displayDialog(CANCEL, titleStr, newGameActionStr, &PrisonerEngine::dialogActionNewGame);

		debug("CLICK ON = %d", clickBoxIndex);
		break;
	case LOAD_GAME:
		inpKeybSetWaitRelease(true);
		loadGameDialog();
		break;
	case SAVE_GAME:
		inpKeybSetWaitRelease(true);
		saveGameDialog();
		break;
	case OPTIONS:
		inpKeybSetWaitRelease(true);
		loadOptionsMenu(false);
		break;
	case EXIT:
		inpKeybSetWaitRelease(true);
		displayDialog(CANCEL, titleStr, exitActionStr, &PrisonerEngine::dialogActionExit);
		break;
	default:
		break;
	}
}

void PrisonerEngine::loadOptionsMenu(bool fromGame) {
	_selectedMenuIndex = OPTIONS_PANEL;
	_currOptionsMenuIndex = kOptionsMenuSound;
	_optionsOpenedFromGame = fromGame;

	_optionsMenuItems.clear();
	for (uint i = 0; i < ARRAYSIZE(kOptionsMenuTextIds); i++) {
		Common::String id = (i == kOptionsMenuBack && fromGame) ? "M_RETG" : kOptionsMenuTextIds[i];
		_optionsMenuItems.push_back(getGlobalText(id));
	}
}

void PrisonerEngine::openMainMenu() {
	memcpy(_menuBackgroundSurface->getPixels(), _screen->getScreen()->getPixels(), 640 * 480);
	_menuBackgroundSaved = true;
	_mainMenuRequested = true;
	_selectedMenuIndex = MAIN_PANEL;
	pauseGame();
}

// F4 in the game opens the options directly, or switches to them from the main menu
void PrisonerEngine::openOptionsMenuFromGame() {
	bool fromGame = !_mainMenuRequested;
	if (fromGame)
		openMainMenu();
	loadOptionsMenu(fromGame);
}

void PrisonerEngine::closeOptionsMenu() {
	if (_optionsOpenedFromGame) {
		resumeGame();
	} else {
		_selectedMenuIndex = MAIN_PANEL;
	}
}

void PrisonerEngine::handleOptionsMenuInput() {
	Common::KeyCode keyState;
	uint16 buttonState;

	getInputStatus(keyState, buttonState);

	int8 itemIndex = findMenuItemAtPos(kOptionsMenuOffsets, kOptionsMenuCount, _mouseX - kMenuPanelX, _mouseY - kMenuPanelY);
	if ((buttonState & kLeftButton) && itemIndex != -1) {
		inpMouseSetWaitRelease(true);
		_currOptionsMenuIndex = itemIndex;
		doOptionsMenuAction(itemIndex);
	} else if (keyState == Common::KEYCODE_DOWN) {
		_currOptionsMenuIndex = (_currOptionsMenuIndex + 1) % kOptionsMenuCount;
		inpKeybSetWaitRelease(true);
	} else if (keyState == Common::KEYCODE_UP) {
		_currOptionsMenuIndex = (_currOptionsMenuIndex + kOptionsMenuCount - 1) % kOptionsMenuCount;
		inpKeybSetWaitRelease(true);
	} else if (keyState == Common::KEYCODE_RETURN) {
		inpKeybSetWaitRelease(true);
		doOptionsMenuAction(_currOptionsMenuIndex);
	} else if (keyState == Common::KEYCODE_ESCAPE) {
		// In the game Escape is handled by handleInput() and returns to the game
		inpKeybSetWaitRelease(true);
		closeOptionsMenu();
	}

	updateMouseCursor();
}

void PrisonerEngine::doOptionsMenuAction(int8 itemIndex) {
	switch (itemIndex) {
	case kOptionsMenuSound:
	case kOptionsMenuTextMusic:
		// TODO: The original has its own sub-menus for these (runOptionsMenu_sub_30628 and
		// runOptionsMenu_sub_33E2C). Use ScummVM's options dialog until they are implemented.
		openConfigDialog();
		break;
	case kOptionsMenuDisplay:
		loadDisplaySettingsMenu();
		break;
	case kOptionsMenuSaveSettings:
		ConfMan.flushToDisk();
		break;
	case kOptionsMenuBack:
		closeOptionsMenu();
		break;
	default:
		break;
	}
}

void PrisonerEngine::loadDisplaySettings() {
	if (ConfMan.hasKey("scrolling"))
		_halfScreenScrolling = ConfMan.getBool("scrolling");
	if (ConfMan.hasKey("standby_minutes"))
		_screensaverTime = CLIP<int>(ConfMan.getInt("standby_minutes"), 0, kMaxStandbyMinutes) * kTicksPerMinute;
	if (ConfMan.hasKey("animation_speed"))
		_animationSpeed = CLIP<int>(ConfMan.getInt("animation_speed"), 0, kAnimationSpeeds[0]);
}

// The display settings screen edits copies of the settings, OK applies them
void PrisonerEngine::loadDisplaySettingsMenu() {
	_selectedMenuIndex = OPTGFX_PANEL;
	_displayRow = kDisplayRowScrolling;
	_displayScrolling = _halfScreenScrolling;
	_displayStandbyMinutes = _screensaverTime / kTicksPerMinute;
	_displaySpeedIndex = 0;
	while (_displaySpeedIndex < kAnimationSpeedCount - 1 && kAnimationSpeeds[_displaySpeedIndex] > _animationSpeed)
		_displaySpeedIndex++;

	_displayMenuItems.clear();
	const char *const textIds[] = {"M_GFX", "M_SCROL", "M_SSAVE", "M_SPEED", "OK", "CANCEL"};
	for (uint i = 0; i < ARRAYSIZE(textIds); i++) {
		Common::String id = textIds[i];
		_displayMenuItems.push_back(getGlobalText(id));
	}
}

void PrisonerEngine::closeDisplaySettingsMenu(bool apply) {
	if (apply) {
		_halfScreenScrolling = _displayScrolling;
		_screensaverTime = _displayStandbyMinutes * kTicksPerMinute;
		_animationSpeed = kAnimationSpeeds[_displaySpeedIndex];
		ConfMan.setBool("scrolling", _halfScreenScrolling);
		ConfMan.setInt("standby_minutes", _displayStandbyMinutes);
		ConfMan.setInt("animation_speed", _animationSpeed);
	}
	_selectedMenuIndex = OPTIONS_PANEL;
}

// Left/minus (delta -1) and right/plus (delta 1) change the value of the current row
void PrisonerEngine::changeDisplaySetting(int16 delta) {
	switch (_displayRow) {
	case kDisplayRowScrolling:
		_displayScrolling = !_displayScrolling;
		break;
	case kDisplayRowStandby:
		_displayStandbyMinutes = CLIP<int16>(_displayStandbyMinutes + delta, 0, kMaxStandbyMinutes);
		break;
	case kDisplayRowSpeed:
		_displaySpeedIndex = CLIP<int16>(_displaySpeedIndex + delta, 0, kAnimationSpeedCount - 1);
		break;
	default:
		break;
	}
}

void PrisonerEngine::handleDisplaySettingsInput() {
	Common::KeyCode keyState;
	uint16 buttonState;

	getInputStatus(keyState, buttonState);

	if (buttonState & kLeftButton) {
		inpMouseSetWaitRelease(true);
		int8 boxIndex = findMenuItemAtPos(kDisplayMenuOffsets, kDisplayBoxCount, _mouseX - kMenuPanelX, _mouseY - kMenuPanelY);
		switch (boxIndex) {
		case kDisplayBoxScrollingToggle:
			_displayRow = kDisplayRowScrolling;
			changeDisplaySetting(1);
			break;
		case kDisplayBoxStandbyMinus:
			_displayRow = kDisplayRowStandby;
			changeDisplaySetting(-1);
			break;
		case kDisplayBoxStandbyPlus:
			_displayRow = kDisplayRowStandby;
			changeDisplaySetting(1);
			break;
		case kDisplayBoxOk:
			closeDisplaySettingsMenu(true);
			break;
		case kDisplayBoxCancel:
			closeDisplaySettingsMenu(false);
			break;
		case kDisplayBoxSpeedSlowest:
			_displayRow = kDisplayRowSpeed;
			_displaySpeedIndex = 0;
			break;
		case kDisplayBoxSpeedNormal:
			_displayRow = kDisplayRowSpeed;
			_displaySpeedIndex = 5;
			break;
		case kDisplayBoxSpeedFastest:
			_displayRow = kDisplayRowSpeed;
			_displaySpeedIndex = kAnimationSpeedCount - 1;
			break;
		default:
			if (boxIndex >= kDisplayBoxSpeedFirst && boxIndex <= kDisplayBoxSpeedLast) {
				_displayRow = kDisplayRowSpeed;
				_displaySpeedIndex = boxIndex - kDisplayBoxSpeedFirst;
			}
			break;
		}
	} else if (buttonState & kRightButton) {
		inpMouseSetWaitRelease(true);
		closeDisplaySettingsMenu(false);
	} else if (keyState != Common::KEYCODE_INVALID) {
		inpKeybSetWaitRelease(true);
		switch (keyState) {
		case Common::KEYCODE_UP:
			if (_displayRow == kDisplayRowOk || _displayRow == kDisplayRowCancel)
				_displayRow = kDisplayRowSpeed;
			else if (_displayRow == kDisplayRowScrolling)
				_displayRow = kDisplayRowOk;
			else
				_displayRow--;
			break;
		case Common::KEYCODE_DOWN:
			if (_displayRow == kDisplayRowOk || _displayRow == kDisplayRowCancel)
				_displayRow = kDisplayRowScrolling;
			else
				_displayRow++;
			break;
		case Common::KEYCODE_HOME:
			_displayRow = kDisplayRowScrolling;
			break;
		case Common::KEYCODE_END:
			_displayRow = kDisplayRowSpeed;
			break;
		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_MINUS:
		case Common::KEYCODE_KP_MINUS:
			if (_displayRow == kDisplayRowCancel)
				_displayRow = kDisplayRowOk;
			else
				changeDisplaySetting(-1);
			break;
		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_PLUS:
		case Common::KEYCODE_KP_PLUS:
			if (_displayRow == kDisplayRowOk)
				_displayRow = kDisplayRowCancel;
			else
				changeDisplaySetting(1);
			break;
		case Common::KEYCODE_SPACE:
			if (_displayRow == kDisplayRowScrolling)
				changeDisplaySetting(1);
			else if (_displayRow == kDisplayRowOk || _displayRow == kDisplayRowCancel)
				closeDisplaySettingsMenu(_displayRow == kDisplayRowOk);
			break;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			closeDisplaySettingsMenu(_displayRow != kDisplayRowCancel);
			break;
		case Common::KEYCODE_ESCAPE:
			closeDisplaySettingsMenu(false);
			break;
		default:
			break;
		}
	}

	updateMouseCursor();
}

// Draws one element of a menu panel. The original's runSeq1 takes the
// element index directly, like drawAnimationElement.
static void drawPanelSequence(Screen *screen, AnimationResource *panel, uint elementIndex, int16 x, int16 y) {
	if (elementIndex < panel->_elements.size())
		screen->drawAnimationElement(panel, elementIndex, x, y, 0);
}

// The panels only have a highlight (element 1, cel 1) for their wide rows, which overhangs
// a row by 3 pixels on the left and 4 on the right. Buttons get the same border by drawing
// its left end over the left half of the button and its right end over the right half.
static void drawButtonHighlight(Screen *screen, AnimationResource *panel, int16 x1, int16 x2, int16 y) {
	if (panel->_elements.size() < 2 || panel->_cels.size() < 2)
		return;
	const int16 highlightWidth = panel->getCelWidth(1);
	const int16 midX = (x1 + x2) / 2;
	screen->setClipRect(x1 - 3, 0, midX, 479);
	drawPanelSequence(screen, panel, 1, x1, y);
	screen->setClipRect(midX + 1, 0, x2 + 4, 479);
	drawPanelSequence(screen, panel, 1, x2 + 4 + 3 - highlightWidth, y);
	screen->setClipRect(0, 0, 639, 479);
}

void PrisonerEngine::updateDisplaySettingsMenu(int16 x, int16 y) {
	AnimationResource *menuPanel = _res->get<AnimationResource>(_menuPanelResourceCacheSlots[OPTGFX_PANEL]);
	drawPanelSequence(_screen, menuPanel, 0, x, y);

	_PakMenuDirectoryEntry panelData = _kPrisonerPanelData[OPTGFX_PANEL];
	setFontColors(_menuFont, panelData.outlineColor, panelData.inkColor);
	setActiveFont(_menuFont);

	const int8 textBoxes[] = {kDisplayBoxTitle, kDisplayBoxScrollingLabel, kDisplayBoxStandbyLabel, kDisplayBoxSpeedLabel, kDisplayBoxOk, kDisplayBoxCancel};
	for (uint i = 0; i < _displayMenuItems.size(); i++) {
		const _MenuOffset *off = &kDisplayMenuOffsets[textBoxes[i]];
		drawTextEx(x + off->x1, x + off->x2, y + off->y1, y + off->y2, _displayMenuItems[i]);
	}

	const _MenuOffset *off = &kDisplayMenuOffsets[kDisplayBoxStandbyValue];
	drawTextEx(x + off->x1, x + off->x2, y + off->y1, y + off->y2, Common::String::format("%02d", _displayStandbyMinutes));

	off = &kDisplayMenuOffsets[kDisplayBoxScrollingToggle];
	drawPanelSequence(_screen, menuPanel, _displayScrolling ? 4 : 3, x + off->x1, y + off->y1);

	if (_displayRow == kDisplayRowOk || _displayRow == kDisplayRowCancel) {
		off = &kDisplayMenuOffsets[_displayRow == kDisplayRowOk ? kDisplayBoxOk : kDisplayBoxCancel];
		drawButtonHighlight(_screen, menuPanel, x + off->x1, x + off->x2, y + off->y1 + 2);
	} else {
		off = &kDisplayMenuOffsets[kDisplayBoxScrollingLabel + _displayRow];
		drawPanelSequence(_screen, menuPanel, 1, x + off->x1, y + off->y1);
	}

	off = &kDisplayMenuOffsets[kDisplayBoxSpeedFirst + _displaySpeedIndex];
	drawPanelSequence(_screen, menuPanel, 2, x + off->x1, y + off->y1);
}

void PrisonerEngine::openConfigDialog() {
	GUI::ConfigDialog dialog;
	runDialog(dialog);
	g_system->applyBackendSettings();
	applyGameSettings();
	syncSoundSettings();
}

void PrisonerEngine::updateDialogMenu(int16 x, int16 y) {
	_screen->drawAnimationElement(_dialogPanel, 0, x, y, 0);

	for (uint8 i = 0; i < _dialogStrings.size(); i++) {
		if (_dialogStrings.size() > 0) {
			auto titleOff = _dialogOffsets[i];
			drawTextEx(x + titleOff.x1, x + titleOff.x2, y + titleOff.y1, y + titleOff.y2, _dialogStrings[i]);
		}
	}

	// The dialog panel has no highlight of its own, use the main menu's
	AnimationResource *mainPanel = _res->get<AnimationResource>(_menuPanelResourceCacheSlots[MAIN_PANEL]);
	const _MenuOffset *off = &_dialogOffsets[_dialogMenuFocus == kDialogFocusOk ? 1 : 3];
	drawButtonHighlight(_screen, mainPanel, x + off->x1, x + off->x2, y + off->y1 + 4);
}

void PrisonerEngine::updateMainMenu(int16 x, int16 y) {
	AnimationResource *menuPanel;

	menuPanel = _res->get<AnimationResource>(_menuPanelResourceCacheSlots[MAIN_PANEL]);
	_screen->drawAnimationElement(menuPanel, 0, x, y, 0);

	// TODO: Remove after debugging
	//_screen->drawLine(x + 111, y + 119, x + 402, y + 140, 250);
	//_screen->drawLine(x + 111, y + 144, x + 402, y + 165, 250);
	//_screen->drawLine(x + 111, y + 177, x + 402, y + 198, 250);
	//_screen->drawLine(x + 111, y + 202, x + 402, y + 223, 250);
	//_screen->drawLine(x + 111, y + 235, x + 402, y + 256, 4);
	//_screen->drawLine(x + 111, y + 260, x + 402, y + 281, 5);
	//_screen->drawLine(x + 98, y + 13, x + 413, y + 39, 6);

	_PakMenuDirectoryEntry mainPanelData = _kPrisonerPanelData[MAIN_PANEL];
	setFontColors(_menuFont, mainPanelData.outlineColor, mainPanelData.inkColor);
	setActiveFont(_menuFont);

	if (_isDialogMenuShowing) {
		_PakMenuDirectoryEntry dialogPanelData = _kPrisonerPanelData[ASKER2_PANEL];
		setFontColors(_menuFont, dialogPanelData.outlineColor, dialogPanelData.inkColor);
		setActiveFont(_menuFont);

		updateDialogMenu(x, y);
		return;
	}

	for (uint8 i = 0; i < _menuItems.size(); i++) {
		const _MenuOffset *off = &(_mainMenuOffsets[i]);
		drawTextEx(
			x + off->x1,
			x + off->x2,
			y + off->y1,
			y + off->y2,
			_menuItems[i]);

		if (_mainMenuRequested && !_inGame) {
			if (!_activeMenuItems[i])
				_screen->drawTransparentRect(
					x + off->x1,
					y + off->y1,
					x + off->x2,
					y + off->y2);
		}
	}

	// The original draws panel element 1 (the highlight) at the focused item's top left corner
	const _MenuOffset *off = &(_mainMenuOffsets[_currMenuIndex]);
	drawPanelSequence(_screen, menuPanel, 1, x + off->x1, y + off->y1);
}

void PrisonerEngine::updateOptionsMenu(int16 x, int16 y) {
	AnimationResource *menuPanel;

	menuPanel = _res->get<AnimationResource>(_menuPanelResourceCacheSlots[OPTIONS_PANEL]);
	_screen->drawAnimationElement(menuPanel, 0, x, y, 0);

	_PakMenuDirectoryEntry optionsPanelData = _kPrisonerPanelData[OPTIONS_PANEL];
	setFontColors(_menuFont, optionsPanelData.outlineColor, optionsPanelData.inkColor);
	setActiveFont(_menuFont);

	for (uint i = 0; i < _optionsMenuItems.size(); i++) {
		const _MenuOffset *off = &kOptionsMenuOffsets[i];
		drawTextEx(x + off->x1, x + off->x2, y + off->y1, y + off->y2, _optionsMenuItems[i]);
	}

	const _MenuOffset *off = &kOptionsMenuOffsets[_currOptionsMenuIndex];
	drawPanelSequence(_screen, menuPanel, 1, x + off->x1, y + off->y1);
}

} // End of namespace Prisoner
