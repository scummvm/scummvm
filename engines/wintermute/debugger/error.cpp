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

#include "error.h"
#include "engines/wintermute/debugger.h"

namespace Wintermute {

Error::Error(ErrorLevel errorLevel,
		ErrorCode errorCode,
		Common::String errorExtraString,
		int errorExtraInt) :
			_errorLevel(errorLevel),
			_errorCode(errorCode),
			_errorExtraInt(errorExtraInt),
			_errorExtraString(errorExtraString){}

Error::Error(ErrorLevel errorLevel,
		ErrorCode errorCode,
		int errorExtraInt) :
			_errorLevel(errorLevel),
			_errorCode(errorCode),
			_errorExtraInt(errorExtraInt),
			_errorExtraString(""){}

Error::Error(ErrorLevel errorLevel,
		ErrorCode errorCode) :
			_errorLevel(errorLevel),
			_errorCode(errorCode),
			_errorExtraInt(0),
			_errorExtraString(""){}

Error::Error(ErrorLevel errorLevel,
		ErrorCode errorCode,
		Common::String errorExtraString) :
			_errorLevel(errorLevel),
			_errorCode(errorCode),
			_errorExtraInt(0),
			_errorExtraString(errorExtraString){}

ErrorLevel Error::getErrorLevel() const {
	return _errorLevel;
}

ErrorCode Error::getErrorCode() const {
	return _errorCode;
}

Common::String Error::getErrorLevelStr() const {
	switch (this->_errorLevel) {
	case SUCCESS:
		return "SUCCESS";
		break;
	case NOTICE:
		return "NOTICE";
		break;
	case WARNING:
		return "WARNING";
		break;
	case ERROR:
		return "ERROR";
		break;
	default:
		break;
	}
	return "SUCCESS";
}

Common::String Error::getErrorDisplayStr() const {

	Common::String errorStr;

	switch (this->_errorLevel) {
	case SUCCESS:
		errorStr += "OK!";
		break;
	case WARNING:
		errorStr += "WARNING: ";
		break;
	case ERROR:
		errorStr += "ERROR: ";
		break;
	case NOTICE:
		errorStr += "NOTICE: ";
		break;
	default:
		// Um...
		break;
	}

	switch (this->_errorCode) {
	case OK:
		break;
	case NOT_ALLOWED:
		errorStr += "Could not execute requested operation. This is allowed only after a break.";
		break;
	case NO_SUCH_SOURCE:
		errorStr += Common::String::format("Can't find source for %s. Double check you source path.", this->_errorExtraString.c_str());
		break;
	case NO_SUCH_BYTECODE:
		errorStr += Common::String::format("No such script: %s. Can't find bytecode; double check the script path.", this->_errorExtraString.c_str());
		break;
	case SOURCE_PATH_NOT_SET:
		errorStr += Common::String("Source path not set. Source won't be displayed. Try 'help " + Common::String(SET_PATH_CMD) + "'.");
		break;
	case NO_SUCH_BREAKPOINT:
		errorStr += Common::String::format("No such breakpoint %d.", this->_errorExtraInt);
		break;
	case WRONG_TYPE:
		errorStr += Common::String::format("Incompatible type: %s.", this->_errorExtraString.c_str());
		break;
	default:
		errorStr += Common::String::format("Unknown condition %d", this->_errorCode);
		break;
	}

	return errorStr;
}

} // End namespace Wintermute
