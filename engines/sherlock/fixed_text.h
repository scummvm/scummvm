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

#ifndef SHERLOCK_FIXED_TEXT_H
#define SHERLOCK_FIXED_TEXT_H

#include "common/scummsys.h"
#include "common/language.h"

namespace Sherlock {

enum FixedTextId {
	// Window buttons
	kFixedText_Window_Exit = 0,
	kFixedText_Window_Up,
	kFixedText_Window_Down,
	// Inventory buttons
	kFixedText_Inventory_Exit,
	kFixedText_Inventory_Look,
	kFixedText_Inventory_Use,
	kFixedText_Inventory_Give,
	// Journal text
	kFixedText_Journal_WatsonsJournal,
	kFixedText_Journal_Page,
	// Journal buttons
	kFixedText_Journal_Exit,
	kFixedText_Journal_Back10,
	kFixedText_Journal_Up,
	kFixedText_Journal_Down,
	kFixedText_Journal_Ahead10,
	kFixedText_Journal_Search,
	kFixedText_Journal_FirstPage,
	kFixedText_Journal_LastPage,
	kFixedText_Journal_PrintText,
	// Journal search
	kFixedText_JournalSearch_Exit,
	kFixedText_JournalSearch_Backward,
	kFixedText_JournalSearch_Forward,
	kFixedText_JournalSearch_NotFound
};

enum FixedTextActionId {
	kFixedTextAction_Invalid = -1,
	kFixedTextAction_Open = 0,
	kFixedTextAction_Close,
	kFixedTextAction_Move,
	kFixedTextAction_Pick,
	kFixedTextAction_Use
};

struct FixedTextActionEntry {
	const Common::String *fixedTextArray;
};

struct FixedTextLanguageEntry {
	Common::Language language;
	const Common::String *fixedTextArray;
	const FixedTextActionEntry *actionArray;
};

class FixedText {
private:
	SherlockEngine *_vm;

	const FixedTextLanguageEntry *_curLanguageEntry;

public:
	FixedText(SherlockEngine *vm);
	~FixedText() {}

	/**
	 * Gets hardcoded text
	 */
	const Common::String getText(FixedTextId fixedTextId);

	const Common::String getActionMessage(FixedTextActionId actionId, int messageIndex);
};

} // End of namespace Sherlock

#endif
