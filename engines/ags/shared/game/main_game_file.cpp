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
#include "ags/shared/ac/dialog_topic.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/ac/view.h"
#include "ags/shared/ac/words_dictionary.h"
#include "ags/shared/ac/dynobj/script_audio_clip.h"
#include "ags/shared/core/asset.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/game/main_game_file.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/gui/gui_label.h"
#include "ags/shared/font/fonts.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/shared/script/cc_common.h"
#include "ags/shared/util/data_ext.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/string_compat.h"
#include "ags/shared/util/string_utils.h"
#include "ags/globals.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

const char *MainGameSource::DefaultFilename_v3 = "game28.dta";
const char *MainGameSource::DefaultFilename_v2 = "ac2game.dta";
const char *MainGameSource::Signature = "Adventure Creator Game File v2";

MainGameSource::MainGameSource()
	: DataVersion(kGameVersion_Undefined) {
}

String GetMainGameFileErrorText(MainGameFileErrorType err) {
	switch (err) {
	case kMGFErr_NoError:
		return "No error.";
	case kMGFErr_FileOpenFailed:
		return "Main game file not found or could not be opened.";
	case kMGFErr_SignatureFailed:
		return "Not an AGS main game file or unsupported format.";
	case kMGFErr_FormatVersionTooOld:
		return "Format version is too old; this engine can only run games made with AGS 2.5 or later.";
	case kMGFErr_FormatVersionNotSupported:
		return "Format version not supported.";
	case kMGFErr_CapsNotSupported:
		return "The game requires extended capabilities which aren't supported by the engine.";
	case kMGFErr_InvalidNativeResolution:
		return "Unable to determine native game resolution.";
	case kMGFErr_TooManySprites:
		return "Too many sprites for this engine to handle.";
	case kMGFErr_InvalidPropertySchema:
		return "Failed to deserialize custom properties schema.";
	case kMGFErr_InvalidPropertyValues:
		return "Errors encountered when reading custom properties.";
	case kMGFErr_CreateGlobalScriptFailed:
		return "Failed to load global script.";
	case kMGFErr_CreateDialogScriptFailed:
		return "Failed to load dialog script.";
	case kMGFErr_CreateScriptModuleFailed:
		return "Failed to load script module.";
	case kMGFErr_GameEntityFailed:
		return "Failed to load one or more game entities.";
	case kMGFErr_PluginDataFmtNotSupported:
		return "Format version of plugin data is not supported.";
	case kMGFErr_PluginDataSizeTooLarge:
		return "Plugin data size is too large.";
	case kMGFErr_ExtListFailed:
		return "There was error reading game data extensions.";
	case kMGFErr_ExtUnknown:
		return "Unknown extension.";
	default:
		break;
	}
	return "Unknown error.";
}

LoadedGameEntities::LoadedGameEntities(GameSetupStruct &game)
	: Game(game)
	, SpriteCount(0) {
}

LoadedGameEntities::~LoadedGameEntities() {}

bool IsMainGameLibrary(const String &filename) {
	// We must not only detect if the given file is a correct AGS data library,
	// we also have to assure that this library contains main game asset.
	// Library may contain some optional data (digital audio, speech etc), but
	// that is not what we want.
	AssetLibInfo lib;
	if (AssetManager::ReadDataFileTOC(filename, lib) != kAssetNoError)
		return false;
	for (size_t i = 0; i < lib.AssetInfos.size(); ++i) {
		if (lib.AssetInfos[i].FileName.CompareNoCase(MainGameSource::DefaultFilename_v3) == 0 ||
		        lib.AssetInfos[i].FileName.CompareNoCase(MainGameSource::DefaultFilename_v2) == 0) {
			return true;
		}
	}
	return false;
}

// Scans given directory for game data libraries, returns first found or none.
// Tracks files with standard AGS package names:
// - *.ags is a standart cross-platform file pattern for AGS games,
// - ac2game.dat is a legacy file name for very old games,
// - *.exe is a MS Win executable; it is included to this case because
//   users often run AGS ports with Windows versions of games.
String FindGameData(const String &path, bool(*fn_testfile)(const String &)) {
	Common::FSNode folder(path.GetCStr());
	Common::FSList files;
	if (folder.getChildren(files, Common::FSNode::kListFilesOnly)) {
		for (Common::FSList::iterator it = files.begin(); it != files.end(); ++it) {
			Common::String test_file = it->getName();
			Common::Path filePath = it->getPath();

			if (test_file.hasSuffixIgnoreCase(".ags") ||
			        test_file.equalsIgnoreCase("ac2game.dat") ||
			        test_file.hasSuffixIgnoreCase(".exe")) {
				if (IsMainGameLibrary(test_file.c_str()) && fn_testfile(filePath.toString('/'))) {
					Debug::Printf("Found game data pak: %s", test_file.c_str());
					return test_file.c_str();
				}
			}
		}
	}

	return "";
}

static bool comparitor(const String &) {
	return true;
}

String FindGameData(const String &path) {
	return FindGameData(path, comparitor);
}

// Begins reading main game file from a generic stream
static HGameFileError OpenMainGameFileBase(Stream *in, MainGameSource &src) {
	// Check data signature
	String data_sig = String::FromStreamCount(in, strlen(MainGameSource::Signature));
	if (data_sig.Compare(MainGameSource::Signature))
		return new MainGameFileError(kMGFErr_SignatureFailed);
	// Read data format version and requested engine version
	src.DataVersion = (GameDataVersion)in->ReadInt32();
	if (src.DataVersion >= kGameVersion_230)
		src.CompiledWith = StrUtil::ReadString(in);
	if (src.DataVersion < kGameVersion_250)
		return new MainGameFileError(kMGFErr_FormatVersionTooOld, String::FromFormat("Required format version: %d, supported %d - %d", src.DataVersion, kGameVersion_250, kGameVersion_Current));
	if (src.DataVersion > kGameVersion_Current)
		return new MainGameFileError(kMGFErr_FormatVersionNotSupported,
		                             String::FromFormat("Game was compiled with %s. Required format version: %d, supported %d - %d", src.CompiledWith.GetCStr(), src.DataVersion, kGameVersion_250, kGameVersion_Current));
	// Read required capabilities
	if (src.DataVersion >= kGameVersion_341) {
		size_t count = in->ReadInt32();
		for (size_t i = 0; i < count; ++i)
			src.Caps.insert(StrUtil::ReadString(in));
	}
	// Remember loaded game data version
	// NOTE: this global variable is embedded in the code too much to get
	// rid of it too easily; the easy way is to set it whenever the main
	// game file is opened.
	_G(loaded_game_file_version) = src.DataVersion;
	_G(game_compiled_version).SetFromString(src.CompiledWith);
	return HGameFileError::None();
}

HGameFileError OpenMainGameFile(const String &filename, MainGameSource &src) {
	// Cleanup source struct
	src = MainGameSource();
	// Try to open given file
	Stream *in = File::OpenFileRead(filename);
	if (!in)
		return new MainGameFileError(kMGFErr_FileOpenFailed, String::FromFormat("Tried filename: %s.", filename.GetCStr()));
	src.Filename = filename;
	src.InputStream.reset(in);
	return OpenMainGameFileBase(in, src);
}

HGameFileError OpenMainGameFileFromDefaultAsset(MainGameSource &src, AssetManager *mgr) {
	// Cleanup source struct
	src = MainGameSource();
	// Try to find and open main game file
	String filename = MainGameSource::DefaultFilename_v3;
	Stream *in = mgr->OpenAsset(filename);
	if (!in) {
		filename = MainGameSource::DefaultFilename_v2;
		in = mgr->OpenAsset(filename);
	}
	if (!in)
		return new MainGameFileError(kMGFErr_FileOpenFailed,
									 String::FromFormat("Tried filenames: %s, %s.", MainGameSource::DefaultFilename_v3, MainGameSource::DefaultFilename_v2));
	src.Filename = filename;
	src.InputStream.reset(in);
	return OpenMainGameFileBase(in, src);
}

HGameFileError ReadDialogScript(PScript &dialog_script, Stream *in, GameDataVersion data_ver) {
	if (data_ver > kGameVersion_310) { // 3.1.1+ dialog script
		dialog_script.reset(ccScript::CreateFromStream(in));
		if (dialog_script == nullptr)
			return new MainGameFileError(kMGFErr_CreateDialogScriptFailed, cc_get_error().ErrorString);
	} else { // 2.x and < 3.1.1 dialog
		dialog_script.reset();
	}
	return HGameFileError::None();
}

HGameFileError ReadScriptModules(std::vector<PScript> &sc_mods, Stream *in, GameDataVersion data_ver) {
	if (data_ver >= kGameVersion_270) { // 2.7.0+ script modules
		int count = in->ReadInt32();
		sc_mods.resize(count);
		for (int i = 0; i < count; ++i) {
			sc_mods[i].reset(ccScript::CreateFromStream(in));
			if (sc_mods[i] == nullptr)
				return new MainGameFileError(kMGFErr_CreateScriptModuleFailed, cc_get_error().ErrorString);
		}
	} else {
		sc_mods.resize(0);
	}
	return HGameFileError::None();
}

void ReadViews(GameSetupStruct &game, std::vector<ViewStruct> &views, Stream *in, GameDataVersion data_ver) {
	views.resize(game.numviews);
	if (data_ver > kGameVersion_272) // 3.x views
	{
		for (int i = 0; i < game.numviews; ++i) {
			views[i].ReadFromFile(in);
		}
	} else // 2.x views
	{
		std::vector<ViewStruct272> oldv(game.numviews);
		for (int i = 0; i < game.numviews; ++i) {
			oldv[i].ReadFromFile(in);
		}
		Convert272ViewsToNew(oldv, views);
	}
}

void ReadDialogs(std::vector<DialogTopic> &dialog,
                 std::vector<std::vector<uint8_t>> &old_dialog_scripts,
                 std::vector<String> &old_dialog_src,
                 std::vector<String> &old_speech_lines,
                 Stream *in, GameDataVersion data_ver, int dlg_count) {
	dialog.resize(dlg_count);
	for (int i = 0; i < dlg_count; ++i) {
		dialog[i].ReadFromFile(in);
	}

	if (data_ver > kGameVersion_310)
		return;

	old_dialog_scripts.resize(dlg_count);
	old_dialog_src.resize(dlg_count);
	for (int i = 0; i < dlg_count; ++i) {
		// NOTE: originally this was read into dialog[i].optionscripts
		old_dialog_scripts[i].resize(dialog[i].codesize);
		in->Read(old_dialog_scripts[i].data(), dialog[i].codesize);

		// Encrypted text script
		int script_text_len = in->ReadInt32();
		if (script_text_len > 1) {
			// Originally in the Editor +20000 bytes more were allocated, with comment:
			//   "add a large buffer because it will get added to if another option is added"
			// which probably referred to this data used by old editor directly to edit dialogs
			char *buffer = new char[script_text_len + 1];
			in->Read(buffer, script_text_len);
			if (data_ver > kGameVersion_260)
				decrypt_text(buffer, script_text_len);
			buffer[script_text_len] = 0;
			old_dialog_src[i] = buffer;
			delete[] buffer;
		} else {
			in->Seek(script_text_len);
		}
	}

	// Read the dialog lines
	//
	// TODO: investigate this: these strings were read much simpler in the editor, see code:
	/*
	    char stringbuffer[1000];
	    for (bb=0;bb<thisgame.numdlgmessage;bb++) {
	        if ((filever >= 26) && (encrypted))
	            read_string_decrypt(iii, stringbuffer);
	        else
	            fgetstring(stringbuffer, iii);
	    }
	*/
	//int i = 0;
	char buffer[1000];
	if (data_ver <= kGameVersion_260) {
		// Plain text on <= 2.60
		bool end_reached = false;

		while (!end_reached) {
			char *nextchar = buffer;

			while (1) {
				*nextchar = in->ReadInt8();
				if (*nextchar == 0)
					break;

				if ((unsigned char)*nextchar == 0xEF) {
					end_reached = true;
					in->Seek(-1);
					break;
				}

				nextchar++;
			}

			if (end_reached)
				break;

			old_speech_lines.push_back(buffer);
			//i++;
		}
	} else {
		// Encrypted text on > 2.60
		while (1) {
			size_t newlen = static_cast<uint32_t>(in->ReadInt32());
			if (newlen == 0xCAFEBEEF) { // GUI magic
				in->Seek(-4);
				break;
			}

			newlen = MIN(newlen, sizeof(buffer) - 1);
			in->Read(buffer, newlen);
			decrypt_text(buffer, newlen);
			buffer[newlen] = 0;
			old_speech_lines.push_back(buffer);
			//i++;
		}
	}
}

HGameFileError ReadPlugins(std::vector<PluginInfo> &infos, Stream *in) {
	int fmt_ver = in->ReadInt32();
	if (fmt_ver != 1)
		return new MainGameFileError(kMGFErr_PluginDataFmtNotSupported, String::FromFormat("Version: %d, supported: %d", fmt_ver, 1));

	int pl_count = in->ReadInt32();
	for (int i = 0; i < pl_count; ++i) {
		String name = String::FromStream(in);
		size_t datasize = in->ReadInt32();
		// just check for silly datasizes
		if (datasize > PLUGIN_SAVEBUFFERSIZE)
			return new MainGameFileError(kMGFErr_PluginDataSizeTooLarge, String::FromFormat("Required: %zu, max: %zu", datasize, (size_t)PLUGIN_SAVEBUFFERSIZE));

		PluginInfo info;
		info.Name = name;
		if (datasize > 0) {
			info.Data.resize(datasize);
			in->Read(&info.Data.front(), datasize);
		}
		infos.push_back(info);
	}
	return HGameFileError::None();
}

// Create the missing audioClips data structure for 3.1.x games.
// This is done by going through the data files and adding all music*.*
// and sound*.* files to it.
void BuildAudioClipArray(const std::vector<String> &assets, std::vector<ScriptAudioClip> &audioclips) {
	char temp_name[30];
	int temp_number;
	char temp_extension[10];

	for (const String &asset : assets) {
		if (sscanf(asset.GetCStr(), "%5s%d.%3s", temp_name, &temp_number, temp_extension) != 3)
			continue;

		ScriptAudioClip clip;
		if (ags_stricmp(temp_extension, "mp3") == 0)
			clip.fileType = eAudioFileMP3;
		else if (ags_stricmp(temp_extension, "wav") == 0)
			clip.fileType = eAudioFileWAV;
		else if (ags_stricmp(temp_extension, "voc") == 0)
			clip.fileType = eAudioFileVOC;
		else if (ags_stricmp(temp_extension, "mid") == 0)
			clip.fileType = eAudioFileMIDI;
		else if ((ags_stricmp(temp_extension, "mod") == 0) || (ags_stricmp(temp_extension, "xm") == 0)
		         || (ags_stricmp(temp_extension, "s3m") == 0) || (ags_stricmp(temp_extension, "it") == 0))
			clip.fileType = eAudioFileMOD;
		else if (ags_stricmp(temp_extension, "ogg") == 0)
			clip.fileType = eAudioFileOGG;
		else
			continue;

		if (ags_stricmp(temp_name, "music") == 0) {
			clip.scriptName.Format("aMusic%d", temp_number);
			clip.fileName.Format("music%d.%s", temp_number, temp_extension);
			clip.bundlingType = (ags_stricmp(temp_extension, "mid") == 0) ? AUCL_BUNDLE_EXE : AUCL_BUNDLE_VOX;
			clip.type = 2;
			clip.defaultRepeat = 1;
		} else if (ags_stricmp(temp_name, "sound") == 0) {
			clip.scriptName.Format("aSound%d", temp_number);
			clip.fileName.Format("sound%d.%s", temp_number, temp_extension);
			clip.bundlingType = AUCL_BUNDLE_EXE;
			clip.type = 3;
			clip.defaultRepeat = 0;
		} else {
			continue;
		}

		clip.defaultVolume = 100;
		clip.defaultPriority = 50;
		clip.id = audioclips.size();
		audioclips.push_back(clip);
	}
}

void ApplySpriteData(GameSetupStruct &game, const LoadedGameEntities &ents, GameDataVersion data_ver) {
	if (ents.SpriteCount == 0)
		return;

	// Apply sprite flags read from original format (sequential array)
	_GP(spriteset).EnlargeTo(ents.SpriteCount - 1);
	for (size_t i = 0; i < ents.SpriteCount; ++i) {
		_GP(game).SpriteInfos[i].Flags = ents.SpriteFlags[i];
	}

	// Promote sprite resolutions and mark legacy resolution setting
	if (data_ver < kGameVersion_350) {
		for (size_t i = 0; i < ents.SpriteCount; ++i) {
			SpriteInfo &info = _GP(game).SpriteInfos[i];
			if (_GP(game).IsLegacyHiRes() == info.IsLegacyHiRes())
				info.Flags &= ~(SPF_HIRES | SPF_VAR_RESOLUTION);
			else
				info.Flags |= SPF_VAR_RESOLUTION;
		}
	}
}

void UpgradeFonts(GameSetupStruct &game, GameDataVersion data_ver) {
	if (data_ver < kGameVersion_350) {
		for (int i = 0; i < _GP(game).numfonts; ++i) {
			FontInfo &finfo = _GP(game).fonts[i];
			// If the game is hi-res but font is designed for low-res, then scale it up
			if (_GP(game).IsLegacyHiRes() && _GP(game).options[OPT_HIRES_FONTS] == 0) {
				finfo.SizeMultiplier = HIRES_COORD_MULTIPLIER;
			} else {
				finfo.SizeMultiplier = 1;
			}
		}
	}
	if (data_ver < kGameVersion_360) {
		for (int i = 0; i < game.numfonts; ++i) {
			FontInfo &finfo = game.fonts[i];
			if (finfo.Outline == FONT_OUTLINE_AUTO) {
				finfo.AutoOutlineStyle = FontInfo::kSquared;
				finfo.AutoOutlineThickness = 1;
			}
		}
	}
	if (data_ver < kGameVersion_360_11) { // use global defaults for the font load flags
		for (int i = 0; i < game.numfonts; ++i) {
			game.fonts[i].Flags |= FFLG_TTF_BACKCOMPATMASK;
		}
	}
}

// Convert audio data to the current version
void UpgradeAudio(GameSetupStruct &game, LoadedGameEntities &ents, GameDataVersion data_ver) {
	if (data_ver >= kGameVersion_320)
		return;

	// An explanation of building audio clips array for pre-3.2 games.
	//
	// When AGS version 3.2 was released, it contained new audio system.
	// In the nutshell, prior to 3.2 audio files had to be manually put
	// to game project directory and their IDs were taken out of filenames.
	// Since 3.2 this information is stored inside the game data.
	// To make the modern engine compatible with pre-3.2 games, we have
	// to scan game data packages for audio files, and enumerate them
	// ourselves, then add this information to game struct.

	// Create soundClips and audioClipTypes structures.
	std::vector<AudioClipType> audiocliptypes;
	std::vector<ScriptAudioClip> audioclips;

	// TODO: find out what is 4 (maybe music, sound, ambient sound, voice?)
	audiocliptypes.resize(4);
	for (int i = 0; i < 4; i++) {
		audiocliptypes[i].reservedChannels = 1;
		audiocliptypes[i].id = i;
		audiocliptypes[i].volume_reduction_while_speech_playing = 10;
	}
	audiocliptypes[3].reservedChannels = 0;

	audioclips.reserve(1000);
	std::vector<String> assets;
	// Read audio clip names from from registered libraries
	for (size_t i = 0; i < _GP(AssetMgr)->GetLibraryCount(); ++i) {
		const AssetLibInfo *game_lib = _GP(AssetMgr)->GetLibraryInfo(i);
		if (File::IsDirectory(game_lib->BasePath))
			continue; // might be a directory

		for (const AssetInfo &info : game_lib->AssetInfos) {
			if (info.FileName.CompareLeftNoCase("music", 5) == 0 || info.FileName.CompareLeftNoCase("sound", 5) == 0)
				assets.push_back(info.FileName);
		}
	}
	// Append contents of the registered directories
	// TODO: implement pattern search or asset query with callback (either of two or both)
	// within AssetManager to avoid doing this in place here. Alternatively we could maybe
	// make AssetManager to do directory scans by demand and fill AssetInfos...
	// but that have to be done consistently if done at all.
	for (size_t i = 0; i < _GP(AssetMgr)->GetLibraryCount(); ++i) {
		const AssetLibInfo *game_lib = _GP(AssetMgr)->GetLibraryInfo(i);
		if (!File::IsDirectory(game_lib->BasePath))
			continue; // might be a library


		Common::FSNode folder(game_lib->BasePath.GetCStr());
		Common::FSList files;
		folder.getChildren(files, Common::FSNode::kListFilesOnly);

		for (Common::FSList::iterator it = files.begin(); it != files.end(); ++it) {
			Common::String name = (*it).getName();

			if (name.hasPrefixIgnoreCase("music") || name.hasPrefixIgnoreCase("sound"))
				assets.push_back(name.c_str());
		}
	}

	BuildAudioClipArray(assets, audioclips);

	// Copy gathered data over to game
	_GP(game).audioClipTypes = audiocliptypes;
	_GP(game).audioClips = audioclips;

	RemapLegacySoundNums(game, ents.Views, data_ver);
}

// Convert character data to the current version
void UpgradeCharacters(GameSetupStruct &game, GameDataVersion data_ver) {
	auto &chars = _GP(game).chars;
	auto &chars2 = _GP(game).chars2;
	const int numcharacters = _GP(game).numcharacters;

	// Fixup character script names for 2.x (EGO -> cEgo)
	// In 2.x versions the "scriptname" field in game data contained a name
	// limited by 14 chars (although serialized in 20 bytes). After reading,
	// it was exported as "cScriptname..." for the script.
	if (data_ver <= kGameVersion_272) {
		char namelwr[LEGACY_MAX_SCRIPT_NAME_LEN - 1];
		for (int i = 0; i < numcharacters; i++) {
			if (chars[i].scrname[0] == 0)
				continue;
			ags_strncpy_s(namelwr, sizeof(namelwr), chars[i].scrname, LEGACY_MAX_SCRIPT_NAME_LEN - 2);
			ags_strlwr(namelwr + 1); // lowercase starting with the second char
			snprintf(chars[i].scrname, sizeof(chars[i].scrname), "c%s", namelwr);
			chars2[i].scrname_new = chars[i].scrname;
		}
	}

	// Fix character walk speed for < 3.1.1
	if (data_ver <= kGameVersion_310) {
		for (int i = 0; i < numcharacters; i++) {
			if (_GP(game).options[OPT_ANTIGLIDE])
				chars[i].flags |= CHF_ANTIGLIDE;
		}
	}

	// Characters can always walk through each other on < 2.54
	if (data_ver < kGameVersion_254) {
		for (int i = 0; i < numcharacters; i++) {
			chars[i].flags |= CHF_NOBLOCKING;
		}
	}
}

void UpgradeGUI(GameSetupStruct &game, GameDataVersion data_ver) {
	// Previously, Buttons and Labels had a fixed Translated behavior
	if (data_ver < kGameVersion_361) {
		for (auto &btn : _GP(guibuts))
			btn.SetTranslated(true); // always translated
		for (auto &lbl : _GP(guilabels))
			lbl.SetTranslated(true); // always translated
	}
}

void UpgradeMouseCursors(GameSetupStruct &game, GameDataVersion data_ver) {
	if (data_ver <= kGameVersion_272) {
		// Change cursor.view from 0 to -1 for non-animating cursors.
		for (int i = 0; i < _GP(game).numcursors; ++i) {
			if (_GP(game).mcurs[i].view == 0)
				_GP(game).mcurs[i].view = -1;
		}
	}
}

// Adjusts score clip id, depending on game data version
void RemapLegacySoundNums(GameSetupStruct &game, std::vector<ViewStruct> &views, GameDataVersion data_ver) {
	if (data_ver >= kGameVersion_320)
		return;

	// Setup sound clip played on score event
	game.scoreClipID = -1;
	if (game.options[OPT_SCORESOUND] > 0) {
		ScriptAudioClip *clip = GetAudioClipForOldStyleNumber(game, false, game.options[OPT_SCORESOUND]);
		if (clip)
			game.scoreClipID = clip->id;
	}

	// Reset view frame clip refs
	// NOTE: we do not map these to real clips right away,
	// instead we do this at runtime whenever we find a non-mapped frame sound.
	for (size_t v = 0; v < (size_t)game.numviews; ++v) {
		for (size_t l = 0; l < (size_t)views[v].numLoops; ++l) {
			for (size_t f = 0; f < (size_t)views[v].loops[l].numFrames; ++f) {
				views[v].loops[l].frames[f].audioclip = -1;
			}
		}
	}
}

// Assigns default global message at given index
void SetDefaultGlmsg(GameSetupStruct &game, int msgnum, const char *val) {
	// TODO: find out why the index should be lowered by 500
	// (or rather if we may pass correct index right away)
	msgnum -= 500;
	if (_GP(game).messages[msgnum].IsEmpty())
		_GP(game).messages[msgnum] = val;
}

// Sets up default global messages (these are used mainly in older games)
void SetDefaultGlobalMessages(GameSetupStruct &game) {
	SetDefaultGlmsg(game, 983, "Sorry, not now.");
	SetDefaultGlmsg(game, 984, "Restore");
	SetDefaultGlmsg(game, 985, "Cancel");
	SetDefaultGlmsg(game, 986, "Select a game to restore:");
	SetDefaultGlmsg(game, 987, "Save");
	SetDefaultGlmsg(game, 988, "Type a name to save as:");
	SetDefaultGlmsg(game, 989, "Replace");
	SetDefaultGlmsg(game, 990, "The save directory is full. You must replace an existing game:");
	SetDefaultGlmsg(game, 991, "Replace:");
	SetDefaultGlmsg(game, 992, "With:");
	SetDefaultGlmsg(game, 993, "Quit");
	SetDefaultGlmsg(game, 994, "Play");
	SetDefaultGlmsg(game, 995, "Are you sure you want to quit?");
	SetDefaultGlmsg(game, 996, "You are carrying nothing.");
}

void FixupSaveDirectory(GameSetupStruct &game) {
	// If the save game folder was not specified by game author, create one of
	// the game name, game GUID, or uniqueid, as a last resort
	if (_GP(game).saveGameFolderName.IsEmpty()) {
		if (!_GP(game).gamename.IsEmpty())
			_GP(game).saveGameFolderName = _GP(game).gamename;
		else if (_GP(game).guid[0])
			_GP(game).saveGameFolderName = _GP(game).guid;
		else
			_GP(game).saveGameFolderName.Format("AGS-Game-%d", _GP(game).uniqueid);
	}
	// Lastly, fixup folder name by removing any illegal characters
	_GP(game).saveGameFolderName = Path::FixupSharedFilename(_GP(game).saveGameFolderName);
}

HGameFileError ReadSpriteFlags(LoadedGameEntities &ents, Stream *in, GameDataVersion data_ver) {
	size_t sprcount;
	if (data_ver < kGameVersion_256)
		sprcount = LEGACY_MAX_SPRITES_V25;
	else
		sprcount = in->ReadInt32();
	if (sprcount > (size_t)SpriteCache::MAX_SPRITE_INDEX + 1)
		return new MainGameFileError(kMGFErr_TooManySprites, String::FromFormat("Count: %zu, max: %zu", sprcount, (size_t)SpriteCache::MAX_SPRITE_INDEX + 1));

	ents.SpriteCount = sprcount;
	ents.SpriteFlags.resize(sprcount);
	in->Read(ents.SpriteFlags.data(), sprcount);
	return HGameFileError::None();
}

// GameDataExtReader reads main game data's extension blocks
class GameDataExtReader : public DataExtReader {
public:
	GameDataExtReader(LoadedGameEntities &ents, GameDataVersion data_ver, Stream *in)
		: DataExtReader(in, kDataExt_NumID8 | kDataExt_File64)
		, _ents(ents)
		, _dataVer(data_ver) {
	}

protected:
	HError ReadBlock(int block_id, const String &ext_id,
		soff_t block_len, bool &read_next) override;

	LoadedGameEntities &_ents;
	GameDataVersion _dataVer;
};

HError GameDataExtReader::ReadBlock(int /*block_id*/, const String &ext_id,
		soff_t /*block_len*/, bool &read_next) {
	read_next = true;
	// Add extensions here checking ext_id, which is an up to 16-chars name, for example:
    // if (ext_id.CompareNoCase("GUI_NEWPROPS") == 0)
    // {
    //     // read new gui properties
    // }
	if (ext_id.CompareNoCase("v360_fonts") == 0) {
		for (FontInfo &finfo : _ents.Game.fonts) {
			// adjustable font outlines
			finfo.AutoOutlineThickness = _in->ReadInt32();
			finfo.AutoOutlineStyle = static_cast<enum FontInfo::AutoOutlineStyle>(_in->ReadInt32());
			// reserved
			_in->ReadInt32();
			_in->ReadInt32();
			_in->ReadInt32();
			_in->ReadInt32();
		}
	} else if (ext_id.CompareNoCase("v360_cursors") == 0) {
		for (MouseCursor &mcur : _ents.Game.mcurs) {
			mcur.animdelay = _in->ReadInt32();
			// reserved
			_in->ReadInt32();
			_in->ReadInt32();
			_in->ReadInt32();
		}
	} else if (ext_id.CompareNoCase("v361_objnames") == 0) {
		// Extended object names and script names:
		// for object types that had hard name length limits
		_ents.Game.gamename = StrUtil::ReadString(_in);
		_ents.Game.saveGameFolderName = StrUtil::ReadString(_in);
		size_t num_chars = _in->ReadInt32();
		if (num_chars != _ents.Game.chars.size())
			return new Error(String::FromFormat("Mismatching number of characters: read %zu expected %zu", num_chars, _ents.Game.chars.size()));
		for (int i = 0; i < _ents.Game.numcharacters; ++i) {
			auto &chinfo = _ents.Game.chars[i];
			auto &chinfo2 = _ents.Game.chars2[i];
			chinfo2.scrname_new = StrUtil::ReadString(_in);
			chinfo2.name_new = StrUtil::ReadString(_in);
			// assign to the legacy fields for compatibility with old plugins
			snprintf(chinfo.scrname, LEGACY_MAX_SCRIPT_NAME_LEN, "%s", chinfo2.scrname_new.GetCStr());
			snprintf(chinfo.name, LEGACY_MAX_CHAR_NAME_LEN, "%s", chinfo2.name_new.GetCStr());
		}
		size_t num_invitems = _in->ReadInt32();
		if (num_invitems != (size_t)_ents.Game.numinvitems)
			return new Error(String::FromFormat("Mismatching number of inventory items: read %zu expected %zu", num_invitems, (size_t)_ents.Game.numinvitems));
		for (int i = 0; i < _ents.Game.numinvitems; ++i) {
			_ents.Game.invinfo[i].name = StrUtil::ReadString(_in);
		}
		size_t num_cursors = _in->ReadInt32();
		if (num_cursors != _ents.Game.mcurs.size())
			return new Error(String::FromFormat("Mismatching number of cursors: read %zu expected %zu", num_cursors, _ents.Game.mcurs.size()));
		for (MouseCursor &mcur : _ents.Game.mcurs) {
			mcur.name = StrUtil::ReadString(_in);
		}
		size_t num_clips = _in->ReadInt32();
		if (num_clips != _ents.Game.audioClips.size())
			return new Error(String::FromFormat("Mismatching number of audio clips: read %zu expected %zu", num_clips, _ents.Game.audioClips.size()));
		for (ScriptAudioClip &clip : _ents.Game.audioClips) {
			clip.scriptName = StrUtil::ReadString(_in);
			clip.fileName = StrUtil::ReadString(_in);
		}
	} else {
		return new MainGameFileError(kMGFErr_ExtUnknown, String::FromFormat("Type: %s", ext_id.GetCStr()));
	}
	return HError::None();
}

// Search and read only data belonging to the general game info
class GameDataExtPreloader : public GameDataExtReader {
public:
	GameDataExtPreloader(LoadedGameEntities &ents, GameDataVersion data_ver, Stream *in)
		: GameDataExtReader(ents, data_ver, in) {}

protected:
	HError ReadBlock(int block_id, const String &ext_id, soff_t block_len, bool &read_next) override;
};

HError GameDataExtPreloader::ReadBlock(int /*block_id*/, const String &ext_id,
									   soff_t /*block_len*/, bool &read_next) {
	// Try reading only data which belongs to the general game info
	read_next = true;
	if (ext_id.CompareNoCase("v361_objnames") == 0) {
		_ents.Game.gamename = StrUtil::ReadString(_in);
		_ents.Game.saveGameFolderName = StrUtil::ReadString(_in);
		read_next = false; // we're done
	}
	SkipBlock(); // prevent assertion trigger
	return HError::None();
}

HGameFileError ReadGameData(LoadedGameEntities &ents, Stream *in, GameDataVersion data_ver) {
	GameSetupStruct &game = ents.Game;

	//-------------------------------------------------------------------------
	// The standard data section.
	//-------------------------------------------------------------------------
	GameSetupStruct::SerializeInfo sinfo;
	game.GameSetupStructBase::ReadFromFile(in, data_ver, sinfo);
	game.read_savegame_info(in, data_ver); // here we also read GUID in v3.* games

	Debug::Printf(kDbgMsg_Info, "Game title: '%s'", game.gamename.GetCStr());
	Debug::Printf(kDbgMsg_Info, "Game uid (old format): `%d`", game.uniqueid);
	Debug::Printf(kDbgMsg_Info, "Game guid: '%s'", game.guid);

	if (game.GetGameRes().IsNull())
		return new MainGameFileError(kMGFErr_InvalidNativeResolution);

	game.read_font_infos(in, data_ver);
	HGameFileError err = ReadSpriteFlags(ents, in, data_ver);
	if (!err)
		return err;
	game.ReadInvInfo(in);
	err = game.read_cursors(in);
	if (!err)
		return err;
	game.read_interaction_scripts(in, data_ver);
	if (sinfo.HasWordsDict)
		game.read_words_dictionary(in);

	if (sinfo.HasCCScript) {
		ents.GlobalScript.reset(ccScript::CreateFromStream(in));
		if (!ents.GlobalScript)
			return new MainGameFileError(kMGFErr_CreateGlobalScriptFailed, cc_get_error().ErrorString);
		err = ReadDialogScript(ents.DialogScript, in, data_ver);
		if (!err)
			return err;
		err = ReadScriptModules(ents.ScriptModules, in, data_ver);
		if (!err)
			return err;
	}

	ReadViews(game, ents.Views, in, data_ver);

	if (data_ver <= kGameVersion_251) {
		// skip unknown data
		int count = in->ReadInt32();
		in->Seek(count * 0x204);
	}

	game.read_characters(in);
	game.read_lipsync(in, data_ver);
	game.read_messages(in, sinfo.HasMessages, data_ver);

	ReadDialogs(ents.Dialogs, ents.OldDialogScripts, ents.OldDialogSources, ents.OldSpeechLines,
		in, data_ver, game.numdialog);
	HError err2 = GUI::ReadGUI(in);
	if (!err2)
		return new MainGameFileError(kMGFErr_GameEntityFailed, err2);
	game.numgui = _GP(guis).size();

	if (data_ver >= kGameVersion_260) {
		err = ReadPlugins(ents.PluginInfos, in);
		if (!err)
			return err;
	}

	err = game.read_customprops(in, data_ver);
	if (!err)
		return err;
	err = game.read_audio(in, data_ver);
	if (!err)
		return err;
	game.read_room_names(in, data_ver);

	if (data_ver <= kGameVersion_350)
		return HGameFileError::None();

	//-------------------------------------------------------------------------
	// All the extended data, for AGS > 3.5.0.
	//-------------------------------------------------------------------------
	GameDataExtReader reader(ents, data_ver, in);
	HError ext_err = reader.Read();
	return ext_err ? HGameFileError::None() : new MainGameFileError(kMGFErr_ExtListFailed, ext_err);
}

HGameFileError UpdateGameData(LoadedGameEntities &ents, GameDataVersion data_ver) {
	GameSetupStruct &game = ents.Game;
	ApplySpriteData(game, ents, data_ver);
	UpgradeFonts(game, data_ver);
	UpgradeAudio(game, ents, data_ver);
	UpgradeCharacters(game, data_ver);
	UpgradeGUI(game, data_ver);
	UpgradeMouseCursors(game, data_ver);
	SetDefaultGlobalMessages(game);
	// Global talking animation speed
	if (data_ver < kGameVersion_312) {
		// Fix animation speed for old formats
		game.options[OPT_GLOBALTALKANIMSPD] = 5;
	} else if (data_ver < kGameVersion_330) {
		// Convert game option for 3.1.2 - 3.2 games
		game.options[OPT_GLOBALTALKANIMSPD] = game.options[OPT_GLOBALTALKANIMSPD] != 0 ? 5 : (-5 - 1);
	}
	// Old dialog options API for pre-3.4.0.2 games
	if (data_ver < kGameVersion_340_2) {
		game.options[OPT_DIALOGOPTIONSAPI] = -1;
	}
	// Relative asset resolution in pre-3.5.0.8 (always enabled)
	if (data_ver < kGameVersion_350) {
		game.options[OPT_RELATIVEASSETRES] = 1;
	}
	FixupSaveDirectory(game);
	return HGameFileError::None();
}

void PreReadGameData(GameSetupStruct &game, Stream *in, GameDataVersion data_ver) {
	GameSetupStruct::SerializeInfo sinfo;
	_GP(game).GameSetupStructBase::ReadFromFile(in, data_ver, sinfo);
	_GP(game).read_savegame_info(in, data_ver);  // here we also read GUID in v3.* games

	// Check for particular expansions that might have data necessary
	// for "preload" purposes
	if (sinfo.ExtensionOffset == 0u)
		return; // either no extensions, or data version is too early

	in->Seek(sinfo.ExtensionOffset, kSeekBegin);
	LoadedGameEntities ents(game);
	GameDataExtPreloader reader(ents, data_ver, in);
	reader.Read();
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
