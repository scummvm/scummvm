#include "stdafx.h"
#include "color.h"

using namespace pyrodactyl::text;

void ColorPool :: Load(const std::string &filename)
{
	pool.clear();
	XMLDoc col_list(filename);
	if(col_list.ready())
	{
		rapidxml::xml_node<char> *node = col_list.Doc()->first_node("colors");
		for(rapidxml::xml_node<char> * n = node->first_node("color"); n != NULL; n = n->next_sibling("color"))
		{
			SDL_Color c;
			LoadColor(c, n);
			pool.push_back(c);
		}
	}
}

SDL_Color& ColorPool :: Get(const int &num)
{
	if(pool.size() <= static_cast<unsigned int>(num))
		return invalid;

	return pool.at(num);
}