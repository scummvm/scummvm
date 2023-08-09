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

#include "file.h"
#include "tvd_data.h"
#include "nancy1_data.h"
#include "nancy2_data.h"
#include "nancy3_data.h"
#include "nancy4_data.h"
#include "nancy5_data.h"

#define NANCYDAT_MAJOR_VERSION 1
#define NANCYDAT_MINOR_VERSION 0

#define NANCYDAT_NUM_GAMES 6

/**
 * Format specifications for nancy.dat:
 * 4 bytes              Magic string 'NNCY'
 * 1 byte               Major version number
 * 1 byte               Minor version number
 * 2 bytes              Number of games (ignoring multiple languages)
 * 4 bytes per game     File offsets for every game's data
 * Rest of file			Game data
 * 
 * Game data contents:
 * 		Various data sections, depending on title;
 * 		e.g.: only nancy1 has a hint section, since later
 * 		titles abandoned the dedicated hint system.
 * 		Each section has the following structure:
 * 			4 bytes		Offset to next section
 * 			4 bytes		Section tag (generated using MKTAG macro)
 * 			variable	Section data
 * 
 * Arrays in the game data are variable-size.
 * All arrays are preceded by a 2-byte size property.
 * 2D arrays with strings (e.g conditional dialogue) are also preceded
 * by a list of 4-byte offsets (one per language).
 * All offsets are absolute (relative to start of file).
 * All data is little endian.
 * 
 * Game order:
 *      The Vampire Diaries
 *      Nancy Drew: Secrets Can Kill
 * 		Nancy Drew: Stay Tuned for Danger
 * 		Nancy Drew: Message in a Haunted Mansion
 * 		Nancy Drew: Treasure in the Royal Tower
 * 		Nancy Drew: The Final Scene
*/

// Add the offset to the next tagged section before the section itself for easier navigation
#define WRAPWITHOFFSET(x) beginOffset = output.pos();\
	output.skip(4);\
	x;\
	endOffset = output.pos();\
	output.seek(beginOffset);\
	output.writeUint32(endOffset);\
	output.seek(endOffset);

void NORETURN_PRE error(const char *s, ...) {
	printf("%s\n", s);
	exit(1);
}

void writeConstants(File &output, const GameConstants &gameConstants) {
	output.writeUint32(MKTAG('C', 'O', 'N', 'S'));
	output.writeUint16(gameConstants.numItems);
	output.writeUint16(gameConstants.numEventFlags);
	writeToFile(output, gameConstants.mapAccessSceneIDs);
	writeToFile(output, gameConstants.genericEventFlags);
	output.writeUint16(gameConstants.numNonItemCursors);
	output.writeUint16(gameConstants.numCurtainAnimationFrames);
	output.writeUint32(gameConstants.logoEndAfter);
}

void writeSoundChannels(File &output, const SoundChannelInfo &soundChannelInfo) {
	output.writeUint32(MKTAG('S', 'C', 'H', 'N'));
	output.writeByte(soundChannelInfo.numChannels);
	output.writeByte(soundChannelInfo.numSceneSpecificChannels);
	writeToFile(output, soundChannelInfo.speechChannels);
	writeToFile(output, soundChannelInfo.musicChannels);
	writeToFile(output, soundChannelInfo.sfxChannels);
}

void writeLanguages(File &output, const Common::Array<Common::Language> &languages) {
	output.writeUint32(MKTAG('L', 'A', 'N', 'G'));
	writeToFile(output, languages);
}

void writeConditionalDialogue(File &output, const Common::Array<Common::Array<ConditionalDialogue>> &conditionalDialogue, const Common::Array<Common::Array<const char *>> &dialogueTexts) {
	output.writeUint32(MKTAG('C', 'D', 'L', 'G'));
	writeToFile(output, conditionalDialogue);
	writeMultilangArray(output, dialogueTexts);
}

void writeGoodbyes(File &output, const Common::Array<Goodbye> &goodbyes, const Common::Array<Common::Array<const char *>> &goodbyeTexts) {
	output.writeUint32(MKTAG('G', 'D', 'B', 'Y'));
	writeToFile(output, goodbyes);
	writeMultilangArray(output, goodbyeTexts);
}

void writeHints(File &output, const Common::Array<Common::Array<Hint>> &hints, const SceneChangeDescription &hintSceneChange, const Common::Array<Common::Array<const char *>> &hintTexts) {
	output.writeUint32(MKTAG('H', 'I', 'N', 'T'));
	writeToFile(output, hintSceneChange);
	writeToFile(output, hints);
	writeMultilangArray(output, hintTexts);
}

void writeRingingTexts(File &output, const Common::Array<const char *> &ringingTexts) {
	output.writeUint32(MKTAG('R', 'I', 'N', 'G'));
	writeToFile(output, ringingTexts);
}

void writeEventFlagNames(File &output, const Common::Array<const char *> &eventFlagNames) {
	output.writeUint32(MKTAG('E', 'F', 'L', 'G'));
	writeToFile(output, eventFlagNames);
}

int main(int argc, char *argv[]) {
	File output;
	if (!output.open("nancy.dat", kFileWriteMode)) {
		error("Unable to open nancy.dat");
	}

	Common::Array<uint32> gameOffsets;
	uint32 beginOffset, endOffset;

	// Write header
	output.writeByte('N');
	output.writeByte('N');
	output.writeByte('C');
	output.writeByte('Y');
	output.writeByte(NANCYDAT_MAJOR_VERSION);
	output.writeByte(NANCYDAT_MINOR_VERSION);
	output.writeUint16(NANCYDAT_NUM_GAMES);

	// Skip game offsets, they'll be written at the end
	uint32 offsetsOffset = output.pos();
	output.skip(NANCYDAT_NUM_GAMES * 4);

	// The Vampire Diaries data
	gameOffsets.push_back(output.pos());
	WRAPWITHOFFSET(writeConstants(output, _tvdConstants))
	WRAPWITHOFFSET(writeSoundChannels(output, _tvdToNancy2SoundChannelInfo))
	WRAPWITHOFFSET(writeLanguages(output, _tvdLanguagesOrder))
	WRAPWITHOFFSET(writeConditionalDialogue(output, _tvdConditionalDialogue, _tvdConditionalDialogueTexts))
	WRAPWITHOFFSET(writeGoodbyes(output, _tvdGoodbyes, _tvdGoodbyeTexts))
	WRAPWITHOFFSET(writeEventFlagNames(output, _tvdEventFlagNames))
	
	// Nancy Drew: Secrets Can Kill data
	gameOffsets.push_back(output.pos());
	WRAPWITHOFFSET(writeConstants(output, _nancy1Constants))
	WRAPWITHOFFSET(writeSoundChannels(output, _tvdToNancy2SoundChannelInfo))
	WRAPWITHOFFSET(writeLanguages(output, _nancy1LanguagesOrder))
	WRAPWITHOFFSET(writeConditionalDialogue(output, _nancy1ConditionalDialogue, _nancy1ConditionalDialogueTexts))
	WRAPWITHOFFSET(writeGoodbyes(output, _nancy1Goodbyes, _nancy1GoodbyeTexts))
	WRAPWITHOFFSET(writeHints(output, _nancy1Hints, _nancy1HintSceneChange, _nancy1HintTexts))
	WRAPWITHOFFSET(writeRingingTexts(output, _nancy1TelephoneRinging))
	WRAPWITHOFFSET(writeEventFlagNames(output, _nancy1EventFlagNames))
	
	// Nancy Drew: Stay Tuned for Danger data
	gameOffsets.push_back(output.pos());
	WRAPWITHOFFSET(writeConstants(output, _nancy2Constants))
	WRAPWITHOFFSET(writeSoundChannels(output, _tvdToNancy2SoundChannelInfo))
	WRAPWITHOFFSET(writeLanguages(output, _nancy2LanguagesOrder))
	WRAPWITHOFFSET(writeConditionalDialogue(output, _nancy2ConditionalDialogue, _nancy2ConditionalDialogueTexts))
	WRAPWITHOFFSET(writeGoodbyes(output, _nancy2Goodbyes, _nancy2GoodbyeTexts))
	WRAPWITHOFFSET(writeRingingTexts(output, _nancy2TelephoneRinging))
	WRAPWITHOFFSET(writeEventFlagNames(output, _nancy2EventFlagNames))
	
	// Nancy Drew: Message in a Haunted Mansion data
	gameOffsets.push_back(output.pos());
	WRAPWITHOFFSET(writeConstants(output, _nancy3Constants))
	WRAPWITHOFFSET(writeSoundChannels(output, _nancy3to5SoundChannelInfo))
	WRAPWITHOFFSET(writeLanguages(output, _nancy3LanguagesOrder))
	WRAPWITHOFFSET(writeConditionalDialogue(output, _nancy3ConditionalDialogue, _nancy3ConditionalDialogueTexts))
	WRAPWITHOFFSET(writeGoodbyes(output, _nancy3Goodbyes, _nancy3GoodbyeTexts))
	WRAPWITHOFFSET(writeRingingTexts(output, _nancy3TelephoneRinging))
	WRAPWITHOFFSET(writeEventFlagNames(output, _nancy3EventFlagNames))
	
	// Nancy Drew: Treasure in the Royal Tower data
	gameOffsets.push_back(output.pos());
	WRAPWITHOFFSET(writeConstants(output, _nancy4Constants))
	WRAPWITHOFFSET(writeSoundChannels(output, _nancy3to5SoundChannelInfo))
	WRAPWITHOFFSET(writeLanguages(output, _nancy4LanguagesOrder))
	WRAPWITHOFFSET(writeConditionalDialogue(output, _nancy4ConditionalDialogue, _nancy4ConditionalDialogueTexts))
	WRAPWITHOFFSET(writeGoodbyes(output, _nancy4Goodbyes, _nancy4GoodbyeTexts))
	WRAPWITHOFFSET(writeRingingTexts(output, _nancy4TelephoneRinging))
	WRAPWITHOFFSET(writeEventFlagNames(output, _nancy4EventFlagNames))

	// Nancy Drew: The Final Scene data
	gameOffsets.push_back(output.pos());
	WRAPWITHOFFSET(writeConstants(output, _nancy5Constants))
	WRAPWITHOFFSET(writeSoundChannels(output, _nancy3to5SoundChannelInfo))
	WRAPWITHOFFSET(writeLanguages(output, _nancy5LanguagesOrder))
	WRAPWITHOFFSET(writeConditionalDialogue(output, _nancy5ConditionalDialogue, _nancy5ConditionalDialogueTexts))
	WRAPWITHOFFSET(writeGoodbyes(output, _nancy5Goodbyes, _nancy5GoodbyeTexts))
	WRAPWITHOFFSET(writeRingingTexts(output, _nancy5TelephoneRinging))
	WRAPWITHOFFSET(writeEventFlagNames(output, _nancy5EventFlagNames))

	// Write the offsets for each game in the header
	output.seek(offsetsOffset);
	for (uint i = 0; i < gameOffsets.size(); ++i) {
		output.writeUint32(gameOffsets[i]);
	}

	output.close();

	return 0;
}
