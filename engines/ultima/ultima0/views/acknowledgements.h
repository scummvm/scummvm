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

#ifndef ULTIMA0_VIEWS_ACKNOWLEDGEMENTS_H
#define ULTIMA0_VIEWS_ACKNOWLEDGEMENTS_H

#include "common/queue.h"
#include "ultima/ultima0/views/view.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

class Acknowledgements : public View {
private:
	Common::Queue<Common::String> _lines;
	Gfx::GfxSurface _pendingLine;
	int _ctr = 0;

	void showTitle() {
		replaceView("Title");
	}
public:
	Acknowledgements() : View("Acknowledgements") {}
	~Acknowledgements() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool tick() override;
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
