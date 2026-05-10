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

#ifndef ULTIMA0_VIEWS_INFO_H
#define ULTIMA0_VIEWS_INFO_H

#include "ultima/ultima0/views/view.h"
#include "ultima/ultima0/data/data.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

class Info : public View {
	class InfoObject : public UIElement {
	public:
		Common::String _text;
		int _id;
	public:
		InfoObject(Info *parent, const Common::Point &pt, int id, const Common::String &text);
		void draw() override;
		bool msgMouseDown(const MouseDownMessage &msg) override;
	};

private:
	InfoObject _options[6] = {
		InfoObject(this, Common::Point(26, 4), 0, OBJECT_INFO[0]._name),
		InfoObject(this, Common::Point(26, 5), 1, OBJECT_INFO[1]._name),
		InfoObject(this, Common::Point(26, 6), 2, OBJECT_INFO[2]._name),
		InfoObject(this, Common::Point(26, 7), 3, OBJECT_INFO[3]._name),
		InfoObject(this, Common::Point(26, 8), 4, OBJECT_INFO[4]._name),
		InfoObject(this, Common::Point(26, 9), 5, OBJECT_INFO[5]._name)
	};

protected:
	virtual void selectObject(int item) {
	}
	virtual void leave();

public:
	Info(const char *viewName = "Info");
	~Info() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
};

} // namespace Views
} // namespace Ultima0
} // namespace Ultima

#endif
