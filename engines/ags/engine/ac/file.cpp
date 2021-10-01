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

#include "common/config-manager.h"
#include "ags/engine/ac/asset_helper.h"
#include "ags/shared/ac/audio_clip_type.h"
#include "ags/engine/ac/file.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/global_file.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/shared/core/asset.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/game_file.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/string.h"
#include "ags/shared/util/string_utils.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/ags.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

// object-based File routines

int File_Exists(const char *fnmm) {

	ResolvedPath rp;
	if (!ResolveScriptPath(fnmm, true, rp))
		return 0;

	return (File::TestReadFile(rp.FullPath) || File::TestReadFile(rp.AltPath)) ? 1 : 0;
}

int File_Delete(const char *fnmm) {

	ResolvedPath rp;
	if (!ResolveScriptPath(fnmm, false, rp))
		return 0;

	if (File::DeleteFile(rp.FullPath))
		return 1;
	if (_G(errnum) == AL_ENOENT && !rp.AltPath.IsEmpty() && rp.AltPath.Compare(rp.FullPath) != 0)
		return File::DeleteFile(rp.AltPath) ? 1 : 0;
	return 0;
}

void *sc_OpenFile(const char *fnmm, int mode) {
	if ((mode < scFileRead) || (mode > scFileAppend))
		quit("!OpenFile: invalid file mode");

	sc_File *scf = new sc_File();
	if (scf->OpenFile(fnmm, mode) == 0) {
		delete scf;
		return nullptr;
	}
	ccRegisterManagedObject(scf, scf);
	return scf;
}

void File_Close(sc_File *fil) {
	fil->Close();
}

void File_WriteString(sc_File *fil, const char *towrite) {
	FileWrite(fil->handle, towrite);
}

void File_WriteInt(sc_File *fil, int towrite) {
	FileWriteInt(fil->handle, towrite);
}

void File_WriteRawChar(sc_File *fil, int towrite) {
	FileWriteRawChar(fil->handle, towrite);
}

void File_WriteRawLine(sc_File *fil, const char *towrite) {
	FileWriteRawLine(fil->handle, towrite);
}

void File_ReadRawLine(sc_File *fil, char *buffer) {
	Stream *in = get_valid_file_stream_from_handle(fil->handle, "File.ReadRawLine");
	check_strlen(buffer);
	int i = 0;
	while (i < _G(MAXSTRLEN) - 1) {
		buffer[i] = in->ReadInt8();
		if (buffer[i] == '\r') {
			// CR -- skip LF and abort
			in->ReadInt8();
			break;
		}
		if (buffer[i] == '\n')  // LF only -- abort
			break;
		if (in->EOS())  // EOF -- abort
			break;
		i++;
	}
	buffer[i] = 0;
}

const char *File_ReadRawLineBack(sc_File *fil) {
	char readbuffer[MAX_MAXSTRLEN + 1];
	File_ReadRawLine(fil, readbuffer);
	return CreateNewScriptString(readbuffer);
}

void File_ReadString(sc_File *fil, char *toread) {
	FileRead(fil->handle, toread);
}

const char *File_ReadStringBack(sc_File *fil) {
	Stream *in = get_valid_file_stream_from_handle(fil->handle, "File.ReadStringBack");
	if (in->EOS()) {
		return CreateNewScriptString("");
	}

	size_t lle = (uint32_t)in->ReadInt32();
	if (lle == 0) {
		debug_script_warn("File.ReadStringBack: file was not written by WriteString");
		return CreateNewScriptString("");
	}

	char *retVal = (char *)malloc(lle);
	in->Read(retVal, lle);

	return CreateNewScriptString(retVal, false);
}

int File_ReadInt(sc_File *fil) {
	return FileReadInt(fil->handle);
}

int File_ReadRawChar(sc_File *fil) {
	return FileReadRawChar(fil->handle);
}

int File_ReadRawInt(sc_File *fil) {
	return FileReadRawInt(fil->handle);
}

int File_Seek(sc_File *fil, int offset, int origin) {
	Stream *in = get_valid_file_stream_from_handle(fil->handle, "File.Seek");
	if (!in->Seek(offset, (StreamSeek)origin)) {
		return -1;
	}
	return in->GetPosition();
}

int File_GetEOF(sc_File *fil) {
	if (fil->handle <= 0)
		return 1;
	return FileIsEOF(fil->handle);
}

int File_GetError(sc_File *fil) {
	if (fil->handle <= 0)
		return 1;
	return FileIsError(fil->handle);
}

int File_GetPosition(sc_File *fil) {
	if (fil->handle <= 0)
		return -1;
	Stream *stream = get_valid_file_stream_from_handle(fil->handle, "File.Position");
	// TODO: a problem is that AGS script does not support unsigned or long int
	return (int)stream->GetPosition();
}

//=============================================================================


const char *GameInstallRootToken = "$INSTALLDIR$";
const char *UserSavedgamesRootToken = "$MYDOCS$";
const char *GameSavedgamesDirToken = "$SAVEGAMEDIR$";
const char *GameDataDirToken = "$APPDATADIR$";
const char *UserConfigFileToken = "$CONFIGFILE$";

void FixupFilename(char *filename) {
	const char *illegal = _G(platform)->GetIllegalFileChars();
	for (char *name_ptr = filename; *name_ptr; ++name_ptr) {
		if (*name_ptr < ' ') {
			*name_ptr = '_';
		} else {
			for (const char *ch_ptr = illegal; *ch_ptr; ++ch_ptr)
				if (*name_ptr == *ch_ptr)
					*name_ptr = '_';
		}
	}
}

String PathFromInstallDir(const String &path) {
	if (Path::IsRelativePath(path))
		return Path::ConcatPaths(_GP(ResPaths).DataDir, path);
	return path;
}

String PreparePathForWriting(const FSLocation &fsloc, const String &filename) {
	if (Directory::CreateAllDirectories(fsloc.BaseDir, fsloc.SubDir))
		return Path::ConcatPaths(fsloc.FullDir, filename);
	return "";
}

FSLocation GetGlobalUserConfigDir() {
	String dir = _G(platform)->GetUserGlobalConfigDirectory();
	if (Path::IsRelativePath(dir)) // relative dir is resolved relative to the game data dir
		return FSLocation(_GP(ResPaths).DataDir, dir);
	return FSLocation(dir);
}

FSLocation GetGameUserConfigDir() {
	String dir = _G(platform)->GetUserConfigDirectory();
	if (Path::IsRelativePath(dir)) // relative dir is resolved relative to the game data dir
		return FSLocation(_GP(ResPaths).DataDir, dir);
	else if (_GP(usetup).local_user_conf) // directive to use game dir location
		return FSLocation(_GP(ResPaths).DataDir);
	// For absolute dir, we assume it's a special directory prepared for AGS engine
	// and therefore amend it with a game own subdir
	return FSLocation(dir, _GP(game).saveGameFolderName);
}

// A helper function that deduces a data directory either using default system location,
// or user option from config. In case of a default location a path is appended with
// game's "save folder" name, which is meant to separate files from different games.
static FSLocation MakeGameDataDir(const String &default_dir, const String &user_option) {
	if (user_option.IsEmpty()) {
		String dir = default_dir;
		if (Path::IsRelativePath(dir)) // relative dir is resolved relative to the game data dir
			return FSLocation(_GP(ResPaths).DataDir, dir);
		// For absolute dir, we assume it's a special directory prepared for AGS engine
		// and therefore amend it with a game own subdir
		return FSLocation(dir, _GP(game).saveGameFolderName);
	}
	// If this location is set up by user config, then use it as is (resolving relative path if necessary)
	String dir = user_option;
	if (Path::IsSameOrSubDir(_GP(ResPaths).DataDir, dir)) // check if it's inside game dir
		return FSLocation(_GP(ResPaths).DataDir, Path::MakeRelativePath(_GP(ResPaths).DataDir, dir));
	dir = Path::MakeAbsolutePath(dir);
	return FSLocation(dir);
}

FSLocation GetGameAppDataDir() {
	return MakeGameDataDir(_G(platform)->GetAllUsersDataDirectory(), _GP(usetup).shared_data_dir);
}

FSLocation GetGameUserDataDir() {
	return MakeGameDataDir(_G(platform)->GetUserSavedgamesDirectory(), _GP(usetup).user_data_dir);
}

bool ResolveScriptPath(const String &orig_sc_path, bool read_only, ResolvedPath &rp) {
	debugC(::AGS::kDebugFilePath, "ResolveScriptPath(%s)", orig_sc_path.GetCStr());
	rp = ResolvedPath();

	bool is_absolute = !is_relative_filename(orig_sc_path.GetCStr());
	if (is_absolute && !read_only) {
		debug_script_warn("Attempt to access file '%s' denied (cannot write to absolute path)", orig_sc_path.GetCStr());
		return false;
	}

	if (is_absolute) {
		rp = ResolvedPath(orig_sc_path);
		return true;
	}

	String sc_path = orig_sc_path;
	FSLocation parent_dir;
	String child_path;
	String alt_path;
	if (sc_path.CompareLeft(GameInstallRootToken) == 0) {
		if (!read_only) {
			debug_script_warn("Attempt to access file '%s' denied (cannot write to game installation directory)",
				sc_path.GetCStr());
			return false;
		}
		parent_dir = FSLocation(_GP(ResPaths).DataDir);
		child_path = sc_path.Mid(strlen(GameInstallRootToken));
	} else if (sc_path.CompareLeft(GameSavedgamesDirToken) == 0) {
		parent_dir = get_save_game_directory();
		child_path = sc_path.Mid(strlen(GameSavedgamesDirToken));
#if AGS_PLATFORM_SCUMMVM
		// Remap "agsgame.*"
		const char *agsSavePrefix = "/agssave.";
		if (child_path.CompareLeft(agsSavePrefix) == 0) {
			debugC(::AGS::kDebugFilePath, "Remapping agssave.* to ScummVM savegame files");
			String suffix = child_path.Mid(strlen(agsSavePrefix));
			if (suffix.CompareLeft("*") == 0) {
				Common::String file_name = ::AGS::g_vm->getSaveStateName(999);
				Common::replace(file_name, "999", "*");
				child_path = file_name;
			} else {
				int slotNum = suffix.ToInt();
				child_path = ::AGS::g_vm->getSaveStateName(slotNum);
			}
		}
#endif
	} else if (sc_path.CompareLeft(GameDataDirToken) == 0) {
		parent_dir = GetGameAppDataDir();
		child_path = sc_path.Mid(strlen(GameDataDirToken));
	} else {
		child_path = sc_path;

		// For cases where a file is trying to write to a game path, always remap
		// it to write to a savefile. For normal reading, we thus need to give
		// preference to any save file with a given name before looking in the
		// game folder. This for example fixes an issue with The Blackwell Legacy,
		// which wants to create a new prog.bwl in the game folder
		parent_dir = FSLocation(SAVE_FOLDER_PREFIX);

		if (read_only)
			alt_path = Path::ConcatPaths(_GP(ResPaths).DataDir, sc_path);
	}

	// Sometimes we have multiple consecutive slashes or backslashes.
	// Remove all of them at the start of the child path.
	while (!child_path.IsEmpty() && (child_path[0u] == '\\' || child_path[0u] == '/'))
		child_path.ClipLeft(1);

#if AGS_PLATFORM_SCUMMVM
	// For files on savepath, always ensure it starts with the game target prefix to avoid
	// conflicts (as we usually have the same save dir for all games).
	// Also flatten the path if needed as we do not support subdirectories in the save folder.
	if (parent_dir.BaseDir == SAVE_FOLDER_PREFIX) {
		debugC(::AGS::kDebugFilePath, "Adding ScummVM game target prefix and flatten path");
		child_path.Replace('/', '-');
		String gameTarget = ConfMan.getActiveDomainName();
		if (child_path.CompareLeftNoCase(gameTarget) != 0)
			child_path = String::FromFormat("%s-%s", gameTarget.GetCStr(), child_path.GetCStr());
	}
#endif

	// don't allow write operations for relative paths outside game dir
	ResolvedPath test_rp = ResolvedPath(parent_dir, child_path, alt_path);
	if (!read_only) {
		if (!Path::IsSameOrSubDir(test_rp.Loc.FullDir, test_rp.FullPath)) {
			debug_script_warn("Attempt to access file '%s' denied (outside of game directory)", sc_path.GetCStr());
			return false;
		}
	}

	rp = test_rp;
	debugC(::AGS::kDebugFilePath, "Final path: %s", rp.FullPath.GetCStr());
	if (!rp.AltPath.IsEmpty())
		debugC(::AGS::kDebugFilePath, "Alt path: %s", rp.AltPath.GetCStr());
	return true;
}

bool ResolveWritePathAndCreateDirs(const String &sc_path, ResolvedPath &rp) {
	if (!ResolveScriptPath(sc_path, false, rp))
		return false;

	if (!rp.Loc.SubDir.IsEmpty() &&
			!Directory::CreateAllDirectories(rp.Loc.BaseDir, rp.Loc.SubDir)) {
		debug_script_warn("ResolveScriptPath: failed to create all subdirectories: %s", rp.FullPath.GetCStr());
		return false;
	}
	return true;
}

Stream *LocateAsset(const AssetPath &path, size_t &asset_size) {
	String assetname = path.Name;
	String filter = path.Filter;
	soff_t asset_sz = 0;
	Stream *asset_stream = _GP(AssetMgr)->OpenAsset(assetname, filter, &asset_sz);
	asset_size = asset_sz;
	return asset_stream;
}

//
// AGS custom PACKFILE callbacks, that use our own Stream object
//
static int ags_pf_fclose(void *userdata) {
	delete(AGS_PACKFILE_OBJ *)userdata;
	return 0;
}

static int ags_pf_getc(void *userdata) {
	AGS_PACKFILE_OBJ *obj = (AGS_PACKFILE_OBJ *)userdata;
	if (obj->remains > 0) {
		obj->remains--;
		return obj->stream->ReadByte();
	}
	return -1;
}

static int ags_pf_ungetc(int c, void *userdata) {
	return -1; // we do not want to support this
}

static long ags_pf_fread(void *p, long n, void *userdata) {
	AGS_PACKFILE_OBJ *obj = (AGS_PACKFILE_OBJ *)userdata;
	if (obj->remains > 0) {
		size_t read = Math::Min(obj->remains, (size_t)n);
		obj->remains -= read;
		return obj->stream->Read(p, read);
	}
	return -1;
}

static int ags_pf_putc(int c, void *userdata) {
	return -1;  // don't support write
}

static long ags_pf_fwrite(AL_CONST void *p, long n, void *userdata) {
	return -1; // don't support write
}

static int ags_pf_fseek(void *userdata, int offset) {
	return -1; // don't support seek
}

static int ags_pf_feof(void *userdata) {
	return ((AGS_PACKFILE_OBJ *)userdata)->remains == 0;
}

static int ags_pf_ferror(void *userdata) {
	return ((AGS_PACKFILE_OBJ *)userdata)->stream->HasErrors() ? 1 : 0;
}

// Custom PACKFILE callback table
static PACKFILE_VTABLE ags_packfile_vtable = {
	ags_pf_fclose,
	ags_pf_getc,
	ags_pf_ungetc,
	ags_pf_fread,
	ags_pf_putc,
	ags_pf_fwrite,
	ags_pf_fseek,
	ags_pf_feof,
	ags_pf_ferror
};
//

PACKFILE *PackfileFromAsset(const AssetPath &path, size_t &asset_size) {
	Stream *asset_stream = LocateAsset(path, asset_size);
	if (asset_stream && asset_size > 0) {
		AGS_PACKFILE_OBJ *obj = new AGS_PACKFILE_OBJ;
		obj->stream.reset(asset_stream);
		obj->asset_size = asset_size;
		obj->remains = asset_size;
		return pack_fopen_vtable(&ags_packfile_vtable, obj);
	}
	return nullptr;
}

bool DoesAssetExistInLib(const AssetPath &path) {
	String assetname = path.Name;
	String filter = path.Filter;
	return _GP(AssetMgr)->DoesAssetExist(assetname, filter);
}

String find_assetlib(const String &filename) {
	String libname = File::FindFileCI(_GP(ResPaths).DataDir, filename);
	if (AssetManager::IsDataFile(libname))
		return libname;
	if (Path::ComparePaths(_GP(ResPaths).DataDir, _GP(ResPaths).DataDir2) != 0) {
		// Hack for running in Debugger
		libname = File::FindFileCI(_GP(ResPaths).DataDir2, filename);
		if (AssetManager::IsDataFile(libname))
			return libname;
	}
	return "";
}

AssetPath get_audio_clip_assetpath(int bundling_type, const String &filename) {
	return AssetPath(filename, "audio");
}

AssetPath get_voice_over_assetpath(const String &filename) {
	return AssetPath(filename, "voice");
}

ScriptFileHandle valid_handles[MAX_OPEN_SCRIPT_FILES + 1];
// [IKM] NOTE: this is not precisely the number of files opened at this moment,
// but rather maximal number of handles that were used simultaneously during game run
int num_open_script_files = 0;
ScriptFileHandle *check_valid_file_handle_ptr(Stream *stream_ptr, const char *operation_name) {
	if (stream_ptr) {
		for (int i = 0; i < num_open_script_files; ++i) {
			if (stream_ptr == valid_handles[i].stream) {
				return &valid_handles[i];
			}
		}
	}

	String exmsg = String::FromFormat("!%s: invalid file handle; file not previously opened or has been closed", operation_name);
	quit(exmsg);
	return nullptr;
}

ScriptFileHandle *check_valid_file_handle_int32(int32_t handle, const char *operation_name) {
	if (handle > 0) {
		for (int i = 0; i < num_open_script_files; ++i) {
			if (handle == valid_handles[i].handle) {
				return &valid_handles[i];
			}
		}
	}

	String exmsg = String::FromFormat("!%s: invalid file handle; file not previously opened or has been closed", operation_name);
	quit(exmsg);
	return nullptr;
}

Stream *get_valid_file_stream_from_handle(int32_t handle, const char *operation_name) {
	ScriptFileHandle *sc_handle = check_valid_file_handle_int32(handle, operation_name);
	return sc_handle ? sc_handle->stream : nullptr;
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// int (const char *fnmm)
RuntimeScriptValue Sc_File_Delete(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(File_Delete, const char);
}

// int (const char *fnmm)
RuntimeScriptValue Sc_File_Exists(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_POBJ(File_Exists, const char);
}

// void *(const char *fnmm, int mode)
RuntimeScriptValue Sc_sc_OpenFile(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO_POBJ_PINT(sc_File, sc_OpenFile, const char);
}

// void (sc_File *fil)
RuntimeScriptValue Sc_File_Close(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(sc_File, File_Close);
}

// int (sc_File *fil)
RuntimeScriptValue Sc_File_ReadInt(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(sc_File, File_ReadInt);
}

// int (sc_File *fil)
RuntimeScriptValue Sc_File_ReadRawChar(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(sc_File, File_ReadRawChar);
}

// int (sc_File *fil)
RuntimeScriptValue Sc_File_ReadRawInt(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(sc_File, File_ReadRawInt);
}

// void (sc_File *fil, char* buffer)
RuntimeScriptValue Sc_File_ReadRawLine(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(sc_File, File_ReadRawLine, char);
}

// const char* (sc_File *fil)
RuntimeScriptValue Sc_File_ReadRawLineBack(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ(sc_File, const char, _GP(myScriptStringImpl), File_ReadRawLineBack);
}

// void (sc_File *fil, char *toread)
RuntimeScriptValue Sc_File_ReadString(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(sc_File, File_ReadString, char);
}

// const char* (sc_File *fil)
RuntimeScriptValue Sc_File_ReadStringBack(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_CONST_OBJCALL_OBJ(sc_File, const char, _GP(myScriptStringImpl), File_ReadStringBack);
}

// void (sc_File *fil, int towrite)
RuntimeScriptValue Sc_File_WriteInt(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(sc_File, File_WriteInt);
}

// void (sc_File *fil, int towrite)
RuntimeScriptValue Sc_File_WriteRawChar(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(sc_File, File_WriteRawChar);
}

// void (sc_File *fil, const char *towrite)
RuntimeScriptValue Sc_File_WriteRawLine(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(sc_File, File_WriteRawLine, const char);
}

// void (sc_File *fil, const char *towrite)
RuntimeScriptValue Sc_File_WriteString(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(sc_File, File_WriteString, const char);
}

RuntimeScriptValue Sc_File_Seek(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_PINT2(sc_File, File_Seek);
}

// int (sc_File *fil)
RuntimeScriptValue Sc_File_GetEOF(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(sc_File, File_GetEOF);
}

// int (sc_File *fil)
RuntimeScriptValue Sc_File_GetError(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(sc_File, File_GetError);
}

RuntimeScriptValue Sc_File_GetPosition(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(sc_File, File_GetPosition);
}


void RegisterFileAPI() {
	ccAddExternalStaticFunction("File::Delete^1", Sc_File_Delete);
	ccAddExternalStaticFunction("File::Exists^1", Sc_File_Exists);
	ccAddExternalStaticFunction("File::Open^2", Sc_sc_OpenFile);
	ccAddExternalObjectFunction("File::Close^0", Sc_File_Close);
	ccAddExternalObjectFunction("File::ReadInt^0", Sc_File_ReadInt);
	ccAddExternalObjectFunction("File::ReadRawChar^0", Sc_File_ReadRawChar);
	ccAddExternalObjectFunction("File::ReadRawInt^0", Sc_File_ReadRawInt);
	ccAddExternalObjectFunction("File::ReadRawLine^1", Sc_File_ReadRawLine);
	ccAddExternalObjectFunction("File::ReadRawLineBack^0", Sc_File_ReadRawLineBack);
	ccAddExternalObjectFunction("File::ReadString^1", Sc_File_ReadString);
	ccAddExternalObjectFunction("File::ReadStringBack^0", Sc_File_ReadStringBack);
	ccAddExternalObjectFunction("File::WriteInt^1", Sc_File_WriteInt);
	ccAddExternalObjectFunction("File::WriteRawChar^1", Sc_File_WriteRawChar);
	ccAddExternalObjectFunction("File::WriteRawLine^1", Sc_File_WriteRawLine);
	ccAddExternalObjectFunction("File::WriteString^1", Sc_File_WriteString);
	ccAddExternalObjectFunction("File::Seek^2", Sc_File_Seek);
	ccAddExternalObjectFunction("File::get_EOF", Sc_File_GetEOF);
	ccAddExternalObjectFunction("File::get_Error", Sc_File_GetError);
	ccAddExternalObjectFunction("File::get_Position", Sc_File_GetPosition);
}

} // namespace AGS3
