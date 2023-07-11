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

#include "crab/ui/ReplyMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;
using namespace pyrodactyl::music;
using namespace pyrodactyl::event;
using namespace pyrodactyl::people;

void ReplyMenu::load(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("conversation");
		if (nodeValid(node)) {
			if (nodeValid("tone", node))
				tone.load(node->first_node("tone"));

			if (nodeValid("reply", node)) {
				rapidxml::xml_node<char> *replynode = node->first_node("reply");
				Menu<ReplyButton>::load(replynode->first_node("menu"));
				tone.value.resize(element.size());

				bg.load(replynode->first_node("bg"));
				loadNum(spacing, "spacing", replynode);
			}
		}
	}
}

int ReplyMenu::handleEvents(Info &info, ConversationData &dat, const Common::String &cur_id, PersonHandler &oh, const Common::Event &Event) {
	// After that, check if the user has clicked on any reply option
	int choice = Menu<ReplyButton>::handleEvents(Event);
	if (choice >= 0 && (unsigned int)choice < dat._reply.size()) {
		bool play_sound = false;

		// Loop through any opinion changes required
		for (auto &i : dat._reply[element[choice].index]._change) {
			if (i._id == cur_id) {
				// This is a special case because we also need to update the opinion bars
				oh.OpinionChange(info, i._id, OPI_LIKE, i._val[OPI_LIKE]);
				oh.OpinionChange(info, i._id, OPI_RESPECT, i._val[OPI_RESPECT]);
				oh.OpinionChange(info, i._id, OPI_FEAR, i._val[OPI_FEAR]);
				play_sound = true;
			} else {
				info.OpinionChange(i._id, OPI_LIKE, i._val[OPI_LIKE]);
				info.OpinionChange(i._id, OPI_RESPECT, i._val[OPI_RESPECT]);
				info.OpinionChange(i._id, OPI_FEAR, i._val[OPI_FEAR]);
				play_sound = true;
			}
		}

		(void)play_sound;

#if 0
		// Right now we play sound randomly
		if (play_sound) {
			if (gRandom.Num() % 2 == 1)
				info.sound.rep_dec = true;
			else
				info.sound.rep_inc = true;
		}
#endif

		return dat._reply[element[choice].index]._nextid;
	}

	return -1;
}

#if 0
int ReplyMenu::handleEvents(Info &info, ConversationData &dat, const Common::String &cur_id, PersonHandler &oh, const SDL_Event &Event) {
	// After that, check if the user has clicked on any reply option
	int choice = Menu<ReplyButton>::handleEvents(Event);
	if (choice >= 0 && choice < dat._reply.size()) {
		bool play_sound = false;

		// Loop through any opinion changes required
		for (auto &i : dat._reply[element[choice].index].change) {
			if (i.id == cur_id) {
				// This is a special case because we also need to update the opinion bars
				oh.OpinionChange(info, i.id, OPI_LIKE, i.val[OPI_LIKE]);
				oh.OpinionChange(info, i.id, OPI_RESPECT, i.val[OPI_RESPECT]);
				oh.OpinionChange(info, i.id, OPI_FEAR, i.val[OPI_FEAR]);
				play_sound = true;
			} else {
				info.OpinionChange(i.id, OPI_LIKE, i.val[OPI_LIKE]);
				info.OpinionChange(i.id, OPI_RESPECT, i.val[OPI_RESPECT]);
				info.OpinionChange(i.id, OPI_FEAR, i.val[OPI_FEAR]);
				play_sound = true;
			}
		}

		// Right now we play sound randomly
		if (play_sound) {
			if (gRandom.Num() % 2 == 1)
				info.sound.rep_dec = true;
			else
				info.sound.rep_inc = true;
		}

		return dat._reply[element[choice].index].nextid;
	}

	return -1;
}
#endif

void ReplyMenu::draw() {
	bg.draw();
	tone.draw(hover_index);

	// Draw the reply options
	Menu<ReplyButton>::draw();
}

void ReplyMenu::Cache(Info &info, ConversationData &dat) {
	// Some replies are locked, which means the other replies move up and take their place -
	// which is why we need two count variables
	unsigned int reply_count = 0, element_count = 0;

	for (auto i = dat._reply.begin(); i != dat._reply.end() && reply_count < dat._reply.size(); ++i, ++reply_count) {
		if (i->_unlock.Evaluate(info)) {
			element[element_count].visible = true;
			element[element_count].index = reply_count;

			tone.value[element_count] = dat._reply[reply_count]._tone;

			const InputType type = static_cast<InputType>(IU_REPLY_0 + element_count);
			Common::String text = g_engine->_inputManager->GetAssociatedKey(type);
			text += ". " + i->_text;
			info.InsertName(text);

			if (element_count == 0)
				element[element_count].Cache(text, spacing, 0, &bg);
			else
				element[element_count].Cache(text, spacing, element[element_count - 1].y + element[element_count - 1].h, &bg);

			// Increment the element count only if the reply is unlocked
			// This means we will keep checking against element 0 until we find an unlocked reply
			// e.g. if replies 0,1,2 are locked, then element[0] will get reply[3]
			element_count++;
		}
	}

	// Unused element buttons are hidden
	for (; element_count < element.size(); element_count++)
		element[element_count].visible = false;
}

void ReplyMenu::setUI() {
	Menu<ReplyButton>::setUI();
	bg.setUI();
	tone.setUI();
}

} // End of namespace Crab
