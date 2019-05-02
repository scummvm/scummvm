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

#include "mutationofjb/commands/definestructcommand.h"
#include "mutationofjb/script.h"
#include "mutationofjb/game.h"
#include "common/debug.h"

/** @file
 * "DEFINE_STRUCT " <numItemGroups> " " <context> " " <objectId> " " <colorString> <CRLF>
 * <itemGroup> { <CRLF> <itemGroup> }
 *
 * item ::= <questionIndex> " " <responseIndex> " " <nextGroup>
 * itemGroup ::= <item> " " <item> " " <item> " " <item> " " <item>
 *
 * Defines the flow of an interactive conversation.
 *
 * Every item group consists of 5 conversation items.
 * "questionIndex" and "responseIndex" specify what the player and the responder say when the conversation item is selected.
 * They refer to the line numbers of TOSAY.GER and RESPONSE.GER, respectively.
 * "nextGroup" refers to the group that follows when the conversation item is selected. A value of 0 indicates end of
 * conversation.
 */

namespace MutationOfJB {

bool DefineStructCommandParser::parse(const Common::String &line, ScriptParseContext &parseCtx, Command *&command) {
	if (line.size() < 24 || !line.hasPrefix("DEFINE_STRUCT")) {
		return false;
	}

	ConversationInfo convInfo;

	const int numLines = atoi(line.c_str() + 14);
	convInfo._context = atoi(line.c_str() + 18);
	convInfo._objectId = atoi(line.c_str() + 20);
	convInfo._color = Game::colorFromString(line.c_str() + 23);

	for (int i = 0; i < numLines; ++i) {
		Common::String convLineStr;
		if (!parseCtx.readLine(convLineStr)) {
			break;
		}

		if (convLineStr.size() != 74) {
			debug("Conversation line in DEFINE_STRUCT with wrong length");
			continue;
		}

		const char *linePtr = convLineStr.c_str();

		ConversationInfo::ItemGroup convGroup;

		for (int j = 0; j < 5; ++j) {
			ConversationInfo::Item convItem;
			convItem._question = atoi(linePtr);
			linePtr += 6;
			convItem._response = atoi(linePtr);
			linePtr += 6;
			convItem._nextGroupIndex = atoi(linePtr);
			linePtr += 3;
			convGroup.push_back(convItem);
		}
		convInfo._itemGroups.push_back(convGroup);
	}

	command = new DefineStructCommand(convInfo);

	return true;
}

Command::ExecuteResult DefineStructCommand::execute(ScriptExecutionContext &scriptExecCtx) {
	scriptExecCtx.getGameData()._conversationInfo = _conversationInfo;
	return Command::Finished;
}

Common::String DefineStructCommand::debugString() const {
	return "DEFINE_STRUCT <data omitted>";
}

}
