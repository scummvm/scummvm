/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#include "crab/mainmenu.h"
#include "crab/url.h"

namespace Crab {

using namespace pyrodactyl::music;
using namespace pyrodactyl::image;
using namespace pyrodactyl::text;
using namespace pyrodactyl::input;
using namespace pyrodactyl::ui;

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
MainMenu::MainMenu() {
	XMLDoc conf(g_engine->_filePath->mainmenu_l);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("main_menu");
		if (nodeValid(node)) {
			me_main.load(node->first_node("main"));
			logo.load(node->first_node("logo"));

			back.load(node->first_node("back"));

			if (!g_engine->_optionMenu->loaded) {
				g_engine->_optionMenu->load(node->first_node("option")->first_attribute("path")->value());
				g_engine->_optionMenu->loaded = true;
			}

			mod.load(node->first_node("mod")->first_attribute("path")->value());
			credits.load(node->first_node("credits")->first_attribute("path")->value());

			{
				XMLDoc loadconf(node->first_node("load")->first_attribute("path")->value());
				if (loadconf.ready()) {
					rapidxml::xml_node<char> *loadnode = loadconf.doc()->first_node("load_menu");
					if (nodeValid(loadnode))
						g_engine->_loadMenu->load(loadnode);
				}
			}

			{
				XMLDoc helpconf(node->first_node("help")->first_attribute("path")->value());
				if (helpconf.ready()) {
					rapidxml::xml_node<char> *hnode = helpconf.doc()->first_node("help");
					if (nodeValid(hnode))
						g_engine->_helpScreen->load(hnode);
				}
			}

			if (nodeValid("scene", node)) {
				rapidxml::xml_node<char> *snode = node->first_node("scene");

				if (nodeValid("bg", snode))
					bg.load(snode->first_node("bg"));

				if (nodeValid("lights", snode)) {
					rapidxml::xml_node<char> *lnode = snode->first_node("lights");

					for (rapidxml::xml_node<char> *n = lnode->first_node("img"); n != NULL; n = n->next_sibling("img"))
						lights.push_back(n);
				}
			}

			if (nodeValid("difficulty", node)) {
				rapidxml::xml_node<char> *dinode = node->first_node("difficulty");

				if (nodeValid("bg", dinode))
					diff.bg.load(dinode->first_node("bg"));

				if (nodeValid("menu", dinode))
					diff.menu.load(dinode->first_node("menu"));

				if (nodeValid("heading", dinode))
					diff.heading.load(dinode->first_node("heading"));
			}

			if (nodeValid("prompt", node)) {
				rapidxml::xml_node<char> *prnode = node->first_node("prompt");

				save.load(prnode);

				if (nodeValid("warning", prnode))
					warning.load(prnode->first_node("warning"));

				if (nodeValid("bg", prnode))
					bg_save.load(prnode->first_node("bg"));

				if (nodeValid("accept", prnode))
					accept.load(prnode->first_node("accept"));

				if (nodeValid("cancel", prnode))
					cancel.load(prnode->first_node("cancel"));
			}

			if (nodeValid("music", node)) {
				loadNum(music_key.normal, "normal", node->first_node("music"));
				loadNum(music_key.credits, "credits", node->first_node("music"));
			}

#ifdef UNREST_DEMO
			if (nodeValid("demo", node)) {
				rapidxml::xml_node<char> *denode = node->first_node("demo");

				if (nodeValid("steam", denode))
					steam.load(denode->first_node("steam"));

				if (nodeValid("direct", denode))
					direct.load(denode->first_node("direct"));
			}
#endif
		}
	}

	if (g_engine->_tempData->credits)
		ChangeState(STATE_CREDITS, true);
	else
		ChangeState(STATE_NORMAL, true);

	g_engine->_tempData->credits = false;
	g_engine->_filePath->current_r = g_engine->_filePath->mainmenu_r;
}

//------------------------------------------------------------------------
// Purpose: Event/input handling Events
//------------------------------------------------------------------------
void MainMenu::HandleEvents(Common::Event &Event, bool &ShouldChangeState, GameStateID &NewStateID) {
	g_engine->_mouse->HandleEvents(Event);

	if (state != STATE_CREDITS) {
		int choice = me_main.HandleEvents(Event);
		if (choice >= 0) {
			for (unsigned i = 0; i < me_main.element.size(); ++i)
				me_main.element[i].State(i == (unsigned int)choice);

			switch (choice) {
			case 0:
				if (g_engine->_loadMenu->SelectNewestFile()) {
					ChangeState(STATE_NORMAL);
					ShouldChangeState = true;
					NewStateID = GAMESTATE_LOAD_GAME;
				}
				break;
			case 1:
				ChangeState(STATE_DIFF);
				break;
			case 2:
				ChangeState(STATE_LOAD);
				g_engine->_loadMenu->ScanDir();
				break;
			case 3:
				ChangeState(STATE_OPTIONS);
				break;
			case 4:
				ChangeState(STATE_MOD);
				break;
			case 5:
				ChangeState(STATE_HELP);
				break;
			case 6:
				ChangeState(STATE_CREDITS);
				credits.Reset();
				break;
			case 7:
				ShouldChangeState = true;
				NewStateID = GAMESTATE_EXIT;
				break;
			default:
				break;
			}
		}
	}

#if 0
	if (g_engine->_inputManager->Equals(IU_BACK, Event) == SDL_PRESSED || (back.HandleEvents(Event) && (state != STATE_SAVENAME && state != STATE_CREDITS))) {
		if (state == STATE_SAVENAME)
			ChangeState(STATE_DIFF);
		else if (state != STATE_NORMAL)
			ChangeState(STATE_NORMAL);
	}
#endif

	switch (state) {
#ifdef UNREST_DEMO
	case STATE_NORMAL:
		if (steam.HandleEvents(Event) == BUAC_LCLICK) {
			// Open steam in browser window
			OpenURL("https://store.steampowered.com/app/292400/");
		}

		if (direct.HandleEvents(Event) == BUAC_LCLICK) {
			// Open humble widget in browser window
			OpenURL("https://www.humblebundle.com/store/unrest/Udg6Ytd8Dfw");
		}
		break;
#endif
	case STATE_OPTIONS:
		if (g_engine->_optionMenu->HandleEvents(back, Event))
			ChangeState(STATE_NORMAL);
		break;

	case STATE_CREDITS:
		if (credits.HandleEvents(Event))
			ChangeState(STATE_NORMAL);
		break;

	case STATE_LOAD:
		if (!g_engine->loadGameDialog())
			ChangeState(STATE_NORMAL);
		else {
			ShouldChangeState = true;
			NewStateID = GAMESTATE_LOAD_GAME;
			return;
		}
		break;

	case STATE_DIFF: {
		int choice = diff.menu.HandleEvents(Event);

		// First menu option is Non-iron man, second is iron man
		// For the second choice, we must display a prompt to choose the name of the save game
		if (choice == 0) {
			g_engine->_tempData->ironman = false;
			ShouldChangeState = true;
			NewStateID = GAMESTATE_NEW_GAME;
		} else if (choice == 1)
			ChangeState(STATE_SAVENAME);
	} break;

	case STATE_SAVENAME:
		if (save.HandleEvents(Event) || accept.HandleEvents(Event)) {
			if (save.text != "") {
				g_engine->_tempData->filename = save.text;
				g_engine->_tempData->ironman = true;
				ShouldChangeState = true;
				NewStateID = GAMESTATE_NEW_GAME;
			} else
				debug("Please enter a valid filename for the iron man save.");
		} else if (cancel.HandleEvents(Event))
			ChangeState(STATE_DIFF);

		break;

	case STATE_MOD:
		if (mod.HandleEvents(Event))
			ChangeState(STATE_NORMAL);
		break;

	case STATE_HELP:
		g_engine->_helpScreen->HandleEvents(Event);
		break;

	default:
		break;
	}
}

#if 0
//------------------------------------------------------------------------
// Purpose: Event/input handling Events
//------------------------------------------------------------------------
void MainMenu::HandleEvents(SDL_Event &Event, bool &ShouldChangeState, GameStateID &NewStateID) {
	g_engine->_mouse->HandleEvents(Event);

	if (state != STATE_CREDITS) {
		int choice = me_main.HandleEvents(Event);
		if (choice >= 0) {
			for (unsigned i = 0; i < me_main.element.size(); ++i)
				me_main.element[i].State(i == choice);

			switch (choice) {
			case 0:
				if (g_engine->_loadMenu->SelectNewestFile()) {
					ChangeState(STATE_NORMAL);
					ShouldChangeState = true;
					NewStateID = GAMESTATE_LOAD_GAME;
				}
				break;
			case 1:
				ChangeState(STATE_DIFF);
				break;
			case 2:
				ChangeState(STATE_LOAD);
				g_engine->_loadMenu->ScanDir();
				break;
			case 3:
				ChangeState(STATE_OPTIONS);
				break;
			case 4:
				ChangeState(STATE_MOD);
				break;
			case 5:
				ChangeState(STATE_HELP);
				break;
			case 6:
				ChangeState(STATE_CREDITS);
				credits.Reset();
				break;
			case 7:
				ShouldChangeState = true;
				NewStateID = GAMESTATE_EXIT;
				break;
			default:
				break;
			}
		}
	}

	if (g_engine->_inputManager->Equals(IU_BACK, Event) == SDL_PRESSED || (back.HandleEvents(Event) && (state != STATE_SAVENAME && state != STATE_CREDITS))) {
		if (state == STATE_SAVENAME)
			ChangeState(STATE_DIFF);
		else if (state != STATE_NORMAL)
			ChangeState(STATE_NORMAL);
	}

	switch (state) {
#ifdef UNREST_DEMO
	case STATE_NORMAL:
		if (steam.HandleEvents(Event) == BUAC_LCLICK) {
			// Open steam in browser window
			OpenURL("https://store.steampowered.com/app/292400/");
		}

		if (direct.HandleEvents(Event) == BUAC_LCLICK) {
			// Open humble widget in browser window
			OpenURL("https://www.humblebundle.com/store/unrest/Udg6Ytd8Dfw");
		}
		break;
#endif
	case STATE_OPTIONS:
		if (g_engine->_optionMenu->HandleEvents(back, Event))
			ChangeState(STATE_NORMAL);
		break;

	case STATE_CREDITS:
		if (credits.HandleEvents(Event))
			ChangeState(STATE_NORMAL);
		break;

	case STATE_LOAD:
		if (g_engine->_loadMenu->HandleEvents(Event)) {
			ChangeState(STATE_NORMAL);
			ShouldChangeState = true;
			NewStateID = GAMESTATE_LOAD_GAME;
			return;
		}
		break;

	case STATE_DIFF: {
		int choice = diff.menu.HandleEvents(Event);

		// First menu option is Non-iron man, second is iron man
		// For the second choice, we must display a prompt to choose the name of the save game
		if (choice == 0) {
			g_engine->_tempData->ironman = false;
			ShouldChangeState = true;
			NewStateID = GAMESTATE_NEW_GAME;
		} else if (choice == 1)
			ChangeState(STATE_SAVENAME);
	} break;

	case STATE_SAVENAME:
		if (save.HandleEvents(Event) || accept.HandleEvents(Event)) {
			if (save.text != "") {
				g_engine->_tempData->filename = save.text;
				g_engine->_tempData->ironman = true;
				ShouldChangeState = true;
				NewStateID = GAMESTATE_NEW_GAME;
			} else
				fprintf(stdout, "Please enter a valid filename for the iron man save.\n");
		} else if (cancel.HandleEvents(Event))
			ChangeState(STATE_DIFF);

		break;

	case STATE_MOD:
		if (mod.HandleEvents(Event))
			ChangeState(STATE_NORMAL);
		break;

	case STATE_HELP:
		g_engine->_helpScreen->HandleEvents(Event);
		break;

	default:
		break;
	}
}
#endif
//------------------------------------------------------------------------
// Purpose: Internal Events
//------------------------------------------------------------------------
void MainMenu::internalEvents(bool &ShouldChangeState, GameStateID &NewStateID) {
	// Make the lights flicker
	for (auto &i : lights)
		i.internalEvents();

	if (state == STATE_OPTIONS)
		g_engine->_optionMenu->internalEvents();
}

//------------------------------------------------------------------------
// Purpose: We need to toggle button visibility
// and enable/disable keyboard for each state change
//------------------------------------------------------------------------
void MainMenu::ChangeState(MenuState ms, const bool &start) {
	// Start = true means this is the first run, and no music is playing
	// Just play the appropriate music
	if (start) {
		if (ms == STATE_CREDITS)
			g_engine->_musicManager->PlayMusic(music_key.credits);
		else
			g_engine->_musicManager->PlayMusic(music_key.normal);
	} else {
		// This is not our first run, which means some music track is already playing
		// Only change tracks when going from main menu->credits or credits->main menu
		if (state == STATE_CREDITS && ms != STATE_CREDITS)
			g_engine->_musicManager->PlayMusic(music_key.normal);
		else if (state != STATE_CREDITS && ms == STATE_CREDITS)
			g_engine->_musicManager->PlayMusic(music_key.credits);
	}

	// Set current state
	state = ms;

	// We are entering the normal state, i.e outside all menus - reset color of menu items
	if (state == STATE_NORMAL) {
		for (unsigned i = 0; i < me_main.element.size(); ++i)
			me_main.element[i].State(false);
	}

	// Enable keyboard navigation if outside all menus, otherwise disable it
	me_main.UseKeyboard((state == STATE_NORMAL));

	// Continue button is only enabled if there is a save to load
	me_main.element[0].visible = !g_engine->_loadMenu->Empty();

	// Enable credits and quit button if outside all menus, otherwise disable it
	me_main.element[6].visible = (state == STATE_NORMAL);
	me_main.element[7].visible = (state == STATE_NORMAL);

	// If switching to help screen, load latest image otherwise remove it from memory
	if (state == STATE_HELP)
		g_engine->_helpScreen->Refresh();
	else
		g_engine->_helpScreen->Clear();
}

//------------------------------------------------------------------------
// Purpose: Drawing function
//------------------------------------------------------------------------
void MainMenu::draw() {
	bg.draw();

	for (auto &i : lights)
		i.draw();

	switch (state) {
	case STATE_NORMAL:
		// Draw the logo
		logo.draw();

		// Draw the game name and mod path if a mod is loaded
		if (g_engine->_filePath->mod_cur != "res/default.xml") {
			g_engine->_textManager->draw(g_engine->_screenSettings->cur.w / 2, g_engine->_screenSettings->cur.h / 2, "Unrest", 0, 0, ALIGN_CENTER);
			//g_engine->_textManager->draw(g_engine->_screenSettings->cur.w / 2, g_engine->_screenSettings->cur.h / 2 + 50, g_engine->_filePath->mod_cur, 5, 1, ALIGN_CENTER);
		}

		me_main.draw();

#ifdef UNREST_DEMO
		g_engine->_textManager->draw(logo.x + logo.w, logo.y + logo.h / 2, "Demo", 0, 0, ALIGN_CENTER);
		steam.draw();
		direct.draw();
#endif
		break;

	case STATE_OPTIONS:
		g_engine->_optionMenu->draw(back);
		me_main.draw();
		break;

	case STATE_CREDITS:
		credits.draw();
		break;

	case STATE_DIFF:
		diff.bg.draw();
		diff.heading.draw();
		diff.menu.draw();
		back.draw();
		me_main.draw();
		break;

	case STATE_SAVENAME:
		bg_save.draw();
		warning.draw();
		save.draw();
		accept.draw();
		cancel.draw();
		me_main.draw();
		break;

	case STATE_MOD:
		mod.draw();
		back.draw();
		me_main.draw();
		break;

	case STATE_HELP:
		g_engine->_helpScreen->draw();
		back.draw();
		me_main.draw();
		break;

	default:
		break;
	}

	g_engine->_mouse->draw();
}

void MainMenu::SetUI() {
	me_main.SetUI();
	logo.SetUI();

	back.SetUI();
	g_engine->_optionMenu->SetUI();
	mod.SetUI();
	g_engine->_loadMenu->SetUI();
	g_engine->_helpScreen->SetUI();
	credits.SetUI();

	accept.SetUI();
	cancel.SetUI();

	bg.SetUI();
	for (auto &i : lights)
		i.SetUI();

	diff.menu.SetUI();
	diff.bg.SetUI();
	diff.heading.SetUI();

	save.SetUI();
	warning.SetUI();
	bg_save.SetUI();

#ifdef UNREST_DEMO
	steam.SetUI();
	direct.SetUI();
#endif
}

} // End of namespace Crab
