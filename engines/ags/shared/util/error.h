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
// Universal error class, that may be used both as a return value or
// thrown as an exception.
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_ERROR_H
#define AGS_SHARED_UTIL_ERROR_H

#include "ags/lib/std/memory.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class Error;
typedef std::shared_ptr<Error> PError;

//
// A simple struct, that provides several fields to describe an error in the program.
// If wanted, may be reworked into subclass of std::exception.
//
class Error {
public:
	Error(int code, String general, PError inner_error = PError()) : _code(code), _general(general), _innerError(inner_error) {
	}
	Error(int code, String general, String comment, PError inner_error = PError()) : _code(code), _general(general), _comment(comment), _innerError(inner_error) {
	}
	Error(String general, PError inner_error = PError()) : _code(0), _general(general), _innerError(inner_error) {
	}
	Error(String general, String comment, PError inner_error = PError()) : _code(0), _general(general), _comment(comment), _innerError(inner_error) {
	}


	// Error code is a number, defining error subtype. It is not much use to the end-user,
	// but may be checked in the program to know more precise cause of the error.
	int    Code() const {
		return _code;
	}
	// General description of this error type and subtype.
	String General() const {
		return _general;
	}
	// Any complementary information.
	String Comment() const {
		return _comment;
	}
	PError InnerError() const {
		return _innerError;
	}
	// Full error message combines general description and comment.
	// NOTE: if made a child of std::exception, FullMessage may be substituted
	// or complemented with virtual const char* what().
	String FullMessage() const {
		String msg;
		const Error *err = this;
		do {
			msg.Append(err->General());
			if (!err->Comment().IsEmpty()) {
				msg.AppendChar('\n');
				msg.Append(err->Comment());
			}
			err = err->InnerError().get();
			if (err)
				msg.AppendChar('\n');
		} while (err);
		return msg;
	}

private:
	int    _code; // numeric code, for specific uses
	String _general; // general description of this error class
	String _comment; // additional information about particular case
	PError _innerError; // previous error that caused this one
};


// ErrorHandle is a helper class that lets you have an Error object
// wrapped in a smart pointer. ErrorHandle's only data member is a
// shared_ptr, which means that it does not cause too much data copying
// when used as a function's return value.
// Note, that the reason to have distinct class instead of a shared_ptr's
// typedef is an inverted boolean comparison:
// shared_ptr converts to 'true' when it contains an object, but ErrorHandle
// returns 'true' when it *does NOT* contain an object, meaning there
// is no error.
template <class T> class ErrorHandle {
public:
	static ErrorHandle<T> None() {
		return ErrorHandle();
	}

	ErrorHandle() {}
	ErrorHandle(T *err) : _error(err) {
	}
	ErrorHandle(std::shared_ptr<T> err) : _error(err) {
	}

	bool HasError() const {
		return _error.get() != NULL;
	}
	explicit operator bool() const {
		return _error.get() == nullptr;
	}
	operator PError() const {
		return _error;
	}
	T *operator ->() const {
		return _error.operator->();
	}
	T &operator *() const {
		return _error.operator * ();
	}

private:
	std::shared_ptr<T> _error;
};


// Basic error handle, containing Error object
typedef ErrorHandle<Error> HError;


// TypedCodeError is the Error's subclass, which only purpose is to override
// error code type in constructor and Code() getter, that may be useful if
// you'd like to restrict code values to particular enumerator.
// TODO: a type identifier as a part of template (string, or perhaps a int16
// to be placed at high-bytes in Code) to be able to distinguish error group.
template <typename CodeType, String(*GetErrorText)(CodeType)>
class TypedCodeError : public Error {
public:
	TypedCodeError(CodeType code, PError inner_error = PError()) : Error(code, GetErrorText(code), inner_error) {
	}
	TypedCodeError(CodeType code, String comment, PError inner_error = PError()) :
		Error(code, GetErrorText(code), comment, inner_error) {
	}

	CodeType Code() const {
		return (CodeType)Error::Code();
	}
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
