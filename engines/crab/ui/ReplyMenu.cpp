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

#include "crab/ReplyMenu.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;
using namespace pyrodactyl::music;
using namespace pyrodactyl::event;
using namespace pyrodactyl::people;

void ReplyMenu::Load(const std::string &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("conversation");
		if (NodeValid(node)) {
			if (NodeValid("tone", node))
				tone.Load(node->first_node("tone"));

			if (NodeValid("reply", node)) {
				rapidxml::xml_node<char> *replynode = node->first_node("reply");
				Menu<ReplyButton>::Load(replynode->first_node("menu"));
				tone.value.resize(element.size());

				bg.Load(replynode->first_node("bg"));
				LoadNum(spacing, "spacing", replynode);
			}
		}
	}
}

int ReplyMenu::HandleEvents(Info &info, ConversationData &dat, const std::string &cur_id, PersonHandler &oh, const SDL_Event &Event) {
	// After that, check if the user has clicked on any reply option
	int choice = Menu<ReplyButton>::HandleEvents(Event);
	if (choice >= 0 && choice < dat.reply.size()) {
		bool play_sound = false;

		// Loop through any opinion changes required
		for (auto &i : dat.reply[element.at(choice).index].change) {
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

		return dat.reply[element.at(choice).index].nextid;
	}

	return -1;
}

void ReplyMenu::Draw() {
	bg.Draw();
	tone.Draw(hover_index);

	// Draw the reply options
	Menu<ReplyButton>::Draw();
}

void ReplyMenu::Cache(Info &info, ConversationData &dat) {
	// Some replies are locked, which means the other replies move up and take their place -
	// which is why we need two count variables
	unsigned int reply_count = 0, element_count = 0;

	for (auto i = dat.reply.begin(); i != dat.reply.end() && reply_count < dat.reply.size(); ++i, ++reply_count) {
		if (i->unlock.Evaluate(info)) {
			element.at(element_count).visible = true;
			element.at(element_count).index = reply_count;

			tone.value.at(element_count) = dat.reply.at(reply_count).tone;

			std::string text = SDL_GetScancodeName(gInput.iv[IU_REPLY_0 + element_count].key);
			text += ". " + i->text;
			info.InsertName(text);

			if (element_count == 0)
				element.at(element_count).Cache(text, spacing, 0, &bg);
			else
				element.at(element_count).Cache(text, spacing, element.at(element_count - 1).y + element.at(element_count - 1).h, &bg);

			// Increment the element count only if the reply is unlocked
			// This means we will keep checking against element 0 until we find an unlocked reply
			// e.g. if replies 0,1,2 are locked, then element[0] will get reply[3]
			element_count++;
		}
	}

	// Unused element buttons are hidden
	for (; element_count < element.size(); element_count++)
		element.at(element_count).visible = false;
}

void ReplyMenu::SetUI() {
	Menu<ReplyButton>::SetUI();
	bg.SetUI();
	tone.SetUI();
}
