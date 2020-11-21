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

#ifndef AGS_SHARED_AC_GAMESETUPSTRUCT_H
#define AGS_SHARED_AC_GAMESETUPSTRUCT_H

#include <vector>
#include "ac/audiocliptype.h"
#include "ac/characterinfo.h" // TODO: constants to separate header
#include "ac/gamesetupstructbase.h"
#include "ac/inventoryiteminfo.h"
#include "ac/mousecursor.h"
#include "ac/dynobj/scriptaudioclip.h"
#include "game/customproperties.h"
#include "game/main_game_file.h" // TODO: constants to separate header or split out reading functions

namespace AGS3 {

namespace AGS {
namespace Common {
struct AssetLibInfo;
struct Interaction;
struct InteractionScripts;
typedef std::shared_ptr<Interaction> PInteraction;
typedef std::shared_ptr<InteractionScripts> PInteractionScripts;
} // namespace Shared
} // namespace AGS

using AGS::Common::PInteraction;
using AGS::Common::PInteractionScripts;
using AGS::Common::HGameFileError;
struct OldGameSetupStruct;


// TODO: split GameSetupStruct into struct used to hold loaded game data, and actual runtime object
struct GameSetupStruct : public GameSetupStructBase {
	// This array is used only to read data into;
	// font parameters are then put and queried in the fonts module
	// TODO: split into installation params (used only when reading) and runtime params
	std::vector<FontInfo> fonts;
	InventoryItemInfo invinfo[MAX_INV];
	MouseCursor       mcurs[MAX_CURSOR];
	std::vector<PInteraction> intrChar;
	PInteraction intrInv[MAX_INV];
	std::vector<PInteractionScripts> charScripts;
	std::vector<PInteractionScripts> invScripts;
	// TODO: why we do not use this in the engine instead of
	// loaded_game_file_version?
	int               filever;  // just used by editor
	Common::String    compiled_with; // version of AGS this data was created by
	char              lipSyncFrameLetters[MAXLIPSYNCFRAMES][50];
	AGS::Common::PropertySchema propSchema;
	std::vector<AGS::Common::StringIMap> charProps;
	AGS::Common::StringIMap invProps[MAX_INV];
	// NOTE: although the view names are stored in game data, they are never
	// used, nor registered as script exports; numeric IDs are used to
	// reference views instead.
	std::vector<Common::String> viewNames;
	Common::String    invScriptNames[MAX_INV];
	std::vector<Common::String> dialogScriptNames;
	char              guid[MAX_GUID_LENGTH];
	char              saveGameFileExtension[MAX_SG_EXT_LENGTH];
	char              saveGameFolderName[MAX_SG_FOLDER_LEN];
	int               roomCount;
	int *roomNumbers;
	char **roomNames;
	std::vector<ScriptAudioClip> audioClips;
	std::vector<AudioClipType> audioClipTypes;
	// A clip to play when player gains score in game
	// TODO: find out why OPT_SCORESOUND option cannot be used to store this in >=3.2 games
	int               scoreClipID;

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
	~GameSetupStruct();

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
	void read_savegame_info(Common::Stream *in, GameDataVersion data_ver);
	void read_font_infos(Common::Stream *in, GameDataVersion data_ver);
	HGameFileError read_cursors(Common::Stream *in, GameDataVersion data_ver);
	void read_interaction_scripts(Common::Stream *in, GameDataVersion data_ver);
	void read_words_dictionary(Common::Stream *in);

	void ReadInvInfo_Aligned(Common::Stream *in);
	void WriteInvInfo_Aligned(Common::Stream *out);
	void ReadMouseCursors_Aligned(Common::Stream *in);
	void WriteMouseCursors_Aligned(Common::Stream *out);
	//------------------------------
	// Part 2
	void read_characters(Common::Stream *in, GameDataVersion data_ver);
	void read_lipsync(Common::Stream *in, GameDataVersion data_ver);
	void read_messages(Common::Stream *in, GameDataVersion data_ver);

	void ReadCharacters_Aligned(Common::Stream *in);
	void WriteCharacters_Aligned(Common::Stream *out);
	//------------------------------
	// Part 3
	HGameFileError read_customprops(Common::Stream *in, GameDataVersion data_ver);
	HGameFileError read_audio(Common::Stream *in, GameDataVersion data_ver);
	void read_room_names(Common::Stream *in, GameDataVersion data_ver);

	void ReadAudioClips_Aligned(Common::Stream *in, size_t count);
	//--------------------------------------------------------------------

	// Functions for reading and writing appropriate data from/to save game
	void ReadFromSaveGame_v321(Common::Stream *in, char *gswas, ccScript *compsc, CharacterInfo *chwas,
		WordsDictionary *olddict, char **mesbk);

	void ReadFromSavegame(Common::PStream in);
	void WriteForSavegame(Common::PStream out);
};

//=============================================================================
// TODO: find out how this function was supposed to be used
void ConvertOldGameStruct(OldGameSetupStruct *ogss, GameSetupStruct *gss);
// Finds an audio clip using legacy convention index
ScriptAudioClip *GetAudioClipForOldStyleNumber(GameSetupStruct &game, bool is_music, int num);

} // namespace AGS3

#endif
