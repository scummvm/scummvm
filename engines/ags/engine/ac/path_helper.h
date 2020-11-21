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

//=============================================================================
//
// Functions related to constructing game and script paths.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_PATHHELPER_H
#define AGS_ENGINE_AC_PATHHELPER_H

#include "util/string.h"

using AGS::Common::String;

// Filepath tokens, which are replaced by platform-specific directory names
extern const String UserSavedgamesRootToken;
extern const String GameSavedgamesDirToken;
extern const String GameDataDirToken;

inline const char *PathOrCurDir(const char *path)
{
    return path ? path : ".";
}

// Subsitutes illegal characters with '_'. This function uses illegal chars array
// specific to current platform.
void FixupFilename(char *filename);
// Checks if there is a slash after special token in the beginning of the
// file path, and adds one if it is missing. If no token is found, string is
// returned unchanged.
String FixSlashAfterToken(const String &path);
// Creates a directory path by combining absolute path to special directory with
// custom game's directory name.
// If the path is relative, keeps it unmodified (no extra subdir added).
String MakeSpecialSubDir(const String &sp_dir);

// ResolvedPath describes an actual location pointed by a user path (e.g. from script)
struct ResolvedPath
{
    String BaseDir; // base directory, one of the special path roots
    String FullPath;// full path
    String AltPath; // alternative full path, for backwards compatibility
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

// Sets an optional path to treat like game's installation directory
void    set_install_dir(const String &path, const String &audio_path, const String &voice_path);
// Returns a path to game installation directory (optionally a custom path could be set);
// does not include trailing '/'
String  get_install_dir();
String  get_audio_install_dir();
String  get_voice_install_dir();
void    get_install_dir_path(char* buffer, const char *fileName);

#endif
