/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "common/substream.h"
#include "common/md5.h"
#include "common/file.h"
#include "common/zlib.h"
#include "common/bufferedstream.h"

#include "engines/grim/patchr.h"
#include "engines/grim/debug.h"

namespace Grim {

class PatchedFile : public Common::SeekableReadStream {
public:
	PatchedFile();
	virtual ~PatchedFile();

	bool load(Common::SeekableReadStream *file, const Common::String &patchName);

	// Common::ReadStream implementation
	virtual bool eos() const override;
	virtual uint32 read(void *dataPtr, uint32 dataSize) override;

	// Common::SeekableReadStream implementation
	virtual int32 pos() const override;
	virtual int32 size() const override;
	virtual bool seek(int32 offset, int whence = SEEK_SET) override;

private:
	// Consts
	static const uint32 _kDiffBufferSize, _kHeaderSize, _kMd5size;
	static const uint16 _kVersionMajor, _kVersionMinor;

	//Flags
	enum Flags {
		FLAG_MIX_DIFF_EXTRA = 1 << 0,
		FLAG_COMPRESS_CTRL = 1 << 1
	};

	// Streams
	Common::SeekableReadStream *_file;
	Common::SeekableReadStream *_ctrl, *_diff, *_extra;

	// Current instruction
	uint32 _diffCopy, _extraCopy;
	int32 _jump;
	int32 _instrLeft;
	bool readNextInst();

	int32 _pos;
	uint32 _flags, _newSize;

	uint8 *_diffBuffer;

	Common::String _patchName;
};

const uint16 PatchedFile::_kVersionMajor = 2;
const uint16 PatchedFile::_kVersionMinor = 0;
const uint32 PatchedFile::_kDiffBufferSize = 1024;
const uint32 PatchedFile::_kHeaderSize = 48;
const uint32 PatchedFile::_kMd5size = 5000;

PatchedFile::PatchedFile():
		_file(nullptr), _ctrl(nullptr), _diff(nullptr), _extra(nullptr), _pos(0),
		_instrLeft(0), _jump(0), _newSize(0), _flags(0), _extraCopy(0),
		_diffCopy(0) {
	_diffBuffer = new uint8[_kDiffBufferSize];
}

PatchedFile::~PatchedFile() {
	delete[] _diffBuffer;

	delete _file;

	delete _ctrl;
	delete _diff;
	if (!(_flags & FLAG_MIX_DIFF_EXTRA))
		delete _extra;
}

bool PatchedFile::load(Common::SeekableReadStream *file, const Common::String &patchName) {
	uint8 md5_p[16], md5_f[16];
	uint32 zctrllen, zdatalen, zextralen;
	Common::File patch;

	_patchName = patchName;

	// Open the patch
	if (!patch.open(_patchName)) {
		error("Unable to open patchfile %s", _patchName.c_str());
		return false;
	}

	// Check for appropriate signature
	if (patch.readUint32BE() != MKTAG('P','A','T','R')) {
		error("%s patchfile is corrupted, wrong siganture", _patchName.c_str());
		return false;
	}

	// Check the version number
	if (patch.readUint16LE() != _kVersionMajor || patch.readUint16LE() > _kVersionMinor) {
		error("%s has a wrong version number (must be major = %d, minor <= %d)", _patchName.c_str(), _kVersionMajor, _kVersionMinor);
		return false;
	}

	_flags = patch.readUint32LE();

	// Check if the file to patch match
	Common::computeStreamMD5(*file, md5_f, _kMd5size);
	file->seek(0, SEEK_SET);
	patch.read(md5_p, 16);
	uint32 fileSize = patch.readUint32LE();
	if (memcmp(md5_p, md5_f, 16) != 0 || (uint32)file->size() != fileSize) {
		Debug::debug(Debug::Patchr, "%s targets a different file", _patchName.c_str());
		if (Debug::isChannelEnabled(Debug::Patchr)) {
			Common::String md5_ps, md5_fs;
			for (int i = 0; i < 16; i++) {
				md5_ps += Common::String::format("%02x", (int)md5_p[i]);
				md5_fs += Common::String::format("%02x", (int)md5_f[i]);
			}
			Debug::debug(Debug::Patchr, "Patch target: size = %d, md5 = %s", fileSize, md5_ps.c_str());
			Debug::debug(Debug::Patchr, "Actual file : size = %d, md5 = %s", (uint32)file->size(), md5_fs.c_str());
		}
		return false;
	}

	// Read lengths from header
	_newSize = patch.readUint32LE();
	zctrllen = patch.readUint32LE();
	zdatalen = patch.readUint32LE();
	zextralen = patch.readUint32LE();

	patch.close();

	// Opens ctrl, diff and extra substreams
	Common::File *tmp;
	tmp = new Common::File;
	tmp->open(_patchName);
	_ctrl = new Common::SeekableSubReadStream(tmp, _kHeaderSize, _kHeaderSize + zctrllen, DisposeAfterUse::YES);
	if (_flags & FLAG_COMPRESS_CTRL)
		_ctrl = Common::wrapCompressedReadStream(_ctrl);

	//ctrl stream sanity checks
	if (_ctrl->size() % (3 * sizeof(uint32)) != 0) {
		error("%s patchfile is corrupted", _patchName.c_str());
		return false;
	}

	_instrLeft = _ctrl->size() / (3 * sizeof(uint32));

	tmp = new Common::File;
	tmp->open(_patchName);
	_diff = new Common::SeekableSubReadStream(tmp, _kHeaderSize + zctrllen, _kHeaderSize + zctrllen + zdatalen, DisposeAfterUse::YES);
	_diff = Common::wrapCompressedReadStream(_diff);

	if (_flags & FLAG_MIX_DIFF_EXTRA)
		_extra = _diff;
	else {
		tmp = new Common::File;
		tmp->open(_patchName);
		_extra = new Common::SeekableSubReadStream(tmp, _kHeaderSize + zctrllen + zdatalen, _kHeaderSize + zctrllen + zdatalen + zextralen, DisposeAfterUse::YES);
		_extra = Common::wrapCompressedReadStream(_extra);
	}

	_file = file;

	readNextInst();

	return true;
}

uint32 PatchedFile::read(void *dataPtr, uint32 dataSize) {
	uint32 readSize, diffRead, toRead, rd;
	byte *data = (byte*)dataPtr;

	toRead = dataSize;
	while (toRead > 0) {
		// Read data from original file and apply the differences
		if (_diffCopy > 0) {
			readSize = MIN(toRead, _diffCopy);
			rd = _file->read(data, readSize);
			if (_file->err() || rd != readSize)
				error("%s: Corrupted patchfile", _patchName.c_str());

			toRead -= readSize;
			_diffCopy -= readSize;

			//Read data from diff as blocks of size _kDiffBufferSize,
			// then xor original data with them in groups of 4 bytes
			while (readSize > 0) {
				diffRead = MIN(readSize, _kDiffBufferSize);
				rd = _diff->read(_diffBuffer, diffRead);
				if (_diff->err() || rd != diffRead)
					error("%s: Corrupted patchfile", _patchName.c_str());

				for (uint32 i = 0; i < diffRead / 4; ++i)
					WRITE_UINT32((uint32 *)data + i, READ_UINT32((uint32 *)data + i) ^ READ_UINT32((uint32 *)_diffBuffer + i));
				for (uint32 i = diffRead - diffRead % 4; i < diffRead; ++i)
					data[i] ^= _diffBuffer[i];
				
				readSize -= diffRead;
				data += diffRead;
			}
		}

		if (toRead == 0)
			break;

		// Read data from extra
		if (_extraCopy > 0) {
			readSize = MIN(toRead, _extraCopy);
			rd = _extra->read(data, readSize);
			if (_extra->err() || rd != readSize)
				error("%s: Corrupted patchfile", _patchName.c_str());

			data += readSize;
			toRead -= readSize;
			_extraCopy -= readSize;
		}

		// Jump and read next instructions
		if (_diffCopy == 0 && _extraCopy == 0) {
			if (_jump != 0)
				_file->seek(_jump, SEEK_CUR);

			//If there aren't new instructions, breaks here
			if (!readNextInst())
				break;
		}
	}

	_pos += dataSize - toRead;
	return (dataSize - toRead);
}

bool PatchedFile::readNextInst() {
	if (_instrLeft == 0) {
		_diffCopy = 0;
		_extraCopy = 0;
		_jump = 0;
		return false;
	}

	_diffCopy = _ctrl->readUint32LE();
	_extraCopy = _ctrl->readUint32LE();
	_jump = _ctrl->readSint32LE();

	//Sanity checks
	if (_ctrl->err() ||
			(int32(_diffCopy) > _file->size() - _file->pos()) ||
			(int32(_diffCopy) > _diff->size() - _diff->pos()) ||
			(int32(_extraCopy) > _extra->size() - _extra->pos()) ||
			(_jump > _file->size() - _file->pos()))
		error("%s: Corrupted patchfile. istrleft = %d", _patchName.c_str(), _instrLeft);

	--_instrLeft;
	return true;
}

bool PatchedFile::eos() const {
	if (_pos >= (int32)_newSize)
		return true;
	else
		return false;
}

int32 PatchedFile::pos() const {
	return _pos;
}

int32 PatchedFile::size() const {
	return _newSize;
}

bool PatchedFile::seek(int32 offset, int whence) {
	int32 totJump, relOffset;
	uint32 skipDiff, skipExtra, skipSize;
	relOffset = 0;
	skipDiff = 0;
	skipExtra = 0;
	totJump = 0;

	switch (whence) {
		case SEEK_SET:
			relOffset = offset - pos();
			break;
		case SEEK_CUR:
			relOffset = offset;
			break;
		case SEEK_END:
			relOffset = (size() + offset) - pos();
			break;
		default:
			error("%s: Invalid seek instruction", _patchName.c_str());
	}

	if (relOffset == 0)
		return true;

	if (relOffset < 0) {
		Debug::debug(Debug::Patchr, "Seeking back to start %s", _patchName.c_str());
		_file->seek(0, SEEK_SET);
		_ctrl->seek(0, SEEK_SET);
		_extra->seek(0, SEEK_SET);
		_instrLeft = _ctrl->size() / (3 * sizeof(uint32));
		readNextInst();
		int p = pos() + relOffset;
		_pos = 0;
		return seek(p, SEEK_SET);
	}

	while (relOffset > 0) {
		if (_diffCopy > 0) {
			skipSize = MIN(_diffCopy, (uint32)relOffset);
			_diffCopy -= skipSize;
			relOffset -= skipSize;
			skipDiff += skipSize;
			totJump += skipSize;
		}
		if (relOffset == 0)
			break;

		if (_extraCopy > 0) {
			skipSize = MIN(_extraCopy, (uint32)relOffset);
			_extraCopy -= skipSize;
			relOffset -= skipSize;
			skipExtra += skipSize;
		}

		if (_diffCopy == 0 && _extraCopy == 0) {
			totJump += _jump;
			readNextInst();
		}
	}
	_diff->seek(skipDiff, SEEK_CUR);
	_extra->seek(skipExtra, SEEK_CUR);
	_file->seek(totJump, SEEK_CUR);

	return true;
}

Common::SeekableReadStream *wrapPatchedFile(Common::SeekableReadStream *rs, const Common::String &filename) {
	if (!rs)
		return nullptr;

	Common::String patchfile = filename + ".patchr";
	int i = 1;
	while (SearchMan.hasFile(patchfile)) {
		Debug::debug(Debug::Patchr, "Patch requested for %s (patch filename %s)", filename.c_str(), patchfile.c_str());

		PatchedFile *pf = new PatchedFile;
		if (pf->load(rs, patchfile)) {
			rs = Common::wrapBufferedSeekableReadStream(pf, 1024, DisposeAfterUse::YES);
			Debug::debug(Debug::Patchr, "Patch for %s successfully loaded", filename.c_str());
			break;
		}

		delete pf;
		patchfile = Common::String::format("%s_%d.patchr", filename.c_str(), i++);
	}

	return rs;
}

} // end of namespace Grim
