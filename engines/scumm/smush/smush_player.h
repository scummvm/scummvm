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

class ScummEngine_v7;
class SmushFont;
class SmushMixer;
class StringResource;
class Codec37Decoder;
class Codec47Decoder;
class IMuseDigital;
class Insane;

class SmushPlayer {
	friend class Insane;
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
	byte _pal[0x300];
	SmushFont *_sf[5];
	StringResource *_strings;
	Codec37Decoder *_codec37;
	Codec47Decoder *_codec47;
	Common::SeekableReadStream *_base;
	uint32 _baseSize;
	byte *_frameBuffer;
	byte *_specialBuffer;

	Common::String _seekFile;
	uint32 _startFrame;
	uint32 _startTime;
	int32 _seekPos;
	uint32 _seekFrame;

	bool _skipNext;
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

protected:
	int _width, _height;

	int _origPitch, _origNumStrips;
	bool _paused;
	uint32 _pauseStartTime;
	uint32 _pauseTime;

	void insanity(bool);
	void setPalette(const byte *palette);
	void setPaletteValue(int n, byte r, byte g, byte b);
	void setDirtyColors(int min, int max);
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
	void decodeFrameObject(int codec, const uint8 *src, int left, int top, int width, int height);
	void handleAnimHeader(int32 subSize, Common::SeekableReadStream &);
	void handleFrame(int32 frameSize, Common::SeekableReadStream &);
	void handleNewPalette(int32 subSize, Common::SeekableReadStream &);
#ifdef USE_ZLIB
	void handleZlibFrameObject(int32 subSize, Common::SeekableReadStream &b);
#endif
	void handleFrameObject(int32 subSize, Common::SeekableReadStream &);
	void handleSAUDChunk(uint8 *srcBuf, uint32 size, int groupId, int vol, int pan, int16 flags, int trkId, int index, int maxFrames);
	void handleStore(int32 subSize, Common::SeekableReadStream &);
	void handleFetch(int32 subSize, Common::SeekableReadStream &);
	void handleIACT(int32 subSize, Common::SeekableReadStream &);
	void handleTextResource(uint32 subType, int32 subSize, Common::SeekableReadStream &);
	void handleDeltaPalette(int32 subSize, Common::SeekableReadStream &);
	void readPalette(byte *, Common::SeekableReadStream &);

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
