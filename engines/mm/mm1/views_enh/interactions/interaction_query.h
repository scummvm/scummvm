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

#ifndef MM1_VIEWS_ENH_INTERACTIONS_INTERACTION_QUERY_H
#define MM1_VIEWS_ENH_INTERACTIONS_INTERACTION_QUERY_H

#include "mm/mm1/views_enh/interactions/interaction.h"
#include "mm/mm1/views_enh/text_entry.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

class InteractionQuery : public Interaction {
private:
	TextEntry _textEntry;
	int _maxChars = 0;

protected:
	bool _showEntry = false;
	Common::String _answer;

	/**
	 * Answer entered
	 */
	virtual void answerEntered() = 0;

public:
	InteractionQuery(const Common::String &name,
		int maxChars, int portrait = -1);

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;

	/**
	 * Passed the entered text
	 */
	void answerEntry(const Common::String &answer);
};

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
