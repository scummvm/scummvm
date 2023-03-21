#pragma once

#include "common_header.h"
#include "menu.h"
#include "person.h"
#include "ItemSlot.h"
#include "StatDrawHelper.h"
#include "StatPreview.h"
#include "ItemDesc.h"

namespace pyrodactyl
{
	namespace item
	{
		//The menu is used to handle interactions like the player clicking on an item to equip it
		class ItemMenu : public pyrodactyl::ui::Menu<ItemSlot>
		{
		protected:
			int preview_index, select_index;

			//Variable to determine if stats of object need updating
			bool update;

		public:
			//The preview for selected item and hovered item
			//StatPreview select, hover;

			ItemMenu(){ preview_index = -1; select_index = -1; update = false; }
			~ItemMenu(){}

			int HoverIndex() { return hover_index; }

			void Draw(pyrodactyl::ui::ItemDesc &item_info);
			/*pyrodactyl::people::Person &obj, pyrodactyl::stat::StatDrawHelper &helper*/

			void Init(const ItemSlot &ref, const Vector2i &inc, const unsigned int &rows,
				const unsigned int &cols, const bool &keyboard);

			void HandleEvents(const SDL_Event &Event, const int &XOffset = 0, const int &YOffset = 0);
			/* pyrodactyl::people::Person &obj,*/

			bool Swap(ItemMenu &target, int index);
			bool Equip(Item &item);
			bool Del(const std::string &id);
			bool Has(const std::string &container, const std::string &id);

			//Used to calculate enabled slots
			//void SetEnable();

			void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
			void LoadState(rapidxml::xml_node<char> *node);
		};
	}
}