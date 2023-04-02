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

#define NANCYDAT_MAJOR_VERSION 0
#define NANCYDAT_MINOR_VERSION 2

#define NANCYDAT_NUM_GAMES 3

/**
 * Format specifications for nancy.dat:
 * 4 bytes              Magic string 'NNCY'
 * 1 byte               Major version number
 * 1 byte               Minor version number
 * 2 bytes              Number of games (ignoring multiple languages)
 * 4 bytes per game     File offsets for every game's data
 * 
 * Game data order:
 *      Game constants structure
 *      Array with the order of game languages
 *      Conditional dialogue (logic)
 *      Goodbyes (logic)
 *      Hints (logic)
 *      Conditional dialogue (text)
 *      Goodbyes (text)
 *      Hints (text)  
 *      Telephone ringing text
 *      Item names
 *      Event flag names
 * 
 * Arrays in the game data are variable-size.
 * All arrays are preceded by a 2-byte size property.
 * 2D arrays with strings (e.g conditional dialogue) are also preceded
 * by a list of 4-byte offsets (one per language).
 * Nonexistent data isn't skipped, but has size 0.
 * All offsets are absolute (relative to start of file)
 * 
 * Game order:
 *      The Vampire Diaries
 *      Nancy Drew: Secrets Can Kill
*/

void NORETURN_PRE error(const char *s, ...) {
	printf("%s\n", s);
	exit(1);
}

void writeGameData( File &output,
                    const GameConstants &gameConstants,
                    const Common::Array<Common::Language> &languages,
                    const Common::Array<Common::Array<ConditionalDialogue>> *conditionalDialogue,
                    const Common::Array<Goodbye> *goodbyes,
                    const Common::Array<Common::Array<Hint>> *hints,
                    const Common::Array<Common::Array<const char *>> *dialogueTexts,
                    const Common::Array<Common::Array<const char *>> *goodbyeTexts,
                    const Common::Array<Common::Array<const char *>> *hintTexts,
                    const Common::Array<const char *> *ringingTexts,
                    const Common::Array<const char *> &eventFlagNames) {
    
    // Write game constants
    output.writeUint16(gameConstants.numItems);
    output.writeUint16(gameConstants.numEventFlags);
    writeToFile(output, gameConstants.mapAccessSceneIDs);
    writeToFile(output, gameConstants.genericEventFlags);
    output.writeUint16(gameConstants.numNonItemCursors);
    output.writeUint16(gameConstants.numCurtainAnimationFrames);
    output.writeUint32(gameConstants.logoEndAfter);

    // Write languages
    writeToFile(output, languages);

    // Write conditional dialogue logic
    if (conditionalDialogue) {
        writeToFile(output, *conditionalDialogue);
    } else {
        output.writeUint16(0);
    }

    // Write Goodbyes logic
    if (goodbyes) {
        writeToFile(output, *goodbyes);
    } else {
        output.writeUint16(0);
    }

    // Write hints logic
    if (hints) {
        writeToFile(output, *hints);
    } else {
        output.writeUint16(0);
    }

    // Write conditional dialogue text
    if (dialogueTexts) {
        writeMultilangArray(output, *dialogueTexts);
    } else {
        output.writeUint16(0);
    }

    // Write goodbyes text
    if (goodbyeTexts) {
        writeMultilangArray(output, *goodbyeTexts);
    } else {
        output.writeUint16(0);
    }

    // Write hints text
    if (hintTexts) {
        writeMultilangArray(output, *hintTexts);
    } else {
        output.writeUint16(0);
    }

    // Write hints text
    if (ringingTexts) {
        writeToFile(output, *ringingTexts);
    } else {
        output.writeUint16(0);
    }

    // Write event flag names
    writeToFile(output, eventFlagNames);
}

int main(int argc, char *argv[]) {
    File output;
	if (!output.open("nancy.dat", kFileWriteMode)) {
		error("Unable to open nancy.dat");
	}

    Common::Array<uint32> gameOffsets;

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
    writeGameData(  output,
                    _tvdConstants,
                    _tvdLanguagesOrder,
                    &_tvdConditionalDialogue,
                    &_tvdGoodbyes,
                    nullptr,
                    &_tvdConditionalDialogueTexts,
                    &_tvdGoodbyeTexts,
                    nullptr,
                    nullptr,
                    _tvdEventFlagNames);
    
    // Nancy Drew: Secrets Can Kill data
    gameOffsets.push_back(output.pos());
    writeGameData(  output,
                    _nancy1Constants,
                    _nancy1LanguagesOrder,
                    &_nancy1ConditionalDialogue,
                    &_nancy1Goodbyes,
                    &_nancy1Hints,
                    &_nancy1ConditionalDialogueTexts,
                    &_nancy1GoodbyeTexts,
                    &_nancy1HintTexts,
                    &_nancy1TelephoneRinging,
                    _nancy1EventFlagNames);
	
	// Nancy Drew: Stay Tuned for Danger data
    gameOffsets.push_back(output.pos());
	writeGameData(  output,
                    _nancy2Constants,
                    _nancy2LanguagesOrder,
                    &_nancy2ConditionalDialogue,
                    &_nancy2Goodbyes,
                    nullptr,
                    &_nancy2ConditionalDialogueTexts,
                    &_nancy2GoodbyeTexts,
                    nullptr,
                    &_nancy2TelephoneRinging,
                    _nancy2EventFlagNames);

    // Write the offsets for each game in the header
    output.seek(offsetsOffset);
    for (uint i = 0; i < gameOffsets.size(); ++i) {
        output.writeUint32(gameOffsets[i]);
    }

    output.close();

    return 0;
}
