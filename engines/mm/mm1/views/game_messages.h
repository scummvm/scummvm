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

#ifndef MM1_VIEWS_GAME_MESSAGES_H
#define MM1_VIEWS_GAME_MESSAGES_H

#include "mm/mm1/messages.h"
#include "mm/mm1/views/text_view.h"

namespace MM {
namespace MM1 {
namespace Views {

class GameMessages : public TextView {
private:
	Common::Array<Line> _lines;
	YNCallback _ynCallback = nullptr;
	KeyCallback _keyCallback = nullptr;
public:
	GameMessages(UIElement *owner);
	virtual ~GameMessages() {}

	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;
	bool msgInfo(const InfoMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	void timeout() override;
};

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
