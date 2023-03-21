#pragma once

#include "common_header.h"
#include "StatDrawHelper.h"
#include "ItemMenu.h"

namespace pyrodactyl
{
	namespace item
	{
		//All the items owned by characters controlled by the player
		class ItemCollection
		{
			//The items for all player characters
			std::unordered_map<std::string, ItemMenu> item;

			//The reference information for these menus used to display these items

			//The reference item slot
			ItemSlot ref;

			//This vector stores the increments in x,y for each new slot
			Vector2i inc;

			//The dimensions of the menu
			unsigned int rows, cols;

			//Draw item description when user clicks an item to select it
			pyrodactyl::ui::ItemDesc item_info;

			//Should we enable keyboard for the menus
			bool usekeyboard;

		public:
			ItemCollection(){ rows = 1; cols = 1; usekeyboard = true; }
			~ItemCollection(){}

			void Load(rapidxml::xml_node<char> *node);
			void HandleEvents(const std::string &char_id, const SDL_Event &Event);

			void Init(const std::string &char_id);
			void Draw(const std::string &char_id);

			//Requires: id of the character, the item information
			void Add(const std::string &char_id, Item &item_data);

			//Requires: id of the character from which to remove the item, and id of the item
			void Del(const std::string &char_id, const std::string &item_id);

			//Requires: id of the character, the name of the container and name of the item
			bool Has(const std::string &char_id, const std::string &container, const std::string &item_id);

			void LoadState(rapidxml::xml_node<char> *node);
			void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);

			void SetUI();
		};
	}
}