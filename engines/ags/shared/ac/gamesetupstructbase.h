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
//
//
//
//=============================================================================
#ifndef __AGS_CN_AC__GAMESETUPSTRUCTBASE_H
#define __AGS_CN_AC__GAMESETUPSTRUCTBASE_H

#include "ac/game_version.h"
#include "ac/gamestructdefines.h"
#include "util/string.h"
#include "util/wgt2allg.h" // color (allegro RGB)

// Forward declaration
namespace AGS { namespace Common { class Stream; } }
using namespace AGS; // FIXME later

struct WordsDictionary;
struct CharacterInfo;
struct ccScript;


struct GameSetupStructBase {
    static const int  GAME_NAME_LENGTH = 50;
    static const int  MAX_OPTIONS = 100;
    static const int  NUM_INTS_RESERVED = 17;

    char              gamename[GAME_NAME_LENGTH];
    int               options[MAX_OPTIONS];
    unsigned char     paluses[256];
    color             defpal[256];
    int               numviews;
    int               numcharacters;
    int               playercharacter;
    int               totalscore;
    short             numinvitems;
    int               numdialog, numdlgmessage;
    int               numfonts;
    int               color_depth;          // in bytes per pixel (ie. 1 or 2)
    int               target_win;
    int               dialog_bullet;        // 0 for none, otherwise slot num of bullet point
    unsigned short    hotdot, hotdotouter;  // inv cursor hotspot dot color
    int               uniqueid;    // random key identifying the game
    int               numgui;
    int               numcursors;
    int               default_lipsync_frame; // used for unknown chars
    int               invhotdotsprite;
    int               reserved[NUM_INTS_RESERVED];
    char             *messages[MAXGLOBALMES];
    WordsDictionary  *dict;
    char             *globalscript;
    CharacterInfo    *chars;
    ccScript         *compiled_script;

    int             *load_messages;
    bool             load_dictionary;
    bool             load_compiled_script;
    // [IKM] 2013-03-30
    // NOTE: it looks like nor 'globalscript', not 'compiled_script' are used
    // to store actual script data anytime; 'ccScript* gamescript' global
    // pointer is used for that instead.

    GameSetupStructBase();
    ~GameSetupStructBase();
    void Free();
    void SetDefaultResolution(GameResolutionType type);
    void SetDefaultResolution(Size game_res);
    void SetGameResolution(GameResolutionType type);
    void SetGameResolution(Size game_res);
    void ReadFromFile(Common::Stream *in);
    void WriteToFile(Common::Stream *out);


    //
    // ** On game resolution.
    //
    // Game resolution is a size of a native game screen in pixels.
    // This is the "game resolution" that developer sets up in AGS Editor.
    // It is in the same units in which sprite and font sizes are defined.
    //
    // Graphic renderer may scale and stretch game's frame as requested by
    // player or system, which will not affect native coordinates in any way.
    //
    // ** Legacy upscale mode.
    //
    // In the past engine had a separation between logical and native screen
    // coordinates and supported running games "upscaled". E.g. 320x200 games
    // could be run as 640x400. This was not done by simply stretching final
    // game's drawn frame to the larger window, but by multiplying all data
    // containing coordinates and graphics either on load or real-time.
    // Games of 640x400 and above were scripted and set up in coordinate units
    // that were always x2 times smaller than the one developer chose.
    // For example, choosing a 640x400 resolution would make game draw itself
    // as 640x400, but all the game logic (object properties, script commands)
    // would work in 320x200 (this also let run 640x400 downscaled to 320x200).
    // Ignoring the obvious complications, the known benefit from such approach
    // was that developers could supply separate sets of fonts and sprites for
    // low-res and high-res modes.
    // The 3rd generation of AGS still allows to achieve same effect by using
    // backwards-compatible option (although it is not recommended except when
    // importing and continuing old projects).
    //
    // In order to support this legacy behavior we have a set of functions for
    // coordinate conversion. They are required to move from "data" resolution
    // to "final game" resolution and back.
    //
    // Some of the script commands, as well as some internal engine data use
    // coordinates in "game resolution" instead (this should be documented).
    // In such case there's another conversion which translates these from
    // default to actual resolution; e.g. when 320x200 game is run as 640x400
    // they should be multiplied by 2.
    //
    // ** TODO.
    //
    // Truth be told, all this is still implemented incorrectly, because no one
    // found time to rewrite the thing. The correct way would perhaps be:
    // 1) treat old games as x2 lower resolution than they say.
    // 2) support drawing particular sprites and texts in x2 higher resolution
    // (assuming display resolution allows). The latter is potentially enabled
    // by "sprite batches" system in the engine and will benefit new games too.

    inline GameResolutionType GetResolutionType() const
    {
        return _resolutionType;
    }

    // Get actual game's resolution
    const Size &GetGameRes() const { return _gameResolution; }
    // Get default resolution the game was created for;
    // this is usually equal to GetGameRes except for legacy modes.
    const Size &GetDefaultRes() const { return _defGameResolution; }
    // Get data & script resolution;
    // this is usually equal to GetGameRes except for legacy modes.
    const Size &GetDataRes() const { return _dataResolution; }
    // Get game data-->final game resolution coordinate multiplier
    inline int GetDataUpscaleMult() const { return _dataUpscaleMult; }
    // Get multiplier for various default UI sizes, meant to keep UI looks
    // more or less readable in any game resolution.
    // TODO: find a better solution for UI sizes, perhaps make variables.
    inline int GetRelativeUIMult() const { return _relativeUIMult; }
    // Get game default res-->final game resolution coordinate multiplier;
    // used to convert coordinates from original game res to actual one
    inline int GetScreenUpscaleMult() const { return _screenUpscaleMult; }
    // Tells if game allows assets defined in relative resolution;
    // that is - have to be converted to this game resolution type
    inline bool AllowRelativeRes() const { return options[OPT_RELATIVEASSETRES] != 0; }
    // Legacy definition of high and low game resolution.
    // Used to determine certain hardcoded coordinate conversion logic, but
    // does not make much sense today when the resolution is arbitrary.
    inline bool IsLegacyHiRes() const
    {
        if (_resolutionType == kGameResolution_Custom)
            return (_gameResolution.Width * _gameResolution.Height) > (320 * 240);
        return ::IsLegacyHiRes(_resolutionType);
    }
    // Tells if data has coordinates in default game resolution
    inline bool IsDataInNativeCoordinates() const { return options[OPT_NATIVECOORDINATES] != 0; }
    
    // Tells if game runs in native letterbox mode (legacy option)
    inline bool IsLegacyLetterbox() const { return options[OPT_LETTERBOX] != 0; }
    // Get letterboxed frame size
    const Size &GetLetterboxSize() const { return _letterboxSize; }

    // Room region/hotspot masks are traditionally 1:1 of the room's size in
    // low-resolution games and 1:2 of the room size in high-resolution games.
    // This also means that mask relation to data resolution is 1:1 if the
    // game uses low-res coordinates in script and 1:2 if high-res.

    // Test if the game is built around old audio system
    inline bool IsLegacyAudioSystem() const
    {
        return loaded_game_file_version < kGameVersion_320;
    }

    // Returns the expected filename of a digital audio package
    inline AGS::Common::String GetAudioVOXName() const
    {
        return IsLegacyAudioSystem() ? "music.vox" : "audio.vox";
    }

private:
    void SetDefaultResolution(GameResolutionType type, Size game_res);
    void SetNativeResolution(GameResolutionType type, Size game_res);
    void OnResolutionSet();

    // Game's native resolution ID, used to init following values.
    GameResolutionType _resolutionType;

    // Determines game's default screen resolution. Use for the reference
    // when comparing with actual screen resolution, which may be modified
    // by certain overriding game modes.
    Size _defGameResolution;
    // Determines game's actual resolution.
    Size _gameResolution;
    // Determines resolution in which loaded data and script define coordinates
    // and sizes (with very little exception).
    Size _dataResolution;
    // Letterboxed frame size. Used when old game is run in native letterbox
    // mode. In all other situations is equal to game's resolution.
    Size _letterboxSize;

    // Game logic to game resolution coordinate factor
    int _dataUpscaleMult;
    // Multiplier for various UI drawin sizes, meant to keep UI elements readable
    int _relativeUIMult;
    // Game default resolution to actual game resolution factor
    int _screenUpscaleMult;
};

#endif // __AGS_CN_AC__GAMESETUPSTRUCTBASE_H