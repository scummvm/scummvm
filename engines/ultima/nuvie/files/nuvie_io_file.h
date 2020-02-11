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

#ifndef NUVIE_FILES_NUVIE_IO_FILE_H
#define NUVIE_FILES_NUVIE_IO_FILE_H

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/files/nuvie_io.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/savefile.h"

namespace Ultima {
namespace Nuvie {

class NuvieIOFile : public NuvieIO {
public:
	NuvieIOFile() {}
	~NuvieIOFile() override {}

	virtual bool open(const Common::String &filename) {
		return false;
	};
};

class NuvieIOFileRead : public NuvieIOFile {
private:
	Common::SeekableReadStream *_file;
	Common::File _srcFile;
public:
	NuvieIOFileRead() : NuvieIOFile(), _file(nullptr) {}
	~NuvieIOFileRead() override;

	bool open(const Common::String &filename) override;
	virtual bool open(Common::InSaveFile *saveFile);
	void close() override;
	void seek(uint32 new_pos) override;

	uint8 read1() override;
	uint16 read2() override;
	uint32 read4() override;

	bool readToBuf(unsigned char *buf, uint32 buf_size) override;

	bool isOpen() const {
		return _file != nullptr;
	}
};

/**
 * File writing class. This can be done in one of two ways.
 * If it's a simple filename, then it uses the savefile interface to
 * write it as uncompresed to the save folder. However, if it has
 * relative paths, such as used by the code to dump out all tiles or maps,
 * it uses a Common::DumpFile instead
 */
class NuvieIOFileWrite : public NuvieIOFile {
private:
	Common::SeekableWriteStream *_file;
	Common::DumpFile _dumpFile;
	Common::OutSaveFile *_saveFile;
	Common::MemoryWriteStreamDynamic _saveFileData;
	Common::String _description;
	bool _isAutosave;
protected:
	bool isOpen() const {
		return _file != nullptr;
	}
public:
	NuvieIOFileWrite();
	~NuvieIOFileWrite() override;
	bool open(const Common::String &filename) override;
	bool open(const Common::String &filename, bool isAutosave);
	void close() override;
	void seek(uint32 new_pos) override;

	bool write1(uint8 src) override;
	bool write2(uint16 src) override;
	bool write4(uint32 src) override;
	void writeDesc(const Common::String &desc) {
		_description = desc;
	}

	uint32 writeBuf(const unsigned char *src, uint32 src_size) override;
	uint32 write(NuvieIO *src) override;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
