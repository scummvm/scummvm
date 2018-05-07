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

#ifndef ERROR_H_
#define ERROR_H_

#include "common/str.h"

namespace Wintermute {

enum ErrorLevel {
	SUCCESS,
	NOTICE,
	WARNING,
	ERROR
};

enum ErrorCode {
	OK,
	NO_SUCH_SOURCE,
	COULD_NOT_OPEN,
	NO_SUCH_LINE,
	NOT_ALLOWED,
	NO_SUCH_BYTECODE,
	DUPLICATE_BREAKPOINT,
	NO_SUCH_BREAKPOINT,
	WRONG_TYPE,
	PARSE_ERROR,
	NOT_YET_IMPLEMENTED,
	SOURCE_PATH_NOT_SET,
	ILLEGAL_PATH,
	UNKNOWN_ERROR
};


class Error {
	const ErrorLevel _errorLevel;
	const ErrorCode _errorCode;
	const int _errorExtraInt;
	const Common::String _errorExtraString;
public:
	Error(ErrorLevel, ErrorCode);
	Error(ErrorLevel, ErrorCode, int errorExtraInt);
	Error(ErrorLevel, ErrorCode, Common::String errorExtraString);
	Error(ErrorLevel, ErrorCode, Common::String errorExtraString, int errorExtraInt);
	ErrorLevel getErrorLevel() const;
	ErrorCode getErrorCode() const;
	Common::String getErrorLevelStr() const;
	Common::String getErrorDisplayStr() const;
};

} // End of namespace Wintermute

#endif /* ERROR_H_ */
