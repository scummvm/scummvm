#pragma once

#include "FileMenu.h"
#include "common_header.h"

namespace pyrodactyl {
namespace ui {
class ModMenu : public FileMenu<ModFileData> {
public:
	ModMenu() {}
	~ModMenu() {}

	void Load(const std::string &filename);
	bool HandleEvents(const SDL_Event &Event);
	void Draw();
};
} // End of namespace ui
} // End of namespace pyrodactyl