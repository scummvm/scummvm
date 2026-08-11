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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_SYSCLASS_H
#define WINTERMUTE_SYSCLASS_H

#include "engines/wintermute/persistent.h"
#include "engines/wintermute/dctypes.h"
#include "common/hashmap.h"
#include "common/func.h"
#include "common/stream.h"

namespace Wintermute {
class SystemInstance;
class BaseGame;
class BasePersistenceManager;
class SystemClass;

}

namespace Common {
template<typename T> struct Hash;

template<> struct Hash<void *> : public UnaryFunction<void *, uint> {
	uint operator()(void *val) const {
		return (uint)((size_t)val);
	}
};

template<> struct Hash<Wintermute::SystemInstance *> : public UnaryFunction<Wintermute::SystemInstance *, uint> {
	uint operator()(Wintermute::SystemInstance *val) const {
		return (uint)((size_t)val);
	}
};


}

namespace Wintermute {

class SystemClass {
public:
	SystemClass(const AnsiString &name, PERSISTBUILD build, PERSISTLOAD load, bool persistentClass);
	~SystemClass();

	int getNumInstances();
	bool removeInstance(void *instance);
	SystemInstance *addInstance(void *instance, int id, int savedId = -1);
	bool removeAllInstances();

	int getInstanceId(void *pointer);
	void *idToPointer(int savedId);

	void setId(int id) {
		_id = id;
	}
	int getId() const {
		return _id;
	}

	int getSavedId() const {
		return _savedId;
	}

	bool isPersistent() const {
		return _persistent;
	}

	AnsiString getName() const {
		return _name;
	}

	void saveTable(BaseGame *game, BasePersistenceManager *persistMgr);
	void loadTable(BaseGame *game, BasePersistenceManager *persistMgr);

	void saveInstances(BaseGame *game, BasePersistenceManager *persistMgr);
	void loadInstance(void *instance, BasePersistenceManager *persistMgr);

	void instanceCallback(SYS_INSTANCE_CALLBACK lpCallback, void *lpData);

	void resetSavedIDs();

	void dump(Common::WriteStream *stream);

private:
	int _numInst;
	bool _persistent;
	SystemClass *_next;
	int _id{};
	int _savedId;
	AnsiString _name;
	PERSISTBUILD _build;
	PERSISTLOAD _load;

	//typedef std::set<SystemInstance *> Instances;
	typedef Common::HashMap<SystemInstance *, SystemInstance *> Instances;
	Instances _instances;

	typedef Common::HashMap<void *, SystemInstance *> InstanceMap;
	InstanceMap _instanceMap;
};

} // End of namespace Wintermute

#endif
