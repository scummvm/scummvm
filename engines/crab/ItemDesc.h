#pragma once

#include "ParagraphData.h"
#include "SectionHeader.h"
#include "Item.h"

namespace pyrodactyl
{
	namespace ui
	{
		class ItemDesc
		{
			//The name of the item
			TextData name;

			//The description of the item
			ParagraphData desc;

		public:
			ItemDesc(){}
			~ItemDesc(){}

			void Load(rapidxml::xml_node<char> *node)
			{
				if (NodeValid("name", node))
					name.Load(node->first_node("name"));

				if (NodeValid("desc", node))
					desc.Load(node->first_node("desc"));
			}

			void Draw(pyrodactyl::item::Item &item)
			{
				name.Draw(item.name);
				desc.Draw(item.desc);
			}

			void SetUI()
			{
				name.SetUI();
				desc.SetUI();
			}
		};
	}
}
