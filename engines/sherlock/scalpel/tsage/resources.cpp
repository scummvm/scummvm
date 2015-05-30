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

#include "common/scummsys.h"
#include "common/memstream.h"
#include "common/stack.h"
#include "sherlock/scalpel/tsage/resources.h"

namespace Sherlock {
namespace Scalpel {
namespace TsAGE {

static uint16 bitMasks[4] = {0x1ff, 0x3ff, 0x7ff, 0xfff};

uint16 BitReader::readToken() {
	assert((numBits >= 9) && (numBits <= 12));
	uint16 result = _remainder;
	int bitsLeft = numBits - _bitsLeft;
	int bitOffset = _bitsLeft;
	_bitsLeft = 0;

	while (bitsLeft >= 0) {
		_remainder = readByte();
		result |= _remainder << bitOffset;
		bitsLeft -= 8;
		bitOffset += 8;
	}

	_bitsLeft = -bitsLeft;
	_remainder >>= 8 - _bitsLeft;
	return result & bitMasks[numBits - 9];
}

/*-------------------------------------------------------------------------*/

TLib::TLib(const Common::String &filename) : _filename(filename) {

	// If the resource strings list isn't yet loaded, load them
	if (_resStrings.size() == 0) {
		Common::File f;
		if (f.open("tsage.cfg")) {
			while (!f.eos()) {
				_resStrings.push_back(f.readLine());
			}
			f.close();
		}
	}

	if (!_file.open(filename))
		error("Missing file %s", filename.c_str());

	loadIndex();
}

TLib::~TLib() {
	_resStrings.clear();
}

/**
 * Load a section index from the given position in the file
 */
void TLib::loadSection(uint32 fileOffset) {
	_resources.clear();
	_file.seek(fileOffset);
	_sections.fileOffset = fileOffset;

	loadSection(_file, _resources);
}

struct DecodeReference {
	uint16 vWord;
	uint8 vByte;
};

/**
 * Gets a resource from the currently loaded section
 */
Common::SeekableReadStream *TLib::getResource(uint16 id, bool suppressErrors) {
	// Scan for an entry for the given Id
	ResourceEntry *re = nullptr;
	ResourceList::iterator iter;
	for (iter = _resources.begin(); iter != _resources.end(); ++iter) {
		if ((*iter).id == id) {
			re = &(*iter);
			break;
		}
	}
	if (!re) {
		if (suppressErrors)
			return nullptr;
		error("Could not find resource Id #%d", id);
	}

	if (!re->isCompressed) {
		// Read in the resource data and return it
		byte *dataP = (byte *)malloc(re->size);
		_file.seek(_sections.fileOffset + re->fileOffset);
		_file.read(dataP, re->size);

		return new Common::MemoryReadStream(dataP, re->size, DisposeAfterUse::YES);
	}

	/*
	 * Decompress the data block
	 */

	_file.seek(_sections.fileOffset + re->fileOffset);
	Common::ReadStream *compStream = _file.readStream(re->size);
	BitReader bitReader(*compStream);

	byte *dataOut = (byte *)malloc(re->uncompressedSize);
	byte *destP = dataOut;
	uint bytesWritten = 0;

	uint16 ctrCurrent = 0x102, ctrMax = 0x200;
	uint16 word_48050 = 0, currentToken = 0, word_48054 =0;
	byte byte_49068 = 0, byte_49069 = 0;

	const uint tableSize = 0x1000;
	DecodeReference *table = (DecodeReference *)malloc(tableSize * sizeof(DecodeReference));
	if (!table)
		error("[TLib::getResource] Cannot allocate table buffer");

	for (uint i = 0; i < tableSize; ++i) {
		table[i].vByte = table[i].vWord = 0;
	}
	Common::Stack<uint16> tokenList;

	for (;;) {
		// Get the next decode token
		uint16 token = bitReader.readToken();

		// Handle the token
		if (token == 0x101) {
			// End of compressed stream
			break;
		} else if (token == 0x100) {
			// Reset bit-rate
			bitReader.numBits = 9;
			ctrMax = 0x200;
			ctrCurrent = 0x102;

			// Set variables with next token
			currentToken = word_48050 = bitReader.readToken();
			byte_49069 = byte_49068 = (byte)currentToken;

			++bytesWritten;
			assert(bytesWritten <= re->uncompressedSize);
			*destP++ = byte_49069;
		} else {
			word_48054 = word_48050 = token;

			if (token >= ctrCurrent) {
				word_48050 = currentToken;
				tokenList.push(byte_49068);
			}

			while (word_48050 >= 0x100) {
				assert(word_48050 < 0x1000);
				tokenList.push(table[word_48050].vByte);
				word_48050 = table[word_48050].vWord;
			}

			byte_49069 = byte_49068 = (byte)word_48050;
			tokenList.push(word_48050);

			// Write out any cached tokens
			while (!tokenList.empty()) {
				++bytesWritten;
				assert(bytesWritten <= re->uncompressedSize);
				*destP++ = tokenList.pop();
			}

			assert(ctrCurrent < 0x1000);
			table[ctrCurrent].vByte = byte_49069;
			table[ctrCurrent].vWord = currentToken;
			++ctrCurrent;

			currentToken = word_48054;
			if ((ctrCurrent >= ctrMax) && (bitReader.numBits != 12)) {
				// Move to the next higher bit-rate
				++bitReader.numBits;
				ctrMax <<= 1;
			}
		}
	}

	free(table);

	assert(bytesWritten == re->uncompressedSize);
	delete compStream;
	return new Common::MemoryReadStream(dataOut, re->uncompressedSize, DisposeAfterUse::YES);
}

/**
 * Finds the correct section and loads the specified resource within it
 */
Common::SeekableReadStream *TLib::getResource(ResourceType resType, uint16 resNum, uint16 rlbNum, bool suppressErrors) {
	SectionList::iterator i = _sections.begin();
	while ((i != _sections.end()) && ((*i).resType != resType || (*i).resNum != resNum))
		++i;
	if (i == _sections.end()) {
		if (suppressErrors)
			return nullptr;
		error("Unknown resource type %d num %d", resType, resNum);
	}

	loadSection((*i).fileOffset);

	return getResource(rlbNum, suppressErrors);
}

/**
 * Gets the offset of the start of a resource in the resource file
 */
uint32 TLib::getResourceStart(ResourceType resType, uint16 resNum, uint16 rlbNum, ResourceEntry &entry) {
	// Find the correct section
	SectionList::iterator i = _sections.begin();
	while ((i != _sections.end()) && ((*i).resType != resType || (*i).resNum != resNum))
		++i;
	if (i == _sections.end()) {
		error("Unknown resource type %d num %d", resType, resNum);
	}

	// Load in the section index
	loadSection((*i).fileOffset);

	// Scan for an entry for the given Id
	ResourceEntry *re = nullptr;
	ResourceList::iterator iter;
	for (iter = _resources.begin(); iter != _resources.end(); ++iter) {
		if ((*iter).id == rlbNum) {
			re = &(*iter);
			break;
		}
	}

	// Throw an error if no resource was found, or the resource is compressed
	if (!re || re->isCompressed)
		error("Invalid resource Id #%d", rlbNum);

	// Return the resource entry as well as the file offset
	entry = *re;
	return _sections.fileOffset + entry.fileOffset;
}

void TLib::loadIndex() {
	uint16 resNum, configId, fileOffset;

	// Load the root resources section
	loadSection(0);

	// Get the single resource from it
	Common::SeekableReadStream *stream = getResource(0);

	_sections.clear();

	// Loop through reading the entries
	while ((resNum = stream->readUint16LE()) != 0xffff) {
		configId = stream->readUint16LE();
		fileOffset = stream->readUint16LE();

		SectionEntry se;
		se.resNum = resNum;
		se.resType = (ResourceType)(configId & 0x1f);
		se.fileOffset = (((configId >> 5) & 0x7ff) << 16) | fileOffset;

		_sections.push_back(se);
	}

	delete stream;
}

/**
 * Retrieves the specified palette resource and returns it's data
 *
 * @paletteNum Specefies the palette number
 */
void TLib::getPalette(byte palette[PALETTE_SIZE], int paletteNum) {
	// Get the specified palette
	Common::SeekableReadStream *stream = getResource(RES_PALETTE, paletteNum, 0, true);
	if (!stream)
		return;

	int startNum = stream->readUint16LE();
	int numEntries = stream->readUint16LE();
	assert((startNum < 256) && ((startNum + numEntries) <= 256));
	stream->skip(2);

	// Copy over the data
	stream->read(&palette[startNum * 3], numEntries * 3);

	delete stream;
}

/**
 * Open up the given resource file using a passed file object. If the desired entry is found
 * in the index, return the index entry for it, and move the file to the start of the resource
 */
bool TLib::scanIndex(Common::File &f, ResourceType resType, int rlbNum, int resNum,
									  ResourceEntry &resEntry) {
	// Load the root section index
	ResourceList resList;
	loadSection(f, resList);

	// Loop through the index for the desired entry
	ResourceList::iterator iter;
	for (iter = resList.begin(); iter != resList.end(); ++iter) {
		ResourceEntry &re = *iter;
		if (re.id == resNum) {
			// Found it, so exit
			resEntry = re;
			f.seek(re.fileOffset);
			return true;
		}
	}

	// No matching entry found
	return false;
}

/**
 * Inner logic for decoding a section index into a passed resource list object
 */
void TLib::loadSection(Common::File &f, ResourceList &resources) {
	if (f.readUint32BE() != 0x544D492D)
		error("Data block is not valid Rlb data");

	/*uint8 unknown1 = */f.readByte();
	uint16 numEntries = f.readByte();

	for (uint i = 0; i < numEntries; ++i) {
		uint16 id = f.readUint16LE();
		uint16 size = f.readUint16LE();
		uint16 uncSize = f.readUint16LE();
		uint8 sizeHi = f.readByte();
		uint8 type = f.readByte() >> 5;
		assert(type <= 1);
		uint32 offset = f.readUint32LE();

		ResourceEntry re;
		re.id = id;
		re.fileOffset = offset;
		re.isCompressed = type != 0;
		re.size = ((sizeHi & 0xF) << 16) | size;
		re.uncompressedSize = ((sizeHi & 0xF0) << 12) | uncSize;

		resources.push_back(re);
	}
}

} // end of namespace TsAGE
} // end of namespace Scalpel
} // end of namespace Sherlock
