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
#include "common/scummsys.h"

namespace Networking {

template<typename T> struct RequestIdPair {
	int32 id;
	T value;

	RequestIdPair(int32 rid, T v) : id(rid), value(v) {}
};

typedef RequestIdPair<void *> RequestDataPair;
typedef Common::BaseCallback<RequestDataPair> *DataCallback;

class Request {
protected:
	/**
	* Callback, which should be called before Request returns true in handle().
	* That's the way Requests pass the result to the code which asked to create this request.
	*/

	DataCallback _callback;

	int32 _id;

public:
	Request(DataCallback cb): _callback(cb), _id(-1) {}
	virtual ~Request() { delete _callback; }

	/**
	* Method, which does actual work. Depends on what this Request is doing.
	*/

	virtual void handle() = 0;

	virtual void restart() = 0;

	void setId(int32 id) { _id = id; }
};

} //end of namespace Cloud

#endif
