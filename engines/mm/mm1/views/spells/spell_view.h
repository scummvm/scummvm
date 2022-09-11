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

#ifndef MM1_VIEWS_SPELLS_SPELL_VIEW_H
#define MM1_VIEWS_SPELLS_SPELL_VIEW_H

#include "common/str.h"
#include "mm/mm1/views/text_view.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Spells {

class SpellView : public TextView {
public:
	SpellView(const Common::String &name) : TextView(name) {}
	SpellView(const Common::String &name, UIElement *owner) :
		TextView(name, owner) {}
	virtual ~SpellView() {}

	/**
	 * Spell failed display
	 */
	void spellFailed();

	/**
	 * Spell done display
	 */
	void spellDone();

	/**
	 * Timeout for the failed/done display
	 */
	void timeout() override;

	/**
	 * Called when the view is focused
	 */
	bool msgFocus(const FocusMessage &msg) override;
};

} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM

#endif
