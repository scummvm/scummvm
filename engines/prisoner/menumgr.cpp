#include "common/config-manager.h"
#include "common/hashmap.h"

#include "engines/dialogs.h"

#include "graphics/paletteman.h"

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

// Confirm boxes (original: some_dialog? and addDialogClickBoxes). The title, then up to six
// lines of text, and the buttons, which depend on how many there are.
static const _MenuOffset kDialogTitleOffset = { 98,  13, 418,  38};

static const _MenuOffset kDialogLineOffsets[] = {
	{ 93,  85, 418, 108},
	{ 94, 119, 419, 142},
	{ 94, 143, 419, 166},
	{ 94, 177, 419, 200},
	{ 94, 201, 419, 224},
	{ 94, 235, 419, 258}
};

static const _MenuOffset kDialogButtonOffsets[3][3] = {
	{{204, 302, 295, 329}},
	{{ 93, 301, 184, 328}, {321, 301, 412, 329}},
	{{ 91, 301, 182, 328}, {206, 301, 297, 328}, {321, 301, 412, 328}}
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

// Text and music settings screen (runOptionsMenu_sub_33E2C), click box tags from
// runOptionsMenu_sub_3351C. The option boxes show icons drawn in the panel.
enum TextMusicMenuBox {
	kTextMusicBoxTitle = 0,
	kTextMusicBoxDialogLabel,
	kTextMusicBoxEndLabel,
	kTextMusicBoxMusicLabel,
	kTextMusicBoxDialogText,
	kTextMusicBoxDialogBoth,
	kTextMusicBoxDialogVoice,
	kTextMusicBoxEndClick,
	kTextMusicBoxEndAuto,
	kTextMusicBoxMusicMidi,
	kTextMusicBoxMusicDigital,
	kTextMusicBoxOk,
	kTextMusicBoxCancel,
	kTextMusicBoxCount
};

static const _MenuOffset kTextMusicMenuOffsets[] = {
	{ 98,  13, 414,  39},
	{ 90, 134, 243, 155},
	{ 90, 191, 243, 212},
	{ 90, 249, 243, 270},
	{268, 134, 299, 156},
	{324, 134, 355, 156},
	{381, 134, 412, 156},
	{296, 191, 327, 213},
	{353, 191, 384, 213},
	{296, 249, 327, 271},
	{353, 249, 384, 271},
	{ 93, 301, 163, 331},
	{349, 302, 419, 332}
};

enum {
	kDialogTypeText = 0,
	kDialogTypeVoice = 1,
	kDialogTypeBoth = 2
};

// Rows of the text and music settings screen
enum {
	kTextMusicRowDialog = 0,
	kTextMusicRowEnd,
	kTextMusicRowMusic,
	kTextMusicRowCount
};

// Sound volume settings screen (runOptionsMenu_sub_30628), click boxes from
// runOptionsMenu_sub_30063. Each row has a 21-cell slider (0 to 100 in steps of 5) and an
// on/off switch. The rows are sound effects, music and voices.
enum VolumeMenuBox {
	kVolumeBoxTitle = 0,
	kVolumeBoxOk,
	kVolumeBoxCancel,
	kVolumeBoxFirstLabel,		// 3 rows
	kVolumeBoxFirstSwitch = kVolumeBoxFirstLabel + 3,
	kVolumeBoxCount = kVolumeBoxFirstSwitch + 3
};

static const _MenuOffset kVolumeMenuOffsets[] = {
	{ 98,  13, 413,  38},
	{ 93, 300, 164, 329},
	{349, 301, 421, 330},
	{ 92, 119, 389, 140},
	{ 92, 177, 389, 198},
	{ 92, 234, 389, 255},
	{394, 116, 431, 164},
	{394, 174, 431, 222},
	{394, 232, 431, 279}
};

static const int16 kVolumeRowCount = 3;
static const int16 kVolumeSliderCells = 21;
static const int16 kVolumeStep = 5;
// The slider cells start at this position and are 15 pixels apart
static const int16 kVolumeCellX = 77;
static const int16 kVolumeCellWidth = 15;
static const int16 kVolumeCellY[kVolumeRowCount] = {143, 201, 258};
static const int16 kVolumeCellHeight = 22;
// Panel elements: the filled sliders (one element per number of lit cells), and the switch
static const int16 kVolumeSliderElements[kVolumeRowCount] = {3, 23, 43};
static const int16 kVolumeSwitchOnElement = 2;
static const int16 kVolumeSwitchOffElement = 63;

// Test sounds (TEST_SAMPLE, TEST_MUSIC_SMP, TEST_MUSIC_MIDI and TEST_VOICE)
static const int16 kTestSampleSlot = 2;
static const int16 kTestDigitalMusicSlot = 1;
static const int16 kTestMidiMusicSlot = 0;
static const int16 kTestVoiceSlot = 21;

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

// The original has panels with one, two and three buttons (ASKER1 to ASKER3). The first button
// confirms, the others close the box. The dialog type sets the buttons: OK alone, OK and Cancel,
// or OK, Ignore and Cancel.
void PrisonerEngine::displayDialog(
	DIALOG_TYPE type,
	Common::String &titleStrId,
	Common::String &actionStrId,
	DialogAction actionCallback
	) {
	Common::StringArray buttonIds;
	buttonIds.push_back("OK");
	if (type == IGNORE)
		buttonIds.push_back("IGNORE");
	if (type == CANCEL || type == IGNORE)
		buttonIds.push_back("CANCEL");

	_isDialogMenuShowing = true;
	_dialogMenuFocus = 0;
	_dialogActionCallback = actionCallback;
	_dialogTitle = getGlobalText(titleStrId);

	TextResource *textResource = _res->get<TextResource>(_globalTextResourceCacheSlot);

	// The game text has no IGNORE label, so that button shows its identifier
	_dialogButtonLabels.clear();
	for (uint i = 0; i < buttonIds.size(); i++)
		_dialogButtonLabels.push_back(textResource->getIndex(buttonIds[i]) >= 0 ? getGlobalText(buttonIds[i]) : buttonIds[i]);

	// All lines of the message, centered vertically in the six text lines
	_dialogLines.clear();
	const TextItem *textItem = textResource->getText(actionStrId);
	if (textItem && !textItem->chunks.empty()) {
		const Common::StringArray &lines = *textItem->chunks[0];
		for (uint i = 0; i < lines.size() && i < ARRAYSIZE(kDialogLineOffsets); i++)
			_dialogLines.push_back(lines[i]);
	}

	const MENU_PANELS panels[] = {ASKER1_PANEL, ASKER2_PANEL, ASKER3_PANEL};
	_dialogPanelIndex = panels[_dialogButtonLabels.size() - 1];
	_dialogPanel = _res->get<AnimationResource>(_menuPanelResourceCacheSlots[_dialogPanelIndex]);
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

const _PakMenuDirectoryEntry &PrisonerEngine::getMenuPanelData(MENU_PANELS panel) const {
	return _kPrisonerPanelData[panel];
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
	} else if (_selectedMenuIndex == DIVOPT_PANEL) {
		handleTextMusicInput();
		return;
	} else if (_selectedMenuIndex == VOLOPT_PANEL) {
		handleVolumeInput();
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

	const int8 buttonCount = _dialogButtonLabels.size();

	if (buttonState & kLeftButton) {
		inpMouseSetWaitRelease(true);
		int8 buttonIndex = findMenuItemAtPos(kDialogButtonOffsets[buttonCount - 1], buttonCount,
			_mouseX - kMenuPanelX, _mouseY - kMenuPanelY);
		if (buttonIndex != -1) {
			_dialogMenuFocus = buttonIndex;
			closeDialogMenu(buttonIndex == 0);
		}
	} else if (keyState != Common::KEYCODE_INVALID) {
		inpKeybSetWaitRelease(true);
		switch (keyState) {
		case Common::KEYCODE_LEFT:
			if (_dialogMenuFocus > 0)
				_dialogMenuFocus--;
			break;
		case Common::KEYCODE_RIGHT:
			if (_dialogMenuFocus < buttonCount - 1)
				_dialogMenuFocus++;
			break;
		case Common::KEYCODE_TAB:
			_dialogMenuFocus = (_dialogMenuFocus + 1) % buttonCount;
			break;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_SPACE:
			closeDialogMenu(_dialogMenuFocus == 0);
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

	// The original never resets the global vars, so a new game started from a running one
	// would inherit its progress.
	resetGlobalScriptVars();
	setTotalPlayTime(0);

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
		openSaveLoadScreen(false);
		break;
	case SAVE_GAME:
		inpKeybSetWaitRelease(true);
		openSaveLoadScreen(true);
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
		// The "Use original menus" game option picks the original screens or ScummVM's options
		if (!(ConfMan.hasKey("original_menus") && ConfMan.getBool("original_menus")))
			openConfigDialog();
		else if (itemIndex == kOptionsMenuSound)
			loadVolumeMenu();
		else
			loadTextMusicMenu();
		break;
	case kOptionsMenuDisplay:
		loadDisplaySettingsMenu();
		break;
	case kOptionsMenuSaveSettings: {
		// Show "Saving..." message in the options title while the settings are written
		Common::String savingId = "MSGSOPT";
		const _MenuOffset *off = &kOptionsMenuOffsets[kOptionsMenuCount];
		AnimationResource *menuPanel = _res->get<AnimationResource>(_menuPanelResourceCacheSlots[OPTIONS_PANEL]);
		_screen->drawAnimationElement(menuPanel, 0, kMenuPanelX, kMenuPanelY, 0);
		drawTextEx(kMenuPanelX + off->x1, kMenuPanelX + off->x2, kMenuPanelY + off->y1, kMenuPanelY + off->y2, getGlobalText(savingId));
		_screen->update();
		ConfMan.flushToDisk();
		_system->delayMillis(500);
		break;
	}
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

// Draws one element of a menu panel. runSeq1 takes the
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

void PrisonerEngine::loadTextMusicSettings() {
	_dialogAutoEnd = ConfMan.hasKey("dialog_auto_end") && ConfMan.getBool("dialog_auto_end");
	// Music type - also set by the "Use MIDI music" game option
	_digitalMusic = !(ConfMan.hasKey("midi_music") && ConfMan.getBool("midi_music"));
}

// The screen edits copies of the settings, OK applies them
void PrisonerEngine::loadTextMusicMenu() {
	_selectedMenuIndex = DIVOPT_PANEL;
	_textMusicRow = kTextMusicRowDialog;
	if (_speechMuted)
		_textMusicDialogType = kDialogTypeText;
	else if (_talkieEnabled)
		_textMusicDialogType = kDialogTypeVoice;
	else
		_textMusicDialogType = kDialogTypeBoth;
	_textMusicAutoEnd = _dialogAutoEnd;
	_textMusicDigital = _digitalMusic;

	_textMusicMenuItems.clear();
	const char *const textIds[] = {"M_DOPT", "M_DIALS", "M_CDIAL", "M_MUSIC", "OK", "CANCEL"};
	for (uint i = 0; i < ARRAYSIZE(textIds); i++) {
		Common::String id = textIds[i];
		_textMusicMenuItems.push_back(getGlobalText(id));
	}
}

void PrisonerEngine::closeTextMusicMenu(bool apply) {
	if (apply) {
		// The type of dialog maps to ScummVM's subtitle and speech settings
		ConfMan.setBool("subtitles", _textMusicDialogType != kDialogTypeVoice);
		ConfMan.setBool("speech_mute", _textMusicDialogType == kDialogTypeText);
		ConfMan.setBool("dialog_auto_end", _textMusicAutoEnd);
		ConfMan.setBool("midi_music", !_textMusicDigital);
		_dialogAutoEnd = _textMusicAutoEnd;
		syncSoundSettings();
		if (_textMusicDigital != _digitalMusic)
			switchMusicType(_textMusicDigital);
	}
	_selectedMenuIndex = OPTIONS_PANEL;
}

// Left/right change the value of the current row
void PrisonerEngine::changeTextMusicSetting(int16 delta) {
	switch (_textMusicRow) {
	case kTextMusicRowDialog: {
		// The boxes are ordered text, both, voice
		const int16 order[] = {kDialogTypeText, kDialogTypeBoth, kDialogTypeVoice};
		int16 position = 0;
		while (order[position] != _textMusicDialogType)
			position++;
		position = (position + 3 + delta) % 3;
		_textMusicDialogType = order[position];
		break;
	}
	case kTextMusicRowEnd:
		_textMusicAutoEnd = !_textMusicAutoEnd;
		break;
	case kTextMusicRowMusic:
		_textMusicDigital = !_textMusicDigital;
		break;
	default:
		break;
	}
}

void PrisonerEngine::handleTextMusicInput() {
	Common::KeyCode keyState;
	uint16 buttonState;

	getInputStatus(keyState, buttonState);

	if (buttonState & kLeftButton) {
		inpMouseSetWaitRelease(true);
		int8 boxIndex = findMenuItemAtPos(kTextMusicMenuOffsets, kTextMusicBoxCount, _mouseX - kMenuPanelX, _mouseY - kMenuPanelY);
		switch (boxIndex) {
		case kTextMusicBoxDialogText:
		case kTextMusicBoxDialogBoth:
		case kTextMusicBoxDialogVoice:
			_textMusicRow = kTextMusicRowDialog;
			_textMusicDialogType = boxIndex == kTextMusicBoxDialogText ? kDialogTypeText :
				(boxIndex == kTextMusicBoxDialogBoth ? kDialogTypeBoth : kDialogTypeVoice);
			break;
		case kTextMusicBoxEndClick:
		case kTextMusicBoxEndAuto:
			_textMusicRow = kTextMusicRowEnd;
			_textMusicAutoEnd = boxIndex == kTextMusicBoxEndAuto;
			break;
		case kTextMusicBoxMusicMidi:
		case kTextMusicBoxMusicDigital:
			_textMusicRow = kTextMusicRowMusic;
			_textMusicDigital = boxIndex == kTextMusicBoxMusicDigital;
			break;
		case kTextMusicBoxOk:
			closeTextMusicMenu(true);
			break;
		case kTextMusicBoxCancel:
			closeTextMusicMenu(false);
			break;
		default:
			break;
		}
	} else if (buttonState & kRightButton) {
		inpMouseSetWaitRelease(true);
		closeTextMusicMenu(false);
	} else if (keyState != Common::KEYCODE_INVALID) {
		inpKeybSetWaitRelease(true);
		switch (keyState) {
		case Common::KEYCODE_UP:
			_textMusicRow = (_textMusicRow + kTextMusicRowCount - 1) % kTextMusicRowCount;
			break;
		case Common::KEYCODE_DOWN:
			_textMusicRow = (_textMusicRow + 1) % kTextMusicRowCount;
			break;
		case Common::KEYCODE_HOME:
			_textMusicRow = kTextMusicRowDialog;
			break;
		case Common::KEYCODE_END:
			_textMusicRow = kTextMusicRowMusic;
			break;
		case Common::KEYCODE_LEFT:
			changeTextMusicSetting(-1);
			break;
		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_SPACE:
			changeTextMusicSetting(1);
			break;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			closeTextMusicMenu(true);
			break;
		case Common::KEYCODE_ESCAPE:
			closeTextMusicMenu(false);
			break;
		default:
			break;
		}
	}

	updateMouseCursor();
}

// Marks the chosen box of an option with the panel's marker and darkens the others
// (original: runOptionsMenu_sub_33770)
void PrisonerEngine::drawTextMusicChoice(AnimationResource *panel, int16 x, int16 y, int16 box, bool chosen) {
	const _MenuOffset *off = &kTextMusicMenuOffsets[box];
	if (chosen)
		drawPanelSequence(_screen, panel, 2, x + off->x1, y + off->y1);
	else
		_screen->drawTransparentRect(x + off->x1, y + off->y1, x + off->x2, y + off->y2);
}

void PrisonerEngine::updateTextMusicMenu(int16 x, int16 y) {
	AnimationResource *menuPanel = _res->get<AnimationResource>(_menuPanelResourceCacheSlots[DIVOPT_PANEL]);
	drawPanelSequence(_screen, menuPanel, 0, x, y);

	_PakMenuDirectoryEntry panelData = _kPrisonerPanelData[DIVOPT_PANEL];
	setFontColors(_menuFont, panelData.outlineColor, panelData.inkColor);
	setActiveFont(_menuFont);

	const int8 textBoxes[] = {kTextMusicBoxTitle, kTextMusicBoxDialogLabel, kTextMusicBoxEndLabel,
		kTextMusicBoxMusicLabel, kTextMusicBoxOk, kTextMusicBoxCancel};
	for (uint i = 0; i < _textMusicMenuItems.size(); i++) {
		const _MenuOffset *off = &kTextMusicMenuOffsets[textBoxes[i]];
		drawTextEx(x + off->x1, x + off->x2, y + off->y1, y + off->y2, _textMusicMenuItems[i]);
	}

	drawTextMusicChoice(menuPanel, x, y, kTextMusicBoxDialogText, _textMusicDialogType == kDialogTypeText);
	drawTextMusicChoice(menuPanel, x, y, kTextMusicBoxDialogBoth, _textMusicDialogType == kDialogTypeBoth);
	drawTextMusicChoice(menuPanel, x, y, kTextMusicBoxDialogVoice, _textMusicDialogType == kDialogTypeVoice);
	drawTextMusicChoice(menuPanel, x, y, kTextMusicBoxEndClick, !_textMusicAutoEnd);
	drawTextMusicChoice(menuPanel, x, y, kTextMusicBoxEndAuto, _textMusicAutoEnd);
	drawTextMusicChoice(menuPanel, x, y, kTextMusicBoxMusicMidi, !_textMusicDigital);
	drawTextMusicChoice(menuPanel, x, y, kTextMusicBoxMusicDigital, _textMusicDigital);

	const _MenuOffset *off = &kTextMusicMenuOffsets[kTextMusicBoxDialogLabel + _textMusicRow];
	drawPanelSequence(_screen, menuPanel, 1, x + off->x1, y + off->y1);
}

// Plays the test sounds while the volume screen is open, so the player hears the
// volumes being set. The game's own sounds and music are paused meanwhile.
void PrisonerEngine::startVolumeTestSounds() {
	suspendMusics();
	suspendSounds();
	setVoicesMuted(true);

	Common::String testPakName = "SOUNDTST";
	_volumeTestSlots[0] = _res->load<SoundResource>(testPakName, kTestSampleSlot, 12);
	if (_digitalMusic) {
		_volumeTestSlots[1] = _res->load<SoundResource>(testPakName, kTestDigitalMusicSlot, 12);
	} else {
		_volumeTestSlots[1] = _res->load<MidiResource>(testPakName, kTestMidiMusicSlot, 13);
		_midi->playMusic(_res->get<MidiResource>(_volumeTestSlots[1]), 100, true);
	}
	Common::String voicePakName = "F_M06R01";
	makeLanguageString(voicePakName);
	_volumeTestSlots[2] = _res->load<SoundResource>(voicePakName, kTestVoiceSlot, 16);

	const Audio::Mixer::SoundType soundTypes[] = {Audio::Mixer::kSFXSoundType, Audio::Mixer::kMusicSoundType, Audio::Mixer::kSpeechSoundType};
	for (int16 row = 0; row < kVolumeRowCount; row++) {
		if (row == 1 && !_digitalMusic)
			continue;
		Audio::AudioStream *stream = _res->get<SoundResource>(_volumeTestSlots[row])->createLoopingAudioStream(0);
		_mixer->playStream(soundTypes[row], &_volumeTestHandles[row], stream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
	}
	applyVolumeSettings();
}

void PrisonerEngine::stopVolumeTestSounds() {
	for (int16 row = 0; row < kVolumeRowCount; row++)
		_mixer->stopHandle(_volumeTestHandles[row]);
	if (!_digitalMusic)
		_midi->stopMusic();
	for (int16 row = 0; row < kVolumeRowCount; row++)
		_res->unload(_volumeTestSlots[row]);

	resumeSounds();
	resumeMusics();
	setVoicesMuted(false);
}

// Sets the mixer to the volumes being edited, so the test sounds follow them
void PrisonerEngine::applyVolumeSettings() {
	const Audio::Mixer::SoundType soundTypes[] = {Audio::Mixer::kSFXSoundType, Audio::Mixer::kMusicSoundType, Audio::Mixer::kSpeechSoundType};
	bool mute = ConfMan.hasKey("mute") && ConfMan.getBool("mute");
	for (int16 row = 0; row < kVolumeRowCount; row++) {
		_mixer->setVolumeForSoundType(soundTypes[row], _volumeValues[row] * Audio::Mixer::kMaxMixerVolume / 100);
		_mixer->muteSoundType(soundTypes[row], mute || !_volumeEnabled[row]);
	}
}

void PrisonerEngine::loadVolumeMenu() {
	_selectedMenuIndex = VOLOPT_PANEL;
	_volumeRow = 0;

	// ScummVM's volumes go from 0 to 256, the game's from 0 to 100 in steps of 5
	const char *const volumeKeys[] = {"sfx_volume", "music_volume", "speech_volume"};
	for (int16 row = 0; row < kVolumeRowCount; row++) {
		int volume = ConfMan.getInt(volumeKeys[row]) * 100 / Audio::Mixer::kMaxMixerVolume;
		_volumeValues[row] = CLIP((volume + kVolumeStep / 2) / kVolumeStep * kVolumeStep, 0, 100);
	}
	_volumeEnabled[0] = !(ConfMan.hasKey("sfx_mute") && ConfMan.getBool("sfx_mute"));
	_volumeEnabled[1] = !(ConfMan.hasKey("music_mute") && ConfMan.getBool("music_mute"));
	_volumeEnabled[2] = !_speechMuted;

	_volumeMenuItems.clear();
	const char *const textIds[] = {"VOLOPT", "OK", "CANCEL", "SMPVOL", "MUSVOL", "VOXVOL"};
	for (uint i = 0; i < ARRAYSIZE(textIds); i++) {
		Common::String id = textIds[i];
		_volumeMenuItems.push_back(getGlobalText(id));
	}

	startVolumeTestSounds();
}

void PrisonerEngine::closeVolumeMenu(bool apply) {
	if (apply) {
		ConfMan.setInt("sfx_volume", _volumeValues[0] * Audio::Mixer::kMaxMixerVolume / 100);
		ConfMan.setInt("music_volume", _volumeValues[1] * Audio::Mixer::kMaxMixerVolume / 100);
		ConfMan.setInt("speech_volume", _volumeValues[2] * Audio::Mixer::kMaxMixerVolume / 100);
		ConfMan.setBool("sfx_mute", !_volumeEnabled[0]);
		ConfMan.setBool("music_mute", !_volumeEnabled[1]);
		ConfMan.setBool("speech_mute", !_volumeEnabled[2]);
	}
	stopVolumeTestSounds();
	syncSoundSettings();
	_selectedMenuIndex = OPTIONS_PANEL;
}

void PrisonerEngine::changeVolumeSetting(int16 row, int16 volume) {
	_volumeValues[row] = CLIP<int16>(volume, 0, 100);
	// Changing the volume switches the row on (original: runOptionsMenu_sub_30628)
	_volumeEnabled[row] = true;
	applyVolumeSettings();
}

void PrisonerEngine::handleVolumeInput() {
	Common::KeyCode keyState;
	uint16 buttonState;

	getInputStatus(keyState, buttonState);

	if (buttonState & kLeftButton) {
		inpMouseSetWaitRelease(true);
		int16 mouseX = _mouseX - kMenuPanelX;
		int16 mouseY = _mouseY - kMenuPanelY;
		int8 boxIndex = findMenuItemAtPos(kVolumeMenuOffsets, kVolumeBoxCount, mouseX, mouseY);
		if (boxIndex == kVolumeBoxOk) {
			closeVolumeMenu(true);
		} else if (boxIndex == kVolumeBoxCancel) {
			closeVolumeMenu(false);
		} else if (boxIndex >= kVolumeBoxFirstSwitch) {
			_volumeRow = boxIndex - kVolumeBoxFirstSwitch;
			_volumeEnabled[_volumeRow] = !_volumeEnabled[_volumeRow];
			applyVolumeSettings();
		} else {
			// A slider cell sets the volume to its position
			for (int16 row = 0; row < kVolumeRowCount; row++) {
				if (mouseY >= kVolumeCellY[row] && mouseY < kVolumeCellY[row] + kVolumeCellHeight &&
					mouseX >= kVolumeCellX && mouseX < kVolumeCellX + kVolumeSliderCells * kVolumeCellWidth) {
					_volumeRow = row;
					changeVolumeSetting(row, (mouseX - kVolumeCellX) / kVolumeCellWidth * kVolumeStep);
				}
			}
		}
	} else if (buttonState & kRightButton) {
		inpMouseSetWaitRelease(true);
		closeVolumeMenu(false);
	} else if (keyState != Common::KEYCODE_INVALID) {
		inpKeybSetWaitRelease(true);
		switch (keyState) {
		case Common::KEYCODE_UP:
			_volumeRow = (_volumeRow + kVolumeRowCount - 1) % kVolumeRowCount;
			break;
		case Common::KEYCODE_DOWN:
			_volumeRow = (_volumeRow + 1) % kVolumeRowCount;
			break;
		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_MINUS:
		case Common::KEYCODE_KP_MINUS:
			changeVolumeSetting(_volumeRow, _volumeValues[_volumeRow] - kVolumeStep);
			break;
		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_PLUS:
		case Common::KEYCODE_KP_PLUS:
			changeVolumeSetting(_volumeRow, _volumeValues[_volumeRow] + kVolumeStep);
			break;
		case Common::KEYCODE_HOME:
			changeVolumeSetting(_volumeRow, 0);
			break;
		case Common::KEYCODE_END:
			changeVolumeSetting(_volumeRow, 100);
			break;
		case Common::KEYCODE_SPACE:
			_volumeEnabled[_volumeRow] = !_volumeEnabled[_volumeRow];
			applyVolumeSettings();
			break;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			closeVolumeMenu(true);
			break;
		case Common::KEYCODE_ESCAPE:
			closeVolumeMenu(false);
			break;
		default:
			break;
		}
	}

	updateMouseCursor();
}

void PrisonerEngine::updateVolumeMenu(int16 x, int16 y) {
	AnimationResource *menuPanel = _res->get<AnimationResource>(_menuPanelResourceCacheSlots[VOLOPT_PANEL]);
	drawPanelSequence(_screen, menuPanel, 0, x, y);

	_PakMenuDirectoryEntry panelData = _kPrisonerPanelData[VOLOPT_PANEL];
	setFontColors(_menuFont, panelData.outlineColor, panelData.inkColor);
	setActiveFont(_menuFont);

	for (uint i = 0; i < _volumeMenuItems.size(); i++) {
		const _MenuOffset *off = &kVolumeMenuOffsets[i];
		drawTextEx(x + off->x1, x + off->x2, y + off->y1, y + off->y2, _volumeMenuItems[i]);
	}

	for (int16 row = 0; row < kVolumeRowCount; row++) {
		int16 litCells = _volumeValues[row] / kVolumeStep;
		if (litCells > 0)
			drawPanelSequence(_screen, menuPanel, kVolumeSliderElements[row] + litCells - 1, x + kVolumeCellX, y + kVolumeCellY[row]);
		const _MenuOffset *off = &kVolumeMenuOffsets[kVolumeBoxFirstSwitch + row];
		drawPanelSequence(_screen, menuPanel, _volumeEnabled[row] ? kVolumeSwitchOnElement : kVolumeSwitchOffElement,
			x + off->x1, y + off->y1);
	}

	const _MenuOffset *off = &kVolumeMenuOffsets[kVolumeBoxFirstLabel + _volumeRow];
	drawPanelSequence(_screen, menuPanel, 1, x + off->x1, y + off->y1);
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

	const _MenuOffset *off = &kDialogTitleOffset;
	drawTextEx(x + off->x1, x + off->x2, y + off->y1, y + off->y2, _dialogTitle);

	uint firstLine = (ARRAYSIZE(kDialogLineOffsets) - _dialogLines.size()) / 2;
	for (uint i = 0; i < _dialogLines.size(); i++) {
		off = &kDialogLineOffsets[firstLine + i];
		drawTextEx(x + off->x1, x + off->x2, y + off->y1, y + off->y2, _dialogLines[i]);
	}

	const int8 buttonCount = _dialogButtonLabels.size();
	for (int8 i = 0; i < buttonCount; i++) {
		off = &kDialogButtonOffsets[buttonCount - 1][i];
		drawTextEx(x + off->x1, x + off->x2, y + off->y1, y + off->y2, _dialogButtonLabels[i]);
	}

	// The dialog panels have no highlight of their own, use the main menu's
	AnimationResource *mainPanel = _res->get<AnimationResource>(_menuPanelResourceCacheSlots[MAIN_PANEL]);
	off = &kDialogButtonOffsets[buttonCount - 1][_dialogMenuFocus];
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
		_PakMenuDirectoryEntry dialogPanelData = _kPrisonerPanelData[_dialogPanelIndex];
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

	// Draws panel element 1 (the highlight) at the focused item's top left corner
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

// Original save/load screens (runMenu_saveLoad), used with the "Use original save/load screens"
// game option. Click box tags from runMenu_saveLoad_addClickBoxes; the arrows are the bounds of
// panel elements 3 and 4.
enum SaveLoadMenuBox {
	kSaveLoadBoxFirstRow = 0,
	kSaveLoadBoxLastRow = 5,
	kSaveLoadBoxOk,
	kSaveLoadBoxDelete,
	kSaveLoadBoxCancel,
	kSaveLoadBoxThumbnail,
	kSaveLoadBoxTitle,
	kSaveLoadBoxUp,
	kSaveLoadBoxDown,
	kSaveLoadBoxCount
};

static const _MenuOffset kSaveLoadMenuOffsets[] = {
	{261,  62, 428,  83},
	{261,  87, 428, 108},
	{261, 120, 428, 141},
	{261, 145, 428, 166},
	{261, 178, 428, 199},
	{261, 203, 428, 224},
	{ 92, 301, 183, 328},
	{207, 301, 298, 328},
	{322, 301, 414, 328},
	{ 89, 103, 248, 183},
	{103,  13, 412,  39},
	{443,  59, 472,  85},
	{444, 198, 473, 223}
};

static const int16 kSaveLoadRows = kSaveLoadBoxLastRow - kSaveLoadBoxFirstRow + 1;
static const int16 kSaveLoadThumbnailWidth = 160;
static const int16 kSaveLoadThumbnailHeight = 80;
// The original thumbnail shows every fourth pixel of the game area, which starts at line 82
static const int16 kSaveLoadThumbnailGameY = 82;
static const uint kSaveLoadMaxDescriptionLength = 98;
// Frames between two blinks of the text cursor, and the two cursor characters
static const int16 kSaveLoadCursorBlinkFrames = 10;
static const char kSaveLoadCursorChars[] = {'\xFF', '\xDD'};
// The original reads double clicks from its mouse driver, use the time from its unused code
static const uint32 kSaveLoadDoubleClickTicks = 50;

static bool compareSaveLoadEntries(const PrisonerEngine::SaveLoadEntry &a, const PrisonerEngine::SaveLoadEntry &b) {
	// Newest first like the original (sg_compareFunc); saves without a date by slot
	if (a.saveDate != b.saveDate)
		return a.saveDate > b.saveDate;
	if (a.saveTime != b.saveTime)
		return a.saveTime > b.saveTime;
	return a.slot > b.slot;
}

// Reads the header of the save in a slot. The caller frees the thumbnail.
static bool readSaveSlotHeader(const Common::String &target, int slot, bool loadThumbnail, PrisonerEngine::SaveHeader &header) {
	header.thumbnail = nullptr;
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(PrisonerEngine::getSavegameFilename(target, slot));
	if (!in)
		return false;
	bool result = PrisonerEngine::readSaveHeader(in, loadThumbnail, header) == PrisonerEngine::kRSHENoError;
	delete in;
	return result;
}

static bool isOriginalSaveLoadEnabled() {
	return ConfMan.hasKey("original_menus") && ConfMan.getBool("original_menus");
}

// F2/F3 and the main menu open the original screens with the game option, ScummVM's otherwise
void PrisonerEngine::openSaveLoadScreen(bool saving) {
	if (!isOriginalSaveLoadEnabled()) {
		if (saving)
			saveGameDialog();
		else
			loadGameDialog();
		return;
	}

	if (saving ? !canSaveGameStateCurrently() : !canLoadGameStateCurrently())
		return;

	_saveLoadFromGame = !_mainMenuRequested;
	if (_saveLoadFromGame)
		openMainMenu();

	_saveLoadSaving = saving;
	_selectedMenuIndex = saving ? SAVE_PANEL : LOAD_PANEL;
	_menuFunctionArray[_selectedMenuIndex] = &PrisonerEngine::updateSaveLoadMenu;

	_saveLoadMenuItems.clear();
	const char *const textIds[] = {saving ? "M_SAVEG" : "M_LOADG", "OK", "DELETE", "CANCEL"};
	for (uint i = 0; i < ARRAYSIZE(textIds); i++) {
		Common::String id = textIds[i];
		_saveLoadMenuItems.push_back(getGlobalText(id));
	}

	loadSaveLoadEntries();
	_saveLoadLastClickTicks = 0;
	_saveLoadLastClickRow = -1;
}

// The save list (sg_loadSavegameList1). Saving adds an empty entry for a new save in the first
// free slot; the autosave slot is only listed for loading.
void PrisonerEngine::loadSaveLoadEntries() {
	const MetaEngine *metaEngine = getMetaEngine();
	SaveStateList saves = metaEngine->listSaves(_targetName.c_str());

	_saveLoadEntries.clear();
	for (uint i = 0; i < saves.size(); i++) {
		if (_saveLoadSaving && saves[i].getSaveSlot() == getAutosaveSlot())
			continue;
		// The description as stored, in the game's character set
		SaveHeader header;
		if (!readSaveSlotHeader(_targetName, saves[i].getSaveSlot(), false, header))
			continue;
		SaveStateDescriptor desc = metaEngine->querySaveMetaInfos(_targetName.c_str(), saves[i].getSaveSlot());
		SaveLoadEntry entry;
		entry.slot = saves[i].getSaveSlot();
		entry.description = header.description;
		entry.saveDate = desc.getSaveDate();
		entry.saveTime = desc.getSaveTime();
		_saveLoadEntries.push_back(entry);
	}

	Common::sort(_saveLoadEntries.begin(), _saveLoadEntries.end(), compareSaveLoadEntries);

	if (_saveLoadSaving) {
		int freeSlot = getAutosaveSlot() + 1;
		bool slotUsed = true;
		while (slotUsed) {
			slotUsed = false;
			for (uint i = 0; i < saves.size() && !slotUsed; i++) {
				if (saves[i].getSaveSlot() == freeSlot) {
					slotUsed = true;
					freeSlot++;
				}
			}
		}
		if (freeSlot <= metaEngine->getMaximumSaveSlot()) {
			SaveLoadEntry entry;
			entry.slot = freeSlot;
			_saveLoadEntries.insert_at(0, entry);
		}
	}

	_saveLoadSelected = 0;
	_saveLoadTop = 0;
	_saveLoadEditing = false;
	_saveLoadTextScroll = 0;
	_saveLoadThumbnailEntry = -1;
}

void PrisonerEngine::closeSaveLoadScreen() {
	if (_saveLoadFromGame)
		resumeGame();
	else
		_selectedMenuIndex = MAIN_PANEL;
}

// The thumbnail of the selected save (sg_readThumbnail). The new save shows the game screen.
void PrisonerEngine::updateSaveLoadThumbnail() {
	if (_saveLoadThumbnailEntry == _saveLoadSelected)
		return;
	_saveLoadThumbnailEntry = _saveLoadSelected;

	_saveLoadThumbnail.resize(kSaveLoadThumbnailWidth * kSaveLoadThumbnailHeight);
	Common::fill(_saveLoadThumbnail.begin(), _saveLoadThumbnail.end(), 0);

	if (_saveLoadEntries.empty())
		return;

	const SaveLoadEntry &entry = _saveLoadEntries[_saveLoadSelected];
	if (entry.description.empty()) {
		if (_saveLoadSaving && _menuBackgroundSaved) {
			for (int16 y = 0; y < kSaveLoadThumbnailHeight; y++) {
				const byte *src = (const byte *)_menuBackgroundSurface->getBasePtr(0, kSaveLoadThumbnailGameY + y * 4);
				for (int16 x = 0; x < kSaveLoadThumbnailWidth; x++)
					_saveLoadThumbnail[y * kSaveLoadThumbnailWidth + x] = src[x * 4];
			}
		}
		return;
	}

	// ScummVM thumbnails are true color and show the whole screen: take the game area and
	// use the nearest colors of the current palette
	SaveHeader header;
	readSaveSlotHeader(_targetName, entry.slot, true, header);
	Graphics::Surface *thumbnail = header.thumbnail;
	if (!thumbnail)
		return;

	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	Common::HashMap<uint32, byte> colorMap;
	for (int16 y = 0; y < kSaveLoadThumbnailHeight; y++) {
		const int srcY = (kSaveLoadThumbnailGameY + y * 4) * thumbnail->h / 480;
		for (int16 x = 0; x < kSaveLoadThumbnailWidth; x++) {
			const int srcX = x * 4 * thumbnail->w / 640;
			byte r, g, b;
			thumbnail->format.colorToRGB(thumbnail->getPixel(srcX, srcY), r, g, b);
			const uint32 rgb = (r << 16) | (g << 8) | b;
			if (!colorMap.contains(rgb)) {
				int bestDistance = -1;
				byte bestColor = 0;
				for (int i = 0; i < 256; i++) {
					const int dr = palette[i * 3] - r;
					const int dg = palette[i * 3 + 1] - g;
					const int db = palette[i * 3 + 2] - b;
					const int distance = dr * dr + dg * dg + db * db;
					if (bestDistance < 0 || distance < bestDistance) {
						bestDistance = distance;
						bestColor = i;
					}
				}
				colorMap[rgb] = bestColor;
			}
			_saveLoadThumbnail[y * kSaveLoadThumbnailWidth + x] = colorMap[rgb];
		}
	}

	thumbnail->free();
	delete thumbnail;
}

// Saves or loads the selected entry (Enter, OK or a double click)
void PrisonerEngine::confirmSaveLoadEntry() {
	if (_saveLoadEntries.empty())
		return;

	const SaveLoadEntry &entry = _saveLoadEntries[_saveLoadSelected];
	const Common::String description = _saveLoadEditing ? _saveLoadEditText : entry.description;
	if (description.empty())
		return;

	inpSetWaitRelease(true);
	if (_saveLoadSaving) {
		saveGameState(entry.slot, description);
		// The original goes back to the game after saving
		_saveLoadFromGame = true;
		closeSaveLoadScreen();
	} else {
		if (isPaused())
			_pauseToken.clear();
		_selectedMenuIndex = MAIN_PANEL;
		loadGameState(entry.slot);
	}
}

void PrisonerEngine::dialogActionDeleteSave() {
	// The empty entry for a new save has no file
	const SaveLoadEntry &entry = _saveLoadEntries[_saveLoadSelected];
	if (!entry.description.empty())
		getMetaEngine()->removeSaveState(_targetName.c_str(), entry.slot);
	loadSaveLoadEntries();
}

// Moves the selection like the original's cursor keys, the list scrolls to keep it visible
void PrisonerEngine::moveSaveLoadSelection(int16 delta) {
	const int16 count = _saveLoadEntries.size();
	if (count == 0)
		return;

	_saveLoadEditing = false;
	_saveLoadTextScroll = 0;
	_saveLoadSelected = CLIP<int16>(_saveLoadSelected + delta, 0, count - 1);
	if (_saveLoadSelected < _saveLoadTop)
		_saveLoadTop = _saveLoadSelected;
	else if (_saveLoadSelected > _saveLoadTop + kSaveLoadRows - 1)
		_saveLoadTop = _saveLoadSelected - (kSaveLoadRows - 1);
}

int16 PrisonerEngine::getSaveLoadTextWidth() {
	const _MenuOffset *off = &kSaveLoadMenuOffsets[kSaveLoadBoxFirstRow];
	Common::String space = " ";
	return off->x2 - off->x1 - 2 * getTextWidth(space);
}

void PrisonerEngine::handleSaveLoadInput() {
	Common::KeyCode keyState;
	uint16 buttonState;

	getInputStatus(keyState, buttonState);

	const int16 count = _saveLoadEntries.size();
	const int16 visibleCount = MIN<int16>(count, kSaveLoadRows);

	if (buttonState & kLeftButton) {
		inpMouseSetWaitRelease(true);
		int8 boxIndex = findMenuItemAtPos(kSaveLoadMenuOffsets, kSaveLoadBoxCount, _mouseX - kMenuPanelX, _mouseY - kMenuPanelY);
		switch (boxIndex) {
		case kSaveLoadBoxOk:
			confirmSaveLoadEntry();
			break;
		case kSaveLoadBoxDelete:
			if (count > 0) {
				Common::String titleStr("T_WARN");
				Common::String actionStr("DELSAVE");
				displayDialog(CANCEL, titleStr, actionStr, &PrisonerEngine::dialogActionDeleteSave);
			}
			break;
		case kSaveLoadBoxCancel:
			closeSaveLoadScreen();
			break;
		case kSaveLoadBoxUp:
			moveSaveLoadSelection(-1);
			break;
		case kSaveLoadBoxDown:
			moveSaveLoadSelection(1);
			break;
		default:
			if (boxIndex >= kSaveLoadBoxFirstRow && boxIndex <= kSaveLoadBoxLastRow && count > 0) {
				int16 entryIndex = MIN<int16>(_saveLoadTop + boxIndex - kSaveLoadBoxFirstRow, count - 1);
				if (entryIndex != _saveLoadSelected) {
					_saveLoadEditing = false;
					_saveLoadTextScroll = 0;
				}
				_saveLoadSelected = entryIndex;
				bool doubleClick = _saveLoadLastClickRow == entryIndex &&
					getTicks() - _saveLoadLastClickTicks < kSaveLoadDoubleClickTicks;
				_saveLoadLastClickRow = entryIndex;
				_saveLoadLastClickTicks = getTicks();
				if (doubleClick)
					confirmSaveLoadEntry();
			}
			break;
		}
	} else if (buttonState & kRightButton) {
		inpMouseSetWaitRelease(true);
		closeSaveLoadScreen();
	} else if (keyState != Common::KEYCODE_INVALID) {
		inpKeybSetWaitRelease(true);
		// Saving takes the typed characters as the new description of the selected entry. Delete
		// (ASCII 127) has no character in the original's keyboard buffer.
		if (_saveLoadSaving && _keyAscii >= 0x20 && _keyAscii <= 0xA5 && _keyAscii != 127 && count > 0) {
			if (!_saveLoadEditing) {
				_saveLoadEditing = true;
				_saveLoadEditText.clear();
				_saveLoadTextScroll = 0;
				_saveLoadCursorTimer = kSaveLoadCursorBlinkFrames;
				_saveLoadCursorShown = false;
			}
			if (_saveLoadEditText.size() < kSaveLoadMaxDescriptionLength)
				_saveLoadEditText += (char)_keyAscii;
			return;
		}

		switch (keyState) {
		case Common::KEYCODE_UP:
			moveSaveLoadSelection(-1);
			break;
		case Common::KEYCODE_DOWN:
			moveSaveLoadSelection(1);
			break;
		case Common::KEYCODE_PAGEUP:
			moveSaveLoadSelection(-kSaveLoadRows);
			break;
		case Common::KEYCODE_PAGEDOWN: {
			// When the list scrolls the original puts the selection at the top
			const int16 oldTop = _saveLoadTop;
			moveSaveLoadSelection(kSaveLoadRows);
			if (_saveLoadTop != oldTop)
				_saveLoadTop = MIN<int16>(_saveLoadSelected, count - visibleCount);
			break;
		}
		case Common::KEYCODE_HOME:
			moveSaveLoadSelection(-count);
			break;
		case Common::KEYCODE_END:
			moveSaveLoadSelection(count);
			_saveLoadTop = count - visibleCount;
			break;
		case Common::KEYCODE_LEFT:
			// Scrolls a description that is wider than its line
			if (!_saveLoadEditing && _saveLoadTextScroll > 0)
				_saveLoadTextScroll--;
			break;
		case Common::KEYCODE_RIGHT:
			if (!_saveLoadEditing && count > 0) {
				const Common::String &description = _saveLoadEntries[_saveLoadSelected].description;
				if ((uint)_saveLoadTextScroll < description.size() &&
					getTextWidth(Common::String(description.c_str() + _saveLoadTextScroll)) > getSaveLoadTextWidth())
					_saveLoadTextScroll++;
			}
			break;
		case Common::KEYCODE_BACKSPACE:
			if (_saveLoadEditing && !_saveLoadEditText.empty()) {
				_saveLoadEditText.deleteLastChar();
				if (_saveLoadTextScroll > 0)
					_saveLoadTextScroll--;
			}
			break;
		case Common::KEYCODE_DELETE:
			if (count > 0) {
				Common::String titleStr("T_WARN");
				Common::String actionStr("DELSAVE");
				displayDialog(CANCEL, titleStr, actionStr, &PrisonerEngine::dialogActionDeleteSave);
			}
			break;
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			confirmSaveLoadEntry();
			break;
		case Common::KEYCODE_ESCAPE:
			closeSaveLoadScreen();
			break;
		default:
			break;
		}
	}

	updateMouseCursor();
}

void PrisonerEngine::updateSaveLoadMenu(int16 x, int16 y) {
	if (_isDialogMenuShowing) {
		// The delete confirmation box
		updateMainMenu(x, y);
		return;
	}

	AnimationResource *menuPanel = _res->get<AnimationResource>(_menuPanelResourceCacheSlots[_selectedMenuIndex]);
	drawPanelSequence(_screen, menuPanel, 0, x, y);

	_PakMenuDirectoryEntry panelData = _kPrisonerPanelData[_selectedMenuIndex];
	setFontColors(_menuFont, panelData.outlineColor, panelData.inkColor);
	setActiveFont(_menuFont);

	const int8 textBoxes[] = {kSaveLoadBoxTitle, kSaveLoadBoxOk, kSaveLoadBoxDelete, kSaveLoadBoxCancel};
	for (uint i = 0; i < _saveLoadMenuItems.size(); i++) {
		const _MenuOffset *off = &kSaveLoadMenuOffsets[textBoxes[i]];
		drawTextEx(x + off->x1, x + off->x2, y + off->y1, y + off->y2, _saveLoadMenuItems[i]);
	}

	const int16 count = _saveLoadEntries.size();
	const int16 visibleCount = MIN<int16>(count - _saveLoadTop, kSaveLoadRows);
	for (int16 i = 0; i < visibleCount; i++) {
		const int16 entryIndex = _saveLoadTop + i;
		const _MenuOffset *off = &kSaveLoadMenuOffsets[kSaveLoadBoxFirstRow + i];
		Common::String text;
		if (entryIndex == _saveLoadSelected) {
			drawPanelSequence(_screen, menuPanel, 1, x + off->x1, y + off->y1);
			if (_saveLoadEditing) {
				if (--_saveLoadCursorTimer == 0) {
					_saveLoadCursorTimer = kSaveLoadCursorBlinkFrames;
					_saveLoadCursorShown = !_saveLoadCursorShown;
				}
				text = _saveLoadEditText + kSaveLoadCursorChars[_saveLoadCursorShown ? 1 : 0];
				// Scroll the text while typing so its end stays visible
				if ((uint)_saveLoadTextScroll < text.size() &&
					getTextWidth(Common::String(text.c_str() + _saveLoadTextScroll)) > getSaveLoadTextWidth())
					_saveLoadTextScroll++;
			} else {
				text = _saveLoadEntries[entryIndex].description;
			}
			text = Common::String(text.c_str() + MIN<uint>(_saveLoadTextScroll, text.size()));
		} else {
			text = _saveLoadEntries[entryIndex].description;
		}
		_screen->setClipRect(x + off->x1, y + off->y1, x + off->x2, y + off->y2);
		drawTextEx(x + off->x1, x + off->x2, y + off->y1, y + off->y2, text);
		_screen->setClipRect(0, 0, 639, 479);
		if (entryIndex == _saveLoadSelected)
			drawPanelSequence(_screen, menuPanel, 2, x + off->x1, y + off->y1);
	}

	updateSaveLoadThumbnail();
	const _MenuOffset *off = &kSaveLoadMenuOffsets[kSaveLoadBoxThumbnail];
	Graphics::Surface *screen = _screen->getScreen();
	for (int16 thumbY = 0; thumbY < kSaveLoadThumbnailHeight; thumbY++)
		memcpy(screen->getBasePtr(x + off->x1, y + off->y1 + thumbY),
			&_saveLoadThumbnail[thumbY * kSaveLoadThumbnailWidth], kSaveLoadThumbnailWidth);

	// The arrows only show when the selection can move that way
	if (count > 0 && (_saveLoadSelected > 0 || _saveLoadTop > 0))
		drawPanelSequence(_screen, menuPanel, 3, x, y);
	if (count > 0 && !(_saveLoadSelected == count - 1 && _saveLoadTop == count - MIN<int16>(count, kSaveLoadRows)))
		drawPanelSequence(_screen, menuPanel, 4, x, y);
}

} // End of namespace Prisoner
