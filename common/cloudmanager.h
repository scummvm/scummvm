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

#ifndef COMMON_CLOUDMANAGER_H
#define COMMON_CLOUDMANAGER_H

#include "backends/cloud/storage.h"

namespace Common {

class CloudManager {
public:
	CloudManager() {}
	virtual ~CloudManager() {}

	/**
	* Loads all information from configs and creates current Storage instance.
	*
	* @note It's called once on startup in scummvm_main().
	*/

	virtual void init() = 0;

	/**
	* Returns active Storage, which could be used to interact
	*  with cloud storage.
	*
	* @return	active Cloud::Storage or null, if there is no active Storage.
	*/

	virtual Cloud::Storage* getCurrentStorage() = 0;

	/**
	* Starts saves syncing process in currently active storage if there is any.
	*/

	virtual void syncSaves(Cloud::Storage::BoolCallback callback = 0) = 0;
};

} //end of namespace Common

#endif
