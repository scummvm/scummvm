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
 * Handles timers.
 */

#ifndef TINSEL_SERIALIZER_H
#define TINSEL_SERIALIZER_H

#include "common/scummsys.h"
#include "common/savefile.h"


namespace Tinsel {


#define SYNC_AS(SUFFIX,TYPE,SIZE) \
	template <class T> \
	void syncAs ## SUFFIX(T &val) { \
		if (_loadStream) \
			val = static_cast<T>(_loadStream->read ## SUFFIX()); \
		else { \
			TYPE tmp = val; \
			_saveStream->write ## SUFFIX(tmp); \
		} \
		_bytesSynced += SIZE; \
	}
	

// TODO: Write comment for this
// TODO: Inspired by the SCUMM engine -- move to common/ code and use in more engines?
class Serializer {
public:
	Serializer(Common::SeekableReadStream *in, Common::OutSaveFile *out)
		: _loadStream(in), _saveStream(out), _bytesSynced(0) {
		assert(in || out);
	}

	bool isSaving() { return (_saveStream != 0); }
	bool isLoading() { return (_loadStream != 0); }
	
	uint bytesSynced() const { return _bytesSynced; }
	
	void syncBytes(byte *buf, uint16 size) {
		if (_loadStream)
			_loadStream->read(buf, size);
		else
			_saveStream->write(buf, size);
		_bytesSynced += size;
	}

	SYNC_AS(Byte, byte, 1)

	SYNC_AS(Uint16LE, uint16, 2)
	SYNC_AS(Uint16BE, uint16, 2)
	SYNC_AS(Sint16LE, int16, 2)
	SYNC_AS(Sint16BE, int16, 2)

	SYNC_AS(Uint32LE, uint32, 4)
	SYNC_AS(Uint32BE, uint32, 4)
	SYNC_AS(Sint32LE, int32, 4)
	SYNC_AS(Sint32BE, int32, 4)

protected:
	Common::SeekableReadStream *_loadStream;
	Common::OutSaveFile *_saveStream;

	uint _bytesSynced;
};

#undef SYNC_AS

// TODO: Make a subclass "VersionedSerializer", which makes it easy to support 
//       multiple versions of a savegame format (again inspired by SCUMM).
/*
class VersionedSerializer : public Serializer {
public:
	// "version" is the version of the savegame we are loading/creating
	VersionedSerializer(Common::SeekableReadStream *in, Common::OutSaveFile *out, int version)
		: Serializer(in, out), _version(version) {
		assert(in || out);
	}

	void syncBytes(byte *buf, uint16 size, int minVersion = 0, int maxVersion = INF) {
		if (_version < minVersion || _version > maxVersion)
			return;	// Do nothing if too old or too new
		if (_loadStream) {
			_loadStream->read(buf, size);
		} else {
			_saveStream->write(buf, size);
		}
	}
	...

};

*/

// Mixin class / interface
// TODO Maybe call it ISerializable or SerializableMixin ?
// TODO: Taken from SCUMM engine -- move to common/ code?
class Serializable {
public:
	virtual ~Serializable() {}
	virtual void saveLoadWithSerializer(Serializer *ser) = 0;
};


} // end of namespace Tinsel

#endif
