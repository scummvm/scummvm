#pragma once

#include "FileMenu.h"
#include "GameEventInfo.h"
#include "OptionMenu.h"
#include "SaveGameMenu.h"
#include "SlideShow.h"
#include "common_header.h"
#include "menu.h"
#include "slider.h"
#include "textarea.h"

namespace pyrodactyl {
namespace ui {
enum PauseSignal { PS_NONE,
				   PS_RESUME,
				   PS_SAVE,
				   PS_LOAD,
				   PS_HELP,
				   PS_QUIT_MENU,
				   PS_QUIT_GAME };

class PauseMenu {
	enum PauseState { STATE_NORMAL,
					  STATE_SAVE,
					  STATE_OPTION,
					  STATE_LOAD } state;

	// The pause menu background
	ImageData bg;

	// The buttons in the menu
	ButtonMenu menu;

	// Save game menu
	GameSaveMenu save;

	// The selected main menu button
	int choice;

public:
	PauseMenu(void) {
		state = STATE_NORMAL;
		choice = -1;
	}
	~PauseMenu(void) {}

	void UpdateMode(const bool &ironman) {
		menu.element.at(PS_SAVE - 1).visible = !ironman;
		menu.element.at(PS_LOAD - 1).visible = !ironman;
	}

	void Load(rapidxml::xml_node<char> *node);
	PauseSignal HandleEvents(const SDL_Event &Event, Button &back);

	// Returns true if inside options menu, false otherwise
	bool Draw(Button &back);

	void Reset() { state = STATE_NORMAL; }
	void ScanDir() { save.ScanDir(); }
	std::string SaveFile() { return save.SelectedPath(); }
	bool DisableHotkeys();

	// Should we allow the pause key(default escape) to quit to main menu?
	// This is done because esc is both the "go back on menu level" and the pause key
	bool ShowLevel() { return state == STATE_NORMAL; }

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl