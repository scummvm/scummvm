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

#include "ags/shared/ac/audio_clip_type.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/ac/old_game_setup_struct.h"
#include "ags/shared/ac/words_dictionary.h"
#include "ags/shared/ac/dynobj/script_audio_clip.h"
#include "ags/shared/game/interactions.h"
#include "ags/shared/util/string_utils.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

GameSetupStruct::GameSetupStruct()
	: filever(0)
	, roomCount(0)
	, scoreClipID(0) {
	memset(lipSyncFrameLetters, 0, sizeof(lipSyncFrameLetters));
	memset(guid, 0, sizeof(guid));
	memset(saveGameFileExtension, 0, sizeof(saveGameFileExtension));
}

GameSetupStruct::~GameSetupStruct() {
	Free();
}

void GameSetupStruct::Free() {
	GameSetupStructBase::Free();

	fonts.clear();
	mcurs.clear();

	intrChar.clear();
	charScripts.clear();
	charProps.clear();

	// TODO: find out if it really needs to begin with 1 here?
	for (size_t i = 1; i < (size_t)MAX_INV; i++) {
		intrInv[i].reset();
		invProps[i].clear();
	}
	invScripts.clear();
	numinvitems = 0;

	viewNames.clear();
	dialogScriptNames.clear();

	roomNames.clear();
	roomNumbers.clear();
	roomCount = 0;

	audioClips.clear();
	audioClipTypes.clear();

	SpriteInfos.clear();
}

// Assigns font info parameters using legacy flags value read from the game data
void SetFontInfoFromLegacyFlags(FontInfo &finfo, const uint8_t data) {
	finfo.Flags = (data >> 6) & 0xFF;
	finfo.Size = data & FFLG_LEGACY_SIZEMASK;
}

void AdjustFontInfoUsingFlags(FontInfo &finfo, const uint32_t flags) {
	finfo.Flags = flags;
	if ((flags & FFLG_SIZEMULTIPLIER) != 0) {
		finfo.SizeMultiplier = finfo.Size;
		finfo.Size = 0;
	}
}

ScriptAudioClip *GetAudioClipForOldStyleNumber(GameSetupStruct &game, bool is_music, int num) {
	String clip_name;
	if (is_music)
		clip_name.Format("aMusic%d", num);
	else
		clip_name.Format("aSound%d", num);

	for (size_t i = 0; i < _GP(game).audioClips.size(); ++i) {
		if (clip_name.CompareNoCase(_GP(game).audioClips[i].scriptName) == 0)
			return &_GP(game).audioClips[i];
	}
	return nullptr;
}

//-----------------------------------------------------------------------------
// Reading Part 1

void GameSetupStruct::read_savegame_info(Shared::Stream *in, GameDataVersion data_ver) {
	if (data_ver > kGameVersion_272) { // only 3.x
		StrUtil::ReadCStrCount(guid, in, MAX_GUID_LENGTH);
		StrUtil::ReadCStrCount(saveGameFileExtension, in, MAX_SG_EXT_LENGTH);
		saveGameFolderName.ReadCount(in, LEGACY_MAX_SG_FOLDER_LEN);
	}
}

void GameSetupStruct::read_font_infos(Shared::Stream *in, GameDataVersion data_ver) {
	fonts.resize(numfonts);
	if (data_ver < kGameVersion_350) {
		for (int i = 0; i < numfonts; ++i)
			SetFontInfoFromLegacyFlags(fonts[i], in->ReadInt8());
		for (int i = 0; i < numfonts; ++i)
			fonts[i].Outline = in->ReadInt8(); // size of char
		if (data_ver < kGameVersion_341)
			return;
		for (int i = 0; i < numfonts; ++i) {
			fonts[i].YOffset = in->ReadInt32();
			if (data_ver >= kGameVersion_341_2)
				fonts[i].LineSpacing = MAX<int32_t>(0, in->ReadInt32());
		}
	} else {
		for (int i = 0; i < numfonts; ++i) {
			uint32_t flags = in->ReadInt32();
			fonts[i].Size = in->ReadInt32();
			fonts[i].Outline = in->ReadInt32();
			fonts[i].YOffset = in->ReadInt32();
			fonts[i].LineSpacing = MAX<int32_t>(0, in->ReadInt32());
			AdjustFontInfoUsingFlags(fonts[i], flags);
		}
	}
}

void GameSetupStruct::ReadInvInfo(Stream *in) {
	for (int i = 0; i < numinvitems; ++i) {
		invinfo[i].ReadFromFile(in);
	}
}

void GameSetupStruct::WriteInvInfo(Stream *out) {
	for (int i = 0; i < numinvitems; ++i) {
		invinfo[i].WriteToFile(out);
	}
}

HGameFileError GameSetupStruct::read_cursors(Shared::Stream *in) {
	mcurs.resize(numcursors);
	ReadMouseCursors(in);
	return HGameFileError::None();
}

void GameSetupStruct::read_interaction_scripts(Shared::Stream *in, GameDataVersion data_ver) {
	_G(numGlobalVars) = 0;

	if (data_ver > kGameVersion_272) { // 3.x
		charScripts.resize(numcharacters);
		invScripts.resize(numinvitems);
		for (size_t i = 0; i < (size_t)numcharacters; ++i)
			charScripts[i].reset(InteractionScripts::CreateFromStream(in));
		// NOTE: new inventory items' events are loaded starting from 1 for some reason
		for (size_t i = 1; i < (size_t)numinvitems; ++i)
			invScripts[i].reset(InteractionScripts::CreateFromStream(in));
	} else { // 2.x
		intrChar.resize(numcharacters);
		for (size_t i = 0; i < (size_t)numcharacters; ++i)
			intrChar[i].reset(Interaction::CreateFromStream(in));
		for (size_t i = 0; i < (size_t)numinvitems; ++i)
			intrInv[i].reset(Interaction::CreateFromStream(in));

		_G(numGlobalVars) = in->ReadInt32();
		for (size_t i = 0; i < (size_t)_G(numGlobalVars); ++i)
			_G(globalvars)[i].Read(in);
	}
}

void GameSetupStruct::read_words_dictionary(Shared::Stream *in) {
	dict.reset(new WordsDictionary());
	read_dictionary(dict.get(), in);
}

void GameSetupStruct::ReadMouseCursors(Stream *in) {
	for (int i = 0; i < numcursors; ++i) {
		mcurs[i].ReadFromFile(in);
	}
}

void GameSetupStruct::WriteMouseCursors(Stream *out) {
	for (int i = 0; i < numcursors; ++i) {
		mcurs[i].WriteToFile(out);
	}
}

//-----------------------------------------------------------------------------
// Reading Part 2

void GameSetupStruct::read_characters(Shared::Stream *in) {
	chars.resize(numcharacters);
	chars2.resize(numcharacters);
	ReadCharacters(in);
}

void GameSetupStruct::read_lipsync(Shared::Stream *in, GameDataVersion data_ver) {
	if (data_ver >= kGameVersion_254) // lip syncing was introduced in 2.54
		in->ReadArray(&lipSyncFrameLetters[0][0], MAXLIPSYNCFRAMES, 50);
}

void GameSetupStruct::read_messages(Shared::Stream *in, const std::array<int32_t> &load_messages, GameDataVersion data_ver) {
	char mbuf[GLOBALMESLENGTH];
	for (int i = 0; i < MAXGLOBALMES; ++i) {
		if (!load_messages[i])
			continue;
		if (data_ver < kGameVersion_261) { // Global messages are not encrypted on < 2.61
			char *nextchar = mbuf;

			// TODO: probably this is same as fgetstring
			while (1) {
				*nextchar = in->ReadInt8();
				if (*nextchar == 0)
					break;
				nextchar++;
			}
		} else {
			read_string_decrypt(in, mbuf, GLOBALMESLENGTH);
		}
		messages[i] = mbuf;
	}
}

void GameSetupStruct::ReadCharacters(Stream *in) {
	for (int i = 0; i < numcharacters; ++i) {
		chars[i].ReadFromFile(in, chars2[i], _G(loaded_game_file_version));
	}
}

void GameSetupStruct::WriteCharacters(Stream *out) {
	for (int i = 0; i < numcharacters; ++i) {
		chars[i].WriteToFile(out);
	}
}

//-----------------------------------------------------------------------------
// Reading Part 3

HGameFileError GameSetupStruct::read_customprops(Shared::Stream *in, GameDataVersion data_ver) {
	dialogScriptNames.resize(numdialog);
	viewNames.resize(numviews);
	if (data_ver >= kGameVersion_260) { // >= 2.60
		if (Properties::ReadSchema(propSchema, in) != kPropertyErr_NoError)
			return new MainGameFileError(kMGFErr_InvalidPropertySchema);

		int errors = 0;

		charProps.resize(numcharacters);
		for (int i = 0; i < numcharacters; ++i) {
			errors += Properties::ReadValues(charProps[i], in);
		}
		for (int i = 0; i < numinvitems; ++i) {
			errors += Properties::ReadValues(invProps[i], in);
		}

		if (errors > 0)
			return new MainGameFileError(kMGFErr_InvalidPropertyValues);

		for (int i = 0; i < numviews; ++i)
			viewNames[i] = String::FromStream(in);

		if (data_ver >= kGameVersion_270) {
			for (int i = 0; i < numinvitems; ++i)
				invScriptNames[i] = String::FromStream(in);

			if (data_ver >= kGameVersion_272) {
				for (int i = 0; i < numdialog; ++i)
					dialogScriptNames[i] = String::FromStream(in);
			}
		}
	}
	return HGameFileError::None();
}

HGameFileError GameSetupStruct::read_audio(Shared::Stream *in, GameDataVersion data_ver) {
	if (data_ver >= kGameVersion_320) {
		size_t audiotype_count = in->ReadInt32();
		audioClipTypes.resize(audiotype_count);
		for (size_t i = 0; i < audiotype_count; ++i) {
			audioClipTypes[i].ReadFromFile(in);
		}

		size_t audioclip_count = in->ReadInt32();
		audioClips.resize(audioclip_count);
		ReadAudioClips(in, audioclip_count);

		scoreClipID = in->ReadInt32();
	}
	return HGameFileError::None();
}

// Temporarily copied this from acruntim.h;
// it is unknown if this should be defined for all solution, or only runtime
#define STD_BUFFER_SIZE 3000

void GameSetupStruct::read_room_names(Stream *in, GameDataVersion data_ver) {
	if ((data_ver >= kGameVersion_301) && (options[OPT_DEBUGMODE] != 0)) {
		roomCount = in->ReadInt32();
		roomNumbers.resize(roomCount);
		roomNames.resize(roomCount);
		for (int i = 0; i < roomCount; ++i) {
			roomNumbers[i] = in->ReadInt32();
			roomNames[i].Read(in);
		}
	} else {
		roomCount = 0;
	}
}

void GameSetupStruct::ReadAudioClips(Shared::Stream *in, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		audioClips[i].ReadFromFile(in);
	}
}

void GameSetupStruct::ReadFromSaveGame_v321(Stream *in) {
	// NOTE: the individual object data is read from legacy saves
	// same way as if it were from a game file
	ReadInvInfo(in);
	ReadMouseCursors(in);

	if (_G(loaded_game_file_version) <= kGameVersion_272) {
		for (int i = 0; i < numinvitems; ++i)
			intrInv[i]->ReadTimesRunFromSave_v321(in);
		for (int i = 0; i < numcharacters; ++i)
			intrChar[i]->ReadTimesRunFromSave_v321(in);
	}

	in->ReadArrayOfInt32(&options[0], OPT_HIGHESTOPTION_321 + 1);
	options[OPT_LIPSYNCTEXT] = in->ReadInt8();

	ReadCharacters(in);
}

//=============================================================================
#if defined (OBSOLETE)

void ConvertOldGameStruct(OldGameSetupStruct *ogss, GameSetupStruct *gss) {
	snprintf(gss->gamename, sizeof(GameSetupStruct::gamename), "%s", ogss->gamename);
	for (int i = 0; i < 20; i++)
		gss->options[i] = ogss->options[i];
	memcpy(&gss->paluses[0], &ogss->paluses[0], 256);
	memcpy(&gss->defpal[0], &ogss->defpal[0], 256 * sizeof(RGB));
	gss->numviews = ogss->numviews;
	gss->numcharacters = ogss->numcharacters;
	gss->playercharacter = ogss->playercharacter;
	gss->totalscore = ogss->totalscore;
	gss->numinvitems = ogss->numinvitems;
	gss->numdialog = ogss->numdialog;
	gss->numdlgmessage = ogss->numdlgmessage;
	gss->numfonts = ogss->numfonts;
	gss->color_depth = ogss->color_depth;
	gss->target_win = ogss->target_win;
	gss->dialog_bullet = ogss->dialog_bullet;
	gss->hotdot = ogss->hotdot;
	gss->hotdotouter = ogss->hotdotouter;
	gss->uniqueid = ogss->uniqueid;
	gss->numgui = ogss->numgui;
	for (int i = 0; i < 10; ++i) {
		SetFontInfoFromLegacyFlags(gss->fonts[i], ogss->fontflags[i]);
		gss->fonts[i].Outline = ogss->fontoutline[i];
	}

	for (int i = 0; i < LEGACY_MAX_SPRITES_V25; ++i) {
		gss->SpriteInfos[i].Flags = ogss->spriteflags[i];
	}

	memcpy(&gss->invinfo[0], &ogss->invinfo[0], 100 * sizeof(InventoryItemInfo));
	for (int i = 0; i < 10; ++i)
		gss->mcurs[i] = ogss->mcurs[i];
	for (int i = 0; i < MAXGLOBALMES; i++)
		gss->messages[i] = ogss->messages[i];
	gss->dict = ogss->dict;
	gss->globalscript = ogss->globalscript;
	gss->chars = nullptr; //ogss->chars;
	gss->compiled_script = ogss->compiled_script;
	gss->numcursors = 10;
}
#endif // OBSOLETE

void GameSetupStruct::ReadFromSavegame(Stream *in) {
	// of GameSetupStruct
	in->ReadArrayOfInt32(options, OPT_HIGHESTOPTION_321 + 1);
	options[OPT_LIPSYNCTEXT] = in->ReadInt32();
	// of GameSetupStructBase
	playercharacter = in->ReadInt32();
	dialog_bullet = in->ReadInt32();
	hotdot = static_cast<uint16_t>(in->ReadInt16());
	hotdotouter = static_cast<uint16_t>(in->ReadInt16());
	invhotdotsprite = in->ReadInt32();
	default_lipsync_frame = in->ReadInt32();
}

void GameSetupStruct::WriteForSavegame(Stream *out) {
	// of GameSetupStruct
	out->WriteArrayOfInt32(options, OPT_HIGHESTOPTION_321 + 1);
	out->WriteInt32(options[OPT_LIPSYNCTEXT]);
	// of GameSetupStructBase
	out->WriteInt32(playercharacter);
	out->WriteInt32(dialog_bullet);
	out->WriteInt16(static_cast<uint16_t>(hotdot));
	out->WriteInt16(static_cast<uint16_t>(hotdotouter));
	out->WriteInt32(invhotdotsprite);
	out->WriteInt32(default_lipsync_frame);
}

} // namespace AGS3
