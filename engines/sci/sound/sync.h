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

#ifndef SCI_SOUND_SYNC_H
#define SCI_SOUND_SYNC_H

#include "sci/engine/selector.h"
#include "sci/engine/vm_types.h"

namespace Sci {

enum AudioSyncCommands {
	kSciAudioSyncStart = 0,
	kSciAudioSyncNext = 1,
	kSciAudioSyncStop = 2
};

class Resource;
class ResourceManager;
class SegManager;

/**
 * Sync class, kDoSync and relevant functions for SCI games.
 * Provides AV synchronization for animations.
 */
class Sync {
	SegManager *_segMan;
	ResourceManager *_resMan;
	Resource *_resource;
	uint _offset;

public:
	Sync(ResourceManager *resMan, SegManager *segMan);
	~Sync();

	void start(const ResourceId id, const reg_t syncObjAddr);
	void next(const reg_t syncObjAddr);
	void stop();
};

} // End of namespace Sci
#endif
