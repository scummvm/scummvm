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

#include "crab/game.h"
#include "crab/backInserter.h"

namespace Crab {

using namespace pyrodactyl::image;
using namespace pyrodactyl::ui;
using namespace pyrodactyl::input;

//------------------------------------------------------------------------
// Purpose: Loading stuff
//------------------------------------------------------------------------
void Game::StartNewGame() {
	Init(g_engine->_filePath->mod_cur);
	LoadLevel(info.curLocID());
	info.ironMan(g_engine->_tempData->ironman);
	savefile.ironman = g_engine->_tempData->filename;
	clock.Start();
	hud._pause.updateMode(info.ironMan());

	CreateSaveGame(SAVEGAME_EVENT);
}

void Game::LoadGame() {
	Init(g_engine->_filePath->mod_cur);
}

void Game::Init(const Common::String &filename) {
	g_engine->_loadingScreen->Dim();
	g_engine->_eventStore->clear();
	game_over.clear(false);
	state = STATE_GAME;
	savefile.auto_slot = false;
	gem.init();
	info.Init();

	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("config");

		info.load(node);

		Common::String path;
		if (nodeValid("level", node)) {
			loadStr(path, "list", node->first_node("level"));
			g_engine->_filePath->LoadLevel(path);
		}

		if (nodeValid("hud", node)) {
			loadStr(path, "layout", node->first_node("hud"));
			hud.load(path, level._talkNotify, level._destMarker);
		}

		if (nodeValid("sprite", node)) {
			loadStr(path, "animation", node->first_node("sprite"));
			level.loadMoves(path);

			loadStr(path, "constant", node->first_node("sprite"));
			level.loadConst(path);
		}

		if (nodeValid("event", node)) {
			gem.load(node->first_node("event"), pop_default);

			loadStr(path, "store", node->first_node("event"));
			g_engine->_eventStore->load(path);
		}

		if (nodeValid("map", node)) {
			loadStr(path, "layout", node->first_node("map"));
			map.load(path, info);
		}

		if (nodeValid("save", node))
			savefile.load(node->first_node("save"));

		if (nodeValid("debug", node)) {
			loadStr(path, "layout", node->first_node("debug"));
			debug_console.load(path);
		}
	}

	_isInited = true;
}

bool Game::LoadLevel(const Common::String &id, int player_x, int player_y) {
	if (g_engine->_filePath->level.contains(id)) {
		g_engine->_loadingScreen->draw();

		// Load the assets local to this level
		// If the filename is same as the previous one, skip loading
		if (g_engine->_filePath->current_r != g_engine->_filePath->level[id].asset) {
			g_engine->_filePath->current_r = g_engine->_filePath->level[id].asset;
			g_engine->_imageManager->loadMap(g_engine->_filePath->level[id].asset);
		}

		// Load the level itself
		level._pop = pop_default;
		level.load(g_engine->_filePath->level[id].layout, info, game_over, player_x, player_y);

		// Set the current location
		info.curLocID(id);
		info.curLocName(g_engine->_filePath->level[id].name);
		map._playerPos = level._mapLoc;

		// Update and center the world map to the player current position
		map.update(info);
		map.center(map._playerPos);

		// If this is our first time visiting a level, reveal the associated area on the world map
		map.revealAdd(level._mapClip._id, level._mapClip._rect);

		// Initialize inventory
		info._inv.init(level.playerId());

		// Initialize journal
		info._journal.init(level.playerId());

		level.preDraw();
		return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void Game::handleEvents(Common::Event &Event, bool &ShouldChangeState, GameStateID &NewStateID) {
	g_engine->_mouse->handleEvents(Event);

	//	if (GameDebug)
	//		debug_console.handleEvents(Event);

	if (!debug_console.restrictInput()) {
		if (state == STATE_LOSE_MENU) {
			switch (hud._gom.handleEvents(Event)) {
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
			if (g_engine->_loadMenu->handleEvents(Event)) {
				ShouldChangeState = true;
				NewStateID = GAMESTATE_LOAD_GAME;
				return;
			}

			if (hud._pausekey.handleEvents(Event) || hud._back.handleEvents(Event) == BUAC_LCLICK)
				state = STATE_LOSE_MENU;
		} else {
			if (!gem.eventInProgress() && !hud._pause.disableHotkeys()) {
				switch (hud.handleEvents(info, Event)) {
				case HS_MAP:
					ToggleState(STATE_MAP);
					break;
				case HS_PAUSE:
					ToggleState(STATE_PAUSE);
					break;
				case HS_CHAR:
					ToggleState(STATE_CHARACTER);
					gem._per.Cache(info, level.playerId(), level);
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
				if (gem.eventInProgress()) {
					gem.handleEvents(info, level.playerId(), Event, hud, level, event_res);
					if (ApplyResult())
						Quit(ShouldChangeState, NewStateID, GAMESTATE_MAIN_MENU);
				} else {
					// Update the talk key state
					info._talkKeyDown = g_engine->_inputManager->state(IG_TALK) || level.containsClick(info.lastPerson(), Event);

					level.handleEvents(info, Event);

					if (!game_over.empty() && game_over.evaluate(info)) {
						state = STATE_LOSE_MENU;
						hud._gom.reset();
						return;
					}

#if 0
					if (g_engine->_inputManager->Equals(IG_QUICKSAVE, Event) == SDL_RELEASED) {
						CreateSaveGame(SAVEGAME_QUICK);
						return;
					} else if (g_engine->_inputManager->Equals(IG_QUICKLOAD, Event) == SDL_RELEASED && !info.IronMan()) {
						ShouldChangeState = true;
						NewStateID = GAMESTATE_LOAD_GAME;
						g_engine->_loadMenu->SelectedPath(FullPath(savefile.quick));
						return;
					}
#endif

					if (hud._pausekey.handleEvents(Event))
						ToggleState(STATE_PAUSE);
				}
			} else if (state == STATE_PAUSE) {
				switch (hud._pause.handleEvents(Event, hud._back)) {
				case PS_RESUME:
					ToggleState(STATE_GAME);
					hud.setTooltip();
					break;

				case PS_SAVE:
					CreateSaveGame(SAVEGAME_NORMAL);
					ToggleState(STATE_GAME);
					hud.setTooltip();
					break;
				case PS_LOAD:
					//ShouldChangeState = true;
					//NewStateID = GAMESTATE_LOAD_GAME;
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
				if (hud._back.handleEvents(Event) == BUAC_LCLICK)
					ToggleState(STATE_GAME);

				switch (state) {
				case STATE_MAP:
					if (map.handleEvents(info, Event)) {
						// We need to load the new level
						LoadLevel(map._curLoc);
						ToggleState(STATE_GAME);
					}
					break;
				case STATE_JOURNAL:
					if (info._journal.handleEvents(level.playerId(), Event)) {
						// This means we selected the "find on map" button, so we need to:
						// switch to the world map, and highlight the appropriate quest marker
						map.selectDest(info._journal._markerTitle);
						ToggleState(STATE_MAP);
					}
					break;
				case STATE_CHARACTER:
					gem._per.handleEvents(info, level.playerId(), Event);
					break;
				case STATE_INVENTORY:
					info._inv.handleEvents(level.playerId(), Event);
					break;
				case STATE_HELP:
					g_engine->_helpScreen->handleEvents(Event);
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
void Game::handleEvents(SDL_Event &Event, bool &ShouldChangeState, GameStateID &NewStateID) {
	g_engine->_mouse->handleEvents(Event);

	if (GameDebug)
		debug_console.handleEvents(Event);

	if (!debug_console.RestrictInput()) {
		if (state == STATE_LOSE_MENU) {
			switch (hud.gom.handleEvents(Event)) {
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
			if (g_engine->_loadMenu->handleEvents(Event)) {
				ShouldChangeState = true;
				NewStateID = GAMESTATE_LOAD_GAME;
				return;
			}

			if (hud.pausekey.handleEvents(Event) || hud.back.handleEvents(Event) == BUAC_LCLICK)
				state = STATE_LOSE_MENU;
		} else {
			if (!gem.EventInProgress() && !hud.pause.DisableHotkeys()) {
				switch (hud.handleEvents(info, Event)) {
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
					gem.handleEvents(info, level.PlayerID(), Event, hud, level, event_res);
					if (ApplyResult())
						Quit(ShouldChangeState, NewStateID, GAMESTATE_MAIN_MENU);
				} else {
					// Update the talk key state
					info.TalkKeyDown = g_engine->_inputManager->State(IG_TALK) || level.ContainsClick(info.LastPerson(), Event);

					level.handleEvents(info, Event);

					if (!game_over.Empty() && game_over.Evaluate(info)) {
						state = STATE_LOSE_MENU;
						hud.gom.reset();
						return;
					}

					if (g_engine->_inputManager->Equals(IG_QUICKSAVE, Event) == SDL_RELEASED) {
						CreateSaveGame(SAVEGAME_QUICK);
						return;
					} else if (g_engine->_inputManager->Equals(IG_QUICKLOAD, Event) == SDL_RELEASED && !info.IronMan()) {
						ShouldChangeState = true;
						NewStateID = GAMESTATE_LOAD_GAME;
						g_engine->_loadMenu->SelectedPath(FullPath(savefile.quick));
						return;
					}

					if (hud.pausekey.handleEvents(Event))
						ToggleState(STATE_PAUSE);
				}
			} else if (state == STATE_PAUSE) {
				switch (hud.pause.handleEvents(Event, hud.back)) {
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
				if (hud.back.handleEvents(Event) == BUAC_LCLICK)
					ToggleState(STATE_GAME);

				switch (state) {
				case STATE_MAP:
					if (map.handleEvents(info, Event)) {
						// We need to load the new level
						LoadLevel(map.cur_loc);
						ToggleState(STATE_GAME);
					}
					break;
				case STATE_JOURNAL:
					if (info.journal.handleEvents(level.PlayerID(), Event)) {
						// This means we selected the "find on map" button, so we need to:
						// switch to the world map, and highlight the appropriate quest marker
						map.SelectDest(info.journal.marker_title);
						ToggleState(STATE_MAP);
					}
					break;
				case STATE_CHARACTER:
					gem.per.handleEvents(info, level.PlayerID(), Event);
					break;
				case STATE_INVENTORY:
					info.inv.handleEvents(level.PlayerID(), Event);
					break;
				case STATE_HELP:
					g_engine->_helpScreen->handleEvents(Event);
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
void Game::internalEvents(bool &ShouldChangeState, GameStateID &NewStateID) {
	switch (state) {
	case STATE_GAME:
		hud.internalEvents(level.showMap());
		event_res.clear();

		{
			// HACK: Since sequences can only be ended in GameEventManager, we use this empty array
			// to get effects to work for levels
			Common::Array<pyrodactyl::event::EventSeqInfo> end_seq;
			ApplyResult(level.internalEvents(info, event_res, end_seq, gem.eventInProgress()));
		}

		gem.internalEvents(info, level, event_res);
		info._talkKeyDown = false;

		if (ApplyResult())
			Quit(ShouldChangeState, NewStateID, GAMESTATE_MAIN_MENU);

		break;
	case STATE_MAP:
		map.internalEvents(info);
		break;
	case STATE_CHARACTER:
		gem._per.internalEvents();
		break;
	default:
		break;
	}
}
//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Game::draw() {
	if (gem._drawGame)
		level.draw(info);
	else
		g_engine->_imageManager->blackScreen();
	switch (state) {
	case STATE_GAME:
		if (gem.eventInProgress())
			gem.draw(info, hud, level);
		else
			hud.draw(info, level.playerId());
		break;
	case STATE_PAUSE:
		g_engine->_imageManager->dimScreen();
		hud._pause.draw(hud._back);
		hud.draw(info, level.playerId());
		break;
	case STATE_MAP:
		g_engine->_imageManager->dimScreen();
		map.draw(info);
		hud.draw(info, level.playerId());
		hud._back.draw();
		break;
	case STATE_JOURNAL:
		g_engine->_imageManager->dimScreen();
		info._journal.draw(level.playerId());
		hud.draw(info, level.playerId());
		hud._back.draw();
		break;
	case STATE_CHARACTER:
		g_engine->_imageManager->dimScreen();
		gem._per.draw(info, level.playerId());
		hud.draw(info, level.playerId());
		hud._back.draw();
		break;
	case STATE_INVENTORY:
		g_engine->_imageManager->dimScreen();
		info.invDraw(level.playerId());
		hud.draw(info, level.playerId());
		hud._back.draw();
		break;
	case STATE_HELP:
		g_engine->_imageManager->dimScreen();
		g_engine->_helpScreen->draw();
		hud._back.draw();
		hud.draw(info, level.playerId());
		break;
	case STATE_LOSE_MENU:
		hud._gom.draw();
		break;
	case STATE_LOSE_LOAD:
		g_engine->_loadMenu->draw();
		hud._back.draw();
		break;
	default:
		break;
	}

	if (GameDebug)
		debug_console.draw(info);
	g_engine->_mouse->draw();
}

//------------------------------------------------------------------------
// Purpose: Apply results of events and levels
//------------------------------------------------------------------------
bool Game::ApplyResult() {
	using namespace pyrodactyl::event;

	for (auto i = event_res.begin(); i != event_res.end(); ++i) {
		switch (i->_type) {
		case ER_MAP:
			if (i->_val == "img")
				map.setImage(i->_y);
			else if (i->_val == "pos") {
				map._playerPos.x = i->_x;
				map._playerPos.y = i->_y;
			}
			break;
		case ER_DEST:
			if (i->_x < 0 || i->_y < 0) {
				info._journal.marker(level.playerId(), i->_val, false);
				map.destDel(i->_val);
			} else {
				map.destAdd(i->_val, i->_x, i->_y);
				info._journal.marker(level.playerId(), i->_val, true);
				info._unread._map = true;
			}
			break;
		case ER_IMG:
			PlayerImg();
			break;
		case ER_TRAIT:
			if (i->_x == 42)
				info.traitDel(i->_val, i->_y);
			else
				info.traitAdd(i->_val, i->_y);
			break;
		case ER_LEVEL:
			if (i->_val == "Map")
				ToggleState(STATE_MAP);
			else
				LoadLevel(i->_val, i->_x, i->_y);
			break;
		case ER_MOVE:
			for (auto &o : level._objects) {
				if (i->_val == o.id()) {
					o.x(i->_x);
					o.y(i->_y);
					break;
				}
			}
			break;
		case ER_PLAYER:
			// First stop the movement of the current player sprite
			level.playerStop();

			// Then swap to the new id
			level.playerId(i->_val, i->_x, i->_y);

			// Stop the new player sprite's movement as well
			level.playerStop();
			break;
		case ER_SAVE:
			CreateSaveGame(SAVEGAME_EVENT);
			break;
		case ER_SYNC:
			level.calcProperties(info);
			map.update(info);
			break;
		case ER_QUIT:
			g_engine->_tempData->credits = (i->_val == "credits");
			return true;
		default:
			break;
		}
	}

	gem._per.Cache(info, level.playerId(), level);
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
		hud._gom.reset();
		break;
	default:
		break;
	}
}

//------------------------------------------------------------------------
// Purpose: Save/load game
//------------------------------------------------------------------------
void Game::loadState(Common::SeekableReadStream *stream) {
	if (!_isInited)
		LoadGame();

	Common::String data = stream->readString();
	// +1 to include > as well
	size_t end = data.findLastOf(">") + 1;

	uint8 *dataC = new uint8[end + 1];
	dataC[end] = '\0';
	memcpy(dataC, data.c_str(), end);

	warning("Output: %s", dataC);

	XMLDoc conf(dataC);

	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("save");
		if (nodeValid(node)) {
			info.loadIronMan(node);
			loadStr(savefile.ironman, "file", node);
			hud._pause.updateMode(info.ironMan());

			if (nodeValid("events", node))
				gem.loadState(node->first_node("events"));

			if (nodeValid("info", node))
				info.loadState(node->first_node("info"));

			if (nodeValid("map", node))
				map.loadState(node->first_node("map"));

			PlayerImg();

			Common::String loc;
			loadStr(loc, "loc_id", node);
			LoadLevel(loc);

			if (nodeValid("level", node))
				level.loadState(node->first_node("level"));

			gem._per.Cache(info, level.playerId(), level);

			Common::String playtime;
			loadStr(playtime, "time", node);
			clock.Start(playtime);
		}
	}
}
//------------------------------------------------------------------------
// Purpose: Write game state to file
//------------------------------------------------------------------------
void Game::saveState(Common::SeekableWriteStream *stream) {
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
	Common::String loc = info.curLocID();
	root->append_attribute(doc.allocate_attribute("loc_id", loc.c_str()));

	// Save location name
	Common::String loc_name = info.curLocName();
	root->append_attribute(doc.allocate_attribute("loc_name", loc_name.c_str()));

	// Save player character name
	Common::String char_name;
	if (info.personValid(level.playerId()))
		char_name = info.personGet(level.playerId())._name;
	root->append_attribute(doc.allocate_attribute("char_name", char_name.c_str()));

	// Difficulty
	Common::String diff = "Normal";
	if (info.ironMan())
		diff = "Iron Man";
	root->append_attribute(doc.allocate_attribute("diff", diff.c_str()));

	// Save file used if iron man
	root->append_attribute(doc.allocate_attribute("file", savefile.ironman.c_str()));

	// Preview image used
	root->append_attribute(doc.allocate_attribute("preview", level._previewPath.c_str()));

	// Time played
	Common::String playtime = clock.GetTime();
	root->append_attribute(doc.allocate_attribute("time", playtime.c_str()));

	rapidxml::xml_node<char> *child_gem = doc.allocate_node(rapidxml::node_element, "events");
	gem.saveState(doc, child_gem);
	root->append_node(child_gem);

	rapidxml::xml_node<char> *child_info = doc.allocate_node(rapidxml::node_element, "info");
	info.saveState(doc, child_info);
	root->append_node(child_info);

	rapidxml::xml_node<char> *child_map = doc.allocate_node(rapidxml::node_element, "map");
	map.saveState(doc, child_map);
	root->append_node(child_map);

	rapidxml::xml_node<char> *child_level = doc.allocate_node(rapidxml::node_element, "level");
	level.saveState(doc, child_level);
	root->append_node(child_level);

	Common::String xml_as_string;
	rapidxml::print(Crab::backInserter(xml_as_string), doc);

	stream->writeString(xml_as_string);

#if 0
	Common::String fullpath = FullPath(filename);

	// We don't check for duplicates for auto-saves and iron man saves
	if (!overwrite) {
		// If a file of this name already exists, find appropriate filename
		if (boost::filesystem::exists(fullpath)) {
			// Copy the original filename, add a _1 at the end
			// Start from one because that's how humans count
			Common::String result = filename + "_1";
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
	g_engine->_imageManager->loadMap(g_engine->_filePath->mainmenu_r);
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
	hud._pause.reset();

	// Only load help screen image if we have to
	if (state == STATE_HELP)
		g_engine->_helpScreen->Refresh();
	else
		g_engine->_helpScreen->Clear();
}

//------------------------------------------------------------------------
// Purpose: Use this function to actually save your games
//------------------------------------------------------------------------
void Game::CreateSaveGame(const SaveGameType &savetype) {
#if 0
	// Disregard type in iron man mode, we only save to one file
	if (info.IronMan())
		saveState(savefile.ironman, true);
	else {
		switch (savetype) {
		case SAVEGAME_NORMAL:
			saveState(hud.pause.SaveFile(), false);
			break;

		case SAVEGAME_EVENT:
			if (savefile.auto_slot)
				saveState(savefile.auto_2, true);
			else
				saveState(savefile.auto_1, true);

			savefile.auto_slot = !savefile.auto_slot;
			break;

		case SAVEGAME_EXIT:
			saveState(savefile.auto_quit, true);
			break;
		case SAVEGAME_QUICK:
			saveState(savefile.quick, true);
			break;
		default:
			break;
		}
	}
#endif
}

void Game::setUI() {
	map.setUI();
	hud.setUI();

	g_engine->_loadMenu->setUI();
	g_engine->_optionMenu->setUI();

	gem.setUI();
	info.setUI();
	level.setUI();
}

} // End of namespace Crab
