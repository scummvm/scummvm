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

#include "common/error.h"
#include "common/util.h"

namespace Common {

/**
 * Error Table: Maps error codes to their default descriptions
 */

struct ErrorMessage {
	Error error;
	const char *errMsg;
};

static const ErrorMessage _errMsgTable[] = {
	{ kInvalidPathError, "Invalid Path" },
	{ kNoGameDataFoundError, "Game Data not found" },
	{ kUnsupportedGameidError, "Game Id not supported" },
	{ kUnsupportedColorMode, "Unsupported Color Mode" },

	{ kReadPermissionDenied, "Read permission denied" },
	{ kWritePermissionDenied, "Write permission denied" },

	// The following three overlap a bit with kInvalidPathError and each other. Which to keep?
	{ kPathDoesNotExist, "Path not exists" },
	{ kPathNotDirectory, "Path not a directory" },
	{ kPathNotFile, "Path not a file" },

	{ kCreatingFileFailed, "Cannot create file" },
	{ kReadingFailed, "Reading failed" },
	{ kWritingFailed, "Writing data failed" },

	{ kUnknownError, "Unknown Error" }
};

const char *errorToString(Error error) {

	for (int i = 0; i < ARRAYSIZE(_errMsgTable); i++) {
		if (error == _errMsgTable[i].error) {
			return _errMsgTable[i].errMsg;
		}
	}

	return "Unknown Error";
}

} // End of namespace Common
