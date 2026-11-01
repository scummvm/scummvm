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

#ifndef GRIM_MOVIE_SMUSH_ANIMATOR_H
#define GRIM_MOVIE_SMUSH_ANIMATOR_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Grim {

class SmushDecoder;
class Blocky8;
class Codec48Decoder;

struct SmushDemoBitmap {
	byte *pixels;
	int width;
	int height;
	int size;
};

struct SmushDemoRect {
	int x;
	int y;
	int width;
	int height;
};

class SmushDemoClient {
public:
	virtual ~SmushDemoClient() {}

	virtual int preFrame(int frame, int frameCount) = 0;
	virtual int postFrame(int frame, int frameCount) = 0;
	virtual int frameCallback(int flags, bool paletteChanged, bool display, int paletteState) = 0;
	virtual int chunkCallback(const byte *chunk, int size, int flags) = 0;
	virtual void setAnnotation(int value) = 0;
	virtual void setFrameRate(int fps) = 0;
	virtual void setFrameBuffer(byte *buffer, int x, int y, int sourceX, int sourceY, int width, int height) = 0;
};

class SmushDemoStream {
private:
	byte *_ring;
	int _ringSize;
	int _maxRead;
	int _writePos;
	int _readPos;
	int _held;
	int _rewindPos;
	int _alignSkip;
	Common::SeekableReadStream *_file;
	bool _fileIsMemory;
	int _fileOffset;
	int _fileSize;
	int _busy;
	int _restart;
	int _hasData;
	int _released;
	int _refCount;
	bool _openPending;
	int _openOffset;
	Common::String _openName;

	bool streamHasSpace(int size);
	int streamDoOpen();

public:
	SmushDemoStream();
	~SmushDemoStream();

	int streamInitialize(int ringSize, int maxRead);
	void streamTerminate();
	void streamInitMemory();
	void streamReset();
	void streamOpen(const Common::String &name, int offset);
	bool streamFill(int limit);
	int streamRead(int size, const byte **data);
	int streamRewind();
	bool streamIsReady();
};

class SmushDemoAnim {
private:
	SmushDemoStream *_stream;
	SmushDemoClient *_client;
	Blocky8 *_blocky8;
	Codec48Decoder *_codec48;

	int _refCount;
	SmushDemoBitmap *_bitmap;
	SmushDemoRect _rect;
	const byte *_block;
	int _blockStatus;
	int _frame;
	int _frameCount;
	int _blockSize;
	int _tag;
	int _originX;
	int _originY;
	int _newAnim;
	int _options;
	int _seeking;
	int _skipping;
	int _quit;
	int _pause;
	int _dirty;
	int _skipUntil;
	int _skipBase;
	int _skipLength;
	int _flags;

	byte *_overlay;
	byte *_preload;
	int _overlaySize;
	int _preloadSize;
	int _preloadUsed;
	int _preloadCursor;
	int _preloadLast;
	int _preloadFirst;
	byte _updateFlags[128];

	byte _palette[768];
	uint16 _paletteAccum[768];
	uint16 _paletteDelta[768];
	int _paletteState;
	int _paletteRefresh;
	int _noDisplay;
	int _displayPending;

	int animatorReadBlock(int size);
	int animatorCheckSkip();
	int animatorDecodeBlocks(int x, int y, int flags);
	void animatorHandleHeader(const byte *header);
	int anmFrameObject(int x, int y, int flags, const byte *chunk, int size);
	void anmStoreObject(byte *destination, const byte *chunk);
	void anmDrawObject(int x, int y, const byte *object, int flags);
	void anmPreloadPut(const byte *chunk, int size);
	bool anmTestUpdateFlag(int index);

public:
	SmushDemoAnim(SmushDemoStream *stream, SmushDemoClient *client);
	~SmushDemoAnim();

	int anmInitialize(int width, int height);
	int anmTerminate();
	void animatorInitialize();
	int animatorRender(int flags);
	void animatorSetOrigin(int x, int y);
	void animatorNewAnim(const Common::String &name, int options, int skipLength, int skipUntil, int seeking, int offset);
	void animatorSetSkip(int skipLength, int skipUntil);
	void animatorResetView();
	int animatorPause(int mode);
	int anmBuffersInitialize(int overlaySize, int preloadSize);
	void anmBuffersTerminate();
	void anmClearUpdateFlags(int index);
	int anmDecodeBody(int x, int y, const byte *block, int size, int flags);
	const byte *anmPreloadGet(int size);
	void anmDisplay(int flags);
	void anmSetPalette(const byte *palette, int mode);
	const byte *anmGetPalette() const { return _palette; }
	int anmRegisterCodecs(int width, int height);
	void anmUnregisterCodecs();
	const SmushDemoBitmap *getBitmap() const { return _bitmap; }
	int getFrame() const { return _frame; }
	int getFrameCount() const { return _frameCount; }
};

class SmushAnimator {

private:
	SmushDecoder *_decoder;
	byte *_buffer;
	uint32 _bufferSize;
	byte *_block;
	int _blockStatus;
	int _frame;
	int _frameCount;
	int _blockSize;
	uint32 _tag;
	int _flags;
	int _pause;

	void animatorInitialize();
	void readBlock(int size);
	void decodeBlocks(int flags);
	bool moreData();

public:
	SmushAnimator(SmushDecoder *decoder);
	~SmushAnimator();

	void animatorNewAnim(int flags);
	int animatorRender(int flags);
	void animatorPause(bool pause);

};

} // End of namespace Grim
#endif
