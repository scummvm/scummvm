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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_ERROR_H
#define COMMON_ERROR_H

namespace Common {

/**
 * This file contains an enum with commonly used error codes.
 */



/**
 * Error codes which may be reported by plugins under various circumstances.
 *
 * @todo Clarify the names; add more codes, resp. verify all existing ones are acutally useful.
 *       Also, try to avoid overlap.
 * @todo Maybe introduce a naming convention? E.g. k-NOUN/ACTION-CONDITION-Error, so
 *       kPathInvalidError would be correct, but these would not be: kInvalidPath,
 *       kPathInvalid, kPathIsInvalid, kInvalidPathError
 */
enum Error {
	kNoError = 0,				///< No error occured
	kInvalidPathError,			///< Engine initialization: Invalid game path was passed
	kNoGameDataFoundError,		///< Engine initialization: No game data was found in the specified location
	kUnsupportedGameidError,	///< Engine initialization: Gameid not supported by this (Meta)Engine
	kUnsupportedColorMode,		///< Engine initialization: Engine does not support backend's color mode


	kReadPermissionDenied,		///< Unable to read data due to missing read permission
	kWritePermissionDenied,		///< Unable to write data due to missing write permission

	// The following three overlap a bit with kInvalidPathError and each other. Which to keep?
	kPathDoesNotExist,			///< The specified path does not exist
	kPathNotDirectory,			///< The specified path does not point to a directory
	kPathNotFile,				///< The specified path does not point to a file

	kCreatingFileFailed,		///< Failed creating a (savestate) file
	kReadingFailed,				///< Failed to read a file (permission denied?)
	kWritingFailed,				///< Failure to write data -- disk full?

	// The following are used by --list-saves
	kPluginNotFound,			///< Failed to find plugin to handle tager
	kPluginNotSupportSaves,		///< Failed if plugin does not support saves
	kNoSavesError,				///< There are no saves to show

	kArgumentNotProcessed,		///< Used in command line parsing
	kUnknownError				///< Catch-all error, used if no other error code matches
};

/**
 * Maps an error code to equivalent string description.
 *
 * @param error error code to be converted
 * @return a pointer to string description of the error
 */
const char *errorToString(Error error);

} // End of namespace Common

#endif //COMMON_ERROR_H
