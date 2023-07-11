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

#include "crab/event/gameeventmanager.h"

namespace Crab {

using namespace pyrodactyl::people;
using namespace pyrodactyl::event;
using namespace pyrodactyl::level;
using namespace pyrodactyl::image;
using namespace pyrodactyl::ui;

void Manager::Init() {
	event_map.clear();
	active_seq = UINT_MAX;
	cur_sp = nullptr;
	player = false;
	cur_event = nullptr;
	draw_game = true;
}

//------------------------------------------------------------------------
// Purpose: Load this
//------------------------------------------------------------------------
void Manager::load(rapidxml::xml_node<char> *node, ParagraphData &popup) {
	if (nodeValid(node)) {
		XMLDoc conf(node->first_attribute("list")->value());
		if (conf.ready()) {
			rapidxml::xml_node<char> *lnode = conf.doc()->first_node("event_list");
			for (rapidxml::xml_node<char> *loc = lnode->first_node("loc"); loc != NULL; loc = loc->next_sibling("loc")) {
				Common::String loc_name;
				loadStr(loc_name, "name", loc);

				for (auto n = loc->first_node("file"); n != NULL; n = n->next_sibling("file")) {
					unsigned int id;
					Common::String path;
					loadNum(id, "name", n);
					loadStr(path, "path", n);
					event_map[loc_name].addSeq(id, path);
				}
			}
		}

		active_seq = UINT_MAX;

		conf.load(node->first_attribute("layout")->value());
		if (conf.ready()) {
			rapidxml::xml_node<char> *layout = conf.doc()->first_node("layout");
			if (nodeValid(layout)) {
				if (nodeValid("character", layout))
					oh.load(layout->first_node("character"));

				if (nodeValid("popup", layout))
					popup.load(layout->first_node("popup"));

				if (nodeValid("intro", layout))
					intro.load(layout->first_node("intro"));
			}
		}

		reply.load(node->first_attribute("conversation")->value());

		per.load(node->first_attribute("char")->value());
	}
}

//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void Manager::handleEvents(Info &info, const Common::String &player_id, Common::Event &Event, HUD &hud, Level &level, Common::Array<EventResult> &result) {
	// If an event is already being performed
	if (event_map.contains(info.CurLocID()) > 0 && event_map[info.CurLocID()].eventInProgress(active_seq)) {
		switch (cur_event->type) {
		case EVENT_DIALOG:
			if (oh.show_journal) {
				info.journal.handleEvents(player_id, Event);

				if (hud.back.handleEvents(Event) == BUAC_LCLICK || hud.pausekey.handleEvents(Event))
					oh.show_journal = false;
			} else {
				// If journal button is select from within an event, go to the entry corresponding to that person's name
				if (oh.HandleCommonEvents(Event)) {
					if (info.PersonValid(cur_event->title)) {
						Person &p = info.PersonGet(cur_event->title);
						if (p.alt_journal_name)
							info.journal.Open(player_id, JE_PEOPLE, p.journal_name);
						else
							info.journal.Open(player_id, JE_PEOPLE, p.name);
					}
				}

				if (oh.HandleDlboxEvents(Event)) {
					event_map[info.CurLocID()].nextEvent(active_seq, info, player_id, result, end_seq);
					oh.show_journal = false;
				}
			}
			break;
		case EVENT_ANIM:
			// Skip animation if key pressed or mouse pressed
			if (Event.type == Common::EVENT_LBUTTONUP || Event.type == Common::EVENT_RBUTTONUP)
				event_map[info.CurLocID()].nextEvent(active_seq, info, player_id, result, end_seq);
			break;
		case EVENT_REPLY:
			if (oh.show_journal) {
				info.journal.handleEvents(player_id, Event);

				if (hud.back.handleEvents(Event) == BUAC_LCLICK || hud.pausekey.handleEvents(Event))
					oh.show_journal = false;
			} else {
				// If journal button is select from within an event, go to the entry corresponding to that person's name
				if (oh.HandleCommonEvents(Event))
					if (info.PersonValid(cur_event->title))
						info.journal.Open(player_id, JE_PEOPLE, info.PersonGet(cur_event->title).name);

				int choice = reply.handleEvents(info, g_engine->_eventStore->_con[cur_event->special], cur_event->title, oh, Event);
				if (choice >= 0) {
					event_map[info.CurLocID()].nextEvent(active_seq, info, player_id, result, end_seq, choice);
					oh.show_journal = false;
				}
			}
			break;
		case EVENT_TEXT:
			// If journal button is select from within an event, go to the entry corresponding to that person's name
			if (oh.HandleCommonEvents(Event))
				if (info.PersonValid(cur_event->title))
					info.journal.Open(player_id, JE_PEOPLE, info.PersonGet(cur_event->title).name);

			if (textin.handleEvents(Event))
				event_map[info.CurLocID()].nextEvent(active_seq, info, player_id, result, end_seq);
			break;
		case EVENT_SPLASH:
			if (intro.show_traits) {
				per.handleEvents(info, cur_event->title, Event);

				if (hud.back.handleEvents(Event) == BUAC_LCLICK || hud.pausekey.handleEvents(Event))
					intro.show_traits = false;
			} else {
				if (intro.handleEvents(Event))
					event_map[info.CurLocID()].nextEvent(active_seq, info, player_id, result, end_seq);

				if (intro.show_traits)
					per.Cache(info, level.PlayerID(), level);
			}
			break;
		default:
			break;
		}

		EndSequence(info.CurLocID());
	}
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void Manager::handleEvents(Info &info, const Common::String &player_id, SDL_Event &Event, HUD &hud, Level &level, Common::Array<EventResult> &result) {
	// If an event is already being performed
	if (event_map.contains(info.CurLocID()) > 0 && event_map[info.CurLocID()].EventInProgress(active_seq)) {
		switch (cur_event->type) {
		case EVENT_DIALOG:
			if (oh.show_journal) {
				info.journal.handleEvents(player_id, Event);

				if (hud.back.handleEvents(Event) == BUAC_LCLICK || hud.pausekey.handleEvents(Event))
					oh.show_journal = false;
			} else {
				// If journal button is select from within an event, go to the entry corresponding to that person's name
				if (oh.HandleCommonEvents(Event)) {
					if (info.PersonValid(cur_event->title)) {
						Person &p = info.PersonGet(cur_event->title);
						if (p.alt_journal_name)
							info.journal.Open(player_id, JE_PEOPLE, p.journal_name);
						else
							info.journal.Open(player_id, JE_PEOPLE, p.name);
					}
				}

				if (oh.HandleDlboxEvents(Event)) {
					event_map[info.CurLocID()].NextEvent(active_seq, info, player_id, result, end_seq);
					oh.show_journal = false;
				}
			}
			break;
		case EVENT_ANIM:
			// Skip animation if key pressed or mouse pressed
			if (Event.type == SDL_KEYUP || Event.type == SDL_MOUSEBUTTONUP)
				event_map[info.CurLocID()].NextEvent(active_seq, info, player_id, result, end_seq);
			break;
		case EVENT_REPLY:
			if (oh.show_journal) {
				info.journal.handleEvents(player_id, Event);

				if (hud.back.handleEvents(Event) == BUAC_LCLICK || hud.pausekey.handleEvents(Event))
					oh.show_journal = false;
			} else {
				// If journal button is select from within an event, go to the entry corresponding to that person's name
				if (oh.HandleCommonEvents(Event))
					if (info.PersonValid(cur_event->title))
						info.journal.Open(player_id, JE_PEOPLE, info.PersonGet(cur_event->title).name);

				int choice = reply.handleEvents(info, g_engine->_eventStore->con[cur_event->special], cur_event->title, oh, Event);
				if (choice >= 0) {
					event_map[info.CurLocID()].NextEvent(active_seq, info, player_id, result, end_seq, choice);
					oh.show_journal = false;
				}
			}
			break;
		case EVENT_TEXT:
			// If journal button is select from within an event, go to the entry corresponding to that person's name
			if (oh.HandleCommonEvents(Event))
				if (info.PersonValid(cur_event->title))
					info.journal.Open(player_id, JE_PEOPLE, info.PersonGet(cur_event->title).name);

			if (textin.handleEvents(Event))
				event_map[info.CurLocID()].NextEvent(active_seq, info, player_id, result, end_seq);
			break;
		case EVENT_SPLASH:
			if (intro.show_traits) {
				per.handleEvents(info, cur_event->title, Event);

				if (hud.back.handleEvents(Event) == BUAC_LCLICK || hud.pausekey.handleEvents(Event))
					intro.show_traits = false;
			} else {
				if (intro.handleEvents(Event))
					event_map[info.CurLocID()].NextEvent(active_seq, info, player_id, result, end_seq);

				if (intro.show_traits)
					per.Cache(info, level.PlayerID(), level);
			}
			break;
		default:
			break;
		}

		EndSequence(info.CurLocID());
	}
}
#endif

//------------------------------------------------------------------------
// Purpose: Internal Events
//------------------------------------------------------------------------
void Manager::internalEvents(Info &info, Level &level, Common::Array<EventResult> &result) {
	if (event_map.contains(info.CurLocID()) > 0) {
		if (event_map[info.CurLocID()].eventInProgress(active_seq)) {
			switch (cur_event->type) {
			case EVENT_DIALOG:
				UpdateDialogBox(info, level);
				break;
			case EVENT_ANIM: {
				using namespace pyrodactyl::anim;

				DrawType draw_val = DRAW_SAME;
				if (g_engine->_eventStore->_anim[cur_event->special].internalEvents(draw_val))
					event_map[info.CurLocID()].nextEvent(active_seq, info, level.PlayerID(), result, end_seq);

				if (draw_val == DRAW_STOP)
					draw_game = false;
				else if (draw_val == DRAW_START)
					draw_game = true;
			} break;
			case EVENT_SILENT:
				event_map[info.CurLocID()].nextEvent(active_seq, info, level.PlayerID(), result, end_seq);
				break;
			case EVENT_REPLY:
				UpdateDialogBox(info, level);
				break;
			case EVENT_SPLASH:
				UpdateDialogBox(info, level);
				break;
			default:
				break;
			}

			EndSequence(info.CurLocID());
		} else {
			event_map[info.CurLocID()].internalEvents(info);
			CalcActiveSeq(info, level, level.Camera());
		}
	}
}

void Manager::UpdateDialogBox(Info &info, Level &level) {
	oh.internalEvents(cur_event->state, cur_sp);
}
//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Manager::draw(Info &info, HUD &hud, Level &level) {
	if (event_map.contains(info.CurLocID()) > 0 && event_map[info.CurLocID()].eventInProgress(active_seq)) {
		switch (cur_event->type) {
		case EVENT_ANIM:
			g_engine->_eventStore->_anim[cur_event->special].draw();
			break;
		case EVENT_DIALOG:
			g_engine->_imageManager->DimScreen();
			if (oh.show_journal) {
				info.journal.draw(level.PlayerID());
				hud.back.draw();
			} else
				oh.draw(info, cur_event, cur_event->title, player, cur_sp);
			break;
		case EVENT_REPLY:
			g_engine->_imageManager->DimScreen();
			if (oh.show_journal) {
				info.journal.draw(level.PlayerID());
				hud.back.draw();
			} else {
				oh.draw(info, cur_event, cur_event->title, player, cur_sp);
				reply.draw();
			}
			break;
		case EVENT_TEXT:
			oh.draw(info, cur_event, cur_event->title, player, cur_sp);
			textin.draw();
			break;
		case EVENT_SPLASH:
			g_engine->_imageManager->DimScreen();
			if (intro.show_traits) {
				per.draw(info, cur_event->title);
				hud.back.draw();
			} else
				intro.draw(info, cur_event->dialog, cur_sp, cur_event->state);

			break;
		default:
			break;
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Calculate the current sequence in progress
//------------------------------------------------------------------------
void Manager::CalcActiveSeq(Info &info, Level &level, const Rect &camera) {
	if (event_map[info.CurLocID()].activeSeq(active_seq)) {
		// Set all the pointers to the new values
		cur_event = event_map[info.CurLocID()].curEvent(active_seq);
		oh.reset(cur_event->title);
		cur_sp = level.GetSprite(cur_event->title);

		// The player character's dialog is drawn a bit differently compared to others
		player = (cur_event->title == level.PlayerID());

		switch (cur_event->type) {
		case EVENT_ANIM:
			g_engine->_eventStore->_anim[cur_event->special].start();
			break;
		case EVENT_REPLY:
			reply.Cache(info, g_engine->_eventStore->_con[cur_event->special]);
			break;
		default:
			break;
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Get/set info
//------------------------------------------------------------------------
void Manager::EndSequence(const Common::String &curloc) {
	if (end_seq.empty() == false) {
		for (auto i = end_seq.begin(); i != end_seq.end(); ++i)
			if (i->_cur)
				event_map[curloc].endSeq(active_seq);
			else if (event_map.contains(i->_loc) > 0)
				event_map[i->_loc].endSeq(StringToNumber<unsigned int>(i->_val));

		active_seq = UINT_MAX;
		end_seq.clear();
	}
}

bool Manager::EventInProgress() {
	if (active_seq == UINT_MAX)
		return false;
	return true;
}

//------------------------------------------------------------------------
// Purpose: Save the state of the object
//------------------------------------------------------------------------
void Manager::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	for (auto i = event_map.begin(); i != event_map.end(); ++i) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "loc");
		child->append_attribute(doc.allocate_attribute("name", i->_key.c_str()));
		i->_value.saveState(doc, child);
		root->append_node(child);
	}
}

//------------------------------------------------------------------------
// Purpose: Load the state of the object
//------------------------------------------------------------------------
void Manager::loadState(rapidxml::xml_node<char> *node) {
	for (auto n = node->first_node("loc"); n != NULL; n = n->next_sibling("loc")) {
		if (n->first_attribute("name") != NULL) {
			Common::String name = n->first_attribute("name")->value();
			if (event_map.contains(name) > 0)
				event_map[name].loadState(n);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Function called when window size is changed to adjust UI
//------------------------------------------------------------------------
void Manager::setUI() {
	oh.setUI();
	reply.setUI();
	textin.setUI();
	per.setUI();
}

} // End of namespace Crab
