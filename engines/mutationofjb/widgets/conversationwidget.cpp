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
#include "mutationofjb/gui.h"
#include "mutationofjb/font.h"

namespace MutationOfJB {

enum {
	CONVERSATION_LINES_X = 5,
	CONVERSATION_LINES_Y = 151,
	CONVERSATION_LINE_HEIGHT = 12
};

ConversationWidget::ConversationWidget(Gui &gui, const Common::Rect &area, const Graphics::Surface &surface) :
	Widget(gui, area),
	_surface(surface) {}


void ConversationWidget::setLine(int lineNo, const Common::String &str) {
	if (lineNo >= CONVERSATION_LINES) {
		return;
	}

	_lines[lineNo] = str;
	markDirty();
}

void ConversationWidget::_draw(Graphics::ManagedSurface &surface) {
	surface.blitFrom(_surface, Common::Point(_area.left, _area.top));

	for (int i = 0; i < CONVERSATION_LINES; ++i) {
		Common::String &line = _lines[i];
		if (line.empty()) {
			continue;
		}

		// TODO: Active line should be Gui::WHITE.
		_gui.getGame().getSystemFont().drawString(line, Gui::LIGHTGRAY, CONVERSATION_LINES_X, CONVERSATION_LINES_Y + i * CONVERSATION_LINE_HEIGHT, surface);
	}
}

}

