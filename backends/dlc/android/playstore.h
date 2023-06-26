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

#ifndef BACKENDS_DLC_PLAYSTORE_PLAYSTORE_H
#define BACKENDS_DLC_PLAYSTORE_PLAYSTORE_H

#include "backends/dlc/store.h"
#include "common/callback.h"

namespace DLC {
namespace PlayStore {

class PlayStore: public DLC::Store {
	
public:	
	PlayStore() {}
	virtual ~PlayStore() {}
	
	virtual void init() override {}

	virtual void requestInfo() override {}

	virtual void getDownloadState() override {}

	virtual void requestDownload() override {}

	virtual void getBytesDownloaded() override {}

	virtual void cancelDownload() override {}
};

} // End of namespace PlayStore
} // End of namespace DLC

#endif
