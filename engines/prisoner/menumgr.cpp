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
	_dialogActionCallback = actionCallback;
	Common::StringArray DialogIds = {"", "OK", "CANCEL", "IGNORE"};
	Common::String title = getGlobalText(titleStrId);
	Common::String action = getGlobalText(actionStrId);
	Common::String option = getGlobalText(DialogIds[type]);

	_dialogStrings.clear();
	_dialogStrings.push_back(title);
	_dialogStrings.push_back(DialogIds[1]); // OK is always on the left
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
	_selectedMenuIndex = 0;
	_menuMouseCursorActive = 1;
	_menuMouseCursor = 13;
}

void PrisonerEngine::loadOnscreenMenuText() {
}

void PrisonerEngine::runMainMenu_initMessages() {
	Common::StringArray MenuIds = {"M_RETG", "M_NEWG", "M_LOADG", "M_SAVEG", "M_OPT", "EXIT_G", "MAINM"};
	for (Common::StringArray::iterator id = MenuIds.begin(); id != MenuIds.end(); id++) {
		_menuItems.push_back(getGlobalText(*id));
	}
}

void PrisonerEngine::runMainMenu_addClickBoxes(int16 x, int16 y) {
	for (uint8 i = 0; i < _menuItems.size(); i++) {
		addClickBox(
			x + _mainMenuOffsets[i].x1,
			y + _mainMenuOffsets[i].y1,
			x + _mainMenuOffsets[i].x2,
			y + _mainMenuOffsets[i].y2,
			i);
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
	int16 clickBoxIndex;

	updateEvents();

	getInputStatus(keyState, buttonState);

	clickBoxIndex = findClickBoxAtPos(_mouseX, _mouseY, -1);
	if ((buttonState & kLeftButton) && clickBoxIndex != -1) {
		doMenuAction(clickBoxIndex);
	}

	if (keyState == Common::KEYCODE_DOWN) {
		_selectedMenuIndex = (_selectedMenuIndex + 1) % MAIN_MENU_SIZE;
		inpKeybSetWaitRelease(true);
	} else if (keyState == Common::KEYCODE_UP) {
		_selectedMenuIndex = (_selectedMenuIndex - 1) % MAIN_MENU_SIZE;
		if (_selectedMenuIndex < 0) {
			_selectedMenuIndex += MAIN_MENU_SIZE;
		}
		inpKeybSetWaitRelease(true);
	} else if (keyState == Common::KEYCODE_RETURN) {
		doMenuAction(_selectedMenuIndex);
	}

	updateMouseCursor();
}

void PrisonerEngine::handleDialogMenuInput() {
	Common::KeyCode keyState;
	uint16 buttonState;
	int16 clickBoxIndex;

	updateEvents();

	getInputStatus(keyState, buttonState);

	clickBoxIndex = findClickBoxAtPos(_mouseX, _mouseY, -1);
	if ((buttonState & kLeftButton) && clickBoxIndex != -1) {
		doMenuAction(clickBoxIndex);
	}

	if (keyState == Common::KEYCODE_ESCAPE) {
		_isDialogMenuShowing = false;
	} else if (keyState == Common::KEYCODE_RETURN) {
		_isDialogMenuShowing = false;
		(this->*_dialogActionCallback)();
	}

	updateMouseCursor();
}

void PrisonerEngine::dialogActionNewGame() {
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
	case NEW_GAME:
		inpKeybSetWaitRelease(true);
		displayDialog(CANCEL, titleStr, newGameActionStr, &PrisonerEngine::dialogActionNewGame);

		debug("CLICK ON = %d", clickBoxIndex);
		break;
	case LOAD_GAME:
		loadGameDialog();
		break;
	case EXIT:
		inpKeybSetWaitRelease(true);
		displayDialog(CANCEL, titleStr, exitActionStr, &PrisonerEngine::dialogActionExit);
		break;
	default:
		break;
	}
}

void PrisonerEngine::updateDialogMenu(int16 x, int16 y) {
	_screen->drawAnimationElement(_dialogPanel, 0, x, y, 0);

	for (uint8 i = 0; i < _dialogStrings.size(); i++) {
		const _MenuOffset *off = &(_dialogOffsets[i]);
		// TODO: Remove after debugging
		//_screen->frameRect(x + off->x1, y + off->y1, x + off->x2, y + off->y2, 255);
		if (_dialogStrings.size() > 0) {
			auto titleOff = _dialogOffsets[i];
			drawTextEx(x + titleOff.x1, x + titleOff.x2, y + titleOff.y1, y + titleOff.y2, _dialogStrings[i]);
		}
	}
}

void PrisonerEngine::updateMenu(int16 x, int16 y) {
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

		if (_mainMenuRequested) {
			if (i == 0 || i == 3)
				_screen->drawTransparentRect(
					x + off->x1,
					y + off->y1,
					x + off->x2,
					y + off->y2);
		}
	}

	// TODO: Fix menu selection (figure out where it's being done in the asm)
	const _MenuOffset *off = &(_mainMenuOffsets[_selectedMenuIndex]);
	_screen->frameRect(x + off->x1, y + off->y1, x + off->x2, y + off->y2, 1);
}

} // End of namespace Prisoner
