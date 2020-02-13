/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SHERLOCK_TATTOO_WIDGET_TALK_H
#define SHERLOCK_TATTOO_WIDGET_TALK_H

#include "common/scummsys.h"
#include "sherlock/tattoo/widget_base.h"

namespace Sherlock {

class SherlockEngine;

namespace Tattoo {

enum Highlight { HL_NO_HIGHLIGHTING, HL_CHANGED_HIGHLIGHTS, HL_SCROLLBAR_ONLY };

/**
 * Handles displaying a dialog with conversation options the player can select from
 */
class WidgetTalk: public WidgetBase {
	struct StatementLine {
		Common::String _line;
		int _num;

		StatementLine() : _num(0) {}
		StatementLine(const Common::String &line, int num) : _line(line), _num(num) {}
	};
private:
	int _talkScrollIndex;
	Common::Array<StatementLine> _statementLines;
	int _selector, _oldSelector;
	int _talkTextX;
	uint32 _dialogTimer;

	/**
	 * Get the needed size for a talk window
	 */
	void getTalkWindowSize();

	/**
	 * Re-renders the contenst of the window to the widget's surface
	 */
	void render(Highlight highlightMode);

	/**
	 * This initializes the _statementLines array, which contains the talk options split up line
	 * by line, as well as which statement a particular line is part of.
	 */
	void setStatementLines();
public:
	WidgetTalk(SherlockEngine *vm);
	~WidgetTalk() override {}

	/**
	 * Figures out how many lines the available talk lines will take up, and opens a text window
	 * of appropriate size
	 */
	void load();

	/**
	 * Refresh the talk display
	 */
	void refresh();

	/**
	 * Handle event processing
	 */
	void handleEvents() override;
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
