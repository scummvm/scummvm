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

#ifndef ULTIMA2_VIEWS_ZTATS_H
#define ULTIMA2_VIEWS_ZTATS_H

#include "common/array.h"
#include "common/str.h"
#include "ultima/ultima1/views/dialog.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {

using namespace Shared::Messages;

class ZStats : public Dialog {
private:
	struct StatLine {
		Common::String _text;
		byte _color;
	};

	// All the lines to display, spanning however many pages are needed.
	// Rebuilt fresh each time the dialog is focused
	Common::Array<StatLine> _lines;

	// Index into _lines where the currently displayed page starts
	uint _pageStart = 0;

	/**
	 * Builds the full set of stat lines from the current savegame
	 */
	void buildLines();

	/**
	 * Adds a single "name....value" line if value is non-zero, in the
	 * highlight color if highlighted is true
	 */
	void addLine(const char *name, int value, bool highlighted = false);

	/**
	 * Formats a single "name....value" line
	 */
	Common::String getLine(const char *name, int value);

	/**
	 * Returns the text-cell position for a line at a given index within
	 * the current page (0-25) - a two-column layout of 13 rows each
	 */
	Common::Point linePos(int lineNum) const;

	void keyPressed();

public:
	ZStats() : Dialog("ZStats") {
	}
	~ZStats() override {
	}

	void draw() override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
