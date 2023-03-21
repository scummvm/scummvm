#pragma once

#include "common_header.h"
#include "ImageManager.h"
#include "Item.h"
#include "StateButton.h"

namespace pyrodactyl
{
	namespace item
	{
		enum SlotType { SLOT_EQUIP, SLOT_STORAGE };

		class ItemSlot : public pyrodactyl::ui::StateButton
		{
			//Ignore the type of item check
			bool no_type;

		public:
			//The type of item allowed in this slot (can be overridden by item_type)
			std::string item_type;

			//Is the slot empty?
			bool empty;

			//Is the slot enabled? (used for stat calculation)
			bool enabled;

			//Is this a new item? Draw the unread notification icon if so
			bool unread;

			//The type of the item slot
			SlotType category;

			//The item contained in the slot
			Item item;

			ItemSlot() { empty = true; enabled = true; category = SLOT_STORAGE; no_type = false; unread = false; }
			~ItemSlot() {}

			void Init(const ItemSlot &ref, const int &XOffset = 0, const int &YOffset = 0);
			void Load(rapidxml::xml_node<char> *node);

			void Draw();
			pyrodactyl::ui::ButtonAction HandleEvents(const SDL_Event &Event, const int &XOffset = 0, const int &YOffset = 0);

			bool CanSwap(ItemSlot &target){ return target.no_type || item.type == target.item_type; }
			bool Swap(ItemSlot &target);
			bool Equip(Item &i);

			void StatChange(pyrodactyl::people::Person &obj, bool increase);

			void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
			void LoadState(rapidxml::xml_node<char> *node);
		};
	}
}