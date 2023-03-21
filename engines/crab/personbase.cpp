#include "stdafx.h"

#include "personbase.h"

using namespace pyrodactyl::stat;

namespace pyrodactyl
{
	namespace people
	{
		PersonType StringToPersonType(const std::string &val)
		{
			if (val == "neutral") return PE_NEUTRAL;
			else if (val == "hostile") return PE_HOSTILE;
			else if (val == "coward") return PE_COWARD;
			else if (val == "immobile") return PE_IMMOBILE;

			return PE_NEUTRAL;
		}

		PersonState StringToPersonState(const std::string &val)
		{
			if (val == "ko") return PST_KO;
			else if (val == "fight") return PST_FIGHT;
			else if (val == "flee") return PST_FLEE;
			else if (val == "dying") return PST_DYING;

			return PST_NORMAL;
		}
	}
}