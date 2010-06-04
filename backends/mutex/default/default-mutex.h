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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef BACKENDS_MUTEX_DEFAULT_H
#define BACKENDS_MUTEX_DEFAULT_H

#include "common/system.h"
#include "common/noncopyable.h"

class DefaultMutexManager : Common::NonCopyable {
public:
	DefaultMutexManager() {}
	~DefaultMutexManager() {}

	bool hasMutexFeature(OSystem::Feature f);
	void setMutexFeatureState(OSystem::Feature f, bool enable) {}
	bool getMutexFeatureState(OSystem::Feature f);

	OSystem::MutexRef createMutex();
	void lockMutex(OSystem::MutexRef mutex) {}
	void unlockMutex(OSystem::MutexRef mutex) {}
	void deleteMutex(OSystem::MutexRef mutex);
};


#endif
