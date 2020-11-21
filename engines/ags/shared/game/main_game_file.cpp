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

#include <cstdio>
#include "ac/audiocliptype.h"
#include "ac/dialogtopic.h"
#include "ac/gamesetupstruct.h"
#include "ac/spritecache.h"
#include "ac/view.h"
#include "ac/wordsdictionary.h"
#include "ac/dynobj/scriptaudioclip.h"
#include "core/asset.h"
#include "core/assetmanager.h"
#include "game/main_game_file.h"
#include "gui/guimain.h"
#include "script/cc_error.h"
#include "util/alignedstream.h"
#include "util/path.h"
#include "util/string_compat.h"
#include "util/string_utils.h"
#include "font/fonts.h"

namespace AGS
{
namespace Common
{

const String MainGameSource::DefaultFilename_v3 = "game28.dta";
const String MainGameSource::DefaultFilename_v2 = "ac2game.dta";
const String MainGameSource::Signature = "Adventure Creator Game File v2";

MainGameSource::MainGameSource()
    : DataVersion(kGameVersion_Undefined)
{
}

String GetMainGameFileErrorText(MainGameFileErrorType err)
{
    switch (err)
    {
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
    case kMGFErr_TooManyCursors:
        return "Too many cursors for this engine to handle.";
    case kMGFErr_InvalidPropertySchema:
        return "Failed to deserialize custom properties schema.";
    case kMGFErr_InvalidPropertyValues:
        return "Errors encountered when reading custom properties.";
    case kMGFErr_NoGlobalScript:
        return "No global script in game.";
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
    }
    return "Unknown error.";
}

LoadedGameEntities::LoadedGameEntities(GameSetupStruct &game, DialogTopic *&dialogs, ViewStruct *&views)
    : Game(game)
    , Dialogs(dialogs)
    , Views(views)
    , SpriteCount(0)
{
}

LoadedGameEntities::~LoadedGameEntities() = default;

bool IsMainGameLibrary(const String &filename)
{
    // We must not only detect if the given file is a correct AGS data library,
    // we also have to assure that this library contains main game asset.
    // Library may contain some optional data (digital audio, speech etc), but
    // that is not what we want.
    AssetLibInfo lib;
    if (AssetManager::ReadDataFileTOC(filename, lib) != kAssetNoError)
        return false;
    for (size_t i = 0; i < lib.AssetInfos.size(); ++i)
    {
        if (lib.AssetInfos[i].FileName.CompareNoCase(MainGameSource::DefaultFilename_v3) == 0 ||
            lib.AssetInfos[i].FileName.CompareNoCase(MainGameSource::DefaultFilename_v2) == 0)
        {
            return true;
        }
    }
    return false;
}

// Begins reading main game file from a generic stream
HGameFileError OpenMainGameFileBase(PStream &in, MainGameSource &src)
{
    // Check data signature
    String data_sig = String::FromStreamCount(in.get(), MainGameSource::Signature.GetLength());
    if (data_sig.Compare(MainGameSource::Signature))
        return new MainGameFileError(kMGFErr_SignatureFailed);
    // Read data format version and requested engine version
    src.DataVersion = (GameDataVersion)in->ReadInt32();
    if (src.DataVersion >= kGameVersion_230)
        src.CompiledWith = StrUtil::ReadString(in.get());
    if (src.DataVersion < kGameVersion_250)
        return new MainGameFileError(kMGFErr_FormatVersionTooOld, String::FromFormat("Required format version: %d, supported %d - %d", src.DataVersion, kGameVersion_250, kGameVersion_Current));
    if (src.DataVersion > kGameVersion_Current)
        return new MainGameFileError(kMGFErr_FormatVersionNotSupported,
            String::FromFormat("Game was compiled with %s. Required format version: %d, supported %d - %d", src.CompiledWith.GetCStr(), src.DataVersion, kGameVersion_250, kGameVersion_Current));
    // Read required capabilities
    if (src.DataVersion >= kGameVersion_341)
    {
        size_t count = in->ReadInt32();
        for (size_t i = 0; i < count; ++i)
            src.Caps.insert(StrUtil::ReadString(in.get()));
    }
    // Everything is fine, return opened stream
    src.InputStream = in;
    // Remember loaded game data version
    // NOTE: this global variable is embedded in the code too much to get
    // rid of it too easily; the easy way is to set it whenever the main
    // game file is opened.
    loaded_game_file_version = src.DataVersion;
    return HGameFileError::None();
}

HGameFileError OpenMainGameFile(const String &filename, MainGameSource &src)
{
    // Cleanup source struct
    src = MainGameSource();
    // Try to open given file
    PStream in(File::OpenFileRead(filename));
    if (!in)
        return new MainGameFileError(kMGFErr_FileOpenFailed, String::FromFormat("Filename: %s.", filename.GetCStr()));
    src.Filename = filename;
    return OpenMainGameFileBase(in, src);
}

HGameFileError OpenMainGameFileFromDefaultAsset(MainGameSource &src)
{
    // Cleanup source struct
    src = MainGameSource();
    // Try to find and open main game file
    String filename = MainGameSource::DefaultFilename_v3;
    PStream in(AssetManager::OpenAsset(filename));
    if (!in)
    {
        filename = MainGameSource::DefaultFilename_v2;
        in = PStream(AssetManager::OpenAsset(filename));
    }
    if (!in)
        return new MainGameFileError(kMGFErr_FileOpenFailed, String::FromFormat("Filename: %s.", filename.GetCStr()));
    src.Filename = filename;
    return OpenMainGameFileBase(in, src);
}

HGameFileError ReadDialogScript(PScript &dialog_script, Stream *in, GameDataVersion data_ver)
{
    if (data_ver > kGameVersion_310) // 3.1.1+ dialog script
    {
        dialog_script.reset(ccScript::CreateFromStream(in));
        if (dialog_script == nullptr)
            return new MainGameFileError(kMGFErr_CreateDialogScriptFailed, ccErrorString);
    }
    else // 2.x and < 3.1.1 dialog
    {
        dialog_script.reset();
    }
    return HGameFileError::None();
}

HGameFileError ReadScriptModules(std::vector<PScript> &sc_mods, Stream *in, GameDataVersion data_ver)
{
    if (data_ver >= kGameVersion_270) // 2.7.0+ script modules
    {
        int count = in->ReadInt32();
        sc_mods.resize(count);
        for (int i = 0; i < count; ++i)
        {
            sc_mods[i].reset(ccScript::CreateFromStream(in));
            if (sc_mods[i] == nullptr)
                return new MainGameFileError(kMGFErr_CreateScriptModuleFailed, ccErrorString);
        }
    }
    else
    {
        sc_mods.resize(0);
    }
    return HGameFileError::None();
}

void ReadViewStruct272_Aligned(std::vector<ViewStruct272> &oldv, Stream *in, size_t count)
{
    AlignedStream align_s(in, Common::kAligned_Read);
    oldv.resize(count);
    for (size_t i = 0; i < count; ++i)
    {
        oldv[i].ReadFromFile(&align_s);
        align_s.Reset();
    }
}

void ReadViews(GameSetupStruct &game, ViewStruct *&views, Stream *in, GameDataVersion data_ver)
{
    int count = game.numviews;
    views = (ViewStruct*)calloc(sizeof(ViewStruct) * count, 1);
    if (data_ver > kGameVersion_272) // 3.x views
    {
        for (int i = 0; i < game.numviews; ++i)
        {
            views[i].ReadFromFile(in);
        }
    }
    else // 2.x views
    {
        std::vector<ViewStruct272> oldv;
        ReadViewStruct272_Aligned(oldv, in, count);
        Convert272ViewsToNew(oldv, views);
    }
}

void ReadDialogs(DialogTopic *&dialog,
                 std::vector< std::shared_ptr<unsigned char> > &old_dialog_scripts,
                 std::vector<String> &old_dialog_src,
                 std::vector<String> &old_speech_lines,
                 Stream *in, GameDataVersion data_ver, int dlg_count)
{
    // TODO: I suspect +5 was a hacky way to "supress" memory access mistakes;
    // double check and remove if proved unnecessary
    dialog = (DialogTopic*)malloc(sizeof(DialogTopic) * dlg_count + 5);
    for (int i = 0; i < dlg_count; ++i)
    {
        dialog[i].ReadFromFile(in);
    }

    if (data_ver > kGameVersion_310)
        return;

    old_dialog_scripts.resize(dlg_count);
    old_dialog_src.resize(dlg_count);
    for (int i = 0; i < dlg_count; ++i)
    {
        // NOTE: originally this was read into dialog[i].optionscripts
        old_dialog_scripts[i].reset(new unsigned char[dialog[i].codesize]);
        in->Read(old_dialog_scripts[i].get(), dialog[i].codesize);

        // Encrypted text script
        int script_text_len = in->ReadInt32();
        if (script_text_len > 1)
        {
            // Originally in the Editor +20000 bytes more were allocated, with comment:
            //   "add a large buffer because it will get added to if another option is added"
            // which probably refered to this data used by old editor directly to edit dialogs
            char *buffer = new char[script_text_len];
            in->Read(buffer, script_text_len);
            if (data_ver > kGameVersion_260)
                decrypt_text(buffer);
            old_dialog_src[i] = buffer;
            delete [] buffer;
        }
        else
        {
            in->Seek(script_text_len);
        }
    }

    // Read the dialog lines
    //
    // TODO: investigate this: these strings were read much simplier in the editor, see code:
    /*
        char stringbuffer[1000];
        for (bb=0;bb<thisgame.numdlgmessage;bb++) {
            if ((filever >= 26) && (encrypted))
                read_string_decrypt(iii, stringbuffer);
            else
                fgetstring(stringbuffer, iii);
        }
    */
    int i = 0;
    char buffer[1000];
    if (data_ver <= kGameVersion_260)
    {
        // Plain text on <= 2.60
        bool end_reached = false;

        while (!end_reached)
        {
            char* nextchar = buffer;

            while (1)
            {
                *nextchar = in->ReadInt8();
                if (*nextchar == 0)
                    break;

                if ((unsigned char)*nextchar == 0xEF)
                {
                    end_reached = true;
                    in->Seek(-1);
                    break;
                }

                nextchar++;
            }

            if (end_reached)
                break;

            old_speech_lines.push_back(buffer);
            i++;
        }
    }
    else
    {
        // Encrypted text on > 2.60
        while (1)
        {
            size_t newlen = in->ReadInt32();
            if (static_cast<int32_t>(newlen) == 0xCAFEBEEF)  // GUI magic
            {
                in->Seek(-4);
                break;
            }

            newlen = Math::Min(newlen, sizeof(buffer) - 1);
            in->Read(buffer, newlen);
            buffer[newlen] = 0;
            decrypt_text(buffer);
            old_speech_lines.push_back(buffer);
            i++;
        }
    }
}

HGameFileError ReadPlugins(std::vector<PluginInfo> &infos, Stream *in)
{
    int fmt_ver = in->ReadInt32();
    if (fmt_ver != 1)
        return new MainGameFileError(kMGFErr_PluginDataFmtNotSupported, String::FromFormat("Version: %d, supported: %d", fmt_ver, 1));

    int pl_count = in->ReadInt32();
    for (int i = 0; i < pl_count; ++i)
    {
        String name = String::FromStream(in);
        size_t datasize = in->ReadInt32();
        // just check for silly datasizes
        if (datasize > PLUGIN_SAVEBUFFERSIZE)
            return new MainGameFileError(kMGFErr_PluginDataSizeTooLarge, String::FromFormat("Required: %u, max: %u", datasize, PLUGIN_SAVEBUFFERSIZE));

        PluginInfo info;
        info.Name = name;
        if (datasize > 0)
        {
            info.Data.reset(new char[datasize]);
            in->Read(info.Data.get(), datasize);
        }
        info.DataLen = datasize;
        infos.push_back(info);
    }
    return HGameFileError::None();
}

// Create the missing audioClips data structure for 3.1.x games.
// This is done by going through the data files and adding all music*.*
// and sound*.* files to it.
void BuildAudioClipArray(const std::vector<String> &assets, std::vector<ScriptAudioClip> &audioclips)
{
    char temp_name[30];
    int temp_number;
    char temp_extension[10];

    for (const String &asset : assets)
    {
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

        if (ags_stricmp(temp_name, "music") == 0)
        {
            clip.scriptName.Format("aMusic%d", temp_number);
            clip.fileName.Format("music%d.%s", temp_number, temp_extension);
            clip.bundlingType = (ags_stricmp(temp_extension, "mid") == 0) ? AUCL_BUNDLE_EXE : AUCL_BUNDLE_VOX;
            clip.type = 2;
            clip.defaultRepeat = 1;
        }
        else if (ags_stricmp(temp_name, "sound") == 0)
        {
            clip.scriptName.Format("aSound%d", temp_number);
            clip.fileName.Format("sound%d.%s", temp_number, temp_extension);
            clip.bundlingType = AUCL_BUNDLE_EXE;
            clip.type = 3;
            clip.defaultRepeat = 0;
        }
        else
        {
            continue;
        }

        clip.defaultVolume = 100;
        clip.defaultPriority = 50;
        clip.id = audioclips.size();
        audioclips.push_back(clip);
    }
}

void ApplySpriteData(GameSetupStruct &game, const LoadedGameEntities &ents, GameDataVersion data_ver)
{
    // Apply sprite flags read from original format (sequential array)
    spriteset.EnlargeTo(ents.SpriteCount - 1);
    for (size_t i = 0; i < ents.SpriteCount; ++i)
    {
        game.SpriteInfos[i].Flags = ents.SpriteFlags[i];
    }

    // Promote sprite resolutions and mark legacy resolution setting
    if (data_ver < kGameVersion_350)
    {
        for (size_t i = 0; i < ents.SpriteCount; ++i)
        {
            SpriteInfo &info = game.SpriteInfos[i];
            if (game.IsLegacyHiRes() == info.IsLegacyHiRes())
                info.Flags &= ~(SPF_HIRES | SPF_VAR_RESOLUTION);
            else
                info.Flags |= SPF_VAR_RESOLUTION;
        }
    }
}

void UpgradeFonts(GameSetupStruct &game, GameDataVersion data_ver)
{
    if (data_ver < kGameVersion_350)
    {
        for (int i = 0; i < game.numfonts; ++i)
        {
            FontInfo &finfo = game.fonts[i];
            // If the game is hi-res but font is designed for low-res, then scale it up
            if (game.IsLegacyHiRes() && game.options[OPT_HIRES_FONTS] == 0)
            {
                finfo.SizeMultiplier = HIRES_COORD_MULTIPLIER;
            }
            else
            {
                finfo.SizeMultiplier = 1;
            }
        }
    }
    if (data_ver < kGameVersion_351)
    {
        for (size_t font = 0; font < game.numfonts; font++)
        {
            FontInfo &finfo = game.fonts[font];
            // Thickness that corresponds to 1 game pixel
            finfo.AutoOutlineThickness =
                // if it's a scaled up bitmap font, move the outline out more
                (is_bitmap_font(font) && get_font_scaling_mul(font) > 1) ?
                get_fixed_pixel_size(1) : 1;
            finfo.AutoOutlineStyle = FontInfo::kSquared;
        }
    }
}

// Convert audio data to the current version
void UpgradeAudio(GameSetupStruct &game, GameDataVersion data_ver)
{
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
    for (int i = 0; i < 4; i++)
    {
        audiocliptypes[i].reservedChannels = 1;
        audiocliptypes[i].id = i;
        audiocliptypes[i].volume_reduction_while_speech_playing = 10;
    }
    audiocliptypes[3].reservedChannels = 0;

    audioclips.reserve(1000);
    // Read audio clip names from "music.vox", then from main library
    // TODO: this may become inconvenient that this code has to know about
    // "music.vox"; there might be better ways to handle this.
    // possibly making AssetManager download several libraries at once will
    // resolve this (as well as make it unnecessary to switch between them)
    std::vector<String> assets;
    // Append contents of "music.vox"
    AssetLibInfo music_lib;
    if (AssetManager::ReadDataFileTOC("music.vox", music_lib) == kAssetNoError)
    {
        for (const AssetInfo &info : music_lib.AssetInfos)
        {
            if (info.FileName.CompareLeftNoCase("music", 5) == 0 || info.FileName.CompareLeftNoCase("sound", 5) == 0)
                assets.push_back(info.FileName);
        }
    }
    // Append contents of the main game file
    const AssetLibInfo *game_lib = AssetManager::GetLibraryTOC();
    if (game_lib)
    {
        for (const AssetInfo &info : game_lib->AssetInfos)
        {
            if (info.FileName.CompareLeftNoCase("music", 5) == 0 || info.FileName.CompareLeftNoCase("sound", 5) == 0)
                assets.push_back(info.FileName);
        }
    }
    // Append contents of the game directory
    // TODO: use explicit path instead of cwd? keep this consistent with AssetManager!
    {
        al_ffblk ff;
        if (al_findfirst("*.*", &ff, FA_ALL & ~(FA_DIREC)) == 0)
        {
            do
            {
                if (ags_strnicmp(ff.name, "music", 5) == 0 || ags_strnicmp(ff.name, "sound", 5) == 0)
                    assets.push_back(ff.name);
            }
            while (al_findnext(&ff) == 0);
            al_findclose(&ff);
        }
    }
    BuildAudioClipArray(assets, audioclips);

    // Copy gathered data over to game
    game.audioClipTypes = audiocliptypes;
    game.audioClips = audioclips;
    
    // Setup sound clip played on score event
    game.scoreClipID = -1;
}

// Convert character data to the current version
void UpgradeCharacters(GameSetupStruct &game, GameDataVersion data_ver)
{
    // TODO: this was done to simplify code transition; ideally we should be
    // working with GameSetupStruct's getters and setters here
    CharacterInfo *&chars = game.chars;
    const int numcharacters = game.numcharacters;

    // Fixup charakter script names for 2.x (EGO -> cEgo)
    if (data_ver <= kGameVersion_272)
    {
        String tempbuffer;
        for (int i = 0; i < numcharacters; i++)
        {
            if (chars[i].scrname[0] == 0)
                continue;
            tempbuffer.Format("c%c%s", chars[i].scrname[0], ags_strlwr(&chars[i].scrname[1]));
            snprintf(chars[i].scrname, MAX_SCRIPT_NAME_LEN, "%s", tempbuffer.GetCStr());
        }
    }

    // Fix character walk speed for < 3.1.1
    if (data_ver <= kGameVersion_310)
    {
        for (int i = 0; i < numcharacters; i++)
        {
            if (game.options[OPT_ANTIGLIDE])
                chars[i].flags |= CHF_ANTIGLIDE;
        }
    }

    // Characters can always walk through each other on < 2.54
    if (data_ver < kGameVersion_254)
    {
        for (int i = 0; i < numcharacters; i++)
        {
            chars[i].flags |= CHF_NOBLOCKING;
        }
    }
}

void UpgradeMouseCursors(GameSetupStruct &game, GameDataVersion data_ver)
{
    if (data_ver <= kGameVersion_272)
    {
        // Change cursor.view from 0 to -1 for non-animating cursors.
        for (int i = 0; i < game.numcursors; ++i)
        {
            if (game.mcurs[i].view == 0)
                game.mcurs[i].view = -1;
        }
    }
}

// Adjusts score clip id, depending on game data version
void AdjustScoreSound(GameSetupStruct &game, GameDataVersion data_ver)
{
    if (data_ver < kGameVersion_320)
    {
        game.scoreClipID = -1;
        if (game.options[OPT_SCORESOUND] > 0)
        {
            ScriptAudioClip* clip = GetAudioClipForOldStyleNumber(game, false, game.options[OPT_SCORESOUND]);
            if (clip)
                game.scoreClipID = clip->id;
            else
                game.scoreClipID = -1;
        }
    }
}

// Assigns default global message at given index
void SetDefaultGlmsg(GameSetupStruct &game, int msgnum, const char *val)
{
    // TODO: find out why the index should be lowered by 500
    // (or rather if we may pass correct index right away)
    msgnum -= 500;
    if (game.messages[msgnum] == nullptr)
        game.messages[msgnum] = ags_strdup(val);
}

// Sets up default global messages (these are used mainly in older games)
void SetDefaultGlobalMessages(GameSetupStruct &game)
{
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

void FixupSaveDirectory(GameSetupStruct &game)
{
    // If the save game folder was not specified by game author, create one of
    // the game name, game GUID, or uniqueid, as a last resort
    if (!game.saveGameFolderName[0])
    {
        if (game.gamename[0])
            snprintf(game.saveGameFolderName, MAX_SG_FOLDER_LEN, "%s", game.gamename);
        else if (game.guid[0])
            snprintf(game.saveGameFolderName, MAX_SG_FOLDER_LEN, "%s", game.guid);
        else
            snprintf(game.saveGameFolderName, MAX_SG_FOLDER_LEN, "AGS-Game-%d", game.uniqueid);
    }
    // Lastly, fixup folder name by removing any illegal characters
    String s = Path::FixupSharedFilename(game.saveGameFolderName);
    snprintf(game.saveGameFolderName, MAX_SG_FOLDER_LEN, "%s", s.GetCStr());
}

HGameFileError ReadSpriteFlags(LoadedGameEntities &ents, Stream *in, GameDataVersion data_ver)
{
    uint32_t sprcount;
    if (data_ver < kGameVersion_256)
        sprcount = LEGACY_MAX_SPRITES_V25;
    else
        sprcount = in->ReadInt32();
    if (sprcount > (uint32_t)SpriteCache::MAX_SPRITE_INDEX + 1)
        return new MainGameFileError(kMGFErr_TooManySprites, String::FromFormat("Count: %u, max: %u", sprcount, (uint32_t)SpriteCache::MAX_SPRITE_INDEX + 1));

    ents.SpriteCount = sprcount;
    ents.SpriteFlags.reset(new char[sprcount]);
    in->Read(ents.SpriteFlags.get(), sprcount);
    return HGameFileError::None();
}

HGameFileError ReadGameData(LoadedGameEntities &ents, Stream *in, GameDataVersion data_ver)
{
    GameSetupStruct &game = ents.Game;

    {
        AlignedStream align_s(in, Common::kAligned_Read);
        game.GameSetupStructBase::ReadFromFile(&align_s);
    }

    if (game.GetGameRes().IsNull())
        return new MainGameFileError(kMGFErr_InvalidNativeResolution);

    game.read_savegame_info(in, data_ver);
    game.read_font_infos(in, data_ver);
    HGameFileError err = ReadSpriteFlags(ents, in, data_ver);
    if (!err)
        return err;
    game.ReadInvInfo_Aligned(in);
    err = game.read_cursors(in, data_ver);
    if (!err)
        return err;
    game.read_interaction_scripts(in, data_ver);
    game.read_words_dictionary(in);

    if (!game.load_compiled_script)
        return new MainGameFileError(kMGFErr_NoGlobalScript);
    ents.GlobalScript.reset(ccScript::CreateFromStream(in));
    if (!ents.GlobalScript)
        return new MainGameFileError(kMGFErr_CreateGlobalScriptFailed, ccErrorString);
    err = ReadDialogScript(ents.DialogScript, in, data_ver);
    if (!err)
        return err;
    err = ReadScriptModules(ents.ScriptModules, in, data_ver);
    if (!err)
        return err;

    ReadViews(game, ents.Views, in, data_ver);

    if (data_ver <= kGameVersion_251)
    {
        // skip unknown data
        int count = in->ReadInt32();
        in->Seek(count * 0x204);
    }

    game.read_characters(in, data_ver);
    game.read_lipsync(in, data_ver);
    game.read_messages(in, data_ver);

    ReadDialogs(ents.Dialogs, ents.OldDialogScripts, ents.OldDialogSources, ents.OldSpeechLines,
                in, data_ver, game.numdialog);
    HError err2 = GUI::ReadGUI(guis, in);
    if (!err2)
        return new MainGameFileError(kMGFErr_GameEntityFailed, err2);
    game.numgui = guis.size();

    if (data_ver >= kGameVersion_260)
    {
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
    return err;
}

HGameFileError UpdateGameData(LoadedGameEntities &ents, GameDataVersion data_ver)
{
    GameSetupStruct &game = ents.Game;
    ApplySpriteData(game, ents, data_ver);
    UpgradeFonts(game, data_ver);
    UpgradeAudio(game, data_ver);
    AdjustScoreSound(game, data_ver);
    UpgradeCharacters(game, data_ver);
    UpgradeMouseCursors(game, data_ver);
    SetDefaultGlobalMessages(game);
    // Global talking animation speed
    if (data_ver < kGameVersion_312)
    {
        // Fix animation speed for old formats
        game.options[OPT_GLOBALTALKANIMSPD] = 5;
    }
    else if (data_ver < kGameVersion_330)
    {
        // Convert game option for 3.1.2 - 3.2 games
        game.options[OPT_GLOBALTALKANIMSPD] = game.options[OPT_GLOBALTALKANIMSPD] != 0 ? 5 : (-5 - 1);
    }
    // Old dialog options API for pre-3.4.0.2 games
    if (data_ver < kGameVersion_340_2)
    {
        game.options[OPT_DIALOGOPTIONSAPI] = -1;
    }
    // Relative asset resolution in pre-3.5.0.8 (always enabled)
    if (data_ver < kGameVersion_350)
    {
        game.options[OPT_RELATIVEASSETRES] = 1;
    }
    FixupSaveDirectory(game);
    return HGameFileError::None();
}

} // namespace Common
} // namespace AGS
