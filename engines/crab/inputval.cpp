#include "inputval.h"
#include "stdafx.h"

using namespace pyrodactyl::input;

InputVal::InputVal() {
	key = SDL_SCANCODE_UNKNOWN;
	alt = SDL_SCANCODE_UNKNOWN;
	c_bu = SDL_CONTROLLER_BUTTON_INVALID;
}

//------------------------------------------------------------------------
// Purpose: Load input values
//------------------------------------------------------------------------
void InputVal::LoadState(rapidxml::xml_node<char> *node) {
	LoadStr(name, "name", node);
	LoadEnum(key, "key", node);
	LoadEnum(alt, "alt", node);
	LoadEnum(c_bu, "bu", node);

	if (NodeValid("axis", node, false))
		c_ax.LoadState(node->first_node("axis"));
}

//------------------------------------------------------------------------
// Purpose: Save them
//------------------------------------------------------------------------
void InputVal::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *title) {
	rapidxml::xml_node<char> *child;
	child = doc.allocate_node(rapidxml::node_element, title);

	child->append_attribute(doc.allocate_attribute("name", name.c_str()));
	child->append_attribute(doc.allocate_attribute("key", gStrPool.Get(key)));
	child->append_attribute(doc.allocate_attribute("alt", gStrPool.Get(alt)));
	child->append_attribute(doc.allocate_attribute("bu", gStrPool.Get(c_bu)));

	if (c_ax.id != SDL_CONTROLLER_AXIS_INVALID)
		c_ax.SaveState(doc, child);

	root->append_node(child);
}

//------------------------------------------------------------------------
// Purpose: See if we have been using the analog sticks
//------------------------------------------------------------------------
const bool InputVal::Equals(const SDL_ControllerAxisEvent &Event) {
	if (c_ax.id == Event.axis) {
		if (!c_ax.toggle)
			c_ax.toggle = (c_ax.greater && Event.value > c_ax.val) || (!c_ax.greater && Event.value < c_ax.val);
		else {
			if (!((c_ax.greater && Event.value > c_ax.val) || (!c_ax.greater && Event.value < c_ax.val))) {
				c_ax.toggle = false;
				return true;
			};
		}
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: See if we have been using the controller buttons sticks
//------------------------------------------------------------------------
const bool InputVal::Equals(const SDL_ControllerButtonEvent &Event) {
	return (c_bu == Event.button);
}

//------------------------------------------------------------------------
// Purpose: See if we have been using the keyboard
//------------------------------------------------------------------------
const bool InputVal::Equals(const SDL_KeyboardEvent &Event) {
	return (key == Event.keysym.scancode || alt == Event.keysym.scancode);
}