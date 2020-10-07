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

#ifndef AGDS_DIALOG_H
#define AGDS_DIALOG_H

#include "common/scummsys.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/str.h"

namespace AGDS {
class AGDSEngine;

class Dialog {
	typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> DialogDefsType;

private:
	AGDSEngine * 				_engine;
	DialogDefsType				_dialogDefs;
	Common::String				_dialogScript;
	uint32						_dialogScriptPos;
	Common::String				_dialogProcessName;
	Common::String				_dialogLine;

	void parseDialogDefs(const Common::String &defs);

public:
	Dialog(AGDSEngine *engine): _engine(engine), _dialogScriptPos(0) { }
	void run(const Common::String &dialogProcess);
	int textDelay(const Common::String &str);
	const Common::String &getNextDialogLine() const {
		return _dialogLine;
	}

	void load(const Common::String &dialogScript, const Common::String & defs);
	bool tick();
};


} // End of namespace AGDS

#endif /* AGDS_DIALOG_H */
