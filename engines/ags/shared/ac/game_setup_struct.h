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
// GameSetupStruct is a contemporary main game data.
//
//=============================================================================

#ifndef AGS_SHARED_AC_GAME_SETUP_STRUCT_H
#define AGS_SHARED_AC_GAME_SETUP_STRUCT_H

#include "common/std/array.h"
#include "common/std/vector.h"
#include "ags/shared/ac/audio_clip_type.h"
#include "ags/shared/ac/character_info.h" // TODO: constants to separate header
#include "ags/shared/ac/game_setup_struct_base.h"
#include "ags/shared/ac/inventory_item_info.h"
#include "ags/shared/ac/mouse_cursor.h"
#include "ags/shared/ac/dynobj/script_audio_clip.h"
#include "ags/shared/game/custom_properties.h"
#include "ags/shared/game/main_game_file.h" // TODO: constants to separate header or split out reading functions

namespace AGS3 {

namespace AGS {
namespace Shared {
struct AssetLibInfo;
struct Interaction;
struct InteractionScripts;
typedef std::shared_ptr<Interaction> PInteraction;
typedef std::shared_ptr<InteractionScripts> PInteractionScripts;
} // namespace Shared
} // namespace AGS

using AGS::Shared::PInteraction;
using AGS::Shared::PInteractionScripts;
using AGS::Shared::HGameFileError;


// TODO: split GameSetupStruct into struct used to hold loaded game data, and actual runtime object
struct GameSetupStruct : public GameSetupStructBase {
	// This array is used only to read data into;
	// font parameters are then put and queried in the fonts module
	// TODO: split into installation params (used only when reading) and runtime params
	std::vector<FontInfo> fonts;
	InventoryItemInfo invinfo[MAX_INV]{};
	std::vector<MouseCursor> mcurs;
	std::vector<PInteraction> intrChar;
	PInteraction intrInv[MAX_INV];
	std::vector<PInteractionScripts> charScripts;
	std::vector<PInteractionScripts> invScripts;
	// TODO: why we do not use this in the engine instead of
	// _G(loaded_game_file_version)?
	int               filever;  // just used by editor
	Shared::String    compiled_with; // version of AGS this data was created by
	char              lipSyncFrameLetters[MAXLIPSYNCFRAMES][50];
	AGS::Shared::PropertySchema propSchema;
	std::vector<AGS::Shared::StringIMap> charProps;
	AGS::Shared::StringIMap invProps[MAX_INV];
	// NOTE: although the view names are stored in game data, they are never
	// used, nor registered as script exports; numeric IDs are used to
	// reference views instead.
	std::vector<Shared::String> viewNames;
	Shared::String    invScriptNames[MAX_INV];
	std::vector<Shared::String> dialogScriptNames;
	char              guid[MAX_GUID_LENGTH];
	char              saveGameFileExtension[MAX_SG_EXT_LENGTH];
	// NOTE: saveGameFolderName is generally used to create game subdirs in common user directories
	Shared::String    saveGameFolderName;
	int               roomCount;
	std::vector<int>  roomNumbers;
	std::vector<Shared::String> roomNames;
	std::vector<ScriptAudioClip> audioClips;
	std::vector<AudioClipType> audioClipTypes;
	// A clip to play when player gains score in game
	// TODO: find out why OPT_SCORESOUND option cannot be used to store this in >=3.2 games
	int               scoreClipID;
	// number of accessible game audio channels (the ones under direct user control)
	int               numGameChannels = 0;
	// backward-compatible channel limit that may be exported to script and reserved by audiotypes
	int               numCompatGameChannels = 0;

	// TODO: I converted original array of sprite infos to vector here, because
	// statistically in most games sprites go in long continious sequences with minimal
	// gaps, and standard hash-map will have relatively big memory overhead compared.
	// Of course vector will not behave very well if user has created e.g. only
	// sprite #1 and sprite #1000000. For that reason I decided to still limit static
	// sprite count to some reasonable number for the time being. Dynamic sprite IDs are
	// added in sequence, so there won't be any issue with these.
	// There could be other collection types, more optimal for this case. For example,
	// we could use a kind of hash map containing fixed-sized arrays, where size of
	// array is calculated based on key spread factor.
	std::vector<SpriteInfo> SpriteInfos;

	// Get game's native color depth (bits per pixel)
	inline int GetColorDepth() const {
		return color_depth * 8;
	}


	GameSetupStruct();
	GameSetupStruct(GameSetupStruct &&gss) = default;
	~GameSetupStruct();

	GameSetupStruct &operator=(GameSetupStruct &&gss) = default;

	void Free();

	// [IKM] Game struct loading code is moved here from Engine's load_game_file
	// function; for now it is not supposed to be called by Editor; although it
	// is possible that eventually will be.
	//
	// Since reading game data is made in a bit inconvenient way I had to
	// a) divide process into three functions (there's some extra stuff
	// being read between them;
	// b) use a helper struct to pass some arguments
	//
	// I also had to move BuildAudioClipArray from the engine and make it
	// GameSetupStruct member.

	//--------------------------------------------------------------------
	// Do not call these directly
	//------------------------------
	// Part 1
	void read_savegame_info(Shared::Stream *in, GameDataVersion data_ver);
	void read_font_infos(Shared::Stream *in, GameDataVersion data_ver);
	HGameFileError read_cursors(Shared::Stream *in);
	void read_interaction_scripts(Shared::Stream *in, GameDataVersion data_ver);
	void read_words_dictionary(Shared::Stream *in);

	void ReadInvInfo(Shared::Stream *in);
	void WriteInvInfo(Shared::Stream *out);
	void ReadMouseCursors(Shared::Stream *in);
	void WriteMouseCursors(Shared::Stream *out);
	//------------------------------
	// Part 2
	void read_characters(Shared::Stream *in);
	void read_lipsync(Shared::Stream *in, GameDataVersion data_ver);
	void read_messages(Shared::Stream *in, const std::array<int32_t> &load_messages, GameDataVersion data_ver);

	void ReadCharacters(Shared::Stream *in);
	void WriteCharacters(Shared::Stream *out);
	//------------------------------
	// Part 3
	HGameFileError read_customprops(Shared::Stream *in, GameDataVersion data_ver);
	HGameFileError read_audio(Shared::Stream *in, GameDataVersion data_ver);
	void read_room_names(Shared::Stream *in, GameDataVersion data_ver);

	void ReadAudioClips(Shared::Stream *in, size_t count);
	//--------------------------------------------------------------------

	// Functions for reading and writing appropriate data from/to save game
	void ReadFromSaveGame_v321(Shared::Stream *in);

	void ReadFromSavegame(Shared::Stream *in);
	void WriteForSavegame(Shared::Stream *out);
};

//=============================================================================
#if defined (OBSOLETE)
struct OldGameSetupStruct;
void ConvertOldGameStruct(OldGameSetupStruct *ogss, GameSetupStruct *gss);
#endif // OBSOLETE

// Finds an audio clip using legacy convention index
ScriptAudioClip *GetAudioClipForOldStyleNumber(GameSetupStruct &game, bool is_music, int num);

} // namespace AGS3

#endif
