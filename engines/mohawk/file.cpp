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

#include "mohawk/file.h"

#include "common/util.h"

namespace Mohawk {

MohawkFile::MohawkFile() {
	_mhk = NULL;
	_curFile = Common::String::emptyString;
	_types = NULL;
	_fileTable = NULL;
}

void MohawkFile::open(Common::String filename) {
	Common::File *file = new Common::File();
	if (!file->open(filename.c_str()))
		error ("Could not open file \'%s\'", filename.c_str());

	_curFile = filename;
	
	open(file);
}

void MohawkFile::close() {
	delete _mhk; _mhk = NULL;	
	delete[] _types; _types = NULL;
	delete[] _fileTable; _fileTable = NULL;
	
	_curFile = Common::String::emptyString;
}

void MohawkFile::open(Common::SeekableReadStream *stream) {
	// Make sure no other file is open...
	close();
	_mhk = stream;

	if (_mhk->readUint32BE() != ID_MHWK)
		error ("Could not find tag \'MHWK\'");

	_fileSize = _mhk->readUint32BE();

	if (_mhk->readUint32BE() != ID_RSRC)
		error ("Could not find tag \'RSRC\'");

	_rsrc.size = _mhk->readUint32BE();
	_rsrc.filesize = _mhk->readUint32BE();
	_rsrc.abs_offset = _mhk->readUint32BE();
	_rsrc.file_table_offset = _mhk->readUint16BE();
	_rsrc.file_table_size = _mhk->readUint16BE();

	debug (3, "Absolute Offset = %08x", _rsrc.abs_offset);

	/////////////////////////////////
	//Resource Dir
	/////////////////////////////////		

	// Type Table
	_mhk->seek(_rsrc.abs_offset);
	_typeTable.name_offset = _mhk->readUint16BE();
	_typeTable.resource_types = _mhk->readUint16BE();

	debug (0, "Name List Offset = %04x  Number of Resource Types = %04x", _typeTable.name_offset, _typeTable.resource_types);

	_types = new Type[_typeTable.resource_types];

	for (uint16 i = 0; i < _typeTable.resource_types; i++) {
		_types[i].tag = _mhk->readUint32BE();
		_types[i].resource_table_offset = _mhk->readUint16BE();
		_types[i].name_table_offset = _mhk->readUint16BE();

		// HACK: Zoombini's SND resource starts will a NULL.
		if (_types[i].tag == ID_SND)
			debug (3, "Type[%02d]: Tag = \'SND\' ResTable Offset = %04x  NameTable Offset = %04x", i, _types[i].resource_table_offset, _types[i].name_table_offset);
		else
			debug (3, "Type[%02d]: Tag = \'%s\' ResTable Offset = %04x  NameTable Offset = %04x", i, tag2str(_types[i].tag), _types[i].resource_table_offset, _types[i].name_table_offset);

		//Resource Table
		_mhk->seek(_rsrc.abs_offset + _types[i].resource_table_offset);
		_types[i].resTable.resources = _mhk->readUint16BE();

		debug (3, "Resources = %04x", _types[i].resTable.resources);

		_types[i].resTable.entries = new Type::ResourceTable::Entries[_types[i].resTable.resources];

		for (uint16 j = 0; j < _types[i].resTable.resources; j++) {
			_types[i].resTable.entries[j].id = _mhk->readUint16BE();
			_types[i].resTable.entries[j].index = _mhk->readUint16BE();

			debug (4, "Entry[%02x]: ID = %04x (%d) Index = %04x", j, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].index); 
		}

		// Name Table
		_mhk->seek(_rsrc.abs_offset + _types[i].name_table_offset);
		_types[i].nameTable.num = _mhk->readUint16BE();

		debug (3, "Names = %04x", _types[i].nameTable.num);

		_types[i].nameTable.entries = new Type::NameTable::Entries[_types[i].nameTable.num];

		for (uint16 j = 0; j < _types[i].nameTable.num; j++) {
			_types[i].nameTable.entries[j].offset = _mhk->readUint16BE();
			_types[i].nameTable.entries[j].index = _mhk->readUint16BE();

			debug (4, "Entry[%02x]: Name List Offset = %04x  Index = %04x", j, _types[i].nameTable.entries[j].offset, _types[i].nameTable.entries[j].index);

			// Name List
			uint32 pos = _mhk->pos();
			_mhk->seek(_rsrc.abs_offset + _typeTable.name_offset + _types[i].nameTable.entries[j].offset);
			char c = (char)_mhk->readByte();
			while (c != 0) {
				_types[i].nameTable.entries[j].name += c;
				c = (char)_mhk->readByte();
			}

			debug (3, "Name = \'%s\'", _types[i].nameTable.entries[j].name.c_str());

			// Get back to next entry
			_mhk->seek(pos);
		}

		// Return to next TypeTable entry
		_mhk->seek(_rsrc.abs_offset + (i + 1) * 8 + 4);

		debug (3, "\n");
	}

	_mhk->seek(_rsrc.abs_offset + _rsrc.file_table_offset);
	_fileTableAmount = _mhk->readUint32BE();
	_fileTable = new FileTable[_fileTableAmount];

	for (uint32 i = 0; i < _fileTableAmount; i++) {
		_fileTable[i].offset = _mhk->readUint32BE();
		_fileTable[i].dataSize = _mhk->readUint16BE();
		_fileTable[i].dataSize += _mhk->readByte() << 16; // Get bits 15-24 of dataSize too
		_fileTable[i].flags = _mhk->readByte();
		_fileTable[i].unk = _mhk->readUint16BE();
		
		// Add in another 3 bits for file size from the flags.
		// The flags are useless to us except for doing this ;)
		_fileTable[i].dataSize += (_fileTable[i].flags & 7) << 24;

		debug (4, "File[%02x]: Offset = %08x  DataSize = %07x  Flags = %02x  Unk = %04x", i, _fileTable[i].offset, _fileTable[i].dataSize, _fileTable[i].flags, _fileTable[i].unk);
	}
}

bool MohawkFile::hasResource(uint32 tag, uint16 id) {
	if (!_mhk)
		return false;

	int16 typeIndex = getTypeIndex(tag);

	if (typeIndex < 0)
		return false;

	return (getIdIndex(typeIndex, id) >= 0);
}

Common::SeekableReadStream *MohawkFile::getRawData(uint32 tag, uint16 id) {
	if (!_mhk)
		error ("MohawkFile::getRawData - No File in Use");

	int16 typeIndex = getTypeIndex(tag);

	if (typeIndex < 0)
		error ("Could not find a tag of \'%s\' in file \'%s\'", tag2str(tag), _curFile.c_str());

	int16 idIndex = getIdIndex(typeIndex, id);

	if (idIndex < 0)
		error ("Could not find \'%s\' %04x in file \'%s\'", tag2str(tag), id, _curFile.c_str());

	// Note: the fileTableIndex is based off 1, not 0. So, subtract 1
	uint16 fileTableIndex = _types[typeIndex].resTable.entries[idIndex].index - 1;
	
	// WORKAROUND: tMOV resources pretty much ignore the size part of the file table,
	// as the original just passed the full Mohawk file to QuickTime and the offset.
	// We need to do this because of the way Mohawk is set up (this is much more "proper"
	// than passing _mhk at the right offset). We may want to do that in the future, though.
	if (_types[typeIndex].tag == ID_TMOV) {
		if (fileTableIndex == _fileTableAmount)
			return new Common::SeekableSubReadStream(_mhk, _fileTable[fileTableIndex].offset, _mhk->size());
		else
			return new Common::SeekableSubReadStream(_mhk, _fileTable[fileTableIndex].offset, _fileTable[fileTableIndex + 1].offset);
	}
	
	return new Common::SeekableSubReadStream(_mhk, _fileTable[fileTableIndex].offset, _fileTable[fileTableIndex].offset + _fileTable[fileTableIndex].dataSize);
}

void OldMohawkFile::open(Common::SeekableReadStream *stream) {
	close();
	_mhk = stream;
	
	// This is for the "old" Mohawk resource format used in some older
	// Living Books. It is very similar, just missing the MHWK tag and
	// some other minor differences, especially with the file table
	// being merged into the resource table.
	
	uint32 headerSize = _mhk->readUint32BE();
	
	// NOTE: There are differences besides endianness! (Subtle changes,
	// but different).
	
	if (headerSize == 6) { // We're in Big Endian mode (Macintosh)
		_mhk->readUint16BE(); // Resource Table Size
		_typeTable.resource_types = _mhk->readUint16BE();
		_types = new OldType[_typeTable.resource_types];
		
		debug (0, "Old Mohawk File (Macintosh): Number of Resource Types = %04x", _typeTable.resource_types);
		
		for (uint16 i = 0; i < _typeTable.resource_types; i++) {
			_types[i].tag = _mhk->readUint32BE();
			_types[i].resource_table_offset = (uint16)_mhk->readUint32BE() + 6;
			_mhk->readUint32BE(); // Unknown (always 0?)
			
			debug (3, "Type[%02d]: Tag = \'%s\'  ResTable Offset = %04x", i, tag2str(_types[i].tag), _types[i].resource_table_offset);

			uint32 oldPos = _mhk->pos();
			
			// Resource Table/File Table
			_mhk->seek(_types[i].resource_table_offset);
			_types[i].resTable.resources = _mhk->readUint16BE();
			_types[i].resTable.entries = new OldType::ResourceTable::Entries[_types[i].resTable.resources];
			
			for (uint16 j = 0; j < _types[i].resTable.resources; j++) {
				_types[i].resTable.entries[j].id = _mhk->readUint16BE();
				_types[i].resTable.entries[j].offset = _mhk->readUint32BE();
				_types[i].resTable.entries[j].size = _mhk->readByte() << 16;
				_types[i].resTable.entries[j].size += _mhk->readUint16BE();
				_mhk->skip(5); // Unknown (always 0?)
				
				debug (4, "Entry[%02x]: ID = %04x (%d)\tOffset = %08x, Size = %08x", j, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].offset, _types[i].resTable.entries[j].size);
			}
			
			_mhk->seek(oldPos);
			debug (3, "\n");
		}
	} else if (SWAP_BYTES_32(headerSize) == 6) { // We're in Little Endian mode (Windows)
		_mhk->readUint16LE(); // Resource Table Size
		_typeTable.resource_types = _mhk->readUint16LE();
		_types = new OldType[_typeTable.resource_types];
		
		debug (0, "Old Mohawk File (Windows): Number of Resource Types = %04x", _typeTable.resource_types);
		
		for (uint16 i = 0; i < _typeTable.resource_types; i++) {
			_types[i].tag = _mhk->readUint32LE();
			_types[i].resource_table_offset = _mhk->readUint16LE() + 6;
			_mhk->readUint16LE(); // Unknown (always 0?)
			
			debug (3, "Type[%02d]: Tag = \'%s\'  ResTable Offset = %04x", i, tag2str(_types[i].tag), _types[i].resource_table_offset);
			
			uint32 oldPos = _mhk->pos();
			
			// Resource Table/File Table
			_mhk->seek(_types[i].resource_table_offset);
			_types[i].resTable.resources = _mhk->readUint16LE();
			_types[i].resTable.entries = new OldType::ResourceTable::Entries[_types[i].resTable.resources];
			
			for (uint16 j = 0; j < _types[i].resTable.resources; j++) {
				_types[i].resTable.entries[j].id = _mhk->readUint16LE();
				_types[i].resTable.entries[j].offset = _mhk->readUint32LE();
				_types[i].resTable.entries[j].size = _mhk->readUint16LE();
				_mhk->readUint32LE(); // Unknown (always 0?)
				
				debug (4, "Entry[%02x]: ID = %04x (%d)\tOffset = %08x, Size = %08x", j, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].id, _types[i].resTable.entries[j].offset, _types[i].resTable.entries[j].size); 
			}
			
			_mhk->seek(oldPos);
			debug (3, "\n");
		}
	} else
		error("Could not determine type of Old Mohawk resource");

}

Common::SeekableReadStream *OldMohawkFile::getRawData(uint32 tag, uint16 id) {
	if (!_mhk)
		error ("OldMohawkFile::getRawData - No File in Use");

	int16 typeIndex = getTypeIndex(tag);

	if (typeIndex < 0)
		error ("Could not find a tag of \'%s\' in file \'%s\'", tag2str(tag), _curFile.c_str());

	int16 idIndex = getIdIndex(typeIndex, id);

	if (idIndex < 0)
		error ("Could not find \'%s\' %04x in file \'%s\'", tag2str(tag), id, _curFile.c_str());

	return new Common::SeekableSubReadStream(_mhk, _types[typeIndex].resTable.entries[idIndex].offset, _types[typeIndex].resTable.entries[idIndex].offset + _types[typeIndex].resTable.entries[idIndex].size);
}

}	// End of namespace Mohawk
