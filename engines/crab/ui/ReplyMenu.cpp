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
				_tone.load(node->first_node("tone"));

			if (nodeValid("reply", node)) {
				rapidxml::xml_node<char> *replynode = node->first_node("reply");
				Menu<ReplyButton>::load(replynode->first_node("menu"));
				_tone._value.resize(_element.size());

				_bg.load(replynode->first_node("bg"));
				loadNum(_spacing, "spacing", replynode);
			}
		}
	}
}

int ReplyMenu::handleEvents(Info &info, ConversationData &dat, const Common::String &curId, PersonHandler &oh, const Common::Event &event) {
	// After that, check if the user has clicked on any reply option
	int choice = Menu<ReplyButton>::handleEvents(event);
	if (choice >= 0 && (unsigned int)choice < dat._reply.size()) {
		bool playSound = false;

		// Loop through any opinion changes required
		for (auto &i : dat._reply[_element[choice]._index]._change) {
			if (i._id == curId) {
				// This is a special case because we also need to update the opinion bars
				oh.opinionChange(info, i._id, OPI_LIKE, i._val[OPI_LIKE]);
				oh.opinionChange(info, i._id, OPI_RESPECT, i._val[OPI_RESPECT]);
				oh.opinionChange(info, i._id, OPI_FEAR, i._val[OPI_FEAR]);
				playSound = true;
			} else {
				info.opinionChange(i._id, OPI_LIKE, i._val[OPI_LIKE]);
				info.opinionChange(i._id, OPI_RESPECT, i._val[OPI_RESPECT]);
				info.opinionChange(i._id, OPI_FEAR, i._val[OPI_FEAR]);
				playSound = true;
			}
		}

		// Right now we play sound randomly
		if (playSound) {
			if (g_engine->getRandomNumber(1))
				info._sound._repDec = true;
			else
				info._sound._repInc = true;
		}

		return dat._reply[_element[choice]._index]._nextid;
	}

	return -1;
}

void ReplyMenu::draw() {
	_bg.draw();
	_tone.draw(_hoverIndex);

	// Draw the reply options
	Menu<ReplyButton>::draw();
}

void ReplyMenu::cache(pyrodactyl::event::Info &info, pyrodactyl::event::ConversationData &dat) {
	// Some replies are locked, which means the other replies move up and take their place -
	// which is why we need two count variables
	unsigned int replyCount = 0, elementCount = 0;

	for (auto i = dat._reply.begin(); i != dat._reply.end() && replyCount < dat._reply.size(); ++i, ++replyCount) {
		if (i->_unlock.evaluate(info)) {
			_element[elementCount]._visible = true;
			_element[elementCount]._index = replyCount;

			_tone._value[elementCount] = dat._reply[replyCount]._tone;

			const InputType type = static_cast<InputType>(IU_REPLY_0 + elementCount);
			Common::String text = g_engine->_inputManager->getAssociatedKey(type);
			text += ". " + i->_text;
			info.insertName(text);

			if (elementCount == 0)
				_element[elementCount].cache(text, _spacing, 0, &_bg);
			else
				_element[elementCount].cache(text, _spacing, _element[elementCount - 1].y + _element[elementCount - 1].h, &_bg);

			// Increment the element count only if the reply is unlocked
			// This means we will keep checking against element 0 until we find an unlocked reply
			// e.g. if replies 0,1,2 are locked, then element[0] will get reply[3]
			elementCount++;
		}
	}

	// Unused element buttons are hidden
	for (; elementCount < _element.size(); elementCount++)
		_element[elementCount]._visible = false;
}

void ReplyMenu::setUI() {
	Menu<ReplyButton>::setUI();
	_bg.setUI();
	_tone.setUI();
}

} // End of namespace Crab
