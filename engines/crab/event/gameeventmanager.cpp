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

#include "crab/gameeventmanager.h"

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
void Manager::Load(rapidxml::xml_node<char> *node, ParagraphData &popup) {
	if (NodeValid(node)) {
		XMLDoc conf(node->first_attribute("list")->value());
		if (conf.ready()) {
			rapidxml::xml_node<char> *lnode = conf.Doc()->first_node("event_list");
			for (rapidxml::xml_node<char> *loc = lnode->first_node("loc"); loc != NULL; loc = loc->next_sibling("loc")) {
				std::string loc_name;
				LoadStr(loc_name, "name", loc);

				for (auto n = loc->first_node("file"); n != NULL; n = n->next_sibling("file")) {
					unsigned int id;
					std::string path;
					LoadNum(id, "name", n);
					LoadStr(path, "path", n);
					event_map[loc_name].AddSeq(id, path);
				}
			}
		}

		active_seq = UINT_MAX;

		conf.Load(node->first_attribute("layout")->value());
		if (conf.ready()) {
			rapidxml::xml_node<char> *layout = conf.Doc()->first_node("layout");
			if (NodeValid(layout)) {
				if (NodeValid("character", layout))
					oh.Load(layout->first_node("character"));

				if (NodeValid("popup", layout))
					popup.Load(layout->first_node("popup"));

				if (NodeValid("intro", layout))
					intro.Load(layout->first_node("intro"));
			}
		}

		reply.Load(node->first_attribute("conversation")->value());

		per.Load(node->first_attribute("char")->value());
	}
}

//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void Manager::HandleEvents(Info &info, const std::string &player_id, SDL_Event &Event, HUD &hud, Level &level, std::vector<EventResult> &result) {
	// If an event is already being performed
	if (event_map.count(info.CurLocID()) > 0 && event_map[info.CurLocID()].EventInProgress(active_seq)) {
		switch (cur_event->type) {
		case EVENT_DIALOG:
			if (oh.show_journal) {
				info.journal.HandleEvents(player_id, Event);

				if (hud.back.HandleEvents(Event) == BUAC_LCLICK || hud.pausekey.HandleEvents(Event))
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
				info.journal.HandleEvents(player_id, Event);

				if (hud.back.HandleEvents(Event) == BUAC_LCLICK || hud.pausekey.HandleEvents(Event))
					oh.show_journal = false;
			} else {
				// If journal button is select from within an event, go to the entry corresponding to that person's name
				if (oh.HandleCommonEvents(Event))
					if (info.PersonValid(cur_event->title))
						info.journal.Open(player_id, JE_PEOPLE, info.PersonGet(cur_event->title).name);

				int choice = reply.HandleEvents(info, gEventStore.con.at(cur_event->special), cur_event->title, oh, Event);
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

			if (textin.HandleEvents(Event))
				event_map[info.CurLocID()].NextEvent(active_seq, info, player_id, result, end_seq);
			break;
		case EVENT_SPLASH:
			if (intro.show_traits) {
				per.HandleEvents(info, cur_event->title, Event);

				if (hud.back.HandleEvents(Event) == BUAC_LCLICK || hud.pausekey.HandleEvents(Event))
					intro.show_traits = false;
			} else {
				if (intro.HandleEvents(Event))
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
//------------------------------------------------------------------------
// Purpose: Internal Events
//------------------------------------------------------------------------
void Manager::InternalEvents(Info &info, Level &level, std::vector<EventResult> &result) {
	if (event_map.count(info.CurLocID()) > 0) {
		if (event_map[info.CurLocID()].EventInProgress(active_seq)) {
			switch (cur_event->type) {
			case EVENT_DIALOG:
				UpdateDialogBox(info, level);
				break;
			case EVENT_ANIM: {
				using namespace pyrodactyl::anim;

				DrawType draw_val = DRAW_SAME;
				if (gEventStore.anim.at(cur_event->special).InternalEvents(draw_val))
					event_map[info.CurLocID()].NextEvent(active_seq, info, level.PlayerID(), result, end_seq);

				if (draw_val == DRAW_STOP)
					draw_game = false;
				else if (draw_val == DRAW_START)
					draw_game = true;
			} break;
			case EVENT_SILENT:
				event_map[info.CurLocID()].NextEvent(active_seq, info, level.PlayerID(), result, end_seq);
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
			event_map[info.CurLocID()].InternalEvents(info);
			CalcActiveSeq(info, level, level.Camera());
		}
	}
}

void Manager::UpdateDialogBox(Info &info, Level &level) {
	oh.InternalEvents(cur_event->state, cur_sp);
}
//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Manager::Draw(Info &info, HUD &hud, Level &level) {
	if (event_map.count(info.CurLocID()) > 0 && event_map[info.CurLocID()].EventInProgress(active_seq)) {
		switch (cur_event->type) {
		case EVENT_ANIM:
			gEventStore.anim.at(cur_event->special).Draw();
			break;
		case EVENT_DIALOG:
			gImageManager.DimScreen();
			if (oh.show_journal) {
				info.journal.Draw(level.PlayerID());
				hud.back.Draw();
			} else
				oh.Draw(info, cur_event, cur_event->title, player, cur_sp);
			break;
		case EVENT_REPLY:
			gImageManager.DimScreen();
			if (oh.show_journal) {
				info.journal.Draw(level.PlayerID());
				hud.back.Draw();
			} else {
				oh.Draw(info, cur_event, cur_event->title, player, cur_sp);
				reply.Draw();
			}
			break;
		case EVENT_TEXT:
			oh.Draw(info, cur_event, cur_event->title, player, cur_sp);
			textin.Draw();
			break;
		case EVENT_SPLASH:
			gImageManager.DimScreen();
			if (intro.show_traits) {
				per.Draw(info, cur_event->title);
				hud.back.Draw();
			} else
				intro.Draw(info, cur_event->dialog, cur_sp, cur_event->state);

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
	if (event_map[info.CurLocID()].ActiveSeq(active_seq)) {
		// Set all the pointers to the new values
		cur_event = event_map[info.CurLocID()].CurEvent(active_seq);
		oh.Reset(cur_event->title);
		cur_sp = level.GetSprite(cur_event->title);

		// The player character's dialog is drawn a bit differently compared to others
		player = (cur_event->title == level.PlayerID());

		switch (cur_event->type) {
		case EVENT_ANIM:
			gEventStore.anim.at(cur_event->special).Start();
			break;
		case EVENT_REPLY:
			reply.Cache(info, gEventStore.con.at(cur_event->special));
			break;
		default:
			break;
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Get/set info
//------------------------------------------------------------------------
void Manager::EndSequence(const std::string &curloc) {
	if (end_seq.empty() == false) {
		for (auto i = end_seq.begin(); i != end_seq.end(); ++i)
			if (i->cur)
				event_map[curloc].EndSeq(active_seq);
			else if (event_map.count(i->loc) > 0)
				event_map.at(i->loc).EndSeq(StringToNumber<unsigned int>(i->val));

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
void Manager::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	for (auto i = event_map.begin(); i != event_map.end(); ++i) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "loc");
		child->append_attribute(doc.allocate_attribute("name", i->first.c_str()));
		i->second.SaveState(doc, child);
		root->append_node(child);
	}
}

//------------------------------------------------------------------------
// Purpose: Load the state of the object
//------------------------------------------------------------------------
void Manager::LoadState(rapidxml::xml_node<char> *node) {
	for (auto n = node->first_node("loc"); n != NULL; n = n->next_sibling("loc")) {
		if (n->first_attribute("name") != NULL) {
			std::string name = n->first_attribute("name")->value();
			if (event_map.count(name) > 0)
				event_map[name].LoadState(n);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Function called when window size is changed to adjust UI
//------------------------------------------------------------------------
void Manager::SetUI() {
	oh.SetUI();
	reply.SetUI();
	textin.SetUI();
	per.SetUI();
}

} // End of namespace Crab
