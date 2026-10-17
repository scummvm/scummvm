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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_CYBER_H
#define RIPPER_CYBER_H

#include "common/array.h"
#include "common/str.h"

namespace Ripper {

class RipperEngine;

class CyberManager {
public:
	enum Result {
		kExited,
		kLoadFailed
	};

	explicit CyberManager(RipperEngine *engine);

	Result run();
	Result runProgram(uint action, const char *scriptBaseName, uint argument);

private:
	struct Session;

	bool suspendSession(Session &session) const;
	void prepareSession(int selectionIndex = -1) const;
	bool runScriptLoop(bool active, uint action = 0,
		const Common::String &programName = Common::String()) const;
	Result restoreSession(Session &session, uint selectionIndex, Result result) const;
	bool captureAudio(Common::Array<byte> &state) const;
	bool restoreAudio(const Common::Array<byte> &state) const;

	RipperEngine *_engine;
};

} // End of namespace Ripper

#endif // RIPPER_CYBER_H
