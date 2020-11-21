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

#include "ac/characterinfo.h"
#include "ac/gamesetupstructbase.h"
#include "ac/game_version.h"
#include "ac/wordsdictionary.h"
#include "script/cc_script.h"
#include "util/stream.h"

using AGS::Common::Stream;

GameSetupStructBase::GameSetupStructBase()
    : numviews(0)
    , numcharacters(0)
    , playercharacter(-1)
    , totalscore(0)
    , numinvitems(0)
    , numdialog(0)
    , numdlgmessage(0)
    , numfonts(0)
    , color_depth(0)
    , target_win(0)
    , dialog_bullet(0)
    , hotdot(0)
    , hotdotouter(0)
    , uniqueid(0)
    , numgui(0)
    , numcursors(0)
    , default_lipsync_frame(0)
    , invhotdotsprite(0)
    , dict(nullptr)
    , globalscript(nullptr)
    , chars(nullptr)
    , compiled_script(nullptr)
    , load_messages(nullptr)
    , load_dictionary(false)
    , load_compiled_script(false)
    , _resolutionType(kGameResolution_Undefined)
    , _dataUpscaleMult(1)
    , _screenUpscaleMult(1)
{
    memset(gamename, 0, sizeof(gamename));
    memset(options, 0, sizeof(options));
    memset(paluses, 0, sizeof(paluses));
    memset(defpal, 0, sizeof(defpal));
    memset(reserved, 0, sizeof(reserved));
    memset(messages, 0, sizeof(messages));
}

GameSetupStructBase::~GameSetupStructBase()
{
    Free();
}

void GameSetupStructBase::Free()
{
    for (int i = 0; i < MAXGLOBALMES; ++i)
    {
        delete[] messages[i];
        messages[i] = nullptr;
    }
    delete[] load_messages;
    load_messages = nullptr;
    delete dict;
    dict = nullptr;
    delete globalscript;
    globalscript = nullptr;
    delete compiled_script;
    compiled_script = nullptr;
    delete[] chars;
    chars = nullptr;
}

void GameSetupStructBase::SetDefaultResolution(GameResolutionType type)
{
    SetDefaultResolution(type, Size());
}

void GameSetupStructBase::SetDefaultResolution(Size size)
{
    SetDefaultResolution(kGameResolution_Custom, size);
}

void GameSetupStructBase::SetDefaultResolution(GameResolutionType type, Size size)
{
    // Calculate native res first then remember it
    SetNativeResolution(type, size);
    _defGameResolution = _gameResolution;
    // Setup data resolution according to legacy settings (if set)
    _dataResolution = _defGameResolution;
    if (IsLegacyHiRes() && options[OPT_NATIVECOORDINATES] == 0)
    {
        _dataResolution = _defGameResolution / HIRES_COORD_MULTIPLIER;
    }
    OnResolutionSet();
}

void GameSetupStructBase::SetNativeResolution(GameResolutionType type, Size game_res)
{
    if (type == kGameResolution_Custom)
    {
        _resolutionType = kGameResolution_Custom;
        _gameResolution = game_res;
        _letterboxSize = _gameResolution;
    }
    else
    {
        _resolutionType = type;
        _gameResolution = ResolutionTypeToSize(_resolutionType, IsLegacyLetterbox());
        _letterboxSize = ResolutionTypeToSize(_resolutionType, false);
    }
}

void GameSetupStructBase::SetGameResolution(GameResolutionType type)
{
    SetNativeResolution(type, Size());
    OnResolutionSet();
}

void GameSetupStructBase::SetGameResolution(Size game_res)
{
    SetNativeResolution(kGameResolution_Custom, game_res);
    OnResolutionSet();
}

void GameSetupStructBase::OnResolutionSet()
{
    // The final data-to-game multiplier is always set after actual game resolution (not default one)
    if (!_dataResolution.IsNull())
        _dataUpscaleMult = _gameResolution.Width / _dataResolution.Width;
    else
        _dataUpscaleMult = 1;
    if (!_defGameResolution.IsNull())
        _screenUpscaleMult = _gameResolution.Width / _defGameResolution.Width;
    else
        _screenUpscaleMult = 1;
    _relativeUIMult = IsLegacyHiRes() ? HIRES_COORD_MULTIPLIER : 1;
}

void GameSetupStructBase::ReadFromFile(Stream *in)
{
    in->Read(&gamename[0], GAME_NAME_LENGTH);
    in->ReadArrayOfInt32(options, MAX_OPTIONS);
    if (loaded_game_file_version < kGameVersion_340_4)
    { // TODO: this should probably be possible to deduce script API level
      // using game data version and other options like OPT_STRICTSCRIPTING
        options[OPT_BASESCRIPTAPI] = kScriptAPI_Undefined;
        options[OPT_SCRIPTCOMPATLEV] = kScriptAPI_Undefined;
    }
    in->Read(&paluses[0], 256);
    // colors are an array of chars
    in->Read(&defpal[0], sizeof(color)*256);
    numviews = in->ReadInt32();
    numcharacters = in->ReadInt32();
    playercharacter = in->ReadInt32();
    totalscore = in->ReadInt32();
    numinvitems = in->ReadInt16();
    numdialog = in->ReadInt32();
    numdlgmessage = in->ReadInt32();
    numfonts = in->ReadInt32();
    color_depth = in->ReadInt32();
    target_win = in->ReadInt32();
    dialog_bullet = in->ReadInt32();
    hotdot = in->ReadInt16();
    hotdotouter = in->ReadInt16();
    uniqueid = in->ReadInt32();
    numgui = in->ReadInt32();
    numcursors = in->ReadInt32();
    GameResolutionType resolution_type = (GameResolutionType)in->ReadInt32();
    Size game_size;
    if (resolution_type == kGameResolution_Custom && loaded_game_file_version >= kGameVersion_330)
    {
        game_size.Width = in->ReadInt32();
        game_size.Height = in->ReadInt32();
    }
    SetDefaultResolution(resolution_type, game_size);

    default_lipsync_frame = in->ReadInt32();
    invhotdotsprite = in->ReadInt32();
    in->ReadArrayOfInt32(reserved, NUM_INTS_RESERVED);
    load_messages = new int32_t[MAXGLOBALMES];
    in->ReadArrayOfInt32(load_messages, MAXGLOBALMES);

    // - GameSetupStruct::read_words_dictionary() checks load_dictionary
    // - load_game_file() checks load_compiled_script
    load_dictionary = in->ReadInt32() != 0;
    in->ReadInt32(); // globalscript
    in->ReadInt32(); // chars
    load_compiled_script = in->ReadInt32() != 0;
}

void GameSetupStructBase::WriteToFile(Stream *out)
{
    out->Write(&gamename[0], 50);
    out->WriteArrayOfInt32(options, 100);
    out->Write(&paluses[0], 256);
    // colors are an array of chars
    out->Write(&defpal[0], sizeof(color)*256);
    out->WriteInt32(numviews);
    out->WriteInt32(numcharacters);
    out->WriteInt32(playercharacter);
    out->WriteInt32(totalscore);
    out->WriteInt16(numinvitems);
    out->WriteInt32(numdialog);
    out->WriteInt32(numdlgmessage);
    out->WriteInt32(numfonts);
    out->WriteInt32(color_depth);
    out->WriteInt32(target_win);
    out->WriteInt32(dialog_bullet);
    out->WriteInt16(hotdot);
    out->WriteInt16(hotdotouter);
    out->WriteInt32(uniqueid);
    out->WriteInt32(numgui);
    out->WriteInt32(numcursors);
    out->WriteInt32(_resolutionType);
    if (_resolutionType == kGameResolution_Custom)
    {
        out->WriteInt32(_defGameResolution.Width);
        out->WriteInt32(_defGameResolution.Height);
    }
    out->WriteInt32(default_lipsync_frame);
    out->WriteInt32(invhotdotsprite);
    out->WriteArrayOfInt32(reserved, 17);
    for (int i = 0; i < MAXGLOBALMES; ++i)
    {
        out->WriteInt32(messages[i] ? 1 : 0);
    }
    out->WriteInt32(dict ? 1 : 0);
    out->WriteInt32(0); // globalscript
    out->WriteInt32(0); // chars
    out->WriteInt32(compiled_script ? 1 : 0);
}

Size ResolutionTypeToSize(GameResolutionType resolution, bool letterbox)
{
    switch (resolution)
    {
    case kGameResolution_Default:
    case kGameResolution_320x200:
        return letterbox ? Size(320, 240) : Size(320, 200);
    case kGameResolution_320x240:
        return Size(320, 240);
    case kGameResolution_640x400:
        return letterbox ? Size(640, 480) : Size(640, 400);
    case kGameResolution_640x480:
        return Size(640, 480);
    case kGameResolution_800x600:
        return Size(800, 600);
    case kGameResolution_1024x768:
        return Size(1024, 768);
    case kGameResolution_1280x720:
        return Size(1280,720);
    }
    return Size();
}
