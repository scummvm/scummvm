//=============================================================================
// Author:   Arvind
// Purpose:  The game state virtual class
//=============================================================================
#pragma once

#include "common_header.h"
#include "gamestates.h"

//------------------------------------------------------------------------
// Purpose: Game State base class
//------------------------------------------------------------------------
class GameState {
public:
	virtual void HandleEvents(SDL_Event &Event, bool &ShouldChangeState, GameStateID &NewStateID) = 0;
	virtual void InternalEvents(bool &ShouldChangeState, GameStateID &NewStateID) = 0;
	virtual void Draw() = 0;
	virtual void SetUI() = 0;
	virtual void AutoSave() = 0;
	virtual ~GameState(){};
};