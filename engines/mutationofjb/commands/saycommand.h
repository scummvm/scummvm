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

#ifndef MUTATIONOFJB_SAYCOMMAND_H
#define MUTATIONOFJB_SAYCOMMAND_H

#include "mutationofjb/commands/seqcommand.h"
#include "common/str.h"

namespace MutationOfJB {

class SayCommandParser : public SeqCommandParser {
public:
	SayCommandParser() {}

	bool parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) override;
};

class SayCommand : public SeqCommand {
public:
	SayCommand(const Common::String &lineToSay, const Common::String &voiceFile, bool waitForPrevious, bool talkingAnimation) :
		_lineToSay(lineToSay),
		_voiceFile(voiceFile),
		_waitForPrevious(waitForPrevious),
		_talkingAnimation(talkingAnimation) {}
	ExecuteResult execute(ScriptExecutionContext &scriptExecCtx) override;
	Common::String debugString() const override;
private:
	Common::String _lineToSay;
	Common::String _voiceFile;
	bool _waitForPrevious;
	bool _talkingAnimation;
};

}

#endif
