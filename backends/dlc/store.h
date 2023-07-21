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

#ifndef DLC_STORE_H
#define DLC_STORE_H

#include "common/str.h"
#include "common/array.h"
#include "common/queue.h"
#include "common/path.h"
#include "backends/dlc/dlcdesc.h"

namespace DLC {

class Store {
	
public:
	Store() {}
	virtual ~Store() {}

	virtual void init() = 0;

	virtual void requestInfo() = 0;

	virtual void getDownloadState() = 0;

	virtual void requestDownload() = 0;

	virtual void getBytesDownloaded() = 0;

	virtual void cancelDownload() = 0;

	virtual void getAllDLCs(Common::Array<DLCDesc*> &dlcs) = 0;

	virtual void startDownloadAsync(const Common::String &id, const Common::String &url) = 0;

	virtual void removeCacheFile(Common::Path file) = 0;
};

} // End of namespace DLC


#endif
