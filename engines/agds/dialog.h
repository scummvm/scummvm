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

#ifndef DIALOG_H
#define DIALOG_H

#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace AGDS {
class AGDSEngine;

class Dialog {
	using DialogDefsType = Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>;

private:
	struct Sound {
		Common::String Name;
		Common::String Sample;
		int Step;

		Sound() : Step(1) {
		}

		Sound(const Common::String &name, const Common::String &sample, int step) : Name(name), Sample(sample), Step(step) {
		}
	};
	using SoundsType = Common::Array<Sound>;

	AGDSEngine *_engine;
	DialogDefsType _dialogDefs;
	Common::String _dialogScript;
	uint32 _dialogScriptPos;
	Common::String _dialogProcessName;
	Common::String _dialogLine;

	SoundsType _sounds;
	Common::String _currentDef;
	int _currentSoundIndex;

	void parseDialogDefs(const Common::String &defs);

public:
	Dialog(AGDSEngine *engine) : _engine(engine), _dialogScriptPos(0), _currentSoundIndex(-1) {}
	int textDelay(const Common::String &str);
	const Common::String &getNextDialogLine() const {
		return _dialogLine;
	}
	Common::String getNextDialogSound();

	void load(const Common::String &processName, const Common::String &dialogScript, const Common::String &defs);
	bool tick();

private:
	void processSoundDirective(const Common::String &line);
	void processDirective(Common::String line);
};

} // End of namespace AGDS

#endif /* AGDS_DIALOG_H */
