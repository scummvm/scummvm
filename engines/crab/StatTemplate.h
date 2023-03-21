#pragma once
#include "common_header.h"

#include "Stat.h"

namespace pyrodactyl
{
	namespace stat
	{
		struct StatTemplates
		{
			std::vector<StatGroup> collection;

		public:
			StatTemplates(void){}
			~StatTemplates(void){}

			void Load(const std::string &filename);
		};
	}
}