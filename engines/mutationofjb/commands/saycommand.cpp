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

#include "mutationofjb/commands/saycommand.h"

#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/script.h"
#include "mutationofjb/tasks/saytask.h"
#include "mutationofjb/tasks/taskmanager.h"

#include "common/str.h"
#include "common/debug.h"
#include "common/debug-channels.h"

/** @file
 * <firstLine> { <CRLF> <additionalLine> }
 *
 * firstLine ::= ("SM" | "SLM" | "NM" | "NLM") " " <lineToSay> [ "<" <voiceFile> | "<!" ]
 * additionalLine ::= <skipped> " " <lineToSay> ( "<" <voiceFile> | "<!" )
 *
 * Say command comes in four variants: SM, SLM, NM and NLM.
 * Note: In script files, they are usually written as *SM.
 * The asterisk is ignored by the readLine function.
 *
 * Each of them plays a voice file (if present) and/or shows a message
 * (if voice file not present or subtitles are enabled).
 *
 * The difference between versions starting with "S" and "N" is that
 * the "N" version does not show talking animation.
 *
 * The "L" versions are "blocking", i.e. they wait for the previous say command to finish.
 *
 * If the line ends with "<!", it means the message continues to the next line.
 * Next line usually has "SM" (or other variant) repeated, but it does not have to.
 * Then we have the rest of the string to say (which is concatenated with the previous line)
 * and possibly the voice file or "<!" again.
 */

namespace MutationOfJB {

bool SayCommandParser::parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) {
	bool waitForPrevious = false;
	bool talkingAnimation = false;

	if (line.hasPrefix("SM")) {
		waitForPrevious = false;
		talkingAnimation = true;
	} else if (line.hasPrefix("SLM")) {
		waitForPrevious = true;
		talkingAnimation = true;
	} else if (line.hasPrefix("NM")) {
		waitForPrevious = false;
		talkingAnimation = false;
	} else if (line.hasPrefix("NLM")) {
		waitForPrevious = true;
		talkingAnimation = false;
	} else {
		return false;
	}

	Common::String currentLine = line;

	Common::String lineToSay;
	Common::String voiceFile;

	bool cont = false;
	bool firstPass = true;

	do {
		cont = false;

		uint startPos;
		for (startPos = 0; startPos < currentLine.size(); ++startPos) {
			if (currentLine[startPos] == ' ') {
				break;
			}
		}
		if (startPos == currentLine.size()) {
			if (!firstPass) {
				warning("Unable to parse line '%s'", currentLine.c_str());
				break;
			}
		}
		if (startPos != currentLine.size()) {
			startPos++;
		}

		uint endPos;
		for (endPos = startPos; endPos < currentLine.size(); ++endPos) {
			if (currentLine[endPos] == '<') {
				break;
			}
		}

		Common::String talkStr(currentLine.c_str() + startPos, endPos - startPos);

		if (endPos != currentLine.size()) {
			const char *end = currentLine.c_str() + endPos + 1;
			if (end[0] == '!') {
				cont = true;
			} else {
				voiceFile = end;
			}
		}

		if (talkStr.lastChar() == '~') {
			debug("Found say command ending with '~'. Please take a look.");
		}

		if (lineToSay.empty()) {
			lineToSay = talkStr;
		} else {
			lineToSay += " " + talkStr;
		}

		if (cont) {
			if (!parseCtx.readLine(currentLine)) {
				cont = false;
			}
		}

		firstPass = false;
	} while (cont);

	command = new SayCommand(lineToSay, voiceFile, waitForPrevious, talkingAnimation);

	return true;
}


Command::ExecuteResult SayCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	Game &game = scriptExecCtx.getGame();

	if (_waitForPrevious) {
		if (game.getActiveSayTask()) {
			return InProgress;
		}
	}

	TaskPtr task(new SayTask(_lineToSay, game.getGameData()._color));
	game.getTaskManager().startTask(task);

	return Finished;
}

Common::String SayCommand::debugString() const {
	return Common::String::format("SHOWMSG%s%s '%s' '%s'", _waitForPrevious ? "+WAIT" : "", _talkingAnimation ? "+TALKANIM" : "", _lineToSay.c_str(), _voiceFile.c_str());
}

}
