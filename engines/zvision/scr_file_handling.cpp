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

#include "zvision/script_manager.h"
#include "zvision/utility.h"
#include "zvision/puzzle.h"
#include "zvision/actions.h"

#include "common/textconsole.h"
#include "common/file.h"
#include "common/tokenizer.h"

namespace ZVision {

void ScriptManager::parseScrFile(Common::String fileName) {
	Common::File file;
	if (!file.open(fileName))
		return; // File.open already throws a warning if the file doesn't exist, so there is no need to throw another

	while(!file.eos()) {
		Common::String line = file.readLine();
		if (file.err()) {
			warning("Error parsing scr file: %s", fileName.c_str());
			return;
		}

		trimCommentsAndWhiteSpace(&line);
		if (line.empty())
			continue;

		if (line.matchString("puzzle:*", true)) {
			Puzzle puzzle;
			sscanf(line.c_str(),"puzzle:%u",&(puzzle.key));

			parsePuzzle(puzzle, file);
			_activePuzzles.push_back(puzzle);
		} else if (line.matchString("control:*", true)) {
			Control *control = parseControl(line, file);
			// Some controls don't require nodes. They just initialize the scene
			if (control != 0) {
				_activeControls.push_back(control);
			}
		}
	}
}

void ScriptManager::parsePuzzle(Puzzle &puzzle, Common::SeekableReadStream &stream) {
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);

	while (!line.contains('}')) {
		if (line.matchString("criteria {", true)) {
			Criteria criteria;
			if (parseCriteria(&criteria, stream)) {
				puzzle.criteriaList.push_back(criteria);
			}
		} else if (line.matchString("results {", true)) {
			parseResults(stream, puzzle.resultActions);
		} else if (line.matchString("flags {", true)) {
			puzzle.flags = parseFlags(stream);
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}
}

bool ScriptManager::parseCriteria(Criteria *criteria, Common::SeekableReadStream &stream) const {
	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);

	// Criteria can be empty
	if (line.contains('}')) {
		return false;
	}

	while (!line.contains('}')) {
		// Split the string into tokens using ' ' as a delimiter
		Common::StringTokenizer tokenizer(line);
		Common::String token;

		// Parse the id out of the first token
		token = tokenizer.nextToken();
		sscanf(token.c_str(), "[%u]", &(criteria->key));

		// Parse the operator out of the second token
		token = tokenizer.nextToken();
		if (token.c_str()[0] == '=')
			criteria->criteriaOperator = EQUAL_TO;
		else if (token.c_str()[0] == '!')
			criteria->criteriaOperator = NOT_EQUAL_TO;
		else if (token.c_str()[0] == '>')
			criteria->criteriaOperator = GREATER_THAN;
		else if (token.c_str()[0] == '<')
			criteria->criteriaOperator = LESS_THAN;

		// First determine if the last token is an id or a value
		// Then parse it into 'argument'
		token = tokenizer.nextToken();
		if (token.contains('[')) {
			sscanf(token.c_str(), "[%u]", &(criteria->argument));
			criteria->argumentIsAKey = true;
		} else {
			sscanf(token.c_str(), "%u", &(criteria->argument));
			criteria->argumentIsAKey = false;
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}

	return true;
}

void ScriptManager::parseResults(Common::SeekableReadStream &stream, Common::List<ResultAction *> &actionList) const {
	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);

	// TODO: Re-order the if-then statements in order of highest occurrence
	while (!line.contains('}')) {
		// Parse for the action type
		if (line.matchString("*:add*", true)) {
			actionList.push_back(new ActionAdd(&line));
		} else if (line.matchString("*:animplay*", true)) {
			actionList.push_back(new ActionPlayAnimation(&line));
		} else if (line.matchString("*:animpreload*", true)) {
			actionList.push_back(new ActionPreloadAnimation(&line));
		} else if (line.matchString("*:animunload*", true)) {
			

		} else if (line.matchString("*:attenuate*", true)) {
			

		} else if (line.matchString("*:assign*", true)) {
			

		} else if (line.matchString("*:change_location*", true)) {
			

		} else if (line.matchString("*:crossfade*", true)) {
			

		} else if (line.matchString("*:debug*", true)) {
			

		} else if (line.matchString("*:delay_render*", true)) {
			

		} else if (line.matchString("*:disable_control*", true)) {
			

		} else if (line.matchString("*:disable_venus*", true)) {
			

		} else if (line.matchString("*:display_message*", true)) {
			

		} else if (line.matchString("*:dissolve*", true)) {
			

		} else if (line.matchString("*:distort*", true)) {
			

		} else if (line.matchString("*:enable_control*", true)) {
			

		} else if (line.matchString("*:flush_mouse_events*", true)) {
			

		} else if (line.matchString("*:inventory*", true)) {
			

		} else if (line.matchString("*:kill*", true)) {
			

		} else if (line.matchString("*:menu_bar_enable*", true)) {
			

		} else if (line.matchString("*:music*", true)) {
			

		} else if (line.matchString("*:pan_track*", true)) {
			

		} else if (line.matchString("*:playpreload*", true)) {
			

		} else if (line.matchString("*:preferences*", true)) {
			

		} else if (line.matchString("*:quit*", true)) {
			

		} else if (line.matchString("*:random*", true)) {
			

		} else if (line.matchString("*:region*", true)) {
			

		} else if (line.matchString("*:restore_game*", true)) {
			

		} else if (line.matchString("*:rotate_to*", true)) {
			

		} else if (line.matchString("*:save_game*", true)) {
			

		} else if (line.matchString("*:set_partial_screen*", true)) {
			

		} else if (line.matchString("*:set_screen*", true)) {
			

		} else if (line.matchString("*:set_venus*", true)) {
			

		} else if (line.matchString("*:stop*", true)) {
			

		} else if (line.matchString("*:streamvideo*", true)) {
			

		} else if (line.matchString("*:syncsound*", true)) {
			

		} else if (line.matchString("*:timer*", true)) {
			

		} else if (line.matchString("*:ttytext*", true)) {
			

		} else if (line.matchString("*:universe_music*", true)) {
			

		} else {
			warning("Unhandled result action type: %s", line.c_str());
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}

	return;
}

byte ScriptManager::parseFlags(Common::SeekableReadStream &stream) const {
	byte flags;

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);

	while (!line.contains('}')) {
		if (line.matchString("ONCE_PER_INST", true)) {
			flags |= ONCE_PER_INST;
		} else if (line.matchString("DO_ME_NOW", true)) {
			flags |= DO_ME_NOW;
		} else if (line.matchString("DISABLED", true)) {
			flags |= DISABLED;
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}

	return flags;
}

Control *ScriptManager::parseControl(Common::String &line, Common::SeekableReadStream &stream) {
	Control *control = 0;
	uint32 key;
	char controlTypeBuffer[20];

	sscanf(line.c_str(), "control:%u %s {", &key, controlTypeBuffer);

	Common::String controlType(controlTypeBuffer);

	if (controlType.equalsIgnoreCase("push_toggle")) {

	} else if (controlType.equalsIgnoreCase("flat")) {
		Control::parseFlatControl(_engine);
		return 0;
	} else if (controlType.equalsIgnoreCase("pana")) {
		Control::parsePanoramaControl(_engine, stream);
		return 0;
	}
	else if (controlType.equalsIgnoreCase("tilt")) {
		Control::parseTiltControl(_engine, stream);
		return 0;
	}

	return control;
}

} // End of namespace ZVision
