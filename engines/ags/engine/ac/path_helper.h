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
// Functions related to constructing game and script paths.
//
// TODO: We need some kind of a "file manager" which deals with opening files
// in defined set of directories. To ensure that rest of the engine code does
// not work with explicit paths or creates directories on its own.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_PATH_HELPER_H
#define AGS_ENGINE_AC_PATH_HELPER_H

#include "ags/shared/util/path.h"

namespace AGS3 {

using AGS::Shared::String;

// Filepath tokens, which are replaced by platform-specific directory names
extern const char *UserSavedgamesRootToken;
extern const char *GameSavedgamesDirToken;
extern const char *GameDataDirToken;
extern const char *DefaultConfigFileName;

// Subsitutes illegal characters with '_'. This function uses illegal chars array
// specific to current platform.
void FixupFilename(char *filename);

// FSLocation describes a file system location defined by two parts:
// a secure path that engine does not own, and sub-path that it owns.
// The meaning of this is that engine is only allowed to create
// sub-path subdirectories, and only if secure path exists.
struct FSLocation {
	String BaseDir; // base directory, which we assume already exists; not our responsibility
	String SubDir;  // sub-directory, relative to BaseDir
	String FullDir; // full path to location
	FSLocation() {}
	FSLocation(const String &base) : BaseDir(base), FullDir(base) {
	}
	FSLocation(const String &base, const String &subdir)
		: BaseDir(base), SubDir(subdir),
		FullDir(AGS::Shared::Path::ConcatPaths(base, subdir)) {
	}
	inline bool IsValid() const {
		return !FullDir.IsEmpty();
	}
	// Concats the given path to the existing full dir
	inline FSLocation Concat(const String &path) const {
		return FSLocation(BaseDir, AGS::Shared::Path::ConcatPaths(FullDir, path));
	}
	// Sets full path as a relative to the existing base dir
	inline FSLocation Rebase(const String &path) const {
		return FSLocation(BaseDir, AGS::Shared::Path::ConcatPaths(BaseDir, path));
	}
};
// Tests the input path, if it's an absolute path then returns it unchanged;
// if it's a relative path then resolves it into absolute, using install dir as a base.
String PathFromInstallDir(const String &path);
FSLocation PathFromInstallDir(const FSLocation &fsloc);
// Makes sure that given system location is available, makes directories if have to (and if it's allowed to)
// Returns full file path on success, empty string on failure.
String PreparePathForWriting(const FSLocation &fsloc, const String &filename);

// Following functions calculate paths to directories according to game setup
// Returns the directory where global user config is to be found
FSLocation GetGlobalUserConfigDir();
// Returns the directory where this game's user config is to be found
FSLocation GetGameUserConfigDir();
// Returns the directory where this game's shared app files are to be found
FSLocation GetGameAppDataDir();
// Returns the directory where this game's saves and user data are to be found
FSLocation GetGameUserDataDir();

// ResolvedPath describes an actual location pointed by a user path (e.g. from script)
struct ResolvedPath {
	FSLocation Loc;  // location (directory)
	String FullPath; // full path, including filename
	String AltPath;  // alternative read-only full path, for backwards compatibility
	bool AssetMgr = false; // file is to be accessed through the asset manager
	ResolvedPath() = default;
	ResolvedPath(const String & file, const String & alt = "")
		: FullPath(file), AltPath(alt) {
	}
	ResolvedPath(const FSLocation & loc, const String & file, const String & alt = "")
		: Loc(loc), FullPath(AGS::Shared::Path::ConcatPaths(loc.FullDir, file)), AltPath(alt) {
	}
};
// Resolves a file path provided by user (e.g. script) into actual file path,
// by substituting special keywords with actual platform-specific directory names.
// Fills in ResolvedPath object on success.
// Returns 'true' on success, and 'false' if either path is impossible to resolve
// or if the file path is forbidden to be accessed in current situation.
bool ResolveScriptPath(const String &sc_path, bool read_only, ResolvedPath &rp);
// Resolves a user file path for writing, and makes sure all the sub-directories are
// created along the actual path.
// Returns 'true' on success, and 'false' if either path is impossible to resolve,
// forbidden for writing, or if failed to create any subdirectories.
bool ResolveWritePathAndCreateDirs(const String &sc_path, ResolvedPath &rp);
// Creates all necessary subdirectories inside the safe parent location.
bool CreateFSDirs(const FSLocation &fs);

} // namespace AGS3

#endif
