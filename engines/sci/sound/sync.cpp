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

#include "sci/engine/kernel.h"
#include "sci/util.h"
#include "sync.h"

namespace Sci {

Sync::Sync(ResourceManager *resMan, SegManager *segMan) :
	_resMan(resMan),
	_segMan(segMan),
	_resource(nullptr),
	_offset(0) {}

Sync::~Sync() {
	stop();
}

void Sync::start(const ResourceId id, const reg_t syncObjAddr) {
	_resource = _resMan->findResource(id, true);
	_offset = 0;

	if (_resource) {
		writeSelectorValue(_segMan, syncObjAddr, SELECTOR(syncCue), 0);
	} else {
		warning("Sync::start: failed to find resource %s", id.toString().c_str());
		// Notify the scripts to stop sound sync
		writeSelectorValue(_segMan, syncObjAddr, SELECTOR(syncCue), SIGNAL_OFFSET);
	}
}

void Sync::next(const reg_t syncObjAddr) {
	if (_resource && (_offset < _resource->size() - 1)) {
		int16 syncCue = -1;
		int16 syncTime = _resource->getInt16SEAt(_offset);

		_offset += 2;

		if ((syncTime != -1) && (_offset < _resource->size() - 1)) {
			syncCue = _resource->getInt16SEAt(_offset);
			_offset += 2;
		}

		writeSelectorValue(_segMan, syncObjAddr, SELECTOR(syncTime), syncTime);
		writeSelectorValue(_segMan, syncObjAddr, SELECTOR(syncCue), syncCue);
	}
}

void Sync::stop() {
	if (_resource) {
		_resMan->unlockResource(_resource);
		_resource = nullptr;
	}
}

} // End of namespace Sci
