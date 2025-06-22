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


#ifndef DIRECTOR_CHUNK_H
#define DIRECTOR_CHUNK_H

namespace Common {
struct Rect;
class SeekableReadStreamEndian;
class SeekableMemoryWriteStream;
}

namespace Director {

class Archive;
class Cast;
struct CastMemberInfo;
class CastMember;
class DirectorEngine;
class Lingo;
struct LingoArchive;
struct LingoEvent;
class ScriptContext;
class Window;
struct Symbol;

enum ArchiveChunkType {
	kCastChunk,
	// kCastListChunk,
	// kCastMemberChunk,
	// kCastInfoChunk,
	kConfigChunk,
	// kInitialMapChunk,
	// kKeyTableChunk,
	// kMemoryMapChunk,
	// kScriptChunk,
	// kScriptContextChunk,
	// kScriptNamesChunk
};

struct Chunk {
	Archive *dir;
	ArchiveChunkType _chunkType;
	bool _writable;

	Chunk(Archive *d, ArchiveChunkType t) : dir(d), _chunkType(t), _writable(false) {}
	virtual ~Chunk() = default;
	virtual void read(Common::SeekableReadStreamEndian &stream) = 0;
	virtual uint16 size() { return 0; }
	virtual void write(Common::SeekableMemoryWriteStream&) {}
	
};

struct ConfigChunk : Chunk {
	/*  0 */ uint16 len;
	/*  2 */ uint16 fileVersion;
	/*  4, 6, 8, 10 */ Common::Rect checkRect; 
    /* 12 */ uint16 castArrayStart;
	/* 14 */ uint16 castArrayEnd;
	/* 16 */ byte readRate;
	/* 17 */ byte lightswitch;

	// Director 6 and below
		/* 18 */ int16 unk1;	// Mentioned in ProjectorRays as preD7field11 
	
	// Director 7 and above
	// Currently not supporting Director 7
		// /* 18 */ int8 D7stageColorG;
		// /* 19 */ int8 D7stageColorB;

	/* 20 */ uint16 commentFont;
	/* 22 */ uint16 commentSize;
	/* 24 */ uint16 commentStyle;

	// Director 6 and below
		/* 26 */ uint16 stageColor;
	// Director 7 and above
		/* 26 */ uint8 D7stageColorIsRGB;
		/* 27 */ uint8 D7stageColorR;

	/* 28 */ uint16 bitDepth;
	/* 30 */ uint8 field17;
	/* 31 */ uint8 field18;
	/* 32 */ int32 field19;
	/* 36 */ int16 directorVersion;
	/* 38 */ int16 field21;
	/* 40 */ int32 field22;
	/* 44 */ int32 field23;
	/* 48 */ int32 field24;
	/* 52 */ int8 field25;
	/* 53 */ uint8 field26;
	/* 54 */ int16 frameRate;
	/* 56 */ int16 platformID;
	/* 58 */ int16 protection;
	/* 60 */ int32 field29;
	/* 64 */ uint32 checksum;
	/* 68 */ uint16 field30;	// Marked as remnants in ProjectorRays

	ConfigChunk(Archive *m) : Chunk(m, kConfigChunk) {
		_writable = true;
	}
	~ConfigChunk() = default;
	void read(Common::SeekableReadStreamEndian &stream) { return; };
	uint16 size() { return len; }
	void write(Common::SeekableMemoryWriteStream &stream) { return; };
	uint32 computeChecksum() { return 0; };
};

}   // End of namespace Director

#endif