#pragma once

#include "common_header.h"
#include "textarea.h"
#include "FileMenu.h"

namespace pyrodactyl
{
	namespace ui
	{
		class GameSaveMenu : public FileMenu<SaveFileData>
		{
			enum State
			{
				STATE_NORMAL,
				STATE_NAME
			} state;

			//This stores the name of the save slot
			TextArea ta_name;

			//The index of the selected button
			int index;

			void AddButton(boost::filesystem::path p, unsigned int &slot_index, unsigned int &menu_index);

		public:
			GameSaveMenu(){ state = STATE_NORMAL; index = 0; }
			~GameSaveMenu(){}

			void ScanDir();
			bool DisableHotkeys() { return state == STATE_NAME; }

			void Load(rapidxml::xml_node<char> *node);
			bool HandleEvents(const SDL_Event &Event);
			void Draw();

			void SetUI();
		};
	}
}