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

#ifndef MM1_VIEWS_ENH_CHARACTER_INFO_H
#define MM1_VIEWS_ENH_CHARACTER_INFO_H

#include "mm/mm1/views_enh/party_view.h"
#include "mm/mm1/views_enh/scroll_popup.h"
#include "mm/shared/xeen/sprites.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define CHAR_ICONS_COUNT 22

class CharacterInfo : public PartyView {
	struct IconPos {
		int _frame; int _x; int _y;
	};
private:
	Shared::Xeen::SpriteResource _viewIcon;
	static const IconPos ICONS[CHAR_ICONS_COUNT];
	Common::String ICONS_TEXT[CHAR_ICONS_COUNT];
	int _cursorCell = 0;
	bool _cursorVisible = false;
	ScrollPopup _statInfo;
private:
	/**
	 * Draw the title text
	 */
	void drawTitle();

	/**
	 * Draw the icons
	 */
	void drawIcons();

	/**
	 * Draw the stats
	 */
	void drawStats();

	/**
	 * Toggle display of cursor
	 */
	void showCursor(bool flag);

	/**
	 * Show the details of a given attribute
	 */
	void showAttribute(int attrib);

protected:
	void timeout() override;

public:
	CharacterInfo();
	virtual ~CharacterInfo() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;
	void draw() override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
