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

#ifndef ULTIMA0_VIEWS_DEAD_H
#define ULTIMA0_VIEWS_DEAD_H

#include "ultima/ultima0/views/view.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

class Dead : public View {
private:
	void showTitle() {
		replaceView("Title");
	}
public:
	Dead();
	~Dead() override {}

	void draw() override;

	bool msgKeypress(const KeypressMessage &msg) override {
		showTitle();
		return true;
	}
	bool msgMouseDown(const MouseDownMessage &msg) override {
		showTitle();
		return true;
	}
	bool msgAction(const ActionMessage &msg) override {
		showTitle();
		return true;
	}
};

} // namespace Views
} // namespace Ultima0
} // namespace Ultima

#endif
