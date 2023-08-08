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

#ifndef CRAB_REPLYMENU_H
#define CRAB_REPLYMENU_H

#include "crab/event/conversationdata.h"
#include "crab/event/eventstore.h"
#include "crab/event/GameEventInfo.h"
#include "crab/ui/emotion.h"
#include "crab/ui/ImageData.h"
#include "crab/ui/PersonHandler.h"
#include "crab/ui/ReplyButton.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class ReplyMenu : public Menu<ReplyButton> {
	// Data about the background image
	ImageData _bg;

	// The minimum spacing between two reply choices
	int _spacing;

	// The emotion indicator used to indicate the type of reply selected
	EmotionIndicator _tone;

public:
	ReplyMenu() {
		_spacing = 20;
	}

	~ReplyMenu() {}

	void load(const Common::String &filename);

	int handleEvents(pyrodactyl::event::Info &info, pyrodactyl::event::ConversationData &dat,
					 const Common::String &curId, PersonHandler &oh, const Common::Event &Event);

	void draw();
	void cache(pyrodactyl::event::Info &info, pyrodactyl::event::ConversationData &dat);

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_REPLYMENU_H
