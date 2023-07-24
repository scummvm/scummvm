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

#include "crab/common_header.h"
#include "crab/event/EventSeqGroup.h"
#include "crab/level/level.h"
#include "crab/ui/ChapterIntro.h"
#include "crab/ui/PersonHandler.h"
#include "crab/ui/PersonScreen.h"
#include "crab/ui/ReplyMenu.h"
#include "crab/ui/hud.h"
#include "crab/ui/journal.h"
#include "crab/ui/textarea.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
class Manager {
protected:
	// All the events in the game
	Common::HashMap<Common::String, EventSeqGroup> _eventMap;

	// The currently happening or active sequence
	unsigned int _activeSeq;

	// THIS IS NOT THE DEFINITIVE LIST OF ENDED SEQUENCES
	// JUST A TEMPORARY LIST OF EVENT SEQUENCES TO PASS AROUND
	Common::Array<EventSeqInfo> _endSeq;

	// The objects used to draw the dialog box and opinion bars
	pyrodactyl::ui::PersonHandler _oh;

	// The _reply menu and the colors and font of the text
	pyrodactyl::ui::ReplyMenu _reply;

	// The field for text input
	pyrodactyl::ui::TextArea _textin;

	// The info for intro events
	pyrodactyl::ui::ChapterIntro _intro;

	// Store the current event data temporarily
	GameEvent *_curEvent;
	bool _player;
	pyrodactyl::anim::Sprite *_curSp;

	void updateDialogBox(Info &info, pyrodactyl::level::Level &level);

public:
	// The object used to draw the character screen
	pyrodactyl::ui::PersonScreen _per;

	// A flag used to stop drawing the game for a better fade in/fade out experience
	bool _drawGame;

	Manager() {
		init();
	}

	~Manager() {}

	void init();
	void load(rapidxml::xml_node<char> *node, pyrodactyl::ui::ParagraphData &popup);

	void draw(Info &info, pyrodactyl::ui::HUD &hud, pyrodactyl::level::Level &level);

	// cur_per is also updated here
	void internalEvents(Info &info, pyrodactyl::level::Level &level, Common::Array<EventResult> &result);

	void handleEvents(Info &info, const Common::String &playerId, Common::Event &event,
					  pyrodactyl::ui::HUD &hud, pyrodactyl::level::Level &level, Common::Array<EventResult> &result);

	void calcActiveSeq(Info &info, pyrodactyl::level::Level &level, const Rect &camera);

	void endSequence(const Common::String &curloc);
	bool eventInProgress();

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void loadState(rapidxml::xml_node<char> *node);

	void setUI();
};
} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_GAMEEVENTMANAGER_H
