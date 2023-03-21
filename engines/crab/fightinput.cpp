#include "stdafx.h"
#include "fightinput.h"

using namespace pyrodactyl::input;

void FightInput::Load(rapidxml::xml_node<char> * node)
{
	LoadEnum(type, "type", node);
	LoadNum(state, "state", node);
}

FightAnimationType FightInput::HandleEvents(const SDL_Event &Event)
{
	if (gInput.State(IG_ATTACK))
		return FA_ATTACK;
	else if (gInput.State(IG_BLOCK))
		return FA_BLOCK;

	return FA_IDLE;
}