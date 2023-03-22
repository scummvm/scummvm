#pragma once

#include "engines/crab/XMLDoc.h"
#include "engines/crab/common_header.h"
#include "engines/crab/gametype.h"
#include "engines/crab/numstr.h"

namespace Crab {

// Function to check if node is valid
// return true for valid, false otherwise
bool NodeValid(rapidxml::xml_node<char> *node, const bool &echo = true);
bool NodeValid(const Common::String &name, rapidxml::xml_node<char> *parent_node, const bool &echo = true);

// Functions to load attributes from xml files
// return true on success, false on failure
bool LoadStr(Common::String &val, const Common::String &name, rapidxml::xml_node<char> *node, const bool &echo = true);

// Used for loading numerical types
template<typename T>
bool LoadNum(T &val, const Common::String &name, rapidxml::xml_node<char> *node, const bool &echo = true) {
	if (node->first_attribute(name.c_str()) != NULL)
		val = StringToNumber<T>(node->first_attribute(name.c_str())->value());
	else {
		/*if (echo)
		{
		Common::String error_msg = "attribute " + name + " not found in node " + node->parent()->name() + " -> " + node->name();
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "XML error", error_msg.c_str(), NULL);
		}*/
		return false;
	}

	return true;
}

// Used for loading enumerator types that are integers
template<typename T>
bool LoadEnum(T &val, const Common::String &name, rapidxml::xml_node<char> *node, const bool &echo = true) {
	if (node->first_attribute(name.c_str()) != NULL)
		val = static_cast<T>(StringToNumber<int>(node->first_attribute(name.c_str())->value()));
	else {
		/*if (echo)
		{
		std::string error_msg = "attribute " + name + " not found in node " + node->parent()->name() + " -> " + node->name();
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "XML error", error_msg.c_str(), NULL);
		}*/
		return false;
	}

	return true;
}

#if 0

//Load Rectangle
bool LoadRect(SDL_Rect &rect, rapidxml::xml_node<char> *node, const bool &echo = true,
	const Common::String &x_name = "x", const Common::String &y_name = "y", const Common::String &w_name = "w", const Common::String &h_name = "h");

//Load Color
bool LoadColor(SDL_Color &col, rapidxml::xml_node<char> *node, const bool &echo = true,
	const Common::String &r_name = "r", const Common::String &g_name = "g", const Common::String &b_name = "b");

//Shortcut to load integer color index to a number
bool LoadColor(int &col, rapidxml::xml_node<char> *node, const bool &echo = true);

#endif

// Load two dimensional coordinates
template<typename T>
bool LoadXY(T &x, T &y, rapidxml::xml_node<char> *node, const bool &echo = true) {
	bool result = LoadNum(x, "x", node, echo);
	result = LoadNum(y, "y", node, echo) && result;

	return result;
}

// Load three dimensional coordinates
template<typename T>
bool LoadXYZ(T &x, T &y, T &z, rapidxml::xml_node<char> *node, const bool &echo = true) {
	bool result = LoadNum(x, "x", node, echo);
	result = LoadNum(y, "y", node, echo) && result;
	result = LoadNum(z, "z", node, echo) && result;

	return result;
}

// Load Boolean variable
bool LoadBool(bool &var, const Common::String &name, rapidxml::xml_node<char> *node, const bool &echo = true);

// Write Boolean variable to file
void SaveBool(const bool &var, const char *name, rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);

// Functions to load various type of objects
bool LoadStatType(pyrodactyl::stat::StatType &type, rapidxml::xml_node<char> *node, const bool &echo = true);

bool LoadAlign(Align &align, rapidxml::xml_node<char> *node, const bool &echo = true, const Common::String &name = "align");
bool LoadDirection(Direction &dir, rapidxml::xml_node<char> *node, const bool &echo = true, const Common::String &name = "dir");

bool LoadTextureFlipType(TextureFlipType &flip, rapidxml::xml_node<char> *node, const bool &echo = true);

// Check the version of a file
unsigned int Version(const Common::String &filename);

} // End of namespace Crab
