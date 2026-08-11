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

//
//=============================================================================
//
// GameSetupStructBase is a base class for main game data.
//
//=============================================================================

#ifndef AGS_SHARED_AC_GAME_SETUP_STRUCT_BASE_H
#define AGS_SHARED_AC_GAME_SETUP_STRUCT_BASE_H

#include "ags/lib/allegro.h" // RGB
#include "common/std/array.h"
#include "common/std/memory.h"
#include "common/std/vector.h"
#include "ags/shared/ac/game_version.h"
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/shared/ac/words_dictionary.h"
#include "ags/shared/util/string.h"
#include "ags/globals.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

struct CharacterInfo;
struct ccScript;


struct GameSetupStructBase {
	static const int  LEGACY_GAME_NAME_LENGTH = 50;
	static const int  MAX_OPTIONS = 100;
	static const int  NUM_INTS_RESERVED = 16;

	Shared::String    gamename;
	int32_t           options[MAX_OPTIONS];
	uint8_t           paluses[256];
	RGB               defpal[256];
	int               numviews;
	int               numcharacters;
	int               playercharacter;
	int               totalscore;
	int               numinvitems;
	int               numdialog;
	int               numdlgmessage;  // [DEPRECATED]
	int               numfonts;
	int               color_depth;          // in bytes per pixel (ie. 1 or 2)
	int               target_win;
	int               dialog_bullet;        // 0 for none, otherwise slot num of bullet point
	int               hotdot;  // inv cursor hotspot dot color
	int               hotdotouter;
	int               uniqueid;    // random key identifying the game
	int               numgui;
	int               numcursors;
	int               default_lipsync_frame; // used for unknown chars
	int               invhotdotsprite;
	int32_t           reserved[NUM_INTS_RESERVED];
	String			  messages[MAXGLOBALMES];
	std::unique_ptr<WordsDictionary> dict;
	std::vector<CharacterInfo> chars;
	std::vector<CharacterInfo2> chars2; // extended character fields

	GameSetupStructBase();
	GameSetupStructBase(GameSetupStructBase &&gss) = default;
	~GameSetupStructBase();

	GameSetupStructBase &operator=(GameSetupStructBase &&gss) = default;

	void Free();
	void SetDefaultResolution(GameResolutionType type);
	void SetDefaultResolution(Size game_res);
	void SetGameResolution(GameResolutionType type);
	void SetGameResolution(Size game_res);

	// Tells whether the serialized game data contains certain components
	struct SerializeInfo {
		bool HasCCScript = false;
		bool HasWordsDict = false;
		std::array<int32_t> HasMessages;
		// File offset at which game data extensions begin
		uint32_t ExtensionOffset = 0u;

		SerializeInfo() {
			HasMessages.resize(MAXGLOBALMES);
		}
	};

	void ReadFromFile(Shared::Stream *in, GameDataVersion game_ver, SerializeInfo &info);
	void WriteToFile(Shared::Stream *out, const SerializeInfo &info) const;

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

	inline GameResolutionType GetResolutionType() const {
		return _resolutionType;
	}

	// Get actual game's resolution
	const Size &GetGameRes() const {
		return _gameResolution;
	}
	// Get default resolution the game was created for;
	// this is usually equal to GetGameRes except for legacy modes.
	const Size &GetDefaultRes() const {
		return _defGameResolution;
	}
	// Get data & script resolution;
	// this is usually equal to GetGameRes except for legacy modes.
	const Size &GetDataRes() const {
		return _dataResolution;
	}
	// Get game data-->final game resolution coordinate multiplier
	inline int GetDataUpscaleMult() const {
		return _dataUpscaleMult;
	}
	// Get multiplier for various default UI sizes, meant to keep UI looks
	// more or less readable in any game resolution.
	// TODO: find a better solution for UI sizes, perhaps make variables.
	inline int GetRelativeUIMult() const {
		return _relativeUIMult;
	}
	// Get game default res-->final game resolution coordinate multiplier;
	// used to convert coordinates from original game res to actual one
	inline int GetScreenUpscaleMult() const {
		return _screenUpscaleMult;
	}
	// Tells if game allows assets defined in relative resolution;
	// that is - have to be converted to this game resolution type
	inline bool AllowRelativeRes() const {
		return options[OPT_RELATIVEASSETRES] != 0;
	}
	// Legacy definition of high and low game resolution.
	// Used to determine certain hardcoded coordinate conversion logic, but
	// does not make much sense today when the resolution is arbitrary.
	inline bool IsLegacyHiRes() const {
		if (_resolutionType == kGameResolution_Custom)
			return (_gameResolution.Width * _gameResolution.Height) > (320 * 240);
		return ::AGS3::IsLegacyHiRes(_resolutionType);
	}
	// Tells if data has coordinates in default game resolution
	inline bool IsDataInNativeCoordinates() const {
		return options[OPT_NATIVECOORDINATES] != 0;
	}

	// Tells if game runs in native letterbox mode (legacy option)
	inline bool IsLegacyLetterbox() const {
		return options[OPT_LETTERBOX] != 0;
	}
	// Get letterboxed frame size
	const Size &GetLetterboxSize() const {
		return _letterboxSize;
	}

	// Room region/hotspot masks are traditionally 1:1 of the room's size in
	// low-resolution games and 1:2 of the room size in high-resolution games.
	// This also means that mask relation to data resolution is 1:1 if the
	// game uses low-res coordinates in script and 1:2 if high-res.

	// Test if the game is built around old audio system
	inline bool IsLegacyAudioSystem() const {
		return _G(loaded_game_file_version) < kGameVersion_320;
	}

	// Returns the expected filename of a digital audio package
	inline AGS::Shared::String GetAudioVOXName() const {
		return IsLegacyAudioSystem() ? "music.vox" : "audio.vox";
	}

	// Returns a list of game options that are forbidden to change at runtime
	inline static Common::Array<int> GetRestrictedOptions() {
		return Common::Array<int> {{
			OPT_DEBUGMODE, OPT_LETTERBOX, OPT_HIRES_FONTS, OPT_SPLITRESOURCES,
			OPT_STRICTSCRIPTING, OPT_LEFTTORIGHTEVAL, OPT_COMPRESSSPRITES, OPT_STRICTSTRINGS,
			OPT_NATIVECOORDINATES, OPT_SAFEFILEPATHS, OPT_DIALOGOPTIONSAPI, OPT_BASESCRIPTAPI,
			OPT_SCRIPTCOMPATLEV, OPT_RELATIVEASSETRES, OPT_GAMETEXTENCODING, OPT_KEYHANDLEAPI,
			OPT_CUSTOMENGINETAG
		}};
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

} // namespace AGS3

#endif
