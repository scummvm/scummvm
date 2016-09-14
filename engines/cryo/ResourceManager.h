#pragma once

#include "common/array.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/debug.h"

namespace Cryo {

template<typename T>
class CryoArray {
private:
	byte *_data;
	bool _ownData;
	uint16 ElementOffset(int num) {
		assert(_data && num < Count())
		return (static_cast<uint16 *>_data)[num];
	}
public:
	CryoArray(void *data, bool ownData) : _data(data), _ownData(ownData) {
	}
	~CryoArray() {
		if (_ownData)
			delete data;
	}
	uint16 Count() {
		return ElementOffset(0) / 2;
	}
	const T *operator[](int index) {
		return static_cast<T *>(_data + ElementOffset(num));
	}
};

class ResourceManager {
private:
	struct DatFileEntry {
		char _name[16];
		unsigned int _size;
		unsigned int _offset;
		byte _flag;
	};

	Common::Array<DatFileEntry> _files;
	Common::File _datFile;

	static void *StreamToBuffer(Common::SeekableReadStream *stream, unsigned int *size);

public:
	ResourceManager(const Common::String &datFileName);
	ResourceManager();
	~ResourceManager();

	bool LoadDatFile(const Common::String &datFileName);

	// Load resource as a seekable stream
	Common::SeekableReadStream *GetFile(const Common::String &resName, unsigned int hintIndex = 0);
	Common::SeekableReadStream *GetFile(unsigned int resIndex);

	// Load resource as a buffer
	void *GetData(const Common::String &resName, unsigned int *size = nullptr);
	void *GetData(int resIndex, unsigned int *size = nullptr);
	void *operator[](int resIndex) {
		return GetData(resIndex);
	}

};

}
