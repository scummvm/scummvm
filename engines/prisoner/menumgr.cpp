#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"

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
	{PANEL_SIZE_END, NULL, 0, 0, 0}
};

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
	{98, 13, 413, 39}
};

void PrisonerEngine::loadMenuPanels() {
	const _PakMenuDirectoryEntry *xentry = _kPrisonerPanelData;
	uint8 i = 0;
	while (xentry->pakName) {
		Common::String pakName = xentry->pakName;
		_menuPanelResourceCacheSlots[i++] = _res->load<AnimationResource>(pakName, xentry->pakSlot, 11);
		xentry++;
	}

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
	//addClickBox(x + 111, y + 119, x + 402, y + 140, 0);
	//addClickBox(x + 111, y + 144, x + 402, y + 165, 1);
	//addClickBox(x + 111, y + 177, x + 402, y + 198, 2);
	//addClickBox(x + 111, y + 202, x + 402, y + 223, 3);
	//addClickBox(x + 111, y + 235, x + 402, y + 256, 4);
	//addClickBox(x + 111, y + 260, x + 402, y + 281, 5);
	//addClickBox(x + 98, y + 13, x + 413, y + 39, 6);
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
		inpKeybSetWaitRelease(true);
	} else if (keyState == Common::KEYCODE_RETURN) {
		doMenuAction(_selectedMenuIndex);
	}

	updateMouseCursor();
}

void PrisonerEngine::doMenuAction(uint8 clickBoxIndex) {
	switch (clickBoxIndex) {
	case NEW_GAME:
		debug("CLICK ON = %d", clickBoxIndex);

		_mainMenuRequested = false;
		_newModuleIndex = 2;
		_newSceneIndex = 39;

		checkForSceneChange();
		break;
	case LOAD_GAME:
		loadGameDialog();
		break;
	case EXIT:
		_mainLoopDone = true;
		break;
	default:
		break;
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
