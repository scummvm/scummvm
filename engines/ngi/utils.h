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

#ifndef NGI_UTILS_H
#define NGI_UTILS_H

#include "common/hash-ptr.h"
#include "common/hash-str.h"
#include "common/array.h"
#include "common/file.h"

namespace NGI {

class CObject;
class NGIArchive;

typedef Common::HashMap<void *, int> ObjHash;

typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ClassMap;

class MfcArchive : public Common::SeekableReadStream, public Common::WriteStream {
	ClassMap _classMap;
	Common::Array<CObject *> _objectMap;
	Common::Array<int> _objectIdMap;
	ObjHash _objectHash;

	int _lastIndex;
	int _level;

	Common::SeekableReadStream *_stream;
	Common::WriteStream *_wstream;

public:
	MfcArchive(Common::SeekableReadStream *file);
	MfcArchive(Common::WriteStream *file);

	Common::String readPascalString(bool twoByte = false);
	void writePascalString(const Common::String &str, bool twoByte = false);
	int readCount();
	CObject *parseClass(bool *isCopyReturned);

	/** ownership of returned object is passed to caller */
	template <typename T>
	T *readClass() {
		CObject *obj = readBaseClass();
		if (!obj)
			return nullptr;

		T *res = dynamic_cast<T *>(obj);
		assert(res);
		return res;
	}

	void writeObject(CObject *obj);

	void incLevel() { _level++; }
	void decLevel() { _level--; }
	int getLevel() { return _level; }

	bool eos() const override { return _stream->eos(); }
	uint32 read(void *dataPtr, uint32 dataSize) override { return _stream->read(dataPtr, dataSize); }
	int32 pos() const override { return _stream ? _stream->pos() : _wstream->pos(); }
	int32 size() const override { return _stream->size(); }
	bool seek(int32 offset, int whence = SEEK_SET) override { return _stream->seek(offset, whence); }

	uint32 write(const void *dataPtr, uint32 dataSize) override { return _wstream->write(dataPtr, dataSize); }

private:
	void init();
	CObject *readBaseClass();
};

enum ObjType {
	kObjTypeDefault,
	kObjTypeExCommand,
	kObjTypeExCommand2,
	kObjTypeModalSaveGame,
	kObjTypeMovGraph,
	kObjTypeMovGraphLink,
	kObjTypeMovGraphNode,
	kObjTypeMctlCompound,
	kObjTypeObjstateCommand,
	kObjTypePictureObject,
	kObjTypeStaticANIObject,
	kObjTypeGameVar
};

class CObject {
public:
	ObjType _objtype;
	uint _cnum;

	CObject() : _objtype(kObjTypeDefault), _cnum(0) {}
	virtual bool load(MfcArchive &in) { return true; }
	virtual void save(MfcArchive &out) { error("Not implemented for obj type: %d", _objtype); }
	virtual ~CObject() {}

	bool loadFile(const Common::String &fname);
};

template <class T>
class ObList : public Common::List<T *>, public CObject {
public:
	bool load(MfcArchive &file) override {
		debugC(5, kDebugLoading, "ObList::load()");
		int count = file.readCount();

		debugC(9, kDebugLoading, "ObList::count: %d:", count);

		for (int i = 0; i < count; i++) {
			debugC(9, kDebugLoading, "ObList::[%d]", i);
			T *t = file.readClass<T>();

			this->push_back(t);
		}

		return true;
	}
};

class MemoryObject : CObject {
	friend class Picture;
	friend class Scene;

 protected:
	Common::String _memfilename;
	int _mfield_8;
	int _mfield_C;
	int _mfield_10;
	char _mfield_14;
	byte *_data;
	int _dataSize;
	int _mflags;
	NGIArchive *_libHandle;

 public:
	MemoryObject();
	~MemoryObject() override;

	bool load(MfcArchive &file) override;
	void loadFile(const Common::String &filename);
	void load() { loadFile(_memfilename); }
	byte *getData();
	byte *loadData();
	int getDataSize() const { return _dataSize; }

	bool testFlags();

	void freeData();
};

class MemoryObject2 : public MemoryObject {
	friend class Picture;

 protected:
	byte **_rows;

 public:
	MemoryObject2();
	~MemoryObject2() override;
	bool load(MfcArchive &file) override;

	void copyData(byte *src, int dataSize);
};

class ObArray : public Common::Array<CObject>, public CObject {
 public:
	bool load(MfcArchive &file) override;
};

class DWordArray : public Common::Array<int32>, public CObject {
 public:
	bool load(MfcArchive &file) override;
};

Common::String genFileName(int superId, int sceneId, const char *ext);
byte *transCyrillic(const Common::String &str);

} // End of namespace NGI

#endif /* NGI_UTILS_H */
