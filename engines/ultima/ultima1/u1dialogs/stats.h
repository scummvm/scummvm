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

#ifndef ULTIMA_ULTIMA1_U1DIALOGS_STATS_H
#define ULTIMA_ULTIMA1_U1DIALOGS_STATS_H

#include "ultima/ultima1/u1dialogs/full_screen_dialog.h"
#include "common/array.h"

namespace Ultima {
namespace Ultima1 {
namespace U1Dialogs {

using Shared::CShowMsg;
using Shared::CCharacterInputMsg;

/**
 * Implements the stats/inventory dialog
 */
class Stats : public FullScreenDialog {
	DECLARE_MESSAGE_MAP;
	bool ShowMsg(CShowMsg &msg);
	bool CharacterInputMsg(CCharacterInputMsg &msg);
public:
	/**
	 * Contains the data for a single stat entry to display
	 */
	struct StatEntry {
		Common::String _line;
		byte _color;

		StatEntry() : _color(0) {}
		StatEntry(const Common::String &line, byte color) : _line(line), _color(color) {}
	};

	/**
	 * Format a name/value text with dots between them
	 * @param name		Stat/item name
	 * @param value		The value/quantity
	 * @returns			The formatted display of name dots value
	 */
	static Common::String formatStat(const char *name, uint value);
private:
	Common::Array<StatEntry> _stats;
	uint _startingIndex;
private:
	/**
	 * Loads the list of stats to display into an array
	 */
	void load();

	/**
	 * Add a range of values into the stats list
	 * @param names			Stat names
	 * @param values		Values
	 * @param start			Starting index
	 * @param end			Ending index
	 * @param equippedIndex	Equipped item index, if applicable
	 * @param row			Starting text row
	 * @returns				Ending text row
	 */
	void addStats(const char *const *names, const uint *values, int start, int end, int equippedIndex = -1);
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Stats(Ultima1Game *game) : FullScreenDialog(game), _startingIndex(0) {
		load();
	}

	/**
	 * Draws the visual item on the screen
	 */
	void draw() override;
};

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
