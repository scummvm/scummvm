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

#ifndef MEDIASTATION_DATAFILE_H
#define MEDIASTATION_DATAFILE_H

#include "common/file.h"
#include "common/stream.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/str.h"

namespace MediaStation {

// The version number of this engine,
// in the form 4.0r8 (major . minor r revision).
struct VersionInfo {
	uint16 major = 0;
	uint16 minor = 0;
	uint16 patch = 0;
};

typedef Common::Array<Common::Point> Polygon;

// A Media Station datafile consists of one or more RIFF-style "subfiles". Aside
// from some oddness at the start of the subfile, each subfile is basically
// standard sequence of chunks inside a LIST chunk, like you'd see in any RIFF
// file. These chunks have special IDs:
//  - igod: Indicates a chunk that contains metadata about actor(s) in metadata sections.
//  - a000, where 000 is a string that represents a 3-digit hexadecimal number.
//          Indicates a chunk that contains actor data (sounds and bitmaps).

enum DatumType {
	kDatumTypeEmpty = 0x00,
	kDatumTypeUint8 = 0x02,
	kDatumTypeUint16 = 0x03,
	kDatumTypeUint32 = 0x04,
	kDatumTypeInt8 = 0x05,
	kDatumTypeInt16 = 0x06,
	kDatumTypeInt32 = 0x07,
	kDatumTypeFloat = 0x08,
	kDatumTypeDouble = 0x09,
	kDatumTypeFilename = 0x0a,
	kDatumTypeRect = 0x0d,
	kDatumTypePoint = 0x0e,
	kDatumTypeGraphicSize = 0x0f,
	kDatumTypeGraphicUnit = 0x10,
	kDatumTypeTime = 0x11,
	kDatumTypeString = 0x12,
	kDatumTypeVersion = 0x13,
	kDatumTypeChannelIdent = 0x1b,
	kDatumTypePolygon = 0x1d,
};

class ParameterReadStream : public Common::SeekableReadStream {
public:
	// Data files are internally little-endian, even on game versions targeting
	// big-endian systems. The original engine has code for swapping byte order
	// at runtime when needed. All of these internally assume the data files are
	// stored little-endian on disk.
	byte readTypedByte();
	uint16 readTypedUint16();
	uint32 readTypedUint32();
	int8 readTypedSByte();
	int16 readTypedSint16();
	int32 readTypedSint32();
	float readTypedFloat();
	double readTypedDouble();
	Common::String readTypedFilename();
	Common::Rect readTypedRect();
	Common::Point readTypedPoint();
	Common::Point readTypedGraphicSize();
	int16 readTypedGraphicUnit();
	double readTypedTime();
	Common::String readTypedString();
	VersionInfo readTypedVersion();
	uint32 readTypedChannelIdent();
	Polygon readTypedPolygon();

private:
	void readAndVerifyType(DatumType type);
};

class Chunk : public ParameterReadStream {
public:
	Chunk() = default;
	Chunk(Common::SeekableReadStream *stream);

	uint32 bytesRemaining();

	uint32 _id = 0;
	uint32 _length = 0;

	// ReadStream implementation
	virtual bool eos() const { return _parentStream->eos(); };
	virtual bool err() const { return _parentStream->err(); };
	virtual void clearErr() { _parentStream->clearErr(); };
	virtual uint32 read(void *dataPtr, uint32 dataSize);
	virtual int64 pos() const { return _parentStream->pos(); };
	virtual int64 size() const { return _parentStream->size(); };
	virtual bool skip(uint32 offset) { return seek(offset, SEEK_CUR); };
	virtual bool seek(int64 offset, int whence = SEEK_SET);

private:
	Common::SeekableReadStream *_parentStream = nullptr;
	uint32 _dataStartOffset = 0;
	uint32 _dataEndOffset = 0;
};

class Subfile {
public:
	Subfile() = default;
	Subfile(Common::SeekableReadStream *stream);

	Chunk nextChunk();
	bool atEnd();

	Chunk _currentChunk;
	uint32 _rate;

private:
	Common::SeekableReadStream *_stream = nullptr;
	Chunk _rootChunk;
};

// The stream loading class hierarchy presented below is a bit complex for reading directly
// from streams, like we can do on modern computers, without needing to worry about
// buffering from CD-ROM. But we are staying close to the original logic and class
// hierarchy where possible, so some of that original architecture is reflected here.
typedef uint32 ChannelIdent;

class CdRomStream : public Common::File {
public:
	CdRomStream() {};
	void openStream(uint streamId);
	void closeStream() { close(); }

	Subfile getNextSubfile();

private:
	uint _fileId = 0;
};

class ChannelClient {
public:
	virtual ~ChannelClient() {};

	void setChannelIdent(ChannelIdent channelIdent) { _channelIdent = channelIdent; }
	ChannelIdent channelIdent() const { return _channelIdent; }

	virtual void readChunk(Chunk &chunk) {};

	void registerWithStreamManager();
	void unregisterWithStreamManager();

protected:
	ChannelIdent _channelIdent = 0;
};

class StreamFeed {
public:
	StreamFeed(uint streamId) : _id(streamId) {};
	virtual ~StreamFeed() {};

	virtual void openFeed(uint streamId, uint startOffset) = 0;

	// The original also has forceCloseFeed, which doesn't do some other cleanup
	// that the regular closeFeed does. However, since we are not doing caching and
	// some other functionality in the original, we don't need this.
	virtual void closeFeed() = 0;
	virtual void stopFeed() = 0;
	virtual void readData() = 0;

	uint _id = 0;

protected:
	CdRomStream *_stream = nullptr;
};

class ImtStreamFeed : public StreamFeed {
public:
	ImtStreamFeed(uint streamId);
	~ImtStreamFeed();

	virtual void openFeed(uint streamId, uint startOffset) override;
	virtual void closeFeed() override;
	// This implementation is currently empty because all this has to do with read timing.
	virtual void stopFeed() override {};
	virtual void readData() override;
};

class StreamFeedManager {
public:
	~StreamFeedManager();

	void registerChannelClient(ChannelClient *client);
	void unregisterChannelClient(ChannelClient *client);
	ChannelClient *channelClientForChannel(uint clientId) { return _channelClients.getValOrDefault(clientId); }

	ImtStreamFeed *openStreamFeed(uint streamId, uint offsetInStream = 0, uint maxBytesToRead = 0);
	void closeStreamFeed(StreamFeed *streamFeed);

private:
	Common::HashMap<uint, StreamFeed *> _streamFeeds;
	Common::HashMap<ChannelIdent, ChannelClient *> _channelClients;
};

} // End of namespace MediaStation

#endif
