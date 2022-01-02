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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_CONSOLE_H
#define BURIED_CONSOLE_H

#include "gui/debugger.h"

#include "buried/navdata.h"

namespace Buried {

class BuriedEngine;
class FrameWindow;

class BuriedConsole : public GUI::Debugger {
public:
	BuriedConsole(BuriedEngine *vm);
	~BuriedConsole();

	bool cmdGiveItem(int argc, const char **argv);
	bool cmdRemoveItem(int argc, const char **argv);
	bool cmdJumpEntry(int argc, const char **argv);
	bool cmdCurLocation(int argc, const char **argv);
	bool cmdAiCommentInfo(int argc, const char **argv);

protected:
	void postEnter();

private:
	BuriedEngine *_vm;

	struct JumpEntry {
		Common::String timeZoneName;
		Common::String locationName;
		Location location;
	};

	typedef Common::Array<JumpEntry> JumpEntryList;
	JumpEntryList _jumpEntryList;
	void loadJumpEntryList();

	FrameWindow *getFrameWindow();
	bool isPlaying() { return getFrameWindow() != 0; }

	Location _jump;
};

} // End of namespace Buried

#endif
