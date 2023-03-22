#pragma once

#include "RadioButtonMenu.h"
#include "ToggleButton.h"
#include "common_header.h"
#include "slider.h"

namespace pyrodactyl {
namespace ui {
class GeneralSettingMenu {
	// The volume sliders and their caption
	Slider vol_music, vol_effects;
	HoverInfo notice_volume;

	// Other settings
	ToggleButton save_on_exit, mouse_trap;

	// The menu for select pop-up text speed
	RadioButtonMenu text_speed;

public:
	GeneralSettingMenu() {}
	~GeneralSettingMenu() {}

	void Load(rapidxml::xml_node<char> *node);
	void HandleEvents(const SDL_Event &Event);
	void InternalEvents();

	void Draw();
	void SetUI();

	void CreateBackup() {
		vol_music.CreateBackup();
		vol_effects.CreateBackup();
	}

	void RestoreBackup();
};
} // End of namespace ui
} // End of namespace pyrodactyl
