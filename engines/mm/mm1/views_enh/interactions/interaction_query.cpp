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

#include "mm/mm1/views_enh/interactions/interaction_query.h"
#include "mm/mm1/views_enh/text_entry.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

InteractionQuery::InteractionQuery(const Common::String &name,
		int maxChars, int portrait) : Interaction(name, portrait),
		_maxChars(maxChars) {
}

bool InteractionQuery::msgFocus(const FocusMessage &msg) {
	Interaction::msgFocus(msg);
	_showEntry = dynamic_cast<TextEntry *>(msg._priorView) == nullptr;
	return true;
}

void InteractionQuery::draw() {
	Interaction::draw();
	if (!_showEntry)
		return;

	int xp = (_innerBounds.width() / 2) - (_maxChars * 8 / 2);
	int yp = (8 + _lines.size()) * 8;

	_textEntry.display(xp + _innerBounds.left, yp + _innerBounds.top,
		_maxChars, false,
		[]() {
			auto *view = static_cast<InteractionQuery *>(g_events->focusedView());
			view->answerEntry("");
		},
		[](const Common::String &answer) {
			auto *view = static_cast<InteractionQuery *>(g_events->focusedView());
			view->answerEntry(answer);
		}
	);
}

void InteractionQuery::answerEntry(const Common::String &answer) {
	close();

	_answer = answer;
	answerEntered();
}


} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
