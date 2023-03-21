#pragma once

#include "common_header.h"
#include "ItemCollection.h"
#include "ImageData.h"
#include "ItemSlot.h"
#include "Stat.h"
#include "button.h"

namespace pyrodactyl
{
	namespace ui
	{
		class Inventory
		{
			//The background image and location
			ImageData bg;

			//The equipment and storage space, stored according to the player character id
			pyrodactyl::item::ItemCollection collection;

			//To draw the player stats
			//pyrodactyl::stat::StatDrawHelper helper;

			//The file where all the item information is stored
			std::string itemfile;

			//Used to draw the money value
			Button money;

		public:
			Inventory(){}
			~Inventory(){}

			void Init(const std::string &char_id) { collection.Init(char_id); }

			void LoadItem(const std::string &char_id, const std::string &name);
			void AddItem(const std::string &char_id, pyrodactyl::item::Item &item);
			void DelItem(const std::string &char_id, const std::string &item_id);
			bool HasItem(const std::string &char_id, const std::string &container, const std::string &item_id);

			void Load(const std::string &filename);
			void Draw(pyrodactyl::people::Person &obj, const int &money_val);
			void HandleEvents(const std::string &char_id, const SDL_Event &Event);

			void LoadState(rapidxml::xml_node<char> *node);
			void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);

			void ItemFile(const std::string &filename) { itemfile = filename; }

			void SetUI();
		};
	}
}