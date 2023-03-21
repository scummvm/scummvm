//=============================================================================
// Author:   Arvind
// Purpose:  A basic screen that flashes an image for a time interval
//=============================================================================
#pragma once

#include "common_header.h"
#include "ScreenSettings.h"
#include "gamestates.h"
#include "gamestate_container.h"
#include "timer.h"
#include "ImageManager.h"
#include "MusicManager.h"
#include "TextManager.h"
#include "LoadingScreen.h"

//------------------------------------------------------------------------
// Purpose: Splash screen class
//------------------------------------------------------------------------
class Splash : public GameState
{
	pyrodactyl::image::Image background;

	int x, y;
	bool first_run, load_complete;

public:
	Splash();
	~Splash();
	void HandleEvents(SDL_Event& Event, bool& ShouldChangeState, GameStateID& NewStateID) {}
	void InternalEvents(bool& ShouldChangeState, GameStateID& NewStateID);
	void Draw();
	void SetUI();

	//We don't need to save game state here
	void AutoSave(){}
};