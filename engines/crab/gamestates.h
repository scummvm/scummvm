#pragma once
#include "common_header.h"

//------------------------------------------------------------------------
// Purpose: Game State Enumerators
//------------------------------------------------------------------------
enum GameStateID
{
	GAMESTATE_NULL = -2,
	GAMESTATE_TITLE = -1,
	GAMESTATE_EXIT = -3,
	GAMESTATE_MAIN_MENU = 0,
	GAMESTATE_NEW_GAME,
	GAMESTATE_LOAD_GAME,
	TOTAL_GAMESTATES //ALWAYS THE LAST VALUE
};