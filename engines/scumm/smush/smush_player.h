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

#if !defined(SCUMM_SMUSH_PLAYER_H) && defined(ENABLE_SCUMM_7_8)
#define SCUMM_SMUSH_PLAYER_H

#include "common/util.h"
#include "common/list.h"
#include "common/rect.h"

namespace Audio {
class SoundHandle;
class QueuingAudioStream;
}

namespace Scumm {

#define SMUSH_MAX_TRACKS 4
#define SMUSH_FADE_SIZE  0xC00

#define IS_SFX       0x00
#define IS_BKG_MUSIC 0x40
#define IS_SPEECH    0x80

#define CHN_BKGMUS 1
#define CHN_SPEECH 2
#define CHN_OTHER  3

#define GRP_MASTER 0xFF01
#define GRP_SFX    0xFF02
#define GRP_BKGMUS 0xFF03
#define GRP_SPEECH 0xFF04

#define TRK_STATE_INACTIVE 0
#define TRK_STATE_PLAYING  1
#define TRK_STATE_FADING   2
#define TRK_STATE_ENDING   3

#define TRK_TYPE_MASK 0xC0

#define SAUD_OP_INIT          1
#define SAUD_OP_UPDATE_HEADER 2
#define SAUD_OP_SET_PARAM     3
#define SAUD_OP_INCR_PARAM    4
#define SAUD_OP_SET_OFFSET    6
#define SAUD_OP_SET_LENGTH    7
#define SAUD_OP_COMPARE_GT    8
#define SAUD_OP_COMPARE_LT    9
#define SAUD_OP_COMPARE_EQ    10
#define SAUD_OP_COMPARE_NE    11

#define SAUD_VALUEID_ALL_VOLS 0xFF
#define SAUD_VALUEID_TRK_VOL  0xFE
#define SAUD_VALUEID_TRK_PAN  0xFD

#define TRK_USERID_SPEECH 1
#define TRK_USERID_MUSIC  2
#define TRK_USERID_SFX    3

#define SMUSH_CODEC_RLE          1
#define SMUSH_CODEC_RLE_ALT      3
#define SMUSH_CODEC_UNCOMPRESSED 20
#define SMUSH_CODEC_LINE_UPDATE  21   // RA2: Skip/copy with literal pixels
#define SMUSH_CODEC_SKIP_RLE     23   // RA2: Skip/copy with embedded RLE
#define SMUSH_CODEC_DELTA_BLOCKS 37
#define SMUSH_CODEC_LINE_UPDATE2 44   // RA2: Variant of codec 21
#define SMUSH_CODEC_RA2_BOMP     45   // RA2: BOMP RLE with variable header
#define SMUSH_CODEC_DELTA_GLYPHS 47

class ScummEngine_v7;
class SmushFont;
class SmushMixer;
class StringResource;
class SmushDeltaBlocksDecoder;
class SmushDeltaGlyphsDecoder;
class IMuseDigital;
class Insane;

class SmushPlayer {
	friend class Insane;
	friend class InsaneRebel2;
private:
	struct SmushAudioDispatch {
		uint8 *headerPtr;
		uint8 *dataBuf;
		int32 dataSize;
		int32 audioRemaining;
		int32 currentOffset;
		int sampleRate;
		int state;
		int fadeSampleRate;
		int fadeVolume;
		int32 fadeRemaining;
		int volumeStep;
		int32 elapsedAudio;
		int32 audioLength;
	};

	struct SmushAudioTrack {
		uint8 *blockPtr;
		uint8 *fadeBuf;
		uint8 *dataBuf;
		uint8 *subChunkPtr;
		int32 blockSize;
		byte volume;
		byte pan;
		int16 state;
		int16 flags;
		int groupId;
		int parsedChunks;
		int32 dataSize;
		int32 availableSize;
		int32 audioRemaining;
		int32 sdatSize;
	};

	ScummEngine_v7 *_vm;
	IMuseDigital *_imuseDigital;
	Insane *_insane;
	int32 _nbframes;
	int16 _deltaPal[0x300];
	int32 _shiftedDeltaPal[0x300];
	byte _pal[0x300];
	SmushFont *_sf[5];
	StringResource *_strings;
	SmushDeltaBlocksDecoder *_deltaBlocksCodec;
	SmushDeltaGlyphsDecoder *_deltaGlyphsCodec;
	Common::SeekableReadStream *_base;
	uint32 _baseSize;
	byte *_frameBuffer;
	byte *_specialBuffer;
	int _specialBufferSize;

	Common::String _seekFile;
	uint32 _startFrame;
	uint32 _startTime;
	int32 _seekPos;
	uint32 _seekFrame;

	bool _skipNext;
	bool _ra2FastForwarding;  // Fast-forwarding RA2 BEG video to establish background
	uint32 _frame;

	Audio::SoundHandle *_IACTchannel;
	Audio::QueuingAudioStream *_IACTstream;

	Audio::SoundHandle *_compressedFileSoundHandle;
	bool _compressedFileMode;
	byte _IACToutput[4096];
	int32 _IACTpos;
	bool _storeFrame;
	int _speed;
	bool _endOfFile;

	byte *_dst;
	bool _updateNeeded;
	bool _warpNeeded;
	int _palDirtyMin, _palDirtyMax;
	int _warpX, _warpY;
	int _warpButtons;
	bool _insanity;
	bool _middleAudio;
	bool _skipPalette;
	int _iactTable[4];

	SmushAudioTrack _smushTracks[SMUSH_MAX_TRACKS];
	SmushAudioDispatch _smushDispatch[SMUSH_MAX_TRACKS];

	int _smushMaxFrames[SMUSH_MAX_TRACKS];
	int _smushTrackIds[SMUSH_MAX_TRACKS];
	int _smushTrackIdxs[SMUSH_MAX_TRACKS];
	uint8 _smushAudioTable[256];
	int _smushNumTracks;
	int _smushTrackFlags[SMUSH_MAX_TRACKS];
	int _smushTrackVols[SMUSH_MAX_TRACKS];

	int _smushAudioSampleRate;
	int _gainReductionLowerBound;
	int _gainReductionFactor;
	int _gainReductionMultiplier;

	bool _smushTracksNeedInit;
	bool _smushAudioInitialized;
	bool _smushAudioCallbackEnabled;

public:
	SmushPlayer(ScummEngine_v7 *scumm, IMuseDigital *_imuseDigital, Insane *insane);
	~SmushPlayer();

	void pause();
	void unpause();

	void play(const char *filename, int32 speed, int32 offset = 0, int32 startFrame = 0);
	void release();
	void warpMouse(int x, int y, int buttons);
	int setChanFlag(int id, int flagVal);
	void setGroupVolume(int groupId, int volValue);
	void processDispatches(int16 feedSize);
	bool isAudioCallbackEnabled();
	byte *getVideoPalette();
	void setCurVideoFlags(int16 flags);

	// Masked regions - areas where video should not update (e.g., destroyed enemies)
	// The Insane class can add/remove regions, and decodeFrameObject will restore
	// these areas from the previous frame after decoding
	void addMaskedRegion(const Common::Rect &rect);
	void removeMaskedRegion(const Common::Rect &rect);
	void clearMaskedRegions();
	Common::List<Common::Rect> _maskedRegions;


protected:
	int _width, _height;

	int _origPitch, _origNumStrips;
	bool _paused;
	uint32 _pauseStartTime;
	uint32 _pauseTime;
	int16 _curVideoFlags = 0;
	int _scrollX;
	int _scrollY;

	void insanity(bool);
	void setPalette(const byte *palette);
	void setPaletteValue(int n, byte r, byte g, byte b);
	void setDirtyColors(int min, int max);
	void setScrollOffset(int x, int y);
	void seekSan(const char *file, int32 pos, int32 contFrame);
	const char *getString(int id);

private:
	SmushFont *getFont(int font);
	void parseNextFrame();
	void init(int32 spped);
	void setupAnim(const char *file);
	void updateScreen();
	void tryCmpFile(const char *filename);

	bool readString(const char *file);
	void decodeFrameObject(int codec, const uint8 *src, int left, int top, int width, int height, int dataSize = 0);
	void handleAnimHeader(int32 subSize, Common::SeekableReadStream &);
	void handleFrame(int32 frameSize, Common::SeekableReadStream &);
	void handleNewPalette(int32 subSize, Common::SeekableReadStream &);
	void handleZlibFrameObject(int32 subSize, Common::SeekableReadStream &b);
	void handleFrameObject(int32 subSize, Common::SeekableReadStream &);
	void handleSAUDChunk(uint8 *srcBuf, uint32 size, int groupId, int vol, int pan, int16 flags, int trkId, int index, int maxFrames);
	void handleStore(int32 subSize, Common::SeekableReadStream &);
	void handleFetch(int32 subSize, Common::SeekableReadStream &);
	void handleIACT(int32 subSize, Common::SeekableReadStream &);
	void handleTextResource(uint32 subType, int32 subSize, Common::SeekableReadStream &);
	void handleDeltaPalette(int32 subSize, Common::SeekableReadStream &);
	void handleLoad(int32 subSize, Common::SeekableReadStream &);
	void readPalette(byte *, Common::SeekableReadStream &);

	// LOAD chunk streaming buffer (RA2 - embedded resource data)
	byte *_loadBuffer;        // Accumulated LOAD data
	int32 _loadBufferSize;    // Allocated buffer size
	int32 _loadBufferOffset;  // Current write position (how much data accumulated)
	int32 _loadReadOffset;    // Current read position (for streaming consumption)
	int16 _lastLoadChunkIdx;  // Last processed chunk index (-1 = none)
	int16 _totalLoadChunks;   // Total chunks expected in current sequence

	void initAudio(int samplerate, int32 maxChunkSize);
	void terminateAudio();
	int isChanActive(int flagId);
	int addAudioTrack(int32 trackBlockSize, int32 maxBlockSize);
	void resetAudioTracks();
	void setGainReductionParams(int16 gainReductionLowerBound, int16 gainReductionMultiplier);
	void fillAudioTrackInfo(uint8 *srcBuf, uint16 *flagsAccumulator, uint32 size, int groupId, int vol, int pan, int16 flags, int trkId, int index, int maxFrames);
	bool processAudioCodes(int idx, int32 &tmpFeedSize, int &mixVolume);
	void feedAudio(uint8 *srcBuf, int groupId, int volume, int pan, int16 flags);
	void sendAudioToDiMUSE(uint8 *mixBuf, int32 mixStartingPoint, int32 mixFeedSize, int32 mixInFrameCount, int volume, int pan);

	void timerCallback();
};

} // End of namespace Scumm

#endif
