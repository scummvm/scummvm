//=============================================================================
// Author:   Arvind
// Purpose:  A basic screen that flashes an image for a time interval
//=============================================================================
#pragma once

#include "ImageManager.h"
#include "LoadingScreen.h"
#include "MusicManager.h"
#include "ScreenSettings.h"
#include "TextManager.h"
#include "common_header.h"
#include "gamestate_container.h"
#include "gamestates.h"
#include "timer.h"

//------------------------------------------------------------------------
// Purpose: Splash screen class
//------------------------------------------------------------------------
class Splash : public GameState {
	pyrodactyl::image::Image background;

	int x, y;
	bool first_run, load_complete;

public:
	Splash();
	~Splash();
	void HandleEvents(SDL_Event &Event, bool &ShouldChangeState, GameStateID &NewStateID) {}
	void InternalEvents(bool &ShouldChangeState, GameStateID &NewStateID);
	void Draw();
	void SetUI();

	// We don't need to save game state here
	void AutoSave() {}
};