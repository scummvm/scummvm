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

#ifndef BACKENDS_CLOUD_STORAGE_H
#define BACKENDS_CLOUD_STORAGE_H

#include "common/str.h"
#include "common/array.h"
#include "backends/cloud/request.h"

namespace Cloud {

class Storage {
	friend void cloudThread(void *); //calls handler()
	bool _timerStarted;

protected:
	Common::Array<Request *> _requests;

	virtual void addRequest(Request *request); //starts the timer if it's not started
	virtual void handler();	
	virtual void startTimer(int interval = 1000000); //1 second is the default interval
	virtual void stopTimer();

public:
	Storage();
	virtual ~Storage() {};

	/**
	* Lists given directory.
	*
	* @param path		directory to list	
	*/

	//TODO: actually make it list directories
	//TODO: add some callback to pass gathered files list

	virtual void listDirectory(Common::String path) = 0;

	/**
	* Starts saves syncing process.
	*/

	virtual void syncSaves() = 0;
};

} //end of namespace Cloud

#endif
