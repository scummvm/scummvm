/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "mutationofjb/widgets/conversationwidget.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/guiscreen.h"
#include "mutationofjb/font.h"
#include "common/events.h"

namespace MutationOfJB {

enum {
	CONVERSATION_LINES_X = 5,
	CONVERSATION_LINES_Y = 151,
	CONVERSATION_LINE_HEIGHT = 12
};

ConversationWidget::ConversationWidget(GuiScreen &gui, const Common::Rect &area, const Graphics::Surface &surface) :
	Widget(gui, area),
	_surface(surface),
	_callback(nullptr) {}


void ConversationWidget::setChoice(int choiceNo, const Common::String &str, uint32 data) {
	if (choiceNo >= CONVERSATION_MAX_CHOICES) {
		return;
	}

	_choices[choiceNo]._str = str;
	_choices[choiceNo]._data = data;
	markDirty();
}

void ConversationWidget::clearChoices() {
	for (int i = 0; i < CONVERSATION_MAX_CHOICES; ++i) {
		_choices[i]._str.clear();
		_choices[i]._data = 0;
	}
	markDirty();
}

void ConversationWidget::draw(Graphics::ManagedSurface &surface) {
	surface.blitFrom(_surface, Common::Point(_area.left, _area.top));

	for (int i = 0; i < CONVERSATION_MAX_CHOICES; ++i) {
		Common::String &str = _choices[i]._str;
		if (str.empty()) {
			continue;
		}

		// TODO: Active line should be WHITE.
		_gui.getGame().getAssets().getSystemFont().drawString(&surface, str, CONVERSATION_LINES_X, CONVERSATION_LINES_Y + i * CONVERSATION_LINE_HEIGHT, _area.width(), LIGHTGRAY);
	}
}

void ConversationWidget::handleEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN: {
		const int16 x = event.mouse.x;
		const int16 y = event.mouse.y;
		if (_area.contains(x, y)) {
			if (_callback) {
				int choiceNo = (y - CONVERSATION_LINES_Y) / CONVERSATION_LINE_HEIGHT;
				if (!_choices[choiceNo]._str.empty()) {
					_callback->onChoiceClicked(this, choiceNo, _choices[choiceNo]._data);
				}
			}
		}
		break;
	}
	default:
		break;
	}
}

}
