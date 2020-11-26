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

#include "ags/lib/aldumb.h"
#include "ags/engine/ac/asset_helper.h"
#include "ags/shared/ac/audiocliptype.h"
#include "ags/engine/ac/file.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/global_file.h"
#include "ags/engine/ac/path_helper.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/shared/util/misc.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/core/assetmanager.h"
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
#include "ags/engine/ac/dynobj/scriptstring.h"

namespace AGS3 {

using namespace AGS::Shared;

extern GameSetup usetup;
extern GameSetupStruct game;
extern AGSPlatformDriver *platform;

extern int MAXSTRLEN;

// TODO: the asset path configuration should certainly be revamped at some
// point, with uniform method of configuring auxiliary paths and packages.

// Installation directory, may contain absolute or relative path
String installDirectory;
// Installation directory, containing audio files
String installAudioDirectory;
// Installation directory, containing voice-over files
String installVoiceDirectory;

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

	if (::remove(rp.FullPath) == 0)
		return 1;
	if (errnum == ENOENT && !rp.AltPath.IsEmpty() && rp.AltPath.Compare(rp.FullPath) != 0)
		return ::remove(rp.AltPath) == 0 ? 1 : 0;
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
	while (i < MAXSTRLEN - 1) {
		buffer[i] = in->ReadInt8();
		if (buffer[i] == 13) {
			// CR -- skip LF and abort
			in->ReadInt8();
			break;
		}
		if (buffer[i] == 10)  // LF only -- abort
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

	int lle = in->ReadInt32();
	if ((lle >= 20000) || (lle < 1))
		quit("!File.ReadStringBack: file was not written by WriteString");

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


const String GameInstallRootToken = "$INSTALLDIR$";
const String UserSavedgamesRootToken = "$MYDOCS$";
const String GameSavedgamesDirToken = "$SAVEGAMEDIR$";
const String GameDataDirToken = "$APPDATADIR$";

void FixupFilename(char *filename) {
	const char *illegal = platform->GetIllegalFileChars();
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

// Tests if there is a special path token in the beginning of the given path;
// if there is and there is no slash between token and the rest of the string,
// then assigns new string that has such slash.
// Returns TRUE if the new string was created, and FALSE if the path was good.
bool FixSlashAfterToken(const String &path, const String &token, String &new_path) {
	if (path.CompareLeft(token) == 0 && path.GetLength() > token.GetLength() &&
		path[token.GetLength()] != '/') {
		new_path = String::FromFormat("%s/%s", token.GetCStr(), path.Mid(token.GetLength()).GetCStr());
		return true;
	}
	return false;
}

String FixSlashAfterToken(const String &path) {
	String fixed_path = path;
	Path::FixupPath(fixed_path);
	if (FixSlashAfterToken(fixed_path, GameInstallRootToken, fixed_path) ||
		FixSlashAfterToken(fixed_path, UserSavedgamesRootToken, fixed_path) ||
		FixSlashAfterToken(fixed_path, GameSavedgamesDirToken, fixed_path) ||
		FixSlashAfterToken(fixed_path, GameDataDirToken, fixed_path))
		return fixed_path;
	return path;
}

String MakeSpecialSubDir(const String &sp_dir) {
	if (is_relative_filename(sp_dir))
		return sp_dir;
	String full_path = sp_dir;
	if (full_path.GetLast() != '/' && full_path.GetLast() != '\\')
		full_path.AppendChar('/');
	full_path.Append(game.saveGameFolderName);
	Directory::CreateDirectory(full_path);
	return full_path;
}

String MakeAppDataPath() {
	String app_data_path = usetup.shared_data_dir;
	if (app_data_path.IsEmpty())
		app_data_path = MakeSpecialSubDir(PathOrCurDir(platform->GetAllUsersDataDirectory()));
	Directory::CreateDirectory(app_data_path);
	app_data_path.AppendChar('/');
	return app_data_path;
}

bool ResolveScriptPath(const String &orig_sc_path, bool read_only, ResolvedPath &rp) {
	rp = ResolvedPath();

	bool is_absolute = !is_relative_filename(orig_sc_path);
	if (is_absolute && !read_only) {
		debug_script_warn("Attempt to access file '%s' denied (cannot write to absolute path)", orig_sc_path.GetCStr());
		return false;
	}

	if (is_absolute) {
		rp.FullPath = orig_sc_path;
		return true;
	}

	String sc_path = FixSlashAfterToken(orig_sc_path);
	String parent_dir;
	String child_path;
	String alt_path;
	if (sc_path.CompareLeft(GameInstallRootToken, GameInstallRootToken.GetLength()) == 0) {
		if (!read_only) {
			debug_script_warn("Attempt to access file '%s' denied (cannot write to game installation directory)",
				sc_path.GetCStr());
			return false;
		}
		parent_dir = get_install_dir();
		parent_dir.AppendChar('/');
		child_path = sc_path.Mid(GameInstallRootToken.GetLength());
	} else if (sc_path.CompareLeft(GameSavedgamesDirToken, GameSavedgamesDirToken.GetLength()) == 0) {
		parent_dir = get_save_game_directory();
		child_path = sc_path.Mid(GameSavedgamesDirToken.GetLength());
	} else if (sc_path.CompareLeft(GameDataDirToken, GameDataDirToken.GetLength()) == 0) {
		parent_dir = MakeAppDataPath();
		child_path = sc_path.Mid(GameDataDirToken.GetLength());
	} else {
		child_path = sc_path;

		// For games which were made without having safe paths in mind,
		// provide two paths: a path to the local directory and a path to
		// AppData directory.
		// This is done in case game writes a file by local path, and would
		// like to read it back later. Since AppData path has higher priority,
		// game will first check the AppData location and find a previously
		// written file.
		// If no file was written yet, but game is trying to read a pre-created
		// file in the installation directory, then such file will be found
		// following the 'alt_path'.
		parent_dir = MakeAppDataPath();
		// Set alternate non-remapped "unsafe" path for read-only operations
		if (read_only)
			alt_path = String::FromFormat("%s/%s", get_install_dir().GetCStr(), sc_path.GetCStr());

		// For games made in the safe-path-aware versions of AGS, report a warning
		// if the unsafe path is used for write operation
		if (!read_only && game.options[OPT_SAFEFILEPATHS]) {
			debug_script_warn("Attempt to access file '%s' denied (cannot write to game installation directory);\nPath will be remapped to the app data directory: '%s'",
				sc_path.GetCStr(), parent_dir.GetCStr());
		}
	}

	if (child_path[0u] == '\\' || child_path[0u] == '/')
		child_path.ClipLeft(1);

	String full_path = String::FromFormat("%s%s", parent_dir.GetCStr(), child_path.GetCStr());
	// don't allow write operations for relative paths outside game dir
	if (!read_only) {
		if (!Path::IsSameOrSubDir(parent_dir, full_path)) {
			debug_script_warn("Attempt to access file '%s' denied (outside of game directory)", sc_path.GetCStr());
			return false;
		}
	}
	rp.BaseDir = parent_dir;
	rp.FullPath = full_path;
	rp.AltPath = alt_path;
	return true;
}

bool ResolveWritePathAndCreateDirs(const String &sc_path, ResolvedPath &rp) {
	if (!ResolveScriptPath(sc_path, false, rp))
		return false;
	if (!Directory::CreateAllDirectories(rp.BaseDir, Path::GetDirectoryPath(rp.FullPath))) {
		debug_script_warn("ResolveScriptPath: failed to create all subdirectories: %s", rp.FullPath.GetCStr());
		return false;
	}
	return true;
}

Stream *LocateAsset(const AssetPath &path, size_t &asset_size) {
	String assetlib = path.first;
	String assetname = path.second;
	bool needsetback = false;
	// Change to the different library, if required
	// TODO: teaching AssetManager to register multiple libraries simultaneously
	// will let us skip this step, and also make this operation much faster.
	if (!assetlib.IsEmpty() && assetlib.CompareNoCase(ResPaths.GamePak.Name) != 0) {
		AssetManager::SetDataFile(get_known_assetlib(assetlib));
		needsetback = true;
	}
	Stream *asset_stream = AssetManager::OpenAsset(assetname);
	asset_size = AssetManager::GetLastAssetSize();
	if (needsetback)
		AssetManager::SetDataFile(ResPaths.GamePak.Path);
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

DUMBFILE *DUMBfileFromAsset(const AssetPath &path, size_t &asset_size) {
	PACKFILE *pf = PackfileFromAsset(path, asset_size);
	if (pf)
		return dumbfile_open_packfile(pf);
	return nullptr;
}

bool DoesAssetExistInLib(const AssetPath &assetname) {
	bool needsetback = false;
	// Change to the different library, if required
	// TODO: teaching AssetManager to register multiple libraries simultaneously
	// will let us skip this step, and also make this operation much faster.
	if (!assetname.first.IsEmpty() && assetname.first.CompareNoCase(ResPaths.GamePak.Name) != 0) {
		AssetManager::SetDataFile(get_known_assetlib(assetname.first));
		needsetback = true;
	}
	bool res = AssetManager::DoesAssetExist(assetname.second);
	if (needsetback)
		AssetManager::SetDataFile(ResPaths.GamePak.Path);
	return res;
}

void set_install_dir(const String &path, const String &audio_path, const String &voice_path) {
	if (path.IsEmpty())
		installDirectory = ".";
	else
		installDirectory = Path::MakePathNoSlash(path);
	if (audio_path.IsEmpty())
		installAudioDirectory = ".";
	else
		installAudioDirectory = Path::MakePathNoSlash(audio_path);
	if (voice_path.IsEmpty())
		installVoiceDirectory = ".";
	else
		installVoiceDirectory = Path::MakePathNoSlash(voice_path);
}

String get_install_dir() {
	return installDirectory;
}

String get_audio_install_dir() {
	return installAudioDirectory;
}

String get_voice_install_dir() {
	return installVoiceDirectory;
}

void get_install_dir_path(char *buffer, const char *fileName) {
	sprintf(buffer, "%s/%s", installDirectory.GetCStr(), fileName);
}

String find_assetlib(const String &filename) {
	String libname = cbuf_to_string_and_free(ci_find_file(ResPaths.DataDir, filename));
	if (AssetManager::IsDataFile(libname))
		return libname;
	if (Path::ComparePaths(ResPaths.DataDir, installDirectory) != 0) {
		// Hack for running in Debugger
		libname = cbuf_to_string_and_free(ci_find_file(installDirectory, filename));
		if (AssetManager::IsDataFile(libname))
			return libname;
	}
	return "";
}

// Looks up for known valid asset library and returns path, or empty string if failed
String get_known_assetlib(const String &filename) {
	// TODO: write now there's only 3 regular PAKs, so we may do this quick
	// string comparison, but if we support more maybe we could use a table.
	if (filename.CompareNoCase(ResPaths.GamePak.Name) == 0)
		return ResPaths.GamePak.Path;
	if (filename.CompareNoCase(ResPaths.AudioPak.Name) == 0)
		return ResPaths.AudioPak.Path;
	if (filename.CompareNoCase(ResPaths.SpeechPak.Name) == 0)
		return ResPaths.SpeechPak.Path;
	return String();
}

Stream *find_open_asset(const String &filename) {
	Stream *asset_s = Shared::AssetManager::OpenAsset(filename);
	if (!asset_s && Path::ComparePaths(ResPaths.DataDir, installDirectory) != 0) {
		// Just in case they're running in Debug, try standalone file in compiled folder
		asset_s = ci_fopen(String::FromFormat("%s/%s", installDirectory.GetCStr(), filename.GetCStr()));
	}
	return asset_s;
}

AssetPath get_audio_clip_assetpath(int bundling_type, const String &filename) {
	// Special case is explicitly defined audio directory, which should be
	// tried first regardless of bundling type.
	if (Path::ComparePaths(ResPaths.DataDir, installAudioDirectory) != 0) {
		String filepath = String::FromFormat("%s/%s", installAudioDirectory.GetCStr(), filename.GetCStr());
		if (Path::IsFile(filepath))
			return AssetPath("", filepath);
	}

	if (bundling_type == AUCL_BUNDLE_EXE)
		return AssetPath(ResPaths.GamePak.Name, filename);
	else if (bundling_type == AUCL_BUNDLE_VOX)
		return AssetPath(game.GetAudioVOXName(), filename);
	return AssetPath();
}

AssetPath get_voice_over_assetpath(const String &filename) {
	// Special case is explicitly defined voice-over directory, which should be
	// tried first.
	if (Path::ComparePaths(ResPaths.DataDir, installVoiceDirectory) != 0) {
		String filepath = String::FromFormat("%s/%s", installVoiceDirectory.GetCStr(), filename.GetCStr());
		if (Path::IsFile(filepath))
			return AssetPath("", filepath);
	}
	return AssetPath(ResPaths.SpeechPak.Name, filename);
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

extern ScriptString myScriptStringImpl;

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
	API_OBJCALL_OBJ(sc_File, const char, myScriptStringImpl, File_ReadRawLineBack);
}

// void (sc_File *fil, char *toread)
RuntimeScriptValue Sc_File_ReadString(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_POBJ(sc_File, File_ReadString, char);
}

// const char* (sc_File *fil)
RuntimeScriptValue Sc_File_ReadStringBack(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(sc_File, const char, myScriptStringImpl, File_ReadStringBack);
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

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("File::Delete^1", (void *)File_Delete);
	ccAddExternalFunctionForPlugin("File::Exists^1", (void *)File_Exists);
	ccAddExternalFunctionForPlugin("File::Open^2", (void *)sc_OpenFile);
	ccAddExternalFunctionForPlugin("File::Close^0", (void *)File_Close);
	ccAddExternalFunctionForPlugin("File::ReadInt^0", (void *)File_ReadInt);
	ccAddExternalFunctionForPlugin("File::ReadRawChar^0", (void *)File_ReadRawChar);
	ccAddExternalFunctionForPlugin("File::ReadRawInt^0", (void *)File_ReadRawInt);
	ccAddExternalFunctionForPlugin("File::ReadRawLine^1", (void *)File_ReadRawLine);
	ccAddExternalFunctionForPlugin("File::ReadRawLineBack^0", (void *)File_ReadRawLineBack);
	ccAddExternalFunctionForPlugin("File::ReadString^1", (void *)File_ReadString);
	ccAddExternalFunctionForPlugin("File::ReadStringBack^0", (void *)File_ReadStringBack);
	ccAddExternalFunctionForPlugin("File::WriteInt^1", (void *)File_WriteInt);
	ccAddExternalFunctionForPlugin("File::WriteRawChar^1", (void *)File_WriteRawChar);
	ccAddExternalFunctionForPlugin("File::WriteRawLine^1", (void *)File_WriteRawLine);
	ccAddExternalFunctionForPlugin("File::WriteString^1", (void *)File_WriteString);
	ccAddExternalFunctionForPlugin("File::get_EOF", (void *)File_GetEOF);
	ccAddExternalFunctionForPlugin("File::get_Error", (void *)File_GetError);
}

} // namespace AGS3
