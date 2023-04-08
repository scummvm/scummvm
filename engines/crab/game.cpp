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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "crab/game.h"

namespace Crab {

using namespace pyrodactyl::image;
using namespace pyrodactyl::ui;
using namespace pyrodactyl::input;

//------------------------------------------------------------------------
// Purpose: Loading stuff
//------------------------------------------------------------------------
void Game::StartNewGame() {
	Init(gFilePath.mod_cur.c_str());
	LoadLevel(info.CurLocID());
	info.IronMan(gTemp.ironman);
	savefile.ironman = gTemp.filename.c_str();
	clock.Start();
	hud.pause.UpdateMode(info.IronMan());

	CreateSaveGame(SAVEGAME_EVENT);
}

void Game::LoadGame(const std::string &filename) {
	Init(gFilePath.mod_cur.c_str());
	LoadState(filename);
}

void Game::Init(const std::string &filename) {
	gLoadScreen.Dim();
	pyrodactyl::event::gEventStore.Clear();
	game_over.Clear(false);
	state = STATE_GAME;
	savefile.auto_slot = false;
	gem.Init();
	info.Init();

	XMLDoc conf(filename.c_str());
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("config");

		info.Load(node);

		std::string path;
		if (NodeValid("level", node)) {
			LoadStr(path, "list", node->first_node("level"));
			gFilePath.LoadLevel(path.c_str());
		}

		if (NodeValid("hud", node)) {
			LoadStr(path, "layout", node->first_node("hud"));
			hud.Load(path, level.talk_notify, level.dest_marker);
		}

		if (NodeValid("sprite", node)) {
			LoadStr(path, "animation", node->first_node("sprite"));
			level.LoadMoves(path);

			LoadStr(path, "constant", node->first_node("sprite"));
			level.LoadConst(path);
		}

		if (NodeValid("event", node)) {
			gem.Load(node->first_node("event"), pop_default);

			LoadStr(path, "store", node->first_node("event"));
			pyrodactyl::event::gEventStore.Load(path);
		}

		if (NodeValid("map", node)) {
			LoadStr(path, "layout", node->first_node("map"));
			map.Load(path, info);
		}

		if (NodeValid("save", node))
			savefile.Load(node->first_node("save"));

		if (NodeValid("debug", node)) {
			LoadStr(path, "layout", node->first_node("debug"));
			debug_console.Load(path);
		}
	}
}

bool Game::LoadLevel(const std::string &id, int player_x, int player_y) {
	if (gFilePath.level.contains(id.c_str())) {
		gLoadScreen.Draw();

		// Load the assets local to this level
		// If the filename is same as the previous one, skip loading
		if (gFilePath.current_r != gFilePath.level[id.c_str()].asset) {
			gFilePath.current_r = gFilePath.level[id.c_str()].asset;
			gImageManager.LoadMap(gFilePath.level[id.c_str()].asset);
		}

		// Load the level itself
		level.pop = pop_default;
		level.Load(gFilePath.level[id.c_str()].layout.c_str(), info, game_over, player_x, player_y);

		// Set the current location
		info.CurLocID(id);
		info.CurLocName(gFilePath.level[id.c_str()].name.c_str());
		map.player_pos = level.map_loc;

		// Update and center the world map to the player current position
		map.Update(info);
		map.Center(map.player_pos);

		// If this is our first time visiting a level, reveal the associated area on the world map
		map.RevealAdd(level.map_clip.id, level.map_clip.rect);

		// Initialize inventory
		info.inv.Init(level.PlayerID());

		// Initialize journal
		info.journal.Init(level.PlayerID());
		return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void Game::HandleEvents(Common::Event &Event, bool &ShouldChangeState, GameStateID &NewStateID) {
	gMouse.HandleEvents(Event);

//	if (GameDebug)
//		debug_console.HandleEvents(Event);

	if (!debug_console.RestrictInput()) {
		if (state == STATE_LOSE_MENU) {
			switch (hud.gom.HandleEvents(Event)) {
			case 0:
				state = STATE_LOSE_LOAD;
				break;
			case 1:
				Quit(ShouldChangeState, NewStateID, GAMESTATE_MAIN_MENU);
				break;
			default:
				break;
			}
		} else if (state == STATE_LOSE_LOAD) {
			if (gLoadMenu.HandleEvents(Event)) {
				ShouldChangeState = true;
				NewStateID = GAMESTATE_LOAD_GAME;
				return;
			}

			if (hud.pausekey.HandleEvents(Event) || hud.back.HandleEvents(Event) == BUAC_LCLICK)
				state = STATE_LOSE_MENU;
		} else {
			if (!gem.EventInProgress() && !hud.pause.DisableHotkeys()) {
				switch (hud.HandleEvents(info, Event)) {
				case HS_MAP:
					ToggleState(STATE_MAP);
					break;
				case HS_PAUSE:
					ToggleState(STATE_PAUSE);
					break;
				case HS_CHAR:
					ToggleState(STATE_CHARACTER);
					gem.per.Cache(info, level.PlayerID(), level);
					break;
				case HS_JOURNAL:
					ToggleState(STATE_JOURNAL);
					break;
				case HS_INV:
					ToggleState(STATE_INVENTORY);
					break;
				default:
					break;
				}
			}

			if (state == STATE_GAME) {
				if (gem.EventInProgress()) {
					gem.HandleEvents(info, level.PlayerID(), Event, hud, level, event_res);
					if (ApplyResult())
						Quit(ShouldChangeState, NewStateID, GAMESTATE_MAIN_MENU);
				} else {
					// Update the talk key state
					info.TalkKeyDown = gInput.State(IG_TALK) || level.ContainsClick(info.LastPerson(), Event);

					level.HandleEvents(info, Event);

					if (!game_over.Empty() && game_over.Evaluate(info)) {
						state = STATE_LOSE_MENU;
						hud.gom.Reset();
						return;
					}

#if 0
					if (gInput.Equals(IG_QUICKSAVE, Event) == SDL_RELEASED) {
						CreateSaveGame(SAVEGAME_QUICK);
						return;
					} else if (gInput.Equals(IG_QUICKLOAD, Event) == SDL_RELEASED && !info.IronMan()) {
						ShouldChangeState = true;
						NewStateID = GAMESTATE_LOAD_GAME;
						gLoadMenu.SelectedPath(FullPath(savefile.quick));
						return;
					}
#endif

					if (hud.pausekey.HandleEvents(Event))
						ToggleState(STATE_PAUSE);
				}
			} else if (state == STATE_PAUSE) {
				switch (hud.pause.HandleEvents(Event, hud.back)) {
				case PS_RESUME:
					ToggleState(STATE_GAME);
					hud.SetTooltip();
					break;

				case PS_SAVE:
					CreateSaveGame(SAVEGAME_NORMAL);
					ToggleState(STATE_GAME);
					hud.SetTooltip();
					break;
				case PS_LOAD:
					ShouldChangeState = true;
					NewStateID = GAMESTATE_LOAD_GAME;
					return;

				case PS_HELP:
					ToggleState(STATE_HELP);
					break;

				case PS_QUIT_MENU:
					CreateSaveGame(SAVEGAME_EXIT);
					Quit(ShouldChangeState, NewStateID, GAMESTATE_MAIN_MENU);
					break;

				case PS_QUIT_GAME:
					CreateSaveGame(SAVEGAME_EXIT);
					Quit(ShouldChangeState, NewStateID, GAMESTATE_EXIT);
					break;
				default:
					break;
				}
			} else {
				if (hud.back.HandleEvents(Event) == BUAC_LCLICK)
					ToggleState(STATE_GAME);

				switch (state) {
				case STATE_MAP:
					if (map.HandleEvents(info, Event)) {
						// We need to load the new level
						LoadLevel(map.cur_loc);
						ToggleState(STATE_GAME);
					}
					break;
				case STATE_JOURNAL:
					if (info.journal.HandleEvents(level.PlayerID(), Event)) {
						// This means we selected the "find on map" button, so we need to:
						// switch to the world map, and highlight the appropriate quest marker
						map.SelectDest(info.journal.marker_title);
						ToggleState(STATE_MAP);
					}
					break;
				case STATE_CHARACTER:
					gem.per.HandleEvents(info, level.PlayerID(), Event);
					break;
				case STATE_INVENTORY:
					info.inv.HandleEvents(level.PlayerID(), Event);
					break;
				case STATE_HELP:
					gHelpScreen.HandleEvents(Event);
				default:
					break;
				}
			}
		}
	}
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void Game::HandleEvents(SDL_Event &Event, bool &ShouldChangeState, GameStateID &NewStateID) {
	gMouse.HandleEvents(Event);

	if (GameDebug)
		debug_console.HandleEvents(Event);

	if (!debug_console.RestrictInput()) {
		if (state == STATE_LOSE_MENU) {
			switch (hud.gom.HandleEvents(Event)) {
			case 0:
				state = STATE_LOSE_LOAD;
				break;
			case 1:
				Quit(ShouldChangeState, NewStateID, GAMESTATE_MAIN_MENU);
				break;
			default:
				break;
			}
		} else if (state == STATE_LOSE_LOAD) {
			if (gLoadMenu.HandleEvents(Event)) {
				ShouldChangeState = true;
				NewStateID = GAMESTATE_LOAD_GAME;
				return;
			}

			if (hud.pausekey.HandleEvents(Event) || hud.back.HandleEvents(Event) == BUAC_LCLICK)
				state = STATE_LOSE_MENU;
		} else {
			if (!gem.EventInProgress() && !hud.pause.DisableHotkeys()) {
				switch (hud.HandleEvents(info, Event)) {
				case HS_MAP:
					ToggleState(STATE_MAP);
					break;
				case HS_PAUSE:
					ToggleState(STATE_PAUSE);
					break;
				case HS_CHAR:
					ToggleState(STATE_CHARACTER);
					gem.per.Cache(info, level.PlayerID(), level);
					break;
				case HS_JOURNAL:
					ToggleState(STATE_JOURNAL);
					break;
				case HS_INV:
					ToggleState(STATE_INVENTORY);
					break;
				default:
					break;
				}
			}

			if (state == STATE_GAME) {
				if (gem.EventInProgress()) {
					gem.HandleEvents(info, level.PlayerID(), Event, hud, level, event_res);
					if (ApplyResult())
						Quit(ShouldChangeState, NewStateID, GAMESTATE_MAIN_MENU);
				} else {
					// Update the talk key state
					info.TalkKeyDown = gInput.State(IG_TALK) || level.ContainsClick(info.LastPerson(), Event);

					level.HandleEvents(info, Event);

					if (!game_over.Empty() && game_over.Evaluate(info)) {
						state = STATE_LOSE_MENU;
						hud.gom.Reset();
						return;
					}

					if (gInput.Equals(IG_QUICKSAVE, Event) == SDL_RELEASED) {
						CreateSaveGame(SAVEGAME_QUICK);
						return;
					} else if (gInput.Equals(IG_QUICKLOAD, Event) == SDL_RELEASED && !info.IronMan()) {
						ShouldChangeState = true;
						NewStateID = GAMESTATE_LOAD_GAME;
						gLoadMenu.SelectedPath(FullPath(savefile.quick));
						return;
					}

					if (hud.pausekey.HandleEvents(Event))
						ToggleState(STATE_PAUSE);
				}
			} else if (state == STATE_PAUSE) {
				switch (hud.pause.HandleEvents(Event, hud.back)) {
				case PS_RESUME:
					ToggleState(STATE_GAME);
					hud.SetTooltip();
					break;

				case PS_SAVE:
					CreateSaveGame(SAVEGAME_NORMAL);
					ToggleState(STATE_GAME);
					hud.SetTooltip();
					break;
				case PS_LOAD:
					ShouldChangeState = true;
					NewStateID = GAMESTATE_LOAD_GAME;
					return;

				case PS_HELP:
					ToggleState(STATE_HELP);
					break;

				case PS_QUIT_MENU:
					CreateSaveGame(SAVEGAME_EXIT);
					Quit(ShouldChangeState, NewStateID, GAMESTATE_MAIN_MENU);
					break;

				case PS_QUIT_GAME:
					CreateSaveGame(SAVEGAME_EXIT);
					Quit(ShouldChangeState, NewStateID, GAMESTATE_EXIT);
					break;
				default:
					break;
				}
			} else {
				if (hud.back.HandleEvents(Event) == BUAC_LCLICK)
					ToggleState(STATE_GAME);

				switch (state) {
				case STATE_MAP:
					if (map.HandleEvents(info, Event)) {
						// We need to load the new level
						LoadLevel(map.cur_loc);
						ToggleState(STATE_GAME);
					}
					break;
				case STATE_JOURNAL:
					if (info.journal.HandleEvents(level.PlayerID(), Event)) {
						// This means we selected the "find on map" button, so we need to:
						// switch to the world map, and highlight the appropriate quest marker
						map.SelectDest(info.journal.marker_title);
						ToggleState(STATE_MAP);
					}
					break;
				case STATE_CHARACTER:
					gem.per.HandleEvents(info, level.PlayerID(), Event);
					break;
				case STATE_INVENTORY:
					info.inv.HandleEvents(level.PlayerID(), Event);
					break;
				case STATE_HELP:
					gHelpScreen.HandleEvents(Event);
				default:
					break;
				}
			}
		}
	}
}
#endif

//------------------------------------------------------------------------
// Purpose: InternalEvents
//------------------------------------------------------------------------
void Game::InternalEvents(bool &ShouldChangeState, GameStateID &NewStateID) {
	switch (state) {
	case STATE_GAME:
		hud.InternalEvents(level.ShowMap());
		event_res.clear();

		{
			// HACK: Since sequences can only be ended in GameEventManager, we use this empty array
			// to get effects to work for levels
			std::vector<pyrodactyl::event::EventSeqInfo> end_seq;
			ApplyResult(level.InternalEvents(info, event_res, end_seq, gem.EventInProgress()));
		}

		gem.InternalEvents(info, level, event_res);
		info.TalkKeyDown = false;

		if (ApplyResult())
			Quit(ShouldChangeState, NewStateID, GAMESTATE_MAIN_MENU);

		break;
	case STATE_MAP:
		map.InternalEvents(info);
		break;
	case STATE_CHARACTER:
		gem.per.InternalEvents();
		break;
	default:
		break;
	}
}
//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Game::Draw() {
	if (gem.draw_game)
		level.Draw(info);
	else
		gImageManager.BlackScreen();
	switch (state) {
	case STATE_GAME:
		if (gem.EventInProgress())
			gem.Draw(info, hud, level);
		else
			hud.Draw(info, level.PlayerID());
		break;
	case STATE_PAUSE:
		gImageManager.DimScreen();
		hud.pause.Draw(hud.back);
		hud.Draw(info, level.PlayerID());
		break;
	case STATE_MAP:
		gImageManager.DimScreen();
		map.Draw(info);
		hud.Draw(info, level.PlayerID());
		hud.back.Draw();
		break;
	case STATE_JOURNAL:
		gImageManager.DimScreen();
		info.journal.Draw(level.PlayerID());
		hud.Draw(info, level.PlayerID());
		hud.back.Draw();
		break;
	case STATE_CHARACTER:
		gImageManager.DimScreen();
		gem.per.Draw(info, level.PlayerID());
		hud.Draw(info, level.PlayerID());
		hud.back.Draw();
		break;
	case STATE_INVENTORY:
		gImageManager.DimScreen();
		info.InvDraw(level.PlayerID());
		hud.Draw(info, level.PlayerID());
		hud.back.Draw();
		break;
	case STATE_HELP:
		gImageManager.DimScreen();
		gHelpScreen.Draw();
		hud.back.Draw();
		hud.Draw(info, level.PlayerID());
		break;
	case STATE_LOSE_MENU:
		hud.gom.Draw();
		break;
	case STATE_LOSE_LOAD:
		gLoadMenu.Draw();
		hud.back.Draw();
		break;
	default:
		break;
	}

	if (GameDebug)
		debug_console.Draw(info);
	gMouse.Draw();
}

//------------------------------------------------------------------------
// Purpose: Apply results of events and levels
//------------------------------------------------------------------------
bool Game::ApplyResult() {
	using namespace pyrodactyl::event;

	for (auto i = event_res.begin(); i != event_res.end(); ++i) {
		switch (i->type) {
		case ER_MAP:
			if (i->val == "img")
				map.SetImage(i->y);
			else if (i->val == "pos") {
				map.player_pos.x = i->x;
				map.player_pos.y = i->y;
			}
			break;
		case ER_DEST:
			if (i->x < 0 || i->y < 0) {
				info.journal.Marker(level.PlayerID(), i->val, false);
				map.DestDel(i->val);
			} else {
				map.DestAdd(i->val, i->x, i->y);
				info.journal.Marker(level.PlayerID(), i->val, true);
				info.unread.map = true;
			}
			break;
		case ER_IMG:
			PlayerImg();
			break;
		case ER_TRAIT:
			if (i->x == 42)
				info.TraitDel(i->val, i->y);
			else
				info.TraitAdd(i->val, i->y);
			break;
		case ER_LEVEL:
			if (i->val == "Map")
				ToggleState(STATE_MAP);
			else
				LoadLevel(i->val, i->x, i->y);
			break;
		case ER_MOVE:
			for (auto &o : level.objects) {
				if (i->val == o.ID()) {
					o.X(i->x);
					o.Y(i->y);
					break;
				}
			}
			break;
		case ER_PLAYER:
			// First stop the movement of the current player sprite
			level.PlayerStop();

			// Then swap to the new id
			level.PlayerID(i->val, i->x, i->y);

			// Stop the new player sprite's movement as well
			level.PlayerStop();
			break;
		case ER_SAVE:
			CreateSaveGame(SAVEGAME_EVENT);
			break;
		case ER_SYNC:
			level.CalcProperties(info);
			map.Update(info);
			break;
		case ER_QUIT:
			gTemp.credits = (i->val == "credits");
			return true;
		default:
			break;
		}
	}

	gem.per.Cache(info, level.PlayerID(), level);
	event_res.clear();
	return false;
}

void Game::ApplyResult(LevelResult result) {
	switch (result.type) {
	case LR_LEVEL:
		if (result.val == "Map")
			ToggleState(STATE_MAP);
		else
			LoadLevel(result.val, result.x, result.y);
		return;
	case LR_GAMEOVER:
		state = STATE_LOSE_MENU;
		hud.gom.Reset();
		break;
	default:
		break;
	}
}

//------------------------------------------------------------------------
// Purpose: Save/load game
//------------------------------------------------------------------------
void Game::LoadState(const std::string &filename) {
	XMLDoc conf(filename.c_str());
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("save");
		if (NodeValid(node)) {
			info.LoadIronMan(node);
			LoadStr(savefile.ironman, "file", node);
			hud.pause.UpdateMode(info.IronMan());

			if (NodeValid("events", node))
				gem.LoadState(node->first_node("events"));

			if (NodeValid("info", node))
				info.LoadState(node->first_node("info"));

			if (NodeValid("map", node))
				map.LoadState(node->first_node("map"));

			PlayerImg();

			std::string loc;
			LoadStr(loc, "loc_id", node);
			LoadLevel(loc);

			if (NodeValid("level", node))
				level.LoadState(node->first_node("level"));

			gem.per.Cache(info, level.PlayerID(), level);

			std::string playtime;
			LoadStr(playtime, "time", node);
			clock.Start(playtime);
		}
	}
}
//------------------------------------------------------------------------
// Purpose: Write game state to file
//------------------------------------------------------------------------
void Game::SaveState(const std::string &filename, const bool &overwrite) {
	warning("STUB: Game::SaveState()");

#if 0
	rapidxml::xml_document<char> doc;

	// xml declaration
	rapidxml::xml_node<char> *decl = doc.allocate_node(rapidxml::node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	// root node
	rapidxml::xml_node<char> *root = doc.allocate_node(rapidxml::node_element, "save");
	doc.append_node(root);

	// Save location id
	std::string loc = info.CurLocID();
	root->append_attribute(doc.allocate_attribute("loc_id", loc.c_str()));

	// Save location name
	std::string loc_name = info.CurLocName();
	root->append_attribute(doc.allocate_attribute("loc_name", loc_name.c_str()));

	// Save player character name
	std::string char_name;
	if (info.PersonValid(level.PlayerID()))
		char_name = info.PersonGet(level.PlayerID()).name;
	root->append_attribute(doc.allocate_attribute("char_name", char_name.c_str()));

	// Difficulty
	std::string diff = "Normal";
	if (info.IronMan())
		diff = "Iron Man";
	root->append_attribute(doc.allocate_attribute("diff", diff.c_str()));

	// Save file used if iron man
	root->append_attribute(doc.allocate_attribute("file", savefile.ironman.c_str()));

	// Preview image used
	root->append_attribute(doc.allocate_attribute("preview", level.preview_path.c_str()));

	// Time played
	std::string playtime = clock.GetTime();
	root->append_attribute(doc.allocate_attribute("time", playtime.c_str()));

	rapidxml::xml_node<char> *child_gem = doc.allocate_node(rapidxml::node_element, "events");
	gem.SaveState(doc, child_gem);
	root->append_node(child_gem);

	rapidxml::xml_node<char> *child_info = doc.allocate_node(rapidxml::node_element, "info");
	info.SaveState(doc, child_info);
	root->append_node(child_info);

	rapidxml::xml_node<char> *child_map = doc.allocate_node(rapidxml::node_element, "map");
	map.SaveState(doc, child_map);
	root->append_node(child_map);

	rapidxml::xml_node<char> *child_level = doc.allocate_node(rapidxml::node_element, "level");
	level.SaveState(doc, child_level);
	root->append_node(child_level);

	std::string xml_as_string;
	rapidxml::print(std::back_inserter(xml_as_string), doc);

	std::string fullpath = FullPath(filename);

	// We don't check for duplicates for auto-saves and iron man saves
	if (!overwrite) {
		// If a file of this name already exists, find appropriate filename
		if (boost::filesystem::exists(fullpath)) {
			// Copy the original filename, add a _1 at the end
			// Start from one because that's how humans count
			std::string result = filename + "_1";
			int count = 1;

			// Keep trying a filename until it no longer exists or we reach an insanely high number
			while (boost::filesystem::exists(FullPath(result)) && count < 1000) {
				++count;
				result = filename + "_" + NumberToString(count);
			}

			// Make result the new save file path
			fullpath = FullPath(result);
		}
	}

	std::ofstream save(fullpath.c_str(), std::ios::out);
	if (save.is_open()) {
		save << xml_as_string;
		save.close();
	}

	doc.clear();
	hud.pause.ScanDir();
#endif
}
//------------------------------------------------------------------------
// Purpose: Quit the game
//------------------------------------------------------------------------
void Game::Quit(bool &ShouldChangeState, GameStateID &NewStateID, const GameStateID &NewStateVal) {
	ShouldChangeState = true;
	NewStateID = NewStateVal;
	gImageManager.LoadMap(gFilePath.mainmenu_r);
}
//------------------------------------------------------------------------
// Purpose: Change our internal state
//------------------------------------------------------------------------
void Game::ToggleState(const State &s) {
	if (state != s)
		state = s;
	else
		state = STATE_GAME;

	// This is because game is the first state, the rest are in order
	hud.State(state - 1);
	hud.pause.Reset();

	// Only load help screen image if we have to
	if (state == STATE_HELP)
		pyrodactyl::ui::gHelpScreen.Refresh();
	else
		pyrodactyl::ui::gHelpScreen.Clear();
}

//------------------------------------------------------------------------
// Purpose: Use this function to actually save your games
//------------------------------------------------------------------------
void Game::CreateSaveGame(const SaveGameType &savetype) {
	// Disregard type in iron man mode, we only save to one file
	if (info.IronMan())
		SaveState(savefile.ironman, true);
	else {
		switch (savetype) {
		case SAVEGAME_NORMAL:
			SaveState(hud.pause.SaveFile(), false);
			break;

		case SAVEGAME_EVENT:
			if (savefile.auto_slot)
				SaveState(savefile.auto_2, true);
			else
				SaveState(savefile.auto_1, true);

			savefile.auto_slot = !savefile.auto_slot;
			break;

		case SAVEGAME_EXIT:
			SaveState(savefile.auto_quit, true);
			break;
		case SAVEGAME_QUICK:
			SaveState(savefile.quick, true);
			break;
		default:
			break;
		}
	}
}

void Game::SetUI() {
	map.SetUI();
	hud.SetUI();

	gLoadMenu.SetUI();
	gOptionMenu.SetUI();

	gem.SetUI();
	info.SetUI();
	level.SetUI();
}

} // End of namespace Crab
