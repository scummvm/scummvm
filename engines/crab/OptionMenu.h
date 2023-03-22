#pragma once

#include "GeneralSettingMenu.h"
#include "GfxSettingMenu.h"
#include "ImageData.h"
#include "KeyBindMenu.h"
#include "StateButton.h"
#include "ToggleButton.h"
#include "common_header.h"
#include "slider.h"

namespace pyrodactyl {
namespace ui {
class OptionMenu {
	// What overall state the menu is in
	enum { STATE_GENERAL,
		   STATE_GRAPHICS,
		   STATE_KEYBOARD,
		   STATE_CONTROLLER,
		   STATE_ENTER_W,
		   STATE_ENTER_H,
		   STATE_CONFIRM } state;

	// The overall menu for switching between states
	// The second last button is save, the last button is cancel
	Menu<StateButton> menu;

	// The background image
	ImageData bg;

	// The graphical settings menu
	GfxSettingMenu gfx;

	// The general settings menu
	GeneralSettingMenu general;

	// Keyboard controls menu
	KeyBindMenu keybind;

	// The controller controls are just drawn in a single image, no reassign options
	ImageData conbind;

	// The UI for accepting/rejecting change in resolution
	HoverInfo notice_res;
	ImageData questionbox;
	Button accept, cancel;

	// If the user wants to input a custom resolution, these are used along with the question box
	TextArea prompt_w, prompt_h;

	// The countdown until the timer resets
	TextData countdown;
	Timer timer;

	// Function to draw the main menu (controls, settings, save, cancel)
	bool HandleTabs(Button &back, const SDL_Event &Event);

public:
	bool loaded;

	OptionMenu(void) {
		loaded = false;
		state = STATE_GENERAL;
		menu.UseKeyboard(true);
	}
	~OptionMenu(void) {}

	void Reset();
	bool DisableHotkeys() { return keybind.DisableHotkeys(); }

	void Load(const std::string &filename);
	void Draw(Button &back);
	bool HandleEvents(Button &back, const SDL_Event &Event);
	void InternalEvents();

	void SetUI();
	void SaveState();
};

extern OptionMenu gOptionMenu;
} // End of namespace ui
} // End of namespace pyrodactyl