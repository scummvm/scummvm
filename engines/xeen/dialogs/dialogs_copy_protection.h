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

#ifndef XEEN_DIALOGS_COPY_PROTECTION_H
#define XEEN_DIALOGS_COPY_PROTECTION_H

#include "xeen/dialogs/dialogs_input.h"
#include "common/array.h"

namespace Xeen {

class CopyProtection : public Input {
	struct ProtectionEntry {
		uint8 _pageNum;
		uint8 _lineNum;
		uint8 _wordNum;
		Common::String _text;
	};
private:
	Common::Array<ProtectionEntry> _entries;
private:
	/**
	 * Constructor
	 */
	CopyProtection(XeenEngine *vm);

	/**
	 * Execute the dialog
	 */
	bool execute();

	/**
	 * Load the copy protection entries
	 */
	void loadEntries();
public:
	/**
	 * Show the dialog
	 */
	static bool show(XeenEngine *vm);
};

} // End of namespace Xeen

#endif /* XEEN_DIALOGS_COPY_PROTECTION_H */
