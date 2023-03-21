#pragma once

#include "common_header.h"
#include "Rectangle.h"

namespace pyrodactyl
{
	namespace ui
	{
		struct MapData
		{
			//The paths of set of map images - background and the fully revealed map overlay
			std::string path_bg, path_overlay;

			//The places the player has revealed in this world map
			std::vector<Rect> reveal;

			struct MarkerData
			{
				//The name of the marker, same name as the quest
				std::string name;

				//Position of the marker
				Vector2i pos;
			};

			//The set of destinations currently active
			std::vector<MarkerData> dest;

			MapData(){}
			MapData(rapidxml::xml_node<char> *node){ Load(node); }
			~MapData(){}

			void Load(rapidxml::xml_node<char> *node);
			void DestAdd(const std::string &name, const int &x, const int &y);

			void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
			void LoadState(rapidxml::xml_node<char> *node);
		};
	}
}