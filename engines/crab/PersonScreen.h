#pragma once

#include "GameEventInfo.h"
#include "ImageData.h"
#include "TraitMenu.h"
#include "gameevent.h"
#include "level.h"
#include "menu.h"
#include "person.h"
#include "sprite.h"

namespace pyrodactyl {
namespace ui {
class PersonScreen {
	// The background
	ImageData bg;

	// The name of the character
	TextData name;

	// The place where the sprite should be drawn
	Element img;

	// The buttons for drawing traits
	TraitMenu menu;

	// Store the current person sprite temporarily
	pyrodactyl::anim::Sprite *cur_sp;

public:
	PersonScreen() { cur_sp = nullptr; }
	~PersonScreen() {}

	void Load(const std::string &filename);
	void Cache(pyrodactyl::event::Info &info, const std::string &id, pyrodactyl::level::Level &level);

	void HandleEvents(pyrodactyl::event::Info &info, const std::string &id, const SDL_Event &Event);
	void InternalEvents();
	void Draw(pyrodactyl::event::Info &info, const std::string &id);

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl