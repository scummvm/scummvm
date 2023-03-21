#pragma once

#include "common_header.h"
#include "FileMenu.h"

namespace pyrodactyl
{
	namespace ui
	{
		class ModMenu : public FileMenu<ModFileData>
		{
		public:

			ModMenu(){}
			~ModMenu(){}

			void Load(const std::string &filename);
			bool HandleEvents(const SDL_Event &Event);
			void Draw();
		};
	}
}