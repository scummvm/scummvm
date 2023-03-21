#pragma once

#include "common_header.h"
#include "loaders.h"

namespace pyrodactyl
{
	namespace event
	{
		struct Quest
		{
			//The title of the quest
			std::string title;

			//The objective text, the last one being the current objective
			std::deque<std::string> text;

			//Keep track of which quests the player has read
			bool unread;

			//Does this quest have an associated destination marker
			bool marker;

			Quest() { unread = true; marker = false; }
			Quest(const std::string &Title, const std::string &Text, const bool &Unread, const bool &Marker);

			void LoadState(rapidxml::xml_node<char> *node);
			void SaveState(rapidxml::xml_document<char> &doc, rapidxml::xml_node<char> *root);
		};
	}
}