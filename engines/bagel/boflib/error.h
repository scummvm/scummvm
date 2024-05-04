
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

#ifndef BAGEL_BOFLIB_ERROR_H
#define BAGEL_BOFLIB_ERROR_H

#include "common/str.h"

namespace Bagel {

/**
 * Error reporting codes
 */
enum ErrorCode {
	ERR_NONE    =  0,   /* no error */
	ERR_MEMORY  =  1,   /* not enough memory */
	ERR_FOPEN   =  2,   /* error opening a file */
	ERR_FCLOSE  =  3,   /* error closing a file */
	ERR_FREAD   =  4,   /* error reading a file */
	ERR_FWRITE  =  5,   /* error writing a file */
	ERR_FSEEK   =  6,   /* error seeking a file */
	ERR_FDEL    =  7,   /* error deleting a file */
	ERR_FFIND   =  8,   /* could not find file */
	ERR_FTYPE   =  9,   /* invalid file type */
	ERR_PATH    = 10,   /* invalid path or filename */
	ERR_DISK    = 11,   /* unrecoverable disk error */
	ERR_UNKNOWN = 12,   /* unknown error */
	ERR_CRC     = 13,   /* file or data failed CRC check */
	ERR_SPACE   = 14,   /* not enough disk space */

	ERR_FUTURE3 = 15,   /* future use */
	ERR_FUTURE4 = 16,   /*            */
	ERR_FUTURE5 = 17    /* future use */
};

#define NUM_ERR_CODES 18
#define MAX_ERRORS 3

extern const char *const g_errList[];

class CBofError {
protected:
	static ErrorCode _errGlobal;
	static int _nErrorCount;

	ErrorCode _errCode;

protected:
	virtual void bofMessageBox(const Common::String &content, const Common::String &title) {}

public:
	CBofError();
	virtual ~CBofError() {}

	/**
	 * Logs specified error to log file.
	 * @remarks     Sets _errCode to specified error condition, and logs the
	 *                   error.
	 * @param errCode       Error to report
	 * @param format        printf style format string
	 */
	void reportError(ErrorCode errCode, const char *format = nullptr, ...);

	bool errorOccurred() {
		return _errCode != ERR_NONE;
	}
	ErrorCode getErrorCode() {
		return _errCode;
	}
	void clearError() {
		_errCode = ERR_NONE;
	}

	static void initialize();
	static ErrorCode getLastError() {
		return _errGlobal;
	}
	static void setLastError(ErrorCode errCode) {
		_errGlobal = errCode;
	}

	static int getErrorCount() {
		return _nErrorCount;
	}
	static void setErrorCount(int nCount) {
		_nErrorCount = nCount;
	}
	static void clearErrorCount() {
		setErrorCount(0);
	}

};

} // namespace Bagel

#endif
