//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#ifndef __AGS_EE_GAME__SAVEGAME_H
#define __AGS_EE_GAME__SAVEGAME_H

#include <memory>
#include "ac/game_version.h"
#include "util/error.h"
#include "util/version.h"


namespace AGS
{

namespace Common { class Bitmap; class Stream; }

namespace Engine
{

using Common::Bitmap;
using Common::ErrorHandle;
using Common::TypedCodeError;
using Common::Stream;
using Common::String;
using Common::Version;

typedef std::shared_ptr<Stream> PStream;

//-----------------------------------------------------------------------------
// Savegame version history
//
// 8      last old style saved game format (of AGS 3.2.1)
// 9      first new style (self-descriptive block-based) format version
//-----------------------------------------------------------------------------
enum SavegameVersion
{
    kSvgVersion_Undefined = 0,
    kSvgVersion_321       = 8,
    kSvgVersion_Components= 9,
    kSvgVersion_Cmp_64bit = 10,
    kSvgVersion_350_final = 11,
    kSvgVersion_350_final2= 12,
    kSvgVersion_Current   = kSvgVersion_350_final2,
    kSvgVersion_LowestSupported = kSvgVersion_321 // change if support dropped
};

// Error codes for save restoration routine
enum SavegameErrorType
{
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
struct SavegameSource
{
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
enum SavegameDescElem
{
    kSvgDesc_None       = 0,
    kSvgDesc_EnvInfo    = 0x0001,
    kSvgDesc_UserText   = 0x0002,
    kSvgDesc_UserImage  = 0x0004,
    kSvgDesc_All        = kSvgDesc_EnvInfo | kSvgDesc_UserText | kSvgDesc_UserImage
};

// SavegameDescription describes savegame with information about the enviroment
// it was created in, and custom data provided by user
struct SavegameDescription
{
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

#endif // __AGS_EE_GAME__SAVEGAME_H
