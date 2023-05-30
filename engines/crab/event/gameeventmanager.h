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

#ifndef CRAB_GAMEEVENTMANAGER_H
#define CRAB_GAMEEVENTMANAGER_H

#include "crab/ui/ChapterIntro.h"
#include "crab/event/EventSeqGroup.h"
#include "crab/ui/PersonHandler.h"
#include "crab/ui/PersonScreen.h"
#include "crab/ui/ReplyMenu.h"
#include "crab/common_header.h"
#include "crab/ui/hud.h"
#include "crab/ui/journal.h"
#include "crab/level/level.h"
#include "crab/ui/textarea.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
class Manager {
protected:
	// All the events in the game
	std::unordered_map<std::string, EventSeqGroup> event_map;

	// The currently happening or active sequence
	unsigned int active_seq;

	// THIS IS NOT THE DEFINITIVE LIST OF ENDED SEQUENCES
	// JUST A TEMPORARY LIST OF EVENT SEQUENCES TO PASS AROUND
	Common::Array<EventSeqInfo> end_seq;

	// The objects used to draw the dialog box and opinion bars
	pyrodactyl::ui::PersonHandler oh;

	// The reply menu and the colors and font of the text
	pyrodactyl::ui::ReplyMenu reply;

	// The field for text input
	pyrodactyl::ui::TextArea textin;

	// The info for intro events
	pyrodactyl::ui::ChapterIntro intro;

	// Store the current event data temporarily
	GameEvent *cur_event;
	bool player;
	pyrodactyl::anim::Sprite *cur_sp;

	void UpdateDialogBox(Info &info, pyrodactyl::level::Level &level);

public:
	// The object used to draw the character screen
	pyrodactyl::ui::PersonScreen per;

	// A flag used to stop drawing the game for a better fade in/fade out experience
	bool draw_game;

	Manager() { Init(); }
	~Manager() {}

	void Init();
	void Load(rapidxml::xml_node<char> *node, pyrodactyl::ui::ParagraphData &popup);

	void Draw(Info &info, pyrodactyl::ui::HUD &hud, pyrodactyl::level::Level &level);

	// cur_per is also updated here
	void InternalEvents(Info &info, pyrodactyl::level::Level &level, Common::Array<EventResult> &result);

	void HandleEvents(Info &info, const std::string &player_id, Common::Event &Event,
					  pyrodactyl::ui::HUD &hud, pyrodactyl::level::Level &level, Common::Array<EventResult> &result);
#if 0
	void HandleEvents(Info &info, const std::string &player_id, SDL_Event &Event,
					  pyrodactyl::ui::HUD &hud, pyrodactyl::level::Level &level, Common::Array<EventResult> &result);
#endif

	void CalcActiveSeq(Info &info, pyrodactyl::level::Level &level, const Rect &camera);

	void EndSequence(const std::string &curloc);
	bool EventInProgress();

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void LoadState(rapidxml::xml_node<char> *node);

	void SetUI();
};
} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_GAMEEVENTMANAGER_H
