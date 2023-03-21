#include "stdafx.h"

//=============================================================================
// Author:   Arvind
// Purpose:  Splash functions
//=============================================================================
#include "splash.h"

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
Splash::Splash()
{
	//Load the background
	background.Load("res/gfx/pyrodactyl.png");

	SetUI();
	load_complete = false;
	first_run = true;
}

//------------------------------------------------------------------------
// Purpose: Destructor
//------------------------------------------------------------------------
Splash :: ~Splash()
{
	background.Delete();
}

//------------------------------------------------------------------------
// Purpose: Event/Input Independent InternalEvents
//------------------------------------------------------------------------
void Splash::InternalEvents(bool& ShouldChangeState, GameStateID& NewStateID)
{
	if (first_run == false)
	{
		gLoadScreen.Load();
		pyrodactyl::image::gImageManager.Init();
		pyrodactyl::text::gTextManager.Init();
		load_complete = true;
	}

	//Have we loaded everything? If yes, time to exit
	if (load_complete)
	{
		ShouldChangeState = true;
		NewStateID = GAMESTATE_MAIN_MENU;
		return;
	}
}

//------------------------------------------------------------------------
// Purpose: Drawing function
//------------------------------------------------------------------------
void Splash::Draw()
{
	background.Draw(x, y);
	first_run = false;
}

//------------------------------------------------------------------------
// Purpose: Reset UI position
//------------------------------------------------------------------------
void Splash::SetUI()
{
	x = (gScreenSettings.cur.w - background.W()) / 2;
	y = (gScreenSettings.cur.h - background.H()) / 2;
}