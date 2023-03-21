#pragma once

#include "common_header.h"
#include "gamestates.h"
#include "gamestate_container.h"
#include "menu.h"
#include "cursor.h"
#include "slider.h"
#include "textarea.h"
#include "ModMenu.h"
#include "FileMenu.h"
#include "SlideShow.h"
#include "ImageData.h"
#include "OptionMenu.h"
#include "AlphaImage.h"
#include "KeyBindMenu.h"
#include "StateButton.h"
#include "CreditScreen.h"

//#define UNREST_DEMO

//------------------------------------------------------------------------
// Purpose: Main Menu class
//------------------------------------------------------------------------
class MainMenu : public GameState
{
	enum MenuState
	{
		STATE_NORMAL,
		STATE_OPTIONS,
		STATE_CREDITS,
		STATE_LOAD,
		STATE_DIFF,
		STATE_SAVENAME,
		STATE_MOD,
		STATE_HELP
	} state;

	//This image covers the whole screen and is drawn centered on screen
	pyrodactyl::ui::ImageData bg;

	//The lights on the background image
	std::vector<pyrodactyl::ui::AlphaImage> lights;

	//The game logo
	pyrodactyl::ui::ImageData logo;

	//Data for the difficulty menu
	struct
	{
		pyrodactyl::ui::ImageData bg;
		pyrodactyl::ui::HoverInfo heading;
		pyrodactyl::ui::ButtonMenu menu;
	} diff;

	//Main menu
	pyrodactyl::ui::Menu<pyrodactyl::ui::StateButton> me_main;

	//The back button is common and is used for all menus here (except credits and main menu)
	pyrodactyl::ui::Button back;

	//Mod menu
	pyrodactyl::ui::ModMenu mod;

	//Game credits
	pyrodactyl::ui::CreditScreen credits;

	//The save game name prompt for iron man
	pyrodactyl::ui::TextArea save;

	//The background image and position of the prompt, along with the warning message displayed
	//and buttons for accept and cancel
	pyrodactyl::ui::ImageData bg_save;
	pyrodactyl::ui::HoverInfo warning;
	pyrodactyl::ui::Button accept, cancel;

	//Music for the main menu
	struct MainMenuMusic
	{
		pyrodactyl::music::MusicKey normal, credits;

		MainMenuMusic(){ normal = -1; credits = -1; }
	} music_key;

#ifdef UNREST_DEMO
	//UI elements related to the demo
	pyrodactyl::ui::Button steam, direct;
#endif

public:
	MainMenu();
	~MainMenu(){}
	void HandleEvents(SDL_Event& Event, bool& ShouldChangeState, GameStateID& NewStateID);
	void InternalEvents(bool& ShouldChangeState, GameStateID& NewStateID);

	void Draw();

	void ChangeState(MenuState ms, const bool &start = false);
	void SetUI();

	//We don't need to save game state here
	void AutoSave(){}
};