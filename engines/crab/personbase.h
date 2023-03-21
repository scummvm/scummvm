#pragma once

#include "common_header.h"
#include "Stat.h"

namespace pyrodactyl
{
	namespace people
	{
		//What each type is, and what it does
		//neutral - peaceful person who will fight back on being attacked
		//hostile - person who will attack you on sight
		//coward - person who will flee on being attacked
		//immobile - person who cannot move but can be killed
		enum PersonType { PE_NEUTRAL, PE_HOSTILE, PE_COWARD, PE_IMMOBILE };

		//What each state is, and what it does
		//normal - person doing his default movement
		//fight - person fighting you
		//flee - person running away from you
		//ko - person is dead/knocked out
		//dying - play the dying animation
		enum PersonState { PST_NORMAL, PST_FIGHT, PST_FLEE, PST_KO, PST_DYING };

		PersonType StringToPersonType(const std::string &val);
		PersonState StringToPersonState(const std::string &val);
	}
}