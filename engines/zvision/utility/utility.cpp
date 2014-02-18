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

#include "common/scummsys.h"

#include "zvision/utility/utility.h"

#include "zvision/zvision.h"
#include "zvision/sound/zork_raw.h"

#include "common/tokenizer.h"
#include "common/file.h"


namespace ZVision {

void writeFileContentsToFile(const Common::String &sourceFile, const Common::String &destFile) {
	Common::File f;
	if (!f.open(sourceFile)) {
		return;
	}

	byte* buffer = new byte[f.size()];
	f.read(buffer, f.size());

	Common::DumpFile dumpFile;
	dumpFile.open(destFile);

	dumpFile.write(buffer, f.size());
	dumpFile.flush();
	dumpFile.close();

	delete[] buffer;
}

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

		for (Common::HashMap<Common::String, bool>::iterator fileUsedIter = fileAlreadyUsed.begin(); fileUsedIter != fileAlreadyUsed.end(); ++fileUsedIter) {
			fileUsedIter->_value = false;
		}
	}

	output.close();
}

Common::String getFileName(const Common::String &fullPath) {
	Common::StringTokenizer tokenizer(fullPath, "/\\");
	Common::String token;
	while (!tokenizer.empty()) {
		token = tokenizer.nextToken();
	}

	return token;
}

void convertRawToWav(const Common::String &inputFile, ZVision *engine, const Common::String &outputFile) {
	Common::File file;
	if (!file.open(inputFile))
		return;

	Audio::AudioStream *audioStream = makeRawZorkStream(inputFile, engine);
	
	Common::DumpFile output;
	output.open(outputFile);

	output.writeUint32BE(MKTAG('R', 'I', 'F', 'F'));
	output.writeUint32LE(file.size() * 2 + 36);
	output.writeUint32BE(MKTAG('W', 'A', 'V', 'E'));
	output.writeUint32BE(MKTAG('f', 'm', 't', ' '));
	output.writeUint32LE(16);
	output.writeUint16LE(1);
	uint16 numChannels;
	if (audioStream->isStereo()) {
		numChannels = 2;
		output.writeUint16LE(2);
	} else {
		numChannels = 1;
		output.writeUint16LE(1);
	}
	output.writeUint32LE(audioStream->getRate());
	output.writeUint32LE(audioStream->getRate() * numChannels * 2);
	output.writeUint16LE(numChannels * 2);
	output.writeUint16LE(16);
	output.writeUint32BE(MKTAG('d', 'a', 't', 'a'));
	output.writeUint32LE(file.size() * 2);
	int16 *buffer = new int16[file.size()];
	audioStream->readBuffer(buffer, file.size());
	output.write(buffer, file.size() * 2);

	delete[] buffer;
}

} // End of namespace ZVision
