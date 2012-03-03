/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

/* Portions of this file are part of libmspack.
 * (C) 2003-2004 Stuart Caie.
 *
 * This source code is adapted and stripped for ResidualVM project.
 *
 * libmspack is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) version 2.1
 *
 * For further details, see the file COPYING.LIB distributed with libmspack
 */

#include "engines/grim/update/mscab.h"

#include "common/file.h"
#include "common/archive.h"
#include "common/memstream.h"
#include "common/zlib.h"
#include "common/str.h"

namespace Grim {

MsCabinet::~MsCabinet() {
	for (CacheMap::iterator it = _cache.begin(); it != _cache.end(); it++)
		delete[] it->_value;

	_folderMap.clear();
	_fileMap.clear();

	if (_data)
		delete _data;

	if (_decompressor)
		delete _decompressor;
}

MsCabinet::MsCabinet(Common::SeekableReadStream* data) :
	_data(data), _decompressor(0) {
	if (!_data)
		return;

	//CFHEADER PARSING

	// Verify Head-signature
	uint32 tag = _data->readUint32BE();
	if (tag != MKTAG('M','S','C','F'))
		return;

	/* uint32 reserved1 = */ _data->readUint32LE();
	uint32 length = _data->readUint32LE();
	if (length > uint32(_data->size()))
		return;

	/* uint32 reserved2 = */ _data->readUint32LE();
	uint32 filesOffset = _data->readUint32LE();
	/* uint32 reserved3 = */ _data->readUint32LE();

	byte versionMinor = _data->readByte();
	byte versionMajor = _data->readByte();
	if (versionMajor != 1 || versionMinor != 3)
		return;

	uint16 numFolders = _data->readUint16LE();
	uint16 numFiles = _data->readUint16LE();
	if (numFolders == 0 || numFiles == 0)
		return;

	//This implementation doesn't support multicabinet and reserved fields
	uint16 flags = _data->readUint16LE();
	if (flags != 0)
		return;

	/* uint16 setId = */ _data->readUint16LE();
	/* uint16 iCab = */ _data->readUint16LE();

	if (_data->err())
		return;

	//CFFOLDERs PARSING
	for (uint16 i = 0; i < numFolders; ++i) {
		FolderEntry fEntry;

		fEntry.offset = _data->readUint32LE();
		fEntry.num_blocks = _data->readUint16LE();
		fEntry.comp_type = _data->readUint16LE();

		if (_data->err())
			return;

		_folderMap[i] = fEntry;
	}

	//CFFILEs PARSING
	_data->seek(filesOffset);
	if (_data->err())
		return;

	for (uint16 i = 0; i < numFiles; ++i) {
		FileEntry fEntry;

		fEntry.length = _data->readUint32LE();
		fEntry.folderOffset = _data->readUint32LE();
		uint16 iFolder = _data->readUint16LE();
		/* uint16 date = */ _data->readUint16LE();
		/* uint16 time = */ _data->readUint16LE();
		/* uint16 attribs = */ _data->readUint16LE();
		Common::String name = readString(_data);
		for (uint l = 0; l < name.size(); ++l)
			if (name[l] == '\\')
				name.setChar('/', l);

		if (_data->err()) {
			_fileMap.clear();
			return;
		}

		if (_folderMap.contains(iFolder))
			fEntry.folder = &_folderMap[iFolder];
		else {
			_fileMap.clear();
			return;
		}

		_fileMap[name] = fEntry;
	}
}

/* read a null-terminated string from a stream
   Copied from ScummVm MohawkEngine_LivingBooks.*/
Common::String MsCabinet::readString(Common::ReadStream *stream) {
	Common::String ret;
	while (!stream->eos()) {
		byte in = stream->readByte();
		if (!in)
			break;
		ret += in;
	}
	return ret;
}

bool MsCabinet::hasFile(const Common::String &name) const {
	return _fileMap.contains(name);
}

int MsCabinet::listMembers(Common::ArchiveMemberList &list) const {
	for (FileMap::const_iterator it = _fileMap.begin(); it != _fileMap.end(); it++)
		list.push_back(getMember(it->_key));

	return _fileMap.size();
}

const Common::ArchiveMemberPtr MsCabinet::getMember(const Common::String &name) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *MsCabinet::createReadStreamForMember(const Common::String &name) const {
	byte *fileBuf;

	if (!hasFile(name))
		return 0;

	const FileEntry &entry = _fileMap[name];

	//Check if the file has already been decompressed and it's in the cache,
	// otherwise decompress it and put it in the cache
	if (_cache.contains(name))
		fileBuf = _cache[name];
	else {
		//Check if the decompressor should be reinitialized
		if (!_decompressor || entry.folder != _decompressor->getFolder()) {
			if (_decompressor)
				delete _decompressor;

			_decompressor = new Decompressor(entry.folder, _data);
		}

		if (!_decompressor->decompressFile(fileBuf, entry))
			return 0;

		_cache[name] = fileBuf;
	}

	return new Common::MemoryReadStream(fileBuf, entry.length, DisposeAfterUse::NO);
}

MsCabinet::Decompressor::Decompressor(const MsCabinet::FolderEntry *folder, Common::SeekableReadStream* data) :
	_curFolder(folder), _data(data), _curBlock(-1), _compressedBlock(0), _decompressedBlock(0), _fileBuf(0) {

	//Alloc the decompression buffers
	_compressedBlock = new byte[kCabInputmax];
	_decompressedBlock = new byte[kCabBlockSize];
}

MsCabinet::Decompressor::~Decompressor() {
	if (_decompressedBlock)
		delete[] _decompressedBlock;

	if (_compressedBlock)
		delete[] _compressedBlock;

	if (_fileBuf)
		delete[] _fileBuf;
}

bool MsCabinet::Decompressor::decompressFile(byte *&fileBuf, const FileEntry &entry) {
#ifdef USE_ZLIB
	// Ref: http://blogs.kde.org/node/3181
	uint32 cksum, cksum2;
	uint16 uncompressedLen, compressedLen;
	byte hdrS[4];
	byte *buf_tmp, *dict;
	bool decRes;

	//Sanity checks
	if (!_compressedBlock || !_compressedBlock)
		return false;

	if (entry.folder != _curFolder)
		return false;

	_startBlock = entry.folderOffset / kCabBlockSize;
	_inBlockStart = entry.folderOffset % kCabBlockSize;
	_endBlock = (entry.folderOffset + entry.length) / kCabBlockSize;
	_inBlockEnd = (entry.folderOffset + entry.length) % kCabBlockSize;

	//Check if the decompressor should be reinitialized
	if (_curBlock > _startBlock || _curBlock == -1) {
		_data->seek(entry.folder->offset);
		//Check the compression method (only mszip supported)
		if (entry.folder->comp_type != kMszipCompression)
			return false;

		_curBlock = -1;		//No block decompressed
	}

	//Check if the file is contained in the folder
	if ((entry.length + entry.folderOffset) / kCabBlockSize > entry.folder->num_blocks)
		return false;

	_fileBuf = new byte[entry.length];
	if (!_fileBuf)
		return false;

	buf_tmp = _fileBuf;

	//if a part of this file has been decompressed in the last block, make a copy of it
	copyBlock(buf_tmp);

	while((_curBlock + 1) <= _endBlock) {
		// Read the CFDATA header
		cksum = _data->readUint32LE();
		_data->read(hdrS, 4);
		compressedLen = READ_LE_UINT16(hdrS);
		uncompressedLen = READ_LE_UINT16(hdrS + 2);

		if (_data->err())
			return false;

		if (compressedLen > kCabInputmax || uncompressedLen > kCabBlockSize)
			return false;

		//Read the compressed block
		if (_data->read(_compressedBlock, compressedLen) != compressedLen)
			return false;

		//Perform checksum test on the block (if one is stored)
		if (cksum != 0) {
			cksum2 = checksum(_compressedBlock, compressedLen, 0);
			if (checksum(hdrS, 4, cksum2) != cksum)
				return false;
		}

		//Check the CK header
		if (_compressedBlock[0] != 'C' || _compressedBlock[1] != 'K')
			return false;

		//Decompress the block. If it isn't the first, provide the previous block as dictonary
		dict = (_curBlock >= 0) ? _decompressedBlock : 0;
		decRes = Common::inflateZlibHeaderless(_decompressedBlock, uncompressedLen, _compressedBlock + 2, compressedLen - 2, dict, kCabBlockSize);
		if (!decRes)
			return false;

		_curBlock++;

		//Copy the decompressed data, if needed
		copyBlock(buf_tmp);
	}

	fileBuf = _fileBuf;
	_fileBuf = 0;
	return true;
#else
	warning("zlib required to extract MSCAB");
	return false;
#endif
}

void MsCabinet::Decompressor::copyBlock(byte *&data_ptr) const {
	uint16 start, end, size;

	if(_startBlock <= _curBlock && _curBlock <= _endBlock) {
		start = (_startBlock == _curBlock) ? _inBlockStart: 0;
		end = (_endBlock == _curBlock) ? _inBlockEnd : uint16(kCabBlockSize);
		size = end - start;

		memcpy(data_ptr, _decompressedBlock + start, size);
		data_ptr += size;
	}
}

uint32 MsCabinet::Decompressor::checksum(byte *data, uint32 bytes, uint32 cksum) const {
	uint32 len, ul = 0;

	for (len = bytes >> 2; len--; data += 4) {
		cksum ^= READ_LE_UINT32(data);
	}

	switch (bytes & 3) {
		case 3: ul |= *data++ << 16;
		case 2: ul |= *data++ <<  8;
		case 1: ul |= *data;
	}
	cksum ^= ul;

	return cksum;
}

} // End of namespace Grim
