#pragma once

#include "common_header.h"
#include "loaders.h"
#include "XMLDoc.h"
#include "GameParam.h"

namespace pyrodactyl
{
	namespace input
	{
		class InputVal
		{
		public:
			//Name of the key (such as "punch", "up")
			std::string name;

			//Keyboard keys
			SDL_Scancode key, alt;

			//Controller button
			SDL_GameControllerButton c_bu;

			//The controller axis
			struct AxisData
			{
				//The axis
				SDL_GameControllerAxis id;

				//Value of controller axis - because controller axes have a range of values
				int val;

				//Do we want to do a "less than" or "greater than" comparison?
				bool greater;

				//For menus, we want to consider a "flick" of the analog stick - i.e. a back and forth movement
				//This means we toggle this flag once the stick hits maximum value, then check if the stick has
				//gone back to its previous value
				bool toggle;

				AxisData() { id = SDL_CONTROLLER_AXIS_INVALID; val = 0; toggle = false; greater = false; }

				void LoadState(rapidxml::xml_node<char> *node)
				{
					LoadEnum(id, "id", node);
					LoadNum(val, "val", node);
					LoadBool(greater, "greater", node);
				}

				void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root)
				{
					rapidxml::xml_node<char> *child;
					child = doc.allocate_node(rapidxml::node_element, "axis");

					child->append_attribute(doc.allocate_attribute("id", gStrPool.Get(id)));
					child->append_attribute(doc.allocate_attribute("val", gStrPool.Get(val)));

					SaveBool(greater, "greater", doc, child);

					root->append_node(child);
				}
			} c_ax;

			InputVal();

			const bool Equals(const SDL_KeyboardEvent &val);
			const bool Equals(const SDL_ControllerButtonEvent &Event);
			const bool Equals(const SDL_ControllerAxisEvent &Event);

			void LoadState(rapidxml::xml_node<char> *node);
			void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char* title);
		};
	}
}