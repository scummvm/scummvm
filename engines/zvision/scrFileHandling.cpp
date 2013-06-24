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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include <stdio.h>

#include "zvision/scriptManager.h"
#include "zvision/utility.h"

#include "common/textconsole.h"
#include "common/file.h"
#include "common/tokenizer.h"

namespace ZVision {


void ScriptManager::parseScrFile(Common::String fileName) {
	Common::File file;
	if (!file.open(fileName))
		return; // File.open already throws a warning if the file doesn't exist, so there is no need to throw another

	char buffer[1024];
	while(!file.eos()) {
		Common::String line = file.readLine();
		if (file.err()) {
			warning("Error parsing scr file: %s", fileName);
			return;
		}

		trimCommentsAndWhiteSpace(line);
		if (line.empty())
			continue;

		if (line.matchString("puzzle:*", true)) {
			Puzzle *puzzle = new Puzzle();
			sscanf(line.c_str(),"puzzle:%u",&(puzzle->id));

			parsePuzzle(puzzle, file);
		} else if (line.matchString("control:*", true)) {
			Control *control = new Control();
			sscanf(line.c_str(),"control:%u",&(control->id));

			parseControl(control, file);
		}
	}
}

void ScriptManager::parsePuzzle(Puzzle *puzzle, Common::SeekableReadStream &stream) {
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(line);

	while (!line.contains('}')) {
		if (line.matchString("criteria {", true))
			puzzle->criteriaList.push_back(parseCriteria(stream));
		else if (line.matchString("results {", true))
			puzzle->resultList.push_back(parseResult(stream));
		else if (line.matchString("flags {", true))
			puzzle->flags = parseFlags(stream);
	}

}

Criteria ScriptManager::parseCriteria(Common::SeekableReadStream &stream) const {
	Criteria criteria;

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(line);

	while (!line.contains('}')) {
		// Split the string into tokens using ' ' as a delimiter
		Common::StringTokenizer tokenizer(line);
		Common::String token;

		// Parse the id out of the first token
		token = tokenizer.nextToken();
		sscanf(token.c_str(), "[%u]", &(criteria.id));

		// Parse the operator out of the second token
		token = tokenizer.nextToken();
		if (token.c_str()[0] == '=')
			criteria.criteriaOperator = EQUAL_TO;
		else if (token.c_str()[0] == '!')
			criteria.criteriaOperator = NOT_EQUAL_TO;
		else if (token.c_str()[0] == '>')
			criteria.criteriaOperator = GREATER_THAN;
		else if (token.c_str()[0] == '<')
			criteria.criteriaOperator = LESS_THAN;

		// First determine if the last token is an id or a value
		// Then parse it into 'argument'
		token = tokenizer.nextToken();
		if (token.contains('[')) {
			sscanf(token.c_str(), "[%u]", &(criteria.argument));
			criteria.argumentIsAnId = true;
		} else {
			sscanf(token.c_str(), "%u", &(criteria.argument));
			criteria.argumentIsAnId = false;
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(line);
	}

	return criteria;
}

Result ScriptManager::parseResult(Common::SeekableReadStream &stream) const {
	Result result;

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(line);

	while (!line.contains('}')) {
		Common::StringTokenizer tokenizer(line, " :(),");
		// We don't care about the first token
		tokenizer.nextToken();
		Common::String token = tokenizer.nextToken();

		// The second token defines the action
		// Use it to set the action enum and then to parse the arguments
		if (token.matchString("add", true)) {
			result.action = ADD;
			ObjectType argTypes[] = { UINT32, UINT32 };
			parseResultArguments(result, argTypes, 2, tokenizer.nextToken(), tokenizer.nextToken());
		} else if (token.matchString("animplay", true)) {
			result.action = ANIM_PLAY;

		} else if (token.matchString("animpreload", true)) {
			result.action = ANIM_PRELOAD;

		} else if (token.matchString("animunload", true)) {
			result.action = ANIM_UNLOAD;

		} else if (token.matchString("attenuate", true)) {
			result.action = ATTENUATE;

		} else if (token.matchString("assign", true)) {
			result.action = ASSIGN;
			ObjectType argTypes[] = { UINT32, UINT32 };
			parseResultArguments(result, argTypes, 2, tokenizer.nextToken(), tokenizer.nextToken());
		} else if (token.matchString("change_location", true)) {
			result.action = CHANGE_LOCATION;
			ObjectType argTypes[] = { STRING, STRING, STRING, UINT32 };
			parseResultArguments(result, argTypes, 4, tokenizer.nextToken(), tokenizer.nextToken(), tokenizer.nextToken(), tokenizer.nextToken());
		} else if (token.matchString("crossfade", true)) {
			result.action = CROSSFADE;

		} else if (token.matchString("debug", true)) {
			result.action = DEBUG;

		} else if (token.matchString("delay_render", true)) {
			result.action = DELAY_RENDER;

		} else if (token.matchString("disable_control", true)) {
			result.action = DISABLE_CONTROL;

		} else if (token.matchString("disable_venus", true)) {
			result.action = DISABLE_VENUS;

		} else if (token.matchString("display_message", true)) {
			result.action = DISPLAY_MESSAGE;

		} else if (token.matchString("dissolve", true)) {
			result.action = DISSOLVE;
		} else if (token.matchString("distort", true)) {
			result.action = DISTORT;

		} else if (token.matchString("enable_control", true)) {
			result.action = ENABLE_CONTROL;

		} else if (token.matchString("flush_mouse_events", true)) {
			result.action = FLUSH_MOUSE_EVENTS;

		} else if (token.matchString("inventory", true)) {
			result.action = INVENTORY;

		} else if (token.matchString("kill", true)) {
			result.action = KILL;

		} else if (token.matchString("menu_bar_enable", true)) {
			result.action = MENU_BAR_ENABLE;

		} else if (token.matchString("music", true)) {
			result.action = MUSIC;
			ObjectType argTypes[] = { UINT32, UINT32, STRING, UINT32, UINT32 };
			parseResultArguments(result, argTypes, 5,
								 tokenizer.nextToken(), tokenizer.nextToken(),
								 tokenizer.nextToken(), tokenizer.nextToken(),
								 tokenizer.nextToken());
		} else if (token.matchString("pan_track", true)) {
			result.action = PAN_TRACK;

		} else if (token.matchString("playpreload", true)) {
			result.action = PLAY_PRELOAD;

		} else if (token.matchString("preferences", true)) {
			result.action = PREFERENCES;

		} else if (token.matchString("quit", true)) {
			result.action = QUIT;

		} else if (token.matchString("random", true)) {
			result.action = RANDOM;
			ObjectType argTypes[] = { UINT32, UINT32 };
			parseResultArguments(result, argTypes, 2, tokenizer.nextToken(), tokenizer.nextToken());
		} else if (token.matchString("region", true)) {
			result.action = REGION;

		} else if (token.matchString("restore_game", true)) {
			result.action = RESTORE_GAME;

		} else if (token.matchString("rotate_to", true)) {
			result.action = ROTATE_TO;

		} else if (token.matchString("save_game", true)) {
			result.action = SAVE_GAME;

		} else if (token.matchString("set_partial_screen", true)) {
			result.action = SET_PARTIAL_SCREEN;

		} else if (token.matchString("set_screen", true)) {
			result.action = SET_SCREEN;

		} else if (token.matchString("set_venus", true)) {
			result.action = SET_VENUS;

		} else if (token.matchString("stop", true)) {
			result.action = STOP;
			ObjectType argTypes[] = { UINT32 };
			parseResultArguments(result, argTypes, 1, tokenizer.nextToken());
		} else if (token.matchString("streamvideo", true)) {
			result.action = STREAM_VIDEO;

		} else if (token.matchString("syncsound", true)) {
			result.action = SYNC_SOUND;

		} else if (token.matchString("timer", true)) {
			result.action = TIMER;
			ObjectType argTypes[] = { UINT32, UINT32 };
			parseResultArguments(result, argTypes, 2, tokenizer.nextToken(), tokenizer.nextToken());
		} else if (token.matchString("ttytext", true)) {
			result.action = TTY_TEXT;

		} else if (token.matchString("universe_music", true)) {
			result.action = UNIVERSE_MUSIC;

		} else {
			warning("Unhandled result action type: ", token);
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(line);
	}

	return result;
}

void ScriptManager::parseResultArguments(Result &result, const ObjectType *types, int numberOfArgs, ...) const {
	va_list argptr;
	va_start(argptr, numberOfArgs);

	for (int i = 0; i < numberOfArgs; i++) {
		if (types[i] == UINT32) {
			Common::String arg = va_arg(argptr, Common::String);
			Object argObject(UINT32);
			sscanf(arg.c_str(), "%u", &argObject);
			result.arguments.push_back(argObject);
			break;
		} else {
			Common::String arg = va_arg(argptr, Common::String);
			result.arguments.push_back(Object(arg));
			break;
		}
	}

	va_end(argptr);
}

byte ScriptManager::parseFlags(Common::SeekableReadStream &stream) const {
	byte flags;

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(line);

	while (!line.contains('}')) {
		if (line.matchString("ONCE_PER_INST", true)) {
			flags |= ONCE_PER_INST;
		} else if (line.matchString("DO_ME_NOW", true)) {
			flags |= DO_ME_NOW;
		} else if (line.matchString("DISABLED", true)) {
			flags |= DISABLED;
		}
	}

	return flags;
}

void ScriptManager::parseControl(Control *control, Common::SeekableReadStream &stream) {

}

} // End namespace ZVision
