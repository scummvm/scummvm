#pragma once

#include "common_header.h"
#include "numstr.h"
#include "gametype.h"
#include "XMLDoc.h"

//Function to check if node is valid
//return true for valid, false otherwise
bool NodeValid(rapidxml::xml_node<char> *node, const bool &echo = true);
bool NodeValid(const std::string &name, rapidxml::xml_node<char> *parent_node, const bool &echo = true);

//Functions to load attributes from xml files
//return true on success, false on failure
bool LoadStr(std::string &val, const std::string &name, rapidxml::xml_node<char> *node, const bool &echo = true);

//Used for loading numerical types
template <typename T>
bool LoadNum(T &val, const std::string &name, rapidxml::xml_node<char> *node, const bool &echo = true)
{
	if (node->first_attribute(name.c_str()) != NULL)
		val = StringToNumber<T>(node->first_attribute(name.c_str())->value());
	else
	{
		/*if (echo)
		{
		std::string error_msg = "attribute " + name + " not found in node " + node->parent()->name() + " -> " + node->name();
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "XML error", error_msg.c_str(), NULL);
		}*/
		return false;
	}

	return true;
}

//Used for loading enumerator types that are integers
template <typename T>
bool LoadEnum(T &val, const std::string &name, rapidxml::xml_node<char> *node, const bool &echo = true)
{
	if (node->first_attribute(name.c_str()) != NULL)
		val = static_cast<T>(StringToNumber<int>(node->first_attribute(name.c_str())->value()));
	else
	{
		/*if (echo)
		{
		std::string error_msg = "attribute " + name + " not found in node " + node->parent()->name() + " -> " + node->name();
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "XML error", error_msg.c_str(), NULL);
		}*/
		return false;
	}

	return true;
}

//Load Rectangle
bool LoadRect(SDL_Rect &rect, rapidxml::xml_node<char> *node, const bool &echo = true,
	const std::string &x_name = "x", const std::string &y_name = "y", const std::string &w_name = "w", const std::string &h_name = "h");

//Load Color
bool LoadColor(SDL_Color &col, rapidxml::xml_node<char> *node, const bool &echo = true,
	const std::string &r_name = "r", const std::string &g_name = "g", const std::string &b_name = "b");

//Shortcut to load integer color index to a number
bool LoadColor(int &col, rapidxml::xml_node<char> *node, const bool &echo = true);

//Load two dimensional coordinates
template <typename T>
bool LoadXY(T &x, T &y, rapidxml::xml_node<char> *node, const bool &echo = true)
{
	bool result = LoadNum(x, "x", node, echo);
	result = LoadNum(y, "y", node, echo) && result;

	return result;
}

//Load three dimensional coordinates
template <typename T>
bool LoadXYZ(T &x, T &y, T &z, rapidxml::xml_node<char> *node, const bool &echo = true)
{
	bool result = LoadNum(x, "x", node, echo);
	result = LoadNum(y, "y", node, echo) && result;
	result = LoadNum(z, "z", node, echo) && result;

	return result;
}

//Load Boolean variable
bool LoadBool(bool &var, const std::string &name, rapidxml::xml_node<char> *node, const bool &echo = true);

//Write Boolean variable to file
void SaveBool(const bool &var, const char *name, rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);

//Functions to load various type of objects
bool LoadStatType(pyrodactyl::stat::StatType &type, rapidxml::xml_node<char> *node, const bool &echo = true);

bool LoadAlign(Align &align, rapidxml::xml_node<char> *node, const bool &echo = true, const std::string &name = "align");
bool LoadDirection(Direction &dir, rapidxml::xml_node<char> *node, const bool &echo = true, const std::string &name = "dir");

bool LoadTextureFlipType(TextureFlipType &flip, rapidxml::xml_node<char> *node, const bool &echo = true);

//Check the version of a file
unsigned int Version(const std::string &filename);