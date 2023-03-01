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

#ifndef MM1_VIEWS_ENH_INTERACTIONS_INTERACTION_H
#define MM1_VIEWS_ENH_INTERACTIONS_INTERACTION_H

#include "mm/mm1/views_enh/scroll_view.h"
#include "mm/shared/xeen/sprites.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Interactions {

class Interaction : public ScrollView {
private:
	Shared::Xeen::SpriteResource _frame;
	Shared::Xeen::SpriteResource _portrait;
	int _tickCtr = 0;
	int _portraitFrameNum = 0;
protected:
	Common::StringArray _lines;
	bool _animated = true;
	int _portraitNum = 0;
protected:
	/**
	 * Handles any action/press
	 */
	virtual void viewAction() {
		leave();
	}

	/**
	 * Adds text for display
	 */
	void addText(const Common::String &str);

public:
	Interaction(const Common::String &name, int portrait);

	/**
	 * Handles game messages
	 */
	bool msgGame(const GameMessage &msg) override;

	/**
	 * Unfocuses the view
	 */
	bool msgUnfocus(const UnfocusMessage &msg) override;

	/**
	 * Draw the location
	 */
	void draw() override;

	/**
	 * Tick handler
	 */
	bool tick() override;

	/**
	 * Leave the location, turning around
	 */
	void leave();

	/**
	 * Keypress handler
	 */
	bool msgKeypress(const KeypressMessage &msg) override;

	/**
	 * Action handler
	 */
	bool msgAction(const ActionMessage &msg) override;

	/**
	 * Mouse click handler
	 */
	bool msgMouseDown(const MouseDownMessage &msg) override;
};

} // namespace Interactions
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
