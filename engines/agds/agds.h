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

#ifndef AGDS_AGDS_H
#define AGDS_AGDS_H

#include "common/scummsys.h"
#include "common/hashmap.h"
#include "engines/advancedDetector.h"
#include "agds/resourceManager.h"
#include "agds/database.h"

/**
 * This is the namespace of the AGDS engine.
 *
 * Status of this engine: In Progress
 *
 * Games using this engine:
 * - Black Mirror (Windows)
 */
namespace AGDS {

class Object;
class Process;

class AGDSEngine : public Engine {
	friend class Process;

public:
	AGDSEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~AGDSEngine();

	Common::Error run();

private:
	bool load();
	void loadObject(Common::String & name);
	int appendToSharedStorage(const Common::String &value);

private:
	typedef Common::HashMap<Common::String, Object *> ObjectsType;
	typedef Common::List<Process> ProcessListType;

	const ADGameDescription *	_gameDescription;
	ResourceManager				_resourceManager;
	Database					_data, _patch; //data and patch databases
	ObjectsType					_objects;
	ProcessListType				_processes;
	Common::String				_nextScreen;
	int							_sharedStorageIndex;
	Common::String				_sharedStorage[10];
};


} // End of namespace AGDS

#endif /* AGDS_AGDS_H */
