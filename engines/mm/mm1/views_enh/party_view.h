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

#ifndef MM1_VIEWS_ENH_PARTY_VIEW_H
#define MM1_VIEWS_ENH_PARTY_VIEW_H

#include "mm/mm1/views_enh/scroll_view.h"
#include "mm/mm1/data/character.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

class PartyView : public ScrollView {
protected:
	/**
	 * Return true if the selected character can be switched
	 */
	virtual bool canSwitchChar() {
		return true;
	}

	/**
	 * Returns true if the destination character can be switched to
	 */
	virtual bool canSwitchToChar(Character *dst) {
		return true;
	}

	/**
	 * Return true if a character should be selected by default
	 */
	virtual bool selectCharByDefault() const {
		return true;
	}

	/**
	 * Called when the selected character has been switched
	 */
	virtual void charSwitched(Character *priorChar);

public:
	PartyView(const Common::String &name) : ScrollView(name) {}
	PartyView(const Common::String &name, UIElement *owner) :
		ScrollView(name, owner) {}
	virtual ~PartyView() {}

	void draw() override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
