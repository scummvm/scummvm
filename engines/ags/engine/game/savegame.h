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

#ifndef AGS_ENGINE_GAME_SAVEGAME_H
#define AGS_ENGINE_GAME_SAVEGAME_H

#include "ags/lib/std/memory.h"
#include "ags/shared/ac/game_version.h"
#include "ags/shared/util/error.h"
#include "ags/shared/util/version.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Bitmap;
class Stream;
} // namespace Shared

namespace Engine {

using Shared::Bitmap;
using Shared::ErrorHandle;
using Shared::TypedCodeError;
using Shared::Stream;
using Shared::String;
using Shared::Version;

typedef std::shared_ptr<Stream> PStream;

//-----------------------------------------------------------------------------
// Savegame version history
//
// 8      last old style saved game format (of AGS 3.2.1)
// 9      first new style (self-descriptive block-based) format version
//-----------------------------------------------------------------------------
enum SavegameVersion {
	kSvgVersion_Undefined = 0,
	kSvgVersion_321 = 8,
	kSvgVersion_Components = 9,
	kSvgVersion_Cmp_64bit = 10,
	kSvgVersion_350_final = 11,
	kSvgVersion_350_final2 = 12,
	kSvgVersion_Current = kSvgVersion_350_final2,
	kSvgVersion_LowestSupported = kSvgVersion_321 // change if support dropped
};

// Error codes for save restoration routine
enum SavegameErrorType {
	kSvgErr_NoError,
	kSvgErr_FileOpenFailed,
	kSvgErr_SignatureFailed,
	kSvgErr_FormatVersionNotSupported,
	kSvgErr_IncompatibleEngine,
	kSvgErr_GameGuidMismatch,
	kSvgErr_ComponentListOpeningTagFormat,
	kSvgErr_ComponentListClosingTagMissing,
	kSvgErr_ComponentOpeningTagFormat,
	kSvgErr_ComponentClosingTagFormat,
	kSvgErr_ComponentSizeMismatch,
	kSvgErr_UnsupportedComponent,
	kSvgErr_ComponentSerialization,
	kSvgErr_ComponentUnserialization,
	kSvgErr_InconsistentFormat,
	kSvgErr_UnsupportedComponentVersion,
	kSvgErr_GameContentAssertion,
	kSvgErr_InconsistentData,
	kSvgErr_InconsistentPlugin,
	kSvgErr_DifferentColorDepth,
	kSvgErr_GameObjectInitFailed,
	kNumSavegameError
};

String GetSavegameErrorText(SavegameErrorType err);

typedef TypedCodeError<SavegameErrorType, GetSavegameErrorText> SavegameError;
typedef ErrorHandle<SavegameError> HSaveError;
typedef std::unique_ptr<Stream> UStream;
typedef std::unique_ptr<Bitmap> UBitmap;

// SavegameSource defines a successfully opened savegame stream
struct SavegameSource {
	// Signature of the current savegame format
	static const String Signature;
	// Signature of the legacy savegame format
	static const String LegacySignature;

	// Name of the savefile
	String              Filename;
	// Savegame format version
	SavegameVersion     Version;
	// A ponter to the opened stream
	PStream             InputStream;

	SavegameSource();
};

// Supported elements of savegame description;
// these may be used as flags to define valid fields
enum SavegameDescElem {
	kSvgDesc_None = 0,
	kSvgDesc_EnvInfo = 0x0001,
	kSvgDesc_UserText = 0x0002,
	kSvgDesc_UserImage = 0x0004,
	kSvgDesc_All = kSvgDesc_EnvInfo | kSvgDesc_UserText | kSvgDesc_UserImage
};

// SavegameDescription describes savegame with information about the enviroment
// it was created in, and custom data provided by user
struct SavegameDescription {
	// Name of the engine that saved the game
	String              EngineName;
	// Version of the engine that saved the game
	Version             EngineVersion;
	// Guid of the game which made this save
	String              GameGuid;
	// Title of the game which made this save
	String              GameTitle;
	// Name of the main data file used; this is needed to properly
	// load saves made by "minigames"
	String              MainDataFilename;
	// Game's main data version; should be checked early to know
	// if the save was made for the supported game format
	GameDataVersion     MainDataVersion;
	// Native color depth of the game; this is required to
	// properly restore dynamic graphics from the save
	int                 ColorDepth;

	String              UserText;
	UBitmap             UserImage;

	SavegameDescription();
};


// Opens savegame for reading; optionally reads description, if any is provided
HSaveError     OpenSavegame(const String &filename, SavegameSource &src,
	SavegameDescription &desc, SavegameDescElem elems = kSvgDesc_All);
// Opens savegame and reads the savegame description
HSaveError     OpenSavegame(const String &filename, SavegameDescription &desc, SavegameDescElem elems = kSvgDesc_All);

// Reads the game data from the save stream and reinitializes game state
HSaveError     RestoreGameState(PStream in, SavegameVersion svg_version);

// Opens savegame for writing and puts in savegame description
PStream        StartSavegame(const String &filename, const String &user_text, const Bitmap *user_image);

// Prepares game for saving state and writes game data into the save stream
void           SaveGameState(PStream out);

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
