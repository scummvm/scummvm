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

#include "common/debug.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/bufferedstream.h"
#include "common/substream.h"

#include "common/compression/gentee_installer.h"

namespace Common {

namespace GenteeInstaller {

struct HuffmanTreeNode {
	HuffmanTreeNode();

	HuffmanTreeNode *_parent;
	HuffmanTreeNode *_children[2];

	// NOTE: This must be signed! The decoder uses a broken normalization algorithm which halves all frequency
	// counts (instead of only halving leaf counts and recomputing parent nodes), which causes parent nodes
	// to desync if both children have odd frequencies.  This can cause the rebalancing algorithm to assign
	// negative numbers to frequencies, which in turn affects other comparisons through the decoder.
	//
	// We need to accurately replicate this bug to get the correct decode behavior.
	int32 _freq;

	uint16 _symbol;
};

HuffmanTreeNode::HuffmanTreeNode() : _parent(nullptr), _children{nullptr, nullptr}, _freq(0), _symbol(0) {
}

class HuffmanTree {
public:
	HuffmanTreeNode *_treeRoot;
	HuffmanTreeNode *_nodes;

	void incrementFreqCount(uint16 symbol);

protected:
	void initTree(HuffmanTreeNode *nodes, uint16 numLeafs);

private:
	void buildTree();
	static void rebalanceTree(HuffmanTreeNode *entry, HuffmanTreeNode *entryParentSibling);

	uint32 _numNodes;
	int32 _maxFreq;
	uint16 _numLeafs;
};

void HuffmanTree::buildTree() {
	HuffmanTreeNode *entries = _nodes;

	uint32 numResultingNodes = _numLeafs * 2 - 1;
	for (uint32 i = 0; i < numResultingNodes; i++)
		entries[i]._parent = nullptr;

	for (uint32 nextNode = _numLeafs; nextNode < numResultingNodes; nextNode++) {
		HuffmanTreeNode *lowest = nullptr;
		HuffmanTreeNode *secondLowest = nullptr;

		for (uint32 ci = 0; ci < nextNode; ci++) {
			HuffmanTreeNode *candidate = entries + ci;

			if (candidate->_parent == nullptr) {
				if (lowest == nullptr)
					lowest = candidate;
				else {
					if (candidate->_freq < lowest->_freq) {
						secondLowest = lowest;
						lowest = candidate;
					} else if (secondLowest == nullptr || candidate->_freq < secondLowest->_freq)
						secondLowest = candidate;
				}
			}
		}

		HuffmanTreeNode *newEntry = entries + nextNode;
		newEntry->_freq = lowest->_freq + secondLowest->_freq;
		newEntry->_symbol = nextNode;
		newEntry->_children[0] = lowest;
		newEntry->_children[1] = secondLowest;
		lowest->_parent = newEntry;
		secondLowest->_parent = newEntry;
	}

	_numNodes = numResultingNodes;
	_treeRoot = entries + numResultingNodes - 1;
}

void HuffmanTree::incrementFreqCount(uint16 symbol) {
	for (HuffmanTreeNode *checkEntry = _nodes + symbol; checkEntry; checkEntry = checkEntry->_parent) {
		HuffmanTreeNode *entryParent = checkEntry->_parent;

		if (checkEntry->_parent) {
			HuffmanTreeNode *parentOfParent = entryParent->_parent;

			if (parentOfParent) {
				HuffmanTreeNode *entryParentSibling = parentOfParent->_children[0];

				if (entryParent == entryParentSibling)
					entryParentSibling = parentOfParent->_children[1];

				if (entryParentSibling->_freq <= checkEntry->_freq)
					rebalanceTree(checkEntry, entryParentSibling);
			}
		}
		checkEntry->_freq = checkEntry->_freq + 1;
	}

	if (_maxFreq <= _treeRoot->_freq) {
		HuffmanTreeNode *entries = _nodes;

		for (uint i = 0; i < _numNodes; i++)
			entries[i]._freq >>= 1;
	}
}

void HuffmanTree::rebalanceTree(HuffmanTreeNode *entry, HuffmanTreeNode *entryParentSibling) {
	for (uint pass = 0; pass < 2; pass++) {
		HuffmanTreeNode *entryParent = entry->_parent;

		HuffmanTreeNode *entryParentChild0 = entryParent->_children[0];
		HuffmanTreeNode *entrySibling = entryParentChild0;

		if (entry == entryParentChild0)
			entrySibling = entryParent->_children[1];

		HuffmanTreeNode *entryParentSiblingHighestFreqChild = entryParentSibling->_children[0];
		if (entryParentSiblingHighestFreqChild && entryParentSiblingHighestFreqChild->_freq <= entryParentSibling->_children[1]->_freq)
			entryParentSiblingHighestFreqChild = entryParentSibling->_children[1];

		if (entry == entryParentChild0)
			entryParent->_children[0] = entryParentSibling;
		else
			entryParent->_children[1] = entryParentSibling;

		HuffmanTreeNode *entryParentParent = entryParent->_parent;

		if (entryParentParent->_children[0] == entryParentSibling)
			entryParentParent->_children[0] = entry;
		else
			entryParentParent->_children[1] = entry;

		entry->_parent = entryParentParent;
		entryParentSibling->_parent = entryParent;

		if (pass > 0 || entryParentSiblingHighestFreqChild == nullptr || entryParentSiblingHighestFreqChild->_freq <= entrySibling->_freq)
			break;

		entryParentSibling->_freq = entryParentSibling->_freq + entrySibling->_freq - entryParentSiblingHighestFreqChild->_freq;
		entry = entryParentSiblingHighestFreqChild;
		entryParentSibling = entrySibling;
	}
}

void HuffmanTree::initTree(HuffmanTreeNode *nodes, uint16 numLeafs) {

	_numLeafs = numLeafs;
	_nodes = nodes;
	_maxFreq = 512;

	for (uint16 i = 0; i < numLeafs; i++) {
		HuffmanTreeNode *entry = &nodes[i];
		entry->_symbol = i;
		entry->_freq = i + 1;
	}
	buildTree();
}

template<uint TNumLeafs>
class HuffmanTreePresized : public HuffmanTree {
public:
	HuffmanTreePresized();

	void reset();

private:
	HuffmanTreeNode _preallocNodes[TNumLeafs * 2];
};

template<uint TNumLeafs>
HuffmanTreePresized<TNumLeafs>::HuffmanTreePresized() {
	reset();
}

template<uint TNumLeafs>
void HuffmanTreePresized<TNumLeafs>::reset() {
	initTree(_preallocNodes, TNumLeafs);
}

class DecompressorState
{
public:
	explicit DecompressorState(Common::ReadStream *inputStream);

	void resetEverything();
	void resetBitstream();

	uint decompressBytes(void *dest, uint size);

private:
	static const uint kWindowSize = 32767;
	static const uint kMatchHistorySize = 4;
	static const uint kNumMatchVLCs = 30;

	HuffmanTreePresized<274> _codeTree;
	HuffmanTreePresized<34> _offsetTree;
	HuffmanTreePresized<237> _lengthTree;

	uint32 _matchOffsetHistory[kMatchHistorySize];

	uint16 _windowOffset;
	byte _window[kWindowSize];

	Common::ReadStream *_inputStream;

	static byte g_matchVLCLengths[kNumMatchVLCs];
	uint16 _matchVLCOffsets[kNumMatchVLCs];

private:
	uint16 decodeDynamicHuffmanValue(HuffmanTree *tree);
	void recordMatchOffset(uint matchOffset);
	byte readBit();
	uint16 readBits(uint16 numBits);
	void emitByte(void *dest, byte b);

	uint _matchReadPos;
	uint _matchRemaining;

	byte _bitstreamByte;
	byte _bitstreamBitsRemaining;
	bool _failed;
};

DecompressorState::DecompressorState(Common::ReadStream *inputStream)
	: _inputStream(inputStream), _bitstreamBitsRemaining(0), _bitstreamByte(0), _matchRemaining(0), _matchReadPos(0), _failed(false), _matchOffsetHistory{0, 0, 0, 0}, _windowOffset(0) {
	resetEverything();
}

void DecompressorState::resetEverything() {
	resetBitstream();

	for (byte &b : _window)
		b = 0;

	_windowOffset = 0;
	
	uint16 nextValue = 0;
	for (int i = 0; i < 30; i++) {
		_matchVLCOffsets[i] = nextValue;
		nextValue = nextValue + (1 << (g_matchVLCLengths[i] & 0x1f));
	}

	for (uint32 i = 0; i < 4; i++)
		_matchOffsetHistory[i] = i;

	_codeTree.reset();
	_offsetTree.reset();
	_lengthTree.reset();

	_failed = false;
}

void DecompressorState::resetBitstream() {
	_bitstreamBitsRemaining = 0;
	_bitstreamByte = 0;
}


uint DecompressorState::decompressBytes(void *dest, uint size) {
	uint remaining = size;

	while (remaining > 0) {
		if (_failed)
			break;

		if (_matchRemaining > 0) {
			if (_matchReadPos == kWindowSize)
				_matchReadPos = 0;

			emitByte(dest, _window[_matchReadPos]);
			dest = static_cast<byte *>(dest) + 1;
			remaining--;
			_matchRemaining--;
			_matchReadPos++;
			continue;
		} else {
			uint16 code = decodeDynamicHuffmanValue(&_codeTree);

			if (code < 256) {
				if (_failed)
					break;

				emitByte(dest, static_cast<byte>(code));
				dest = static_cast<byte *>(dest) + 1;
				remaining--;
			} else {
				uint matchLength = 0;
				if (code > 272)
					matchLength = decodeDynamicHuffmanValue(&_lengthTree) + 20;
				else
					matchLength = code - 253;

				code = decodeDynamicHuffmanValue(&_offsetTree);

				uint matchOffset = 0;
				if (code < 30) {
					// Coded offset
					matchOffset = readBits(g_matchVLCLengths[code]) + _matchVLCOffsets[code];
				} else {
					// Historic offset
					matchOffset = _matchOffsetHistory[code - 30];
				}

				uint backDistance = matchLength + matchOffset;
				_matchReadPos = _windowOffset;

				while (_matchReadPos < backDistance)
					_matchReadPos += kWindowSize;

				_matchReadPos -= backDistance;
				_matchRemaining = matchLength;

				recordMatchOffset(matchOffset);
			}
		}
	}

	return size - remaining;
}

uint16 DecompressorState::decodeDynamicHuffmanValue(HuffmanTree *tree) {
	HuffmanTreeNode *node = tree->_treeRoot;
	while (node->_children[0] != nullptr)
		node = node->_children[readBit()];

	tree->incrementFreqCount(node->_symbol);
	return node->_symbol;
}

byte DecompressorState::readBit() {
	if (_bitstreamBitsRemaining == 0) {
		if (_failed)
			return 0;

		if (!_inputStream->read(&_bitstreamByte, 1)) {
			_failed = true;
			return 0;
		}

		_bitstreamBitsRemaining = 7;
	} else
		_bitstreamBitsRemaining--;

	byte bit = _bitstreamByte >> 7;
	_bitstreamByte <<= 1;
	return bit;
}

uint16 DecompressorState::readBits(uint16 numBits) {
	uint16 result = 0;
	uint16 bitToInsert = 1;

	while (numBits > 0) {
		if (readBit())
			result |= bitToInsert;

		bitToInsert <<= 1;
		numBits--;
	}

	return result;
}

void DecompressorState::emitByte(void *dest, byte b) {
	*static_cast<byte *>(dest) = b;

	_window[_windowOffset] = b;
	_windowOffset++;

	if (_windowOffset == kWindowSize)
		_windowOffset = 0;
}

void DecompressorState::recordMatchOffset(uint matchOffset) {
	uint expungeIndex = kMatchHistorySize - 1;

	for (uint i = 0; i < kMatchHistorySize - 1u; i++) {
		if (_matchOffsetHistory[i] == matchOffset) {
			expungeIndex = i;
			break;
		}
	}

	if (expungeIndex == 0)
		return;

	for (uint i = 0; i < expungeIndex; i++)
		_matchOffsetHistory[expungeIndex - i] = _matchOffsetHistory[expungeIndex - i - 1];

	_matchOffsetHistory[0] = matchOffset;
}

byte DecompressorState::g_matchVLCLengths[DecompressorState::kNumMatchVLCs] = {
	0, 1, 1, 2, 2,
	2, 3, 3, 3, 3,
	4, 4, 5, 5, 6,
	6, 7, 7, 8, 8,
	9, 9, 10, 10, 11,
	11, 12, 12, 13, 13
};

class DecompressingStream : public Common::SeekableReadStream {
public:
	DecompressingStream(Common::SeekableReadStream *baseStream, uint32 compressedSize, uint32 decompressedSize);
	~DecompressingStream();

	int64 pos() const override;
	int64 size() const override;
	bool seek(int64 offset, int whence) override;
	bool skip(uint32 offset) override;
	bool eos() const override;
	uint32 read(void *dataPtr, uint32 dataSize) override;
	bool err() const override;
	void clearErr() override;

private:
	bool rewind();

	DecompressorState _decomp;

	Common::SeekableReadStream *_baseStream;

	uint32 _pos;
	uint32 _compressedSize;
	uint32 _decompressedSize;
	bool _eosFlag;
	bool _errFlag;
};

DecompressingStream::DecompressingStream(Common::SeekableReadStream *baseStream, uint32 compressedSize, uint32 decompressedSize)
	: _baseStream(baseStream), _compressedSize(compressedSize), _decompressedSize(decompressedSize), _pos(0), _eosFlag(false), _errFlag(false), _decomp(baseStream) {
}

DecompressingStream::~DecompressingStream() {
	delete _baseStream;
}

int64 DecompressingStream::pos() const {
	return _pos;
}

int64 DecompressingStream::size() const {
	return _decompressedSize;
}

bool DecompressingStream::seek(int64 offset, int whence) {
	switch (whence) {
	case SEEK_SET:
		if (offset == _pos)
			return true;

		if (offset < 0)
			return false;

		if (offset == 0)
			return rewind();

		if (offset > static_cast<int64>(_decompressedSize))
			return false;

		if (offset == static_cast<int64>(_decompressedSize)) {
			// Just set position to EOF
			_pos = _decompressedSize;
			return true;
		}

		if (offset < static_cast<int64>(_pos)) {
			if (!rewind())
				return false;
		}

		return skip(static_cast<uint32>(offset) - _pos);
	case SEEK_END:
		return seek(static_cast<int64>(_decompressedSize) + offset, SEEK_SET);
	case SEEK_CUR:
		return seek(static_cast<int64>(_pos) + offset, SEEK_SET);
	default:
		return false;
	}
}

bool DecompressingStream::skip(uint32 offset) {
	const uint kSkipBufSize = 1024;
	byte skipBuf[kSkipBufSize];

	while (offset > 0) {
		uint32 skipAmount = kSkipBufSize;
		if (skipAmount > offset)
			skipAmount = offset;

		uint32 amountRead = read(skipBuf, skipAmount);
		if (amountRead != skipAmount)
			return false;

		offset -= amountRead;
	}

	return true;
}

bool DecompressingStream::eos() const {
	return _eosFlag;
}

uint32 DecompressingStream::read(void *dataPtr, uint32 dataSize) {
	if (_errFlag)
		return 0;

	uint32 bytesAvailable = _decompressedSize - _pos;

	if (dataSize > bytesAvailable) {
		_eosFlag = true;
		dataSize = bytesAvailable;
	}

	if (dataSize == 0)
		return 0;

	uint32 numBytesDecompressed = _decomp.decompressBytes(dataPtr, dataSize);
	if (numBytesDecompressed < dataSize)
		_errFlag = true;

	_pos += numBytesDecompressed;

	return numBytesDecompressed;
}

bool DecompressingStream::err() const {
	return _errFlag;
}

void DecompressingStream::clearErr() {
	_errFlag = false;
	_eosFlag = false;
	_baseStream->clearErr();
}

bool DecompressingStream::rewind() {
	if (!_baseStream->seek(0)) {
		_errFlag = true;
		return false;
	}

	_decomp.resetEverything();
	_pos = 0;
	return true;
}

class ArchiveItem : public Common::ArchiveMember {
public:
	ArchiveItem(Common::SeekableReadStream *stream, Common::Mutex *guardMutex, const Common::String &path, const Common::String &name, int64 filePos, uint compressedSize, uint decompressedSize, bool isCompressed);

	Common::SeekableReadStream *createReadStream() const override;
	Common::String getName() const override;

	const Common::String &getPath() const;

private:
	Common::SeekableReadStream *_stream;
	Common::Mutex *_guardMutex;
	Common::String _path;
	Common::String _name;
	int64 _filePos;
	uint _compressedSize;
	uint _decompressedSize;
	bool _isCompressed;
};

ArchiveItem::ArchiveItem(Common::SeekableReadStream *stream, Common::Mutex *guardMutex, const Common::String &path, const Common::String &name, int64 filePos, uint compressedSize, uint decompressedSize, bool isCompressed)
	: _stream(stream), _guardMutex(guardMutex), _path(path), _name(name), _filePos(filePos), _compressedSize(compressedSize), _decompressedSize(decompressedSize), _isCompressed(isCompressed) {
}

Common::SeekableReadStream *ArchiveItem::createReadStream() const {
	Common::SeekableReadStream *sliceSubstream = nullptr;

	if (_guardMutex)
		sliceSubstream = new Common::SafeMutexedSeekableSubReadStream(_stream, static_cast<uint32>(_filePos), static_cast<uint32>(_filePos) + _compressedSize, DisposeAfterUse::NO, *_guardMutex);
	else
		sliceSubstream = new Common::SeekableSubReadStream(_stream, static_cast<uint32>(_filePos), static_cast<uint32>(_filePos) + _compressedSize, DisposeAfterUse::NO);

	// Add buffering since seekable substreams can be extremely slow!
	sliceSubstream = Common::wrapBufferedSeekableReadStream(sliceSubstream, 4096, DisposeAfterUse::YES);

	if (_isCompressed)
		return new DecompressingStream(sliceSubstream, _compressedSize, _decompressedSize);
	else
		return sliceSubstream;
}

Common::String ArchiveItem::getName() const {
	return _name;
}

const Common::String &ArchiveItem::getPath() const {
	return _path;
}

class PackageArchive : public Common::Archive {
public:
	explicit PackageArchive(Common::SeekableReadStream *stream);
	~PackageArchive();

	bool load(const char *prefix);

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	int listMatchingMembers(Common::ArchiveMemberList &list, const Common::Path &pattern, bool matchPathComponents) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

protected:
	virtual Common::Mutex *getGuardMutex();

private:
	bool decodeDataChunk(DecompressorState *decompState, Common::Array<byte> &data);
	static Common::String normalizePath(const Common::Path &path);

	Common::Array<Common::SharedPtr<ArchiveItem> > _items;
	Common::HashMap<Common::String, uint> _pathToItemIndex;

	Common::SeekableReadStream *_stream;
};

PackageArchive::PackageArchive(Common::SeekableReadStream *stream) : _stream(stream) {
}

PackageArchive::~PackageArchive() {
	delete _stream;
}

bool PackageArchive::load(const char *prefix) {
	byte pakFileSizeBytes[4];

	int64 pakFileStartPos = _stream->pos();
	int64 maxPakFileSize = _stream->size() - pakFileStartPos;

	if (_stream->read(pakFileSizeBytes, 4) != 4) {
		warning("GenteeInstaller::PackageArchive::load: Couldn't read pak file size declaration");
		return false;
	}

	uint32 pakFileSize = READ_LE_UINT32(pakFileSizeBytes);

	if (static_cast<int64>(pakFileSize) < maxPakFileSize) {
		warning("GenteeInstaller::PackageArchive::load: Pak file size was larger than would be possible");
		return false;
	}

	if (pakFileStartPos != 0 || maxPakFileSize != pakFileSize) {
		_stream = new Common::SeekableSubReadStream(_stream, pakFileStartPos, static_cast<uint32>(pakFileStartPos) + pakFileSize, DisposeAfterUse::YES);
		if (!_stream->seek(4)) {
			warning("GenteeInstaller::PackageArchive::load: Couldn't reset pak position");
			return false;
		}
	}

	byte pakFileHeader[16];
	if (_stream->read(pakFileHeader, 16) != 16) {
		warning("GenteeInstaller::PackageArchive::load: Couldn't load pak header");
		return false;
	}

	Common::ScopedPtr<DecompressorState> cmdContextPtr(new DecompressorState(_stream));

	DecompressorState *cmdContext = cmdContextPtr.get();

	Common::Array<byte> firstChunk;

	if (!decodeDataChunk(cmdContext, firstChunk))
		return false;

	if (firstChunk.size() < 3) {
		warning("GenteeInstaller::PackageArchive::load: First chunk is malformed");
		return false;
	}

	bool allFilesAreStored = (firstChunk[0] != 0);

	// The second chunk appears to not be a commandlet either, should figure out what it is

	while (_stream->pos() < _stream->size()) {
		Common::Array<byte> commandletChunk;

		if (!decodeDataChunk(cmdContext, commandletChunk))
			return false;

		if (commandletChunk.size() < 3) {
			warning("GenteeInstaller::PackageArchive::load: Commandlet was malformed");
			return false;
		}

		uint16 commandletCode = READ_LE_UINT16(&commandletChunk[0]);

		if (commandletCode == 0x87f4) {
			// Unpack file commandlet
			if (commandletChunk.size() < 36 || commandletChunk.back() != 0) {
				warning("GenteeInstaller::PackageArchive::load: File commandlet was malformed");
				return false;
			}

			Common::String fileName = Common::String(reinterpret_cast<const char *>(&commandletChunk[34]));

			bool isCompressed = (!allFilesAreStored) && (commandletChunk[28] != 0);

			int64 dataStart = _stream->pos();
			int64 dataEnd = dataStart;
			uint32 decompressedSize = 0;

			if (isCompressed) {
				// Extremely annoying: The compressed data size isn't stored, so we must decompress the entire file
				// to find the next commandlet
				Common::ScopedPtr<DecompressorState> fileCtx(new DecompressorState(_stream));

				byte decompressedSizeBytes[4];
				if (_stream->read(decompressedSizeBytes, 4) != 4) {
					warning("GenteeInstaller::PackageArchive::load: Decompressed file size was malformed");
					return false;
				}

				decompressedSize = READ_LE_UINT32(decompressedSizeBytes);
				dataStart += 4;

				const uint kSkipBufSize = 1024;

				byte skipBuf[1024];
				uint32 skipRemaining = decompressedSize;
				while (skipRemaining > 0) {
					uint32 amountToSkip = skipRemaining;
					if (amountToSkip > kSkipBufSize)
						amountToSkip = kSkipBufSize;

					if (fileCtx->decompressBytes(skipBuf, amountToSkip) != amountToSkip) {
						warning("GenteeInstaller::PackageArchive::load: Couldn't decompress file data to skip it");
						return false;

					}

					skipRemaining -= amountToSkip;
				}

				dataEnd = _stream->pos();
			} else {
				decompressedSize = READ_LE_UINT32(&commandletChunk[7]);
				if (!_stream->skip(decompressedSize)) {
					warning("GenteeInstaller::PackageArchive::load: Failed to skip uncompressed file data");
					return false;
				}
				dataEnd = _stream->pos();
			}

			debug(3, "GenteeInstaller: Detected %s item '%s' size %u at pos %u .. %u", (isCompressed ? "compressed" : "stored"), fileName.c_str(), static_cast<uint>(decompressedSize), static_cast<uint>(dataStart), static_cast<uint>(dataEnd));

			if (fileName.hasPrefix(prefix)) {
				fileName = fileName.substr(strlen(prefix));

				size_t bsPos = fileName.findFirstOf('\\');
				while (bsPos != Common::String::npos) {
					fileName.replace(bsPos, 1, "/");
					bsPos = fileName.findFirstOf('\\');
				}

				Common::String fileNameNoDir = fileName;

				size_t lastSlashPos = fileNameNoDir.findLastOf('/');
				if (lastSlashPos != Common::String::npos)
					fileNameNoDir = fileNameNoDir.substr(lastSlashPos + 1);

				Common::SharedPtr<ArchiveItem> item(new ArchiveItem(_stream, getGuardMutex(), fileName, fileNameNoDir, dataStart, static_cast<uint>(dataEnd - dataStart), decompressedSize, isCompressed));

				fileName.toLowercase();
				_pathToItemIndex[fileName] = _items.size();

				_items.push_back(item);
			}
		}
	}

	return true;
}

bool PackageArchive::decodeDataChunk(DecompressorState *decompState, Common::Array<byte> &commandletData) {
	byte sizeBytes[4];
	if (_stream->read(sizeBytes, 4) != 4) {
		warning("GenteeInstaller::PackageArchive::load: Couldn't read commandlet size");
		return false;
	}

	uint32 size = READ_LE_UINT32(sizeBytes);

	if (size > 4 * 1024 * 1024) {
		warning("GenteeInstaller::PackageArchive::load: Commandlet was abnormally large, possibly corrupt data or a decompression bug");
		return false;
	}

	commandletData.resize(size);

	if (size > 0) {
		decompState->resetBitstream();
		if (decompState->decompressBytes(&commandletData[0], size) != size) {
			warning("GenteeInstaller::PackageArchive::load: Commandlet packet decompression failed");
			return false;
		}
	}

	return true;
}

Common::String PackageArchive::normalizePath(const Common::Path &path) {
	Common::String normalizedPath = path.toString();
	normalizedPath.toLowercase();
	return normalizedPath;
}

bool PackageArchive::hasFile(const Common::Path &path) const {
	return _pathToItemIndex.find(normalizePath(path)) != _pathToItemIndex.end();
}

int PackageArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (const Common::SharedPtr<ArchiveItem> &item : _items)
		list.push_back(item.staticCast<Common::ArchiveMember>());

	return _items.size();
}

int PackageArchive::listMatchingMembers(Common::ArchiveMemberList &list, const Common::Path &pattern, bool matchPathComponents) const {
	Common::String patternString = pattern.toString();
	int matches = 0;
	const char *wildcardExclusions = matchPathComponents ? NULL : "/";

	for (const Common::SharedPtr<ArchiveItem> &item : _items) {
		if (item->getPath().matchString(patternString, true, wildcardExclusions)) {
			list.push_back(item.staticCast<Common::ArchiveMember>());
			matches++;
		}
	}

	return matches;
}

const Common::ArchiveMemberPtr PackageArchive::getMember(const Common::Path &path) const {
	Common::HashMap<Common::String, uint>::const_iterator it = _pathToItemIndex.find(normalizePath(path));
	if (it == _pathToItemIndex.end())
		return nullptr;

	return _items[it->_value].staticCast<Common::ArchiveMember>();
}

Common::SeekableReadStream *PackageArchive::createReadStreamForMember(const Common::Path &path) const {
	const Common::ArchiveMemberPtr member = getMember(path);

	if (!member)
		return nullptr;

	return member->createReadStream();
}

Common::Mutex *PackageArchive::getGuardMutex() {
	return nullptr;
}


class ThreadSafePackageArchive : public PackageArchive {
public:
	explicit ThreadSafePackageArchive(Common::SeekableReadStream *stream);

protected:
	Common::Mutex *getGuardMutex() override;

private:
	Common::Mutex _guardMutex;
};

ThreadSafePackageArchive::ThreadSafePackageArchive(Common::SeekableReadStream *stream) : PackageArchive(stream) {
}

Common::Mutex *ThreadSafePackageArchive::getGuardMutex() {
	return &_guardMutex;
}

} // End of namespace GenteeInstaller



Common::Archive *createGenteeInstallerArchive(Common::SeekableReadStream *stream, const char *prefixToRemove, bool threadSafe) {
	if (!prefixToRemove)
		prefixToRemove = "";

	GenteeInstaller::PackageArchive *archive = nullptr;

	if (threadSafe)
		archive = new GenteeInstaller::ThreadSafePackageArchive(stream);
	else
		archive = new GenteeInstaller::PackageArchive(stream);

	if (!archive->load(prefixToRemove)) {
		delete archive;
		return nullptr;
	}

	return archive;
}

} // End of namespace Common
