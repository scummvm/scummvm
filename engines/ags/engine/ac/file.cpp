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
#include "ags/engine/ac/dynobj/dynobj_manager.h"
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

	if (rp.AssetMgr)
		return _GP(AssetMgr)->DoesAssetExist(rp.FullPath);

	return (File::IsFile(rp.FullPath) || File::IsFile(rp.AltPath)) ? 1 : 0;
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

const char *File_ResolvePath(const char *fnmm) {
	ResolvedPath rp;
	ResolveScriptPath(fnmm, true, rp);
	// Make path pretty -
	String path = Path::MakeAbsolutePath(rp.FullPath);
	return CreateNewScriptString(path.GetCStr());
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

void File_WriteRawInt(sc_File *fil, int towrite) {
	Stream *out = get_valid_file_stream_from_handle(fil->handle, "FileWriteRawInt");
	out->WriteInt32(towrite);
}

void File_WriteRawLine(sc_File *fil, const char *towrite) {
	FileWriteRawLine(fil->handle, towrite);
}

// Reads line of chars until linebreak is met or buffer is filled;
// returns whether reached the end of line (false in case not enough buffer);
// guarantees null-terminator in the buffer.
static bool File_ReadRawLineImpl(sc_File *fil, char *buffer, size_t buf_len) {
	if (buf_len == 0) return false;
	Stream *in = get_valid_file_stream_from_handle(fil->handle, "File.ReadRawLine");
	for (size_t i = 0; i < buf_len - 1; ++i) {
		int c = in->ReadByte();
		if (c < 0 || c == '\n') // EOF or LF
		{
			buffer[i] = 0;
			return true;
		}
		if (c == '\r') // CR or CRLF
		{
			c = in->ReadByte();
			// Look for '\n', but it may be missing, which is also a valid case
			if (c >= 0 && c != '\n') in->Seek(-1, kSeekCurrent);
			buffer[i] = 0;
			return true;
		}
		buffer[i] = c;
	}
	buffer[buf_len - 1] = 0;
	return false; // not enough buffer
}

void File_ReadRawLine(sc_File *fil, char *buffer) {
	size_t buflen = check_scstrcapacity(buffer);
	File_ReadRawLineImpl(fil, buffer, buflen);
	commit_scstr_update(buffer);
}

const char *File_ReadRawLineBack(sc_File *fil) {
	char readbuffer[MAX_MAXSTRLEN];
	if (File_ReadRawLineImpl(fil, readbuffer, MAX_MAXSTRLEN))
		return CreateNewScriptString(readbuffer);
	String sbuf = readbuffer;
	bool done = false;
	while (!done) {
		done = File_ReadRawLineImpl(fil, readbuffer, MAX_MAXSTRLEN);
		sbuf.Append(readbuffer);
	};
	return CreateNewScriptString(sbuf.GetCStr());
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

	char *buffer = CreateNewScriptString(lle);
	in->Read(buffer, lle);
	return buffer;
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
	return in->Seek(offset, (StreamSeek)origin);
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

const char *File_GetPath(sc_File *fil) {
	if (fil->handle <= 0)
		return nullptr;
	Stream *stream = get_valid_file_stream_from_handle(fil->handle, "File.Path");
	return CreateNewScriptString(stream->GetPath());
}

//=============================================================================


const char *GameInstallRootToken = "$INSTALLDIR$";
const char *UserSavedgamesRootToken = "$MYDOCS$";
const char *GameSavedgamesDirToken = "$SAVEGAMEDIR$";
const char *GameDataDirToken = "$APPDATADIR$";
const char *GameAssetToken = "$DATA$";
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

FSLocation PathFromInstallDir(const FSLocation &fsloc) {
	if (is_relative_filename(fsloc.FullDir))
		return FSLocation(_GP(ResPaths).DataDir).Rebase(fsloc.FullDir);
	return fsloc;
}

String PreparePathForWriting(const FSLocation &fsloc, const String &filename) {
	if (Directory::CreateAllDirectories(fsloc.BaseDir, fsloc.SubDir))
		return Path::ConcatPaths(fsloc.FullDir, filename);
	return "";
}

FSLocation GetGlobalUserConfigDir() {
	FSLocation dir = _G(platform)->GetUserGlobalConfigDirectory();
	if (is_relative_filename(dir.FullDir)) // relative dir is resolved relative to the game data dir
		return FSLocation(_GP(ResPaths).DataDir).Rebase(dir.FullDir);
	return dir;
}

FSLocation GetGameUserConfigDir() {
	FSLocation dir = _G(platform)->GetUserConfigDirectory();
	if (is_relative_filename(dir.FullDir)) // relative dir is resolved relative to the game data dir
		return FSLocation(_GP(ResPaths).DataDir).Rebase(dir.FullDir);
	else if (_GP(usetup).local_user_conf) // directive to use game dir location
		return FSLocation(_GP(ResPaths).DataDir);
	// For absolute dir, we assume it's a special directory prepared for AGS engine
	// and therefore append a game's own subdir
	return dir.Concat(_GP(game).saveGameFolderName);
}

// Constructs data dir using rules for default system location
inline FSLocation MakeDefaultDataDir(const FSLocation &def_dir) {
	// Relative dir is resolved relative to the game data dir
	if (is_relative_filename(def_dir.FullDir))
		return FSLocation(_GP(ResPaths).DataDir).Rebase(def_dir.FullDir);
	// For absolute dir, we assume it's a special directory prepared for AGS engine
	// and therefore amend it with a game's own subdir (to separate files from different games)
	return def_dir.Concat(_GP(game).saveGameFolderName);
}

// Constructs data dir using rules for the user-specified location
inline FSLocation MakeUserDataDir(const String &user_dir) {
	// If user-set location is inside game dir, then form a relative path
	if (is_relative_filename(user_dir))
		return FSLocation(_GP(ResPaths).DataDir).Rebase(user_dir);
	// Otherwise treat it as an absolute path
	return FSLocation(Path::MakeAbsolutePath(user_dir));
}

FSLocation GetGameAppDataDir() {
	if (_GP(usetup).shared_data_dir.IsEmpty())
		return MakeDefaultDataDir(_G(platform)->GetAllUsersDataDirectory());
	return MakeUserDataDir(_GP(usetup).shared_data_dir);
}

FSLocation GetGameUserDataDir() {
	if (_GP(usetup).user_data_dir.IsEmpty())
		return MakeDefaultDataDir(_G(platform)->GetUserSavedgamesDirectory());
	return MakeUserDataDir(_GP(usetup).user_data_dir);
}

bool ResolveScriptPath(const String &orig_sc_path, bool read_only, ResolvedPath &rp) {
	rp = ResolvedPath();

	// Make sure that the file path has system-compatible form
	String sc_path = orig_sc_path;
	sc_path.Replace('\\', '/');

	// File tokens (they must be the only thing in script path)
	if (sc_path.Compare(UserConfigFileToken) == 0) {
		auto loc = GetGameUserConfigDir();
		rp = ResolvedPath(loc, DefaultConfigFileName);
		return true;
	}

	// Test absolute paths
	if (!Path::IsRelativePath(sc_path)) {
		if (!read_only) {
			debug_script_warn("Attempt to access file '%s' denied (cannot write to absolute path)", sc_path.GetCStr());
			return false;
		}
		rp = ResolvedPath(sc_path);
		return true;
	}

	if (sc_path.CompareLeft(GameAssetToken, strlen(GameAssetToken)) == 0) {
		if (!read_only) {
			debug_script_warn("Attempt to access file '%s' denied (cannot write to game assets)", orig_sc_path.GetCStr());
			return false;
		}
		rp.FullPath = sc_path.Mid(strlen(GameAssetToken) + 1);
		rp.AssetMgr = true;
		return true;
	}

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
#if AGS_PLATFORM_SCUMMVM
		// the shared data dir should be remapped to the savedir otherwise some games
		// are unable to create additional files
		debugC(::AGS::kDebugFilePath, "Remapping %s to save folder", GameDataDirToken);
		parent_dir = get_save_game_directory();
		child_path = sc_path.Mid(strlen(GameSavedgamesDirToken));
#else
		parent_dir = GetGameAppDataDir();
		child_path = sc_path.Mid(strlen(GameDataDirToken));
#endif
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

		// When in Quest for Glory II "import a hero" window, don't add the gamename prefix when listing saves. This way
		// we can show saves created by the Sierra games
		if (strcmp(_GP(game).guid, "{a46a9171-f6f9-456c-9b2b-a509b560ddc0}") || !(_G(displayed_room) == 1) || !read_only) {
			if (child_path.CompareLeftNoCase(gameTarget) != 0)
				child_path = String::FromFormat("%s-%s", gameTarget.GetCStr(), child_path.GetCStr());
		} else {
			debug("ResolveScriptPath: Skipping gameprefix for QfG2AGDI!");
		}
	}
#else
	// Create a proper ResolvedPath with FSLocation separating base location
	// (which the engine is not allowed to create) and sub-dirs (created by the engine).
	parent_dir = parent_dir.Concat(Path::GetDirectoryPath(child_path));
	child_path = Path::GetFilename(child_path);
#endif

	ResolvedPath test_rp = ResolvedPath(parent_dir, child_path, alt_path);
	// don't allow write operations for relative paths outside game dir
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

bool CreateFSDirs(const FSLocation &fs) {
	return Directory::CreateAllDirectories(fs.BaseDir, fs.FullDir);
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

static int ags_pf_ungetc(int /*c*/, void * /*userdata*/) {
	return -1; // we do not want to support this
}

static long ags_pf_fread(void *p, long n, void *userdata) {
	AGS_PACKFILE_OBJ *obj = (AGS_PACKFILE_OBJ *)userdata;
	if (obj->remains > 0) {
		size_t read = MIN(obj->remains, (size_t)n);
		obj->remains -= read;
		return obj->stream->Read(p, read);
	}
	return -1;
}

static int ags_pf_putc(int /*c*/, void * /*userdata*/) {
	return -1;  // don't support write
}

static long ags_pf_fwrite(AL_CONST void * /*p*/, long /*n*/, void * /*userdata*/) {
	return -1; // don't support write
}

static int ags_pf_fseek(void * /*userdata*/, int /*offset*/) {
	return -1; // don't support seek
}

static int ags_pf_feof(void *userdata) {
	return ((AGS_PACKFILE_OBJ *)userdata)->remains == 0;
}

static int ags_pf_ferror(void *userdata) {
	return ((AGS_PACKFILE_OBJ *)userdata)->stream->GetError() ? 1 : 0;
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

PACKFILE *PackfileFromAsset(const AssetPath &path) {
	Stream *asset_stream = _GP(AssetMgr)->OpenAsset(path);
	if (!asset_stream) return nullptr;
	const size_t asset_size = asset_stream->GetLength();
	if (asset_size == 0) return nullptr;
	AGS_PACKFILE_OBJ *obj = new AGS_PACKFILE_OBJ;
	obj->stream.reset(asset_stream);
	obj->asset_size = asset_size;
	obj->remains = asset_size;
	return pack_fopen_vtable(&ags_packfile_vtable, obj);
}

String find_assetlib(const String &filename) {
	String libname = File::FindFileCI(_GP(ResPaths).DataDir, filename);
	if (AssetManager::IsDataFile(libname))
		return libname;
	if (!_GP(ResPaths).DataDir2.IsEmpty() &&
		Path::ComparePaths(_GP(ResPaths).DataDir, _GP(ResPaths).DataDir2) != 0) {
		// Hack for running in Debugger
		libname = File::FindFileCI(_GP(ResPaths).DataDir2, filename);
		if (AssetManager::IsDataFile(libname))
			return libname;
	}
	return "";
}

AssetPath get_audio_clip_assetpath(int /*bundling_type*/, const String &filename) {
	// NOTE: bundling_type is ignored now
	return AssetPath(filename, "audio");
}

AssetPath get_voice_over_assetpath(const String &filename) {
	return AssetPath(filename, "voice");
}

ScriptFileHandle *check_valid_file_handle_ptr(Stream *stream_ptr, const char *operation_name) {
	if (stream_ptr) {
		for (int i = 0; i < _G(num_open_script_files); ++i) {
			if (stream_ptr == _G(valid_handles)[i].stream.get()) {
				return &_G(valid_handles)[i];
			}
		}
	}

	String exmsg = String::FromFormat("!%s: invalid file handle; file not previously opened or has been closed", operation_name);
	quit(exmsg);
	return nullptr;
}

ScriptFileHandle *check_valid_file_handle_int32(int32_t handle, const char *operation_name) {
	if (handle > 0) {
		for (int i = 0; i < _G(num_open_script_files); ++i) {
			if (handle == _G(valid_handles)[i].handle) {
				return &_G(valid_handles)[i];
			}
		}
	}

	String exmsg = String::FromFormat("!%s: invalid file handle; file not previously opened or has been closed", operation_name);
	quit(exmsg);
	return nullptr;
}

Stream *get_valid_file_stream_from_handle(int32_t handle, const char *operation_name) {
	ScriptFileHandle *sc_handle = check_valid_file_handle_int32(handle, operation_name);
	return sc_handle ? sc_handle->stream.get() : nullptr;
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

RuntimeScriptValue Sc_File_ResolvePath(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_POBJ(const char, _GP(myScriptStringImpl), File_ResolvePath, const char);
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
	API_OBJCALL_OBJ(sc_File, const char, _GP(myScriptStringImpl), File_ReadRawLineBack);
}

// void (sc_File *fil, char *toread)
RuntimeScriptValue Sc_File_ReadString(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(sc_File, File_ReadString, char);
}

// const char* (sc_File *fil)
RuntimeScriptValue Sc_File_ReadStringBack(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(sc_File, const char, _GP(myScriptStringImpl), File_ReadStringBack);
}

// void (sc_File *fil, int towrite)
RuntimeScriptValue Sc_File_WriteInt(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(sc_File, File_WriteInt);
}

// void (sc_File *fil, int towrite)
RuntimeScriptValue Sc_File_WriteRawChar(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(sc_File, File_WriteRawChar);
}

RuntimeScriptValue Sc_File_WriteRawInt(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(sc_File, File_WriteRawInt);
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

RuntimeScriptValue Sc_File_GetPath(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(sc_File, const char, _GP(myScriptStringImpl), File_GetPath);
}

void RegisterFileAPI() {
	ScFnRegister file_api[] = {
		{"File::Delete^1", API_FN_PAIR(File_Delete)},
		{"File::Exists^1", API_FN_PAIR(File_Exists)},
		{"File::Open^2", API_FN_PAIR(sc_OpenFile)},
		{"File::ResolvePath^1", API_FN_PAIR(File_ResolvePath)},

		{"File::Close^0", API_FN_PAIR(File_Close)},
		{"File::ReadInt^0", API_FN_PAIR(File_ReadInt)},
		{"File::ReadRawChar^0", API_FN_PAIR(File_ReadRawChar)},
		{"File::ReadRawInt^0", API_FN_PAIR(File_ReadRawInt)},
		{"File::ReadRawLine^1", API_FN_PAIR(File_ReadRawLine)},
		{"File::ReadRawLineBack^0", API_FN_PAIR(File_ReadRawLineBack)},
		{"File::ReadString^1", API_FN_PAIR(File_ReadString)},
		{"File::ReadStringBack^0", API_FN_PAIR(File_ReadStringBack)},
		{"File::WriteInt^1", API_FN_PAIR(File_WriteInt)},
		{"File::WriteRawChar^1", API_FN_PAIR(File_WriteRawChar)},
		{"File::WriteRawInt^1", API_FN_PAIR(File_WriteRawInt)},
		{"File::WriteRawLine^1", API_FN_PAIR(File_WriteRawLine)},
		{"File::WriteString^1", API_FN_PAIR(File_WriteString)},
		{"File::Seek^2", API_FN_PAIR(File_Seek)},
		{"File::get_EOF", API_FN_PAIR(File_GetEOF)},
		{"File::get_Error", API_FN_PAIR(File_GetError)},
		{"File::get_Position", API_FN_PAIR(File_GetPosition)},
		{"File::get_Path", API_FN_PAIR(File_GetPath)},
	};

	ccAddExternalFunctions361(file_api);
}

} // namespace AGS3
