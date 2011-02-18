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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/saveload.h $
 * $Id: saveload.h 209 2011-02-06 00:46:36Z dreammaster $
 *
 */

#ifndef TSAGE_SAVELOAD_H
#define TSAGE_SAVELOAD_H

#include "common/scummsys.h"
#include "common/list.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/serializer.h"

namespace tSage {

typedef void (*SaveNotifierFn)(bool postFlag);

#define TSAGE_SAVEGAME_VERSION 1

class SavedObject;

struct tSageSavegameHeader {
	uint8 version;
	Common::String saveName;
	Graphics::Surface *thumbnail;
	int saveYear, saveMonth, saveDay;
	int saveHour, saveMinutes;
	int totalFrames;
};

/*--------------------------------------------------------------------------*/

#define SYNC_POINTER(x) s.syncPointer((SavedObject **)&x)
#define SYNC_ENUM(FIELD, TYPE) int v_##FIELD = (int)FIELD; s.syncAsUint16LE(v_##FIELD); \
	if (s.isLoading()) FIELD = (TYPE)v_##FIELD;

/**
 * Derived serialiser class with extra synchronisation types
 */
class Serialiser: public Common::Serializer {
public:
	Serialiser(Common::SeekableReadStream *in, Common::WriteStream *out): Common::Serializer(in, out) {}

	void syncPointer(SavedObject **ptr, Common::Serializer::Version minVersion = 0, 
		Common::Serializer::Version maxVersion = kLastVersion);
	void validate(const Common::String &s, Common::Serializer::Version minVersion = 0, 
		Common::Serializer::Version maxVersion = kLastVersion);
	void validate(int v, Common::Serializer::Version minVersion = 0, 
		Common::Serializer::Version maxVersion = kLastVersion);
};

/*--------------------------------------------------------------------------*/

class Serialisable {
public:
	virtual ~Serialisable() {}
	virtual void synchronise(Serialiser &s) = 0;
};

class SaveListener {
public:
	virtual ~SaveListener() {}
	virtual void listenerSynchronise(Serialiser &s) = 0;
};

/*--------------------------------------------------------------------------*/

class SavedObject: public Serialisable {
public:
	SavedObject();
	virtual ~SavedObject();

	virtual Common::String getClassName() { return "SavedObject"; }
	virtual void synchronise(Serialiser &s) {}

	static SavedObject *createInstance(const Common::String &className);
};

/*--------------------------------------------------------------------------*/

/**
 * Derived list class with extra functionality
 */
template<typename T>
class List: public Common::List<T> {
public:
	bool contains(T v) {
		for (typename List<T>::iterator i = this->begin(); i != this->end(); ++i)
			if (*i == v)
				return true;
		return false;
	}

	typedef void (*ForEachFn)(T fn);
	void forEach(ForEachFn Fn) {
		for (typename List<T>::iterator i = this->begin(); i != this->end(); ++i)
			Fn(*i);
	}

	void synchronise(Serialiser &s) {
		int entryCount;

		if (s.isLoading()) {
			List<T>::clear();
			s.syncAsUint32LE(entryCount);

			for (int idx = 0; idx < entryCount; ++idx) {
				List<T>::push_back(static_cast<T>((T)NULL));
				T &obj = List<T>::back();
				s.syncPointer((SavedObject **)&obj);
			}
		} else {
			// Get the list size
			entryCount = 0;
			typename List<T>::iterator i;
			for (i = List<T>::begin(); i != List<T>::end(); ++i, ++entryCount)
				;

			// Write out list 
			s.syncAsUint32LE(entryCount);
			for (i = List<T>::begin(); i != List<T>::end(); ++i) {
				s.syncPointer((SavedObject **)&*i);
			}
		}		
	}
};

/**
 * Derived list class for holding function pointers
 */
template<typename T>
class FunctionList: public List<void (*)(T)> {
public:
	void notify(T v) {
		for (typename List<void (*)(T)>::iterator i = this->begin(); i != this->end(); ++i) {
			(*i)(v);
		}
	}
};

/*--------------------------------------------------------------------------*/

class SavedObjectRef {
public:
	SavedObject **_savedObject;
	int _objIndex;

	SavedObjectRef(): _savedObject(NULL), _objIndex(-1) {}
	SavedObjectRef(SavedObject **so, int objIndex): _savedObject(so),  _objIndex(objIndex) {}
};

typedef SavedObject *(*SavedObjectFactory)(const Common::String &className);

class Saver {
private:
	List<SavedObject *> _objList;
	FunctionList<bool> _saveNotifiers;
	FunctionList<bool> _loadNotifiers;
	List<SaveListener *> _listeners;

	Common::List<SavedObjectRef> _unresolvedPtrs;
	SavedObjectFactory _factoryPtr;

	bool _macroSaveFlag;
	bool _macroRestoreFlag;
	int _saveSlot;

	void resolveLoadPointers();
public:
	Saver();
	~Saver();

	Common::Error save(int slot, const Common::String &saveName);
	Common::Error restore(int slot);
	static bool readSavegameHeader(Common::InSaveFile *in, tSageSavegameHeader &header);
	static void writeSavegameHeader(Common::OutSaveFile *out, tSageSavegameHeader &header);

	void addListener(SaveListener *obj);
	void addSaveNotifier(SaveNotifierFn fn);
	void addLoadNotifier(SaveNotifierFn fn);
	void addObject(SavedObject *obj);
	void removeObject(SavedObject *obj);
	void addFactory(SavedObjectFactory fn) { _factoryPtr = fn; }
	void addSavedObjectPtr(SavedObject **ptr, int objIndex) { 
		_unresolvedPtrs.push_back(SavedObjectRef(ptr, objIndex));
	}

	bool savegamesExist() const;
	bool getMacroSaveFlag() const { return _macroSaveFlag; }
	bool getMacroRestoreFlag() const { return _macroRestoreFlag; }
	int blockIndexOf(SavedObject *p);
};

extern Saver *_saver;

} // End of namespace tSage

#endif
