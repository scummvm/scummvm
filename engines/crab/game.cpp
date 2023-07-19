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
void Game::startNewGame() {
	init(g_engine->_filePath->_modCur);
	loadLevel(_info.curLocID());
	_info.ironMan(g_engine->_tempData->_ironman);
	_savefile._ironman = g_engine->_tempData->_filename;
	_clock.start();
	_hud._pause.updateMode(_info.ironMan());

	createSaveGame(SAVEGAME_EVENT);
}

void Game::loadGame() {
	init(g_engine->_filePath->_modCur);
}

void Game::init(const Common::String &filename) {
	g_engine->_loadingScreen->dim();
	g_engine->_eventStore->clear();
	_gameOver.clear(false);
	_state = STATE_GAME;
	_savefile._autoSlot = false;
	_gem.init();
	_info.Init();

	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("config");

		_info.load(node);

		Common::String path;
		if (nodeValid("level", node)) {
			loadStr(path, "list", node->first_node("level"));
			g_engine->_filePath->loadLevel(path);
		}

		if (nodeValid("hud", node)) {
			loadStr(path, "layout", node->first_node("hud"));
			_hud.load(path, _level._talkNotify, _level._destMarker);
		}

		if (nodeValid("sprite", node)) {
			loadStr(path, "animation", node->first_node("sprite"));
			_level.loadMoves(path);

			loadStr(path, "constant", node->first_node("sprite"));
			_level.loadConst(path);
		}

		if (nodeValid("event", node)) {
			_gem.load(node->first_node("event"), _popDefault);

			loadStr(path, "store", node->first_node("event"));
			g_engine->_eventStore->load(path);
		}

		if (nodeValid("map", node)) {
			loadStr(path, "layout", node->first_node("map"));
			_map.load(path, _info);
		}

		if (nodeValid("save", node))
			_savefile.load(node->first_node("save"));

		if (nodeValid("debug", node)) {
			loadStr(path, "layout", node->first_node("debug"));
			_debugConsole.load(path);
		}
	}

	_isInited = true;
}

bool Game::loadLevel(const Common::String &id, int playerX, int playerY) {
	if (g_engine->_filePath->_level.contains(id)) {
		g_engine->_loadingScreen->draw();

		// Load the assets local to this level
		// If the filename is same as the previous one, skip loading
		if (g_engine->_filePath->_currentR != g_engine->_filePath->_level[id]._asset) {
			g_engine->_filePath->_currentR = g_engine->_filePath->_level[id]._asset;
			g_engine->_imageManager->loadMap(g_engine->_filePath->_level[id]._asset);
		}

		// Load the level itself
		_level._pop = _popDefault;
		_level.load(g_engine->_filePath->_level[id]._layout, _info, _gameOver, playerX, playerY);

		// Set the current location
		_info.curLocID(id);
		_info.curLocName(g_engine->_filePath->_level[id]._name);
		_map._playerPos = _level._mapLoc;

		// Update and center the world map to the player current position
		_map.update(_info);
		_map.center(_map._playerPos);

		// If this is our first time visiting a level, reveal the associated area on the world map
		_map.revealAdd(_level._mapClip._id, _level._mapClip._rect);

		// Initialize inventory
		_info._inv.init(_level.playerId());

		// Initialize journal
		_info._journal.init(_level.playerId());

		_level.preDraw();
		return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void Game::handleEvents(Common::Event &event, bool &shouldChangeState, GameStateID &newStateId) {
	g_engine->_mouse->handleEvents(event);

	//	if (GameDebug)
	//		debug_console.handleEvents(Event);

	if (!_debugConsole.restrictInput()) {
		if (_state == STATE_LOSE_MENU) {
			switch (_hud._gom.handleEvents(event)) {
			case 0:
				_state = STATE_LOSE_LOAD;
				break;
			case 1:
				quit(shouldChangeState, newStateId, GAMESTATE_MAIN_MENU);
				break;
			default:
				break;
			}
		} else if (_state == STATE_LOSE_LOAD) {
			if (g_engine->_loadMenu->handleEvents(event)) {
				shouldChangeState = true;
				newStateId = GAMESTATE_LOAD_GAME;
				return;
			}

			if (_hud._pausekey.handleEvents(event) || _hud._back.handleEvents(event) == BUAC_LCLICK)
				_state = STATE_LOSE_MENU;
		} else {
			if (!_gem.eventInProgress() && !_hud._pause.disableHotkeys()) {
				switch (_hud.handleEvents(_info, event)) {
				case HS_MAP:
					toggleState(STATE_MAP);
					break;
				case HS_PAUSE:
					toggleState(STATE_PAUSE);
					break;
				case HS_CHAR:
					toggleState(STATE_CHARACTER);
					_gem._per.Cache(_info, _level.playerId(), _level);
					break;
				case HS_JOURNAL:
					toggleState(STATE_JOURNAL);
					break;
				case HS_INV:
					toggleState(STATE_INVENTORY);
					break;
				default:
					break;
				}
			}

			if (_state == STATE_GAME) {
				if (_gem.eventInProgress()) {
					_gem.handleEvents(_info, _level.playerId(), event, _hud, _level, _eventRes);
					if (applyResult())
						quit(shouldChangeState, newStateId, GAMESTATE_MAIN_MENU);
				} else {
					// Update the talk key state
					_info._talkKeyDown = g_engine->_inputManager->state(IG_TALK) || _level.containsClick(_info.lastPerson(), event);

					_level.handleEvents(_info, event);

					if (!_gameOver.empty() && _gameOver.evaluate(_info)) {
						_state = STATE_LOSE_MENU;
						_hud._gom.reset();
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

					if (_hud._pausekey.handleEvents(event))
						toggleState(STATE_PAUSE);
				}
			} else if (_state == STATE_PAUSE) {
				switch (_hud._pause.handleEvents(event, _hud._back)) {
				case PS_RESUME:
					toggleState(STATE_GAME);
					_hud.setTooltip();
					break;

				case PS_SAVE:
					createSaveGame(SAVEGAME_NORMAL);
					toggleState(STATE_GAME);
					_hud.setTooltip();
					break;
				case PS_LOAD:
					//ShouldChangeState = true;
					//NewStateID = GAMESTATE_LOAD_GAME;
					return;

				case PS_HELP:
					toggleState(STATE_HELP);
					break;

				case PS_QUIT_MENU:
					createSaveGame(SAVEGAME_EXIT);
					quit(shouldChangeState, newStateId, GAMESTATE_MAIN_MENU);
					break;

				case PS_QUIT_GAME:
					createSaveGame(SAVEGAME_EXIT);
					quit(shouldChangeState, newStateId, GAMESTATE_EXIT);
					break;
				default:
					break;
				}
			} else {
				if (_hud._back.handleEvents(event) == BUAC_LCLICK)
					toggleState(STATE_GAME);

				switch (_state) {
				case STATE_MAP:
					if (_map.handleEvents(_info, event)) {
						// We need to load the new level
						loadLevel(_map._curLoc);
						toggleState(STATE_GAME);
					}
					break;
				case STATE_JOURNAL:
					if (_info._journal.handleEvents(_level.playerId(), event)) {
						// This means we selected the "find on map" button, so we need to:
						// switch to the world map, and highlight the appropriate quest marker
						_map.selectDest(_info._journal._markerTitle);
						toggleState(STATE_MAP);
					}
					break;
				case STATE_CHARACTER:
					_gem._per.handleEvents(_info, _level.playerId(), event);
					break;
				case STATE_INVENTORY:
					_info._inv.handleEvents(_level.playerId(), event);
					break;
				case STATE_HELP:
					g_engine->_helpScreen->handleEvents(event);
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
void Game::internalEvents(bool &shouldChangeState, GameStateID &newStateId) {
	switch (_state) {
	case STATE_GAME:
		_hud.internalEvents(_level.showMap());
		_eventRes.clear();

		{
			// HACK: Since sequences can only be ended in GameEventManager, we use this empty array
			// to get effects to work for levels
			Common::Array<pyrodactyl::event::EventSeqInfo> endSeq;
			applyResult(_level.internalEvents(_info, _eventRes, endSeq, _gem.eventInProgress()));
		}

		_gem.internalEvents(_info, _level, _eventRes);
		_info._talkKeyDown = false;

		if (applyResult())
			quit(shouldChangeState, newStateId, GAMESTATE_MAIN_MENU);

		break;
	case STATE_MAP:
		_map.internalEvents(_info);
		break;
	case STATE_CHARACTER:
		_gem._per.internalEvents();
		break;
	default:
		break;
	}
}
//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Game::draw() {
	if (_gem._drawGame)
		_level.draw(_info);
	else
		g_engine->_imageManager->blackScreen();
	switch (_state) {
	case STATE_GAME:
		if (_gem.eventInProgress())
			_gem.draw(_info, _hud, _level);
		else
			_hud.draw(_info, _level.playerId());
		break;
	case STATE_PAUSE:
		g_engine->_imageManager->dimScreen();
		_hud._pause.draw(_hud._back);
		_hud.draw(_info, _level.playerId());
		break;
	case STATE_MAP:
		g_engine->_imageManager->dimScreen();
		_map.draw(_info);
		_hud.draw(_info, _level.playerId());
		_hud._back.draw();
		break;
	case STATE_JOURNAL:
		g_engine->_imageManager->dimScreen();
		_info._journal.draw(_level.playerId());
		_hud.draw(_info, _level.playerId());
		_hud._back.draw();
		break;
	case STATE_CHARACTER:
		g_engine->_imageManager->dimScreen();
		_gem._per.draw(_info, _level.playerId());
		_hud.draw(_info, _level.playerId());
		_hud._back.draw();
		break;
	case STATE_INVENTORY:
		g_engine->_imageManager->dimScreen();
		_info.invDraw(_level.playerId());
		_hud.draw(_info, _level.playerId());
		_hud._back.draw();
		break;
	case STATE_HELP:
		g_engine->_imageManager->dimScreen();
		g_engine->_helpScreen->draw();
		_hud._back.draw();
		_hud.draw(_info, _level.playerId());
		break;
	case STATE_LOSE_MENU:
		_hud._gom.draw();
		break;
	case STATE_LOSE_LOAD:
		g_engine->_loadMenu->draw();
		_hud._back.draw();
		break;
	default:
		break;
	}

	if (GameDebug)
		_debugConsole.draw(_info);
	g_engine->_mouse->draw();
}

//------------------------------------------------------------------------
// Purpose: Apply results of events and levels
//------------------------------------------------------------------------
bool Game::applyResult() {
	using namespace pyrodactyl::event;

	for (auto i = _eventRes.begin(); i != _eventRes.end(); ++i) {
		switch (i->_type) {
		case ER_MAP:
			if (i->_val == "img")
				_map.setImage(i->_y);
			else if (i->_val == "pos") {
				_map._playerPos.x = i->_x;
				_map._playerPos.y = i->_y;
			}
			break;
		case ER_DEST:
			if (i->_x < 0 || i->_y < 0) {
				_info._journal.marker(_level.playerId(), i->_val, false);
				_map.destDel(i->_val);
			} else {
				_map.destAdd(i->_val, i->_x, i->_y);
				_info._journal.marker(_level.playerId(), i->_val, true);
				_info._unread._map = true;
			}
			break;
		case ER_IMG:
			playerImg();
			break;
		case ER_TRAIT:
			if (i->_x == 42)
				_info.traitDel(i->_val, i->_y);
			else
				_info.traitAdd(i->_val, i->_y);
			break;
		case ER_LEVEL:
			if (i->_val == "Map")
				toggleState(STATE_MAP);
			else
				loadLevel(i->_val, i->_x, i->_y);
			break;
		case ER_MOVE:
			for (auto &o : _level._objects) {
				if (i->_val == o.id()) {
					o.x(i->_x);
					o.y(i->_y);
					break;
				}
			}
			break;
		case ER_PLAYER:
			// First stop the movement of the current player sprite
			_level.playerStop();

			// Then swap to the new id
			_level.playerId(i->_val, i->_x, i->_y);

			// Stop the new player sprite's movement as well
			_level.playerStop();
			break;
		case ER_SAVE:
			createSaveGame(SAVEGAME_EVENT);
			break;
		case ER_SYNC:
			_level.calcProperties(_info);
			_map.update(_info);
			break;
		case ER_QUIT:
			g_engine->_tempData->_credits = (i->_val == "credits");
			return true;
		default:
			break;
		}
	}

	_gem._per.Cache(_info, _level.playerId(), _level);
	_eventRes.clear();
	return false;
}

void Game::applyResult(LevelResult result) {
	switch (result._type) {
	case LR_LEVEL:
		if (result._val == "Map")
			toggleState(STATE_MAP);
		else
			loadLevel(result._val, result._x, result._y);
		return;
	case LR_GAMEOVER:
		_state = STATE_LOSE_MENU;
		_hud._gom.reset();
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
		loadGame();

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
			_info.loadIronMan(node);
			loadStr(_savefile._ironman, "file", node);
			_hud._pause.updateMode(_info.ironMan());

			if (nodeValid("events", node))
				_gem.loadState(node->first_node("events"));

			if (nodeValid("info", node))
				_info.loadState(node->first_node("info"));

			if (nodeValid("map", node))
				_map.loadState(node->first_node("map"));

			playerImg();

			Common::String loc;
			loadStr(loc, "loc_id", node);
			loadLevel(loc);

			if (nodeValid("level", node))
				_level.loadState(node->first_node("level"));

			_gem._per.Cache(_info, _level.playerId(), _level);

			Common::String playtime;
			loadStr(playtime, "time", node);
			_clock.start(playtime);
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
	Common::String loc = _info.curLocID();
	root->append_attribute(doc.allocate_attribute("loc_id", loc.c_str()));

	// Save location name
	Common::String loc_name = _info.curLocName();
	root->append_attribute(doc.allocate_attribute("loc_name", loc_name.c_str()));

	// Save player character name
	Common::String char_name;
	if (_info.personValid(_level.playerId()))
		char_name = _info.personGet(_level.playerId())._name;
	root->append_attribute(doc.allocate_attribute("char_name", char_name.c_str()));

	// Difficulty
	Common::String diff = "Normal";
	if (_info.ironMan())
		diff = "Iron Man";
	root->append_attribute(doc.allocate_attribute("diff", diff.c_str()));

	// Save file used if iron man
	root->append_attribute(doc.allocate_attribute("file", _savefile._ironman.c_str()));

	// Preview image used
	root->append_attribute(doc.allocate_attribute("preview", _level._previewPath.c_str()));

	// Time played
	Common::String playtime = _clock.getTime();
	root->append_attribute(doc.allocate_attribute("time", playtime.c_str()));

	rapidxml::xml_node<char> *child_gem = doc.allocate_node(rapidxml::node_element, "events");
	_gem.saveState(doc, child_gem);
	root->append_node(child_gem);

	rapidxml::xml_node<char> *child_info = doc.allocate_node(rapidxml::node_element, "info");
	_info.saveState(doc, child_info);
	root->append_node(child_info);

	rapidxml::xml_node<char> *child_map = doc.allocate_node(rapidxml::node_element, "map");
	_map.saveState(doc, child_map);
	root->append_node(child_map);

	rapidxml::xml_node<char> *child_level = doc.allocate_node(rapidxml::node_element, "level");
	_level.saveState(doc, child_level);
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
void Game::quit(bool &shouldChangeState, GameStateID &newStateId, const GameStateID &newStateVal) {
	shouldChangeState = true;
	newStateId = newStateVal;
	g_engine->_imageManager->loadMap(g_engine->_filePath->_mainmenuR);
}
//------------------------------------------------------------------------
// Purpose: Change our internal state
//------------------------------------------------------------------------
void Game::toggleState(const State &s) {
	if (_state != s)
		_state = s;
	else
		_state = STATE_GAME;

	// This is because game is the first state, the rest are in order
	_hud.State(_state - 1);
	_hud._pause.reset();

	// Only load help screen image if we have to
	if (_state == STATE_HELP)
		g_engine->_helpScreen->refresh();
	else
		g_engine->_helpScreen->clear();
}

//------------------------------------------------------------------------
// Purpose: Use this function to actually save your games
//------------------------------------------------------------------------
void Game::createSaveGame(const SaveGameType &savetype) {
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
	_map.setUI();
	_hud.setUI();

	g_engine->_loadMenu->setUI();
	g_engine->_optionMenu->setUI();

	_gem.setUI();
	_info.setUI();
	_level.setUI();
}

} // End of namespace Crab
