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

#ifndef BACKENDS_NETWORKING_CURL_REQUEST_H
#define BACKENDS_NETWORKING_CURL_REQUEST_H

#include "common/callback.h"

namespace Networking {

class Request {
protected:
	/**
	* Callback, which should be called before Request returns true in handle().
	* That's the way Requests pass the result to the code which asked to create this request.
	*/

	Common::BaseCallback<> *_callback;

public:
	Request(Common::BaseCallback<> *cb): _callback(cb) {};
	virtual ~Request() {};

	/**
	* Method, which does actual work. Depends on what this Request is doing.
	*
	* @return true if request's work is complete and it may be removed from Storage's list
	*/

	virtual bool handle() = 0;
};

} //end of namespace Cloud

#endif
