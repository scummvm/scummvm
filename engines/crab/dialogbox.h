//=============================================================================
// Author:   Arvind
// Purpose:  Dialog box!
//=============================================================================
#pragma once

#include "common_header.h"
#include "GameEventInfo.h"
#include "ParagraphData.h"
#include "button.h"

namespace pyrodactyl
{
	namespace ui
	{
		class GameDialogBox
		{
			//The position of the dialog box
			Element pos;

			//The area you click to skip to the next dialog
			Button button;

			//Information related to drawing the dialog box
			ParagraphData text;

			//The usual background
			ImageKey bg;

			//The background drawn when we don't want to show the opinion bars
			ImageKey bg_p;

		public:
			GameDialogBox() { bg = 0; bg_p = 0; }
			~GameDialogBox(){}

			void Load(rapidxml::xml_node<char> *node);

			void Draw(pyrodactyl::event::Info &info, std::string &message);
			void Draw(const bool &player);

			bool HandleEvents(const SDL_Event &Event);

			void SetUI();
		};
	}
}