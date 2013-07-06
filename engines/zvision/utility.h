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
 *
 */

#ifndef ZVISION_UTILITY_H
#define ZVISION_UTILITY_H

#include "common/str.h"
#include "common/file.h"

namespace ZVision {

/**
 * Opens the sourceFile utilizing Common::File (aka SearchMan) and writes the
 * contents to destFile. destFile is created in the working directory
 *
 * @param sourceFile    The 'file' you want the contents of
 * @param destFile      The name of the file where the content will be written to
 */
void writeFileContentsToFile(const Common::String &sourceFile, const Common::String &destFile) {
	Common::File f;
	f.open(sourceFile);
	byte* buffer = new byte[f.size()];
	f.read(buffer, f.size());

	Common::DumpFile dumpFile;
	dumpFile.open(destFile);

	uint32 writtenBytes = dumpFile.write(buffer, f.size());
	dumpFile.flush();
	dumpFile.close();

	delete[] buffer;
}

/**
 * Removes any line comments using '#' as a sequence start.
 * Then removes any trailing and leading 'whitespace' using String::trim()
 * Note: String::trim uses isspace() to determine what is whitespace and what is not.
 *
 * @param string    The string to modify. It is modified in place
 */
void trimCommentsAndWhiteSpace(Common::String *string) {
	for (int i = string->size() - 1; i >= 0; i--) {
		if ((*string)[i] == '#') {
			string->erase(i);
		}
	}

	string->trim();
}

void tryToDumpLine(const Common::String &key,
	Common::String &line,
	Common::HashMap<Common::String, byte> *count,
	Common::HashMap<Common::String, bool> *fileAlreadyUsed,
	Common::DumpFile &output) {
		const byte numberOfExamplesPerType = 8;

		if ((*count)[key] < numberOfExamplesPerType && !(*fileAlreadyUsed)[key]) {
			output.writeString(line);
			output.writeByte('\n');
			(*count)[key]++;
			(*fileAlreadyUsed)[key] = true;
		}
}

/**
 * Searches through all the .scr files and dumps 'numberOfExamplesPerType' examples of each type of ResultAction
 * ZVision::initialize() must have been called before this function can be used.
 *
 * @param destFile    Where to write the examples
 */
void dumpEveryResultAction(const Common::String &destFile) {
	

	Common::HashMap<Common::String, byte> count;
	Common::HashMap<Common::String, bool> fileAlreadyUsed;

	Common::DumpFile output;
	output.open(destFile);

	// Find scr files
	Common::ArchiveMemberList list;
	SearchMan.listMatchingMembers(list, "*.scr");

	for (Common::ArchiveMemberList::iterator iter = list.begin(); iter != list.end(); ++iter) {
		Common::SeekableReadStream *stream = (*iter)->createReadStream();

		Common::String line = stream->readLine();
		trimCommentsAndWhiteSpace(&line);

		while (!stream->eos()) {
			if (line.matchString("*:add*", true)) {
				tryToDumpLine("add", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:animplay*", true)) {
				tryToDumpLine("animplay", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:animpreload*", true)) {
				tryToDumpLine("animpreload", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:animunload*", true)) {
				tryToDumpLine("animunload", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:attenuate*", true)) {
				tryToDumpLine("attenuate", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:assign*", true)) {
				tryToDumpLine("assign", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:change_location*", true)) {
				tryToDumpLine("change_location", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:crossfade*", true) && !fileAlreadyUsed["add"]) {
				tryToDumpLine("crossfade", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:debug*", true)) {
				tryToDumpLine("debug", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:delay_render*", true)) {
				tryToDumpLine("delay_render", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:disable_control*", true)) {
				tryToDumpLine("disable_control", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:disable_venus*", true)) {
				tryToDumpLine("disable_venus", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:display_message*", true)) {
				tryToDumpLine("display_message", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:dissolve*", true)) {
				tryToDumpLine("dissolve", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:distort*", true)) {
				tryToDumpLine("distort", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:enable_control*", true)) {
				tryToDumpLine("enable_control", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:flush_mouse_events*", true)) {
				tryToDumpLine("flush_mouse_events", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:inventory*", true)) {
				tryToDumpLine("inventory", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:kill*", true)) {
				tryToDumpLine("kill", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:menu_bar_enable*", true)) {
				tryToDumpLine("menu_bar_enable", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:music*", true)) {
				tryToDumpLine("music", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:pan_track*", true)) {
				tryToDumpLine("pan_track", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:playpreload*", true)) {
				tryToDumpLine("playpreload", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:preferences*", true)) {
				tryToDumpLine("preferences", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:quit*", true)) {
				tryToDumpLine("quit", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:random*", true)) {
				tryToDumpLine("random", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:region*", true)) {
				tryToDumpLine("region", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:restore_game*", true)) {
				tryToDumpLine("restore_game", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:rotate_to*", true)) {
				tryToDumpLine("rotate_to", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:save_game*", true)) {
				tryToDumpLine("save_game", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:set_partial_screen*", true)) {
				tryToDumpLine("set_partial_screen", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:set_screen*", true)) {
				tryToDumpLine("set_screen", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:set_venus*", true)) {
				tryToDumpLine("set_venus", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:stop*", true)) {
				tryToDumpLine("stop", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:streamvideo*", true)) {
				tryToDumpLine("streamvideo", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:syncsound*", true)) {
				tryToDumpLine("syncsound", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:timer*", true)) {
				tryToDumpLine("timer", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:ttytext*", true)) {
				tryToDumpLine("ttytext", line, &count, &fileAlreadyUsed, output);
			} else if (line.matchString("*:universe_music*", true)) {
				tryToDumpLine("universe_music", line, &count, &fileAlreadyUsed, output);
			}

			line = stream->readLine();
			trimCommentsAndWhiteSpace(&line);
		}

		for (Common::HashMap<Common::String, bool>::iterator iter = fileAlreadyUsed.begin(); iter != fileAlreadyUsed.end(); ++iter) {
			(*iter)._value = false;
		}
	}

	output.close();
}

} // End of namespace ZVision

#endif
