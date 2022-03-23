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

 //=============================================================================
 //
 // This unit provides functions for reading compiled translation file.
 //
 //=============================================================================

#ifndef AGS_SHARED_GAME_TRA_FILE_H
#define AGS_SHARED_GAME_TRA_FILE_H

#include "ags/shared/util/error.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_types.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

enum TraFileErrorType {
	kTraFileErr_NoError,
	kTraFileErr_SignatureFailed,
	kTraFileErr_FormatNotSupported,
	kTraFileErr_GameIDMismatch,
	kTraFileErr_UnexpectedEOF,
	kTraFileErr_UnknownBlockType,
	kTraFileErr_BlockDataOverlapping,
};

enum TraFileBlock {
	kTraFblk_ExtStrID = 0,
	kTraFblk_Dict = 1,
	kTraFblk_GameID = 2,
	kTraFblk_TextOpts = 3,
	// End of data tag
	kTraFile_EOF = -1
};

String GetTraFileErrorText(TraFileErrorType err);
String GetTraBlockName(TraFileBlock id);

typedef TypedCodeError<TraFileErrorType, GetTraFileErrorText> TraFileError;


struct Translation {
	// Game identifiers, for matching the translation file with the game
	int GameUid;
	String GameName;
	// Translation dictionary in source/dest pairs
	StringMap Dict;
	// Localization parameters
	int NormalFont = -1; // replacement for normal font, or -1 for default
	int SpeechFont = -1; // replacement for speech font, or -1 for default
	int RightToLeft = -1; // r2l text mode (0, 1), or -1 for default
	StringMap StrOptions; // to store extended options with string values
};


// Parses translation data and tests whether it matches the given game
HError TestTraGameID(int game_uid, const String &game_name, Stream *in);
// Reads full translation data from the provided stream
HError ReadTraData(Translation &tra, Stream *in);
// Writes all translation data to the stream
void WriteTraData(const Translation &tra, Stream *out);

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
