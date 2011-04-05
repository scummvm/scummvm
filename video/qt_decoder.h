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

//
// Heavily based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#ifndef VIDEO_QT_DECODER_H
#define VIDEO_QT_DECODER_H

#include "common/scummsys.h"
#include "common/queue.h"
#include "common/rational.h"

#include "video/video_decoder.h"
#include "video/codecs/codec.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

namespace Common {
	class File;
	class MacResManager;
}

namespace Video {

/**
 * Decoder for QuickTime videos.
 *
 * Video decoder used in engines:
 *  - mohawk
 *  - sci
 */
class QuickTimeDecoder : public SeekableVideoDecoder {
public:
	QuickTimeDecoder();
	virtual ~QuickTimeDecoder();

	/**
	 * Returns the width of the video
	 * @return the width of the video
	 */
	uint16 getWidth() const;

	/**
	 * Returns the height of the video
	 * @return the height of the video
	 */
	uint16 getHeight() const;

	/**
	 * Returns the amount of frames in the video
	 * @return the amount of frames in the video
	 */
	uint32 getFrameCount() const;

	/**
	 * Load a video file
	 * @param filename	the filename to load
	 */
	bool loadFile(const Common::String &filename);

	/**
	 * Load a QuickTime video file from a SeekableReadStream
	 * @param stream	the stream to load
	 */
	bool loadStream(Common::SeekableReadStream *stream);

	/**
	 * Close a QuickTime encoded video file
	 */
	void close();

	/**
	 * Returns the palette of the video
	 * @return the palette of the video
	 */
	const byte *getPalette() { _dirtyPalette = false; return _palette; }
	bool hasDirtyPalette() const { return _dirtyPalette; }

	/**
	 * Set the beginning offset of the video so we can modify the offsets in the stco
	 * atom of videos inside the Mohawk archives
	 * @param the beginning offset of the video
	 */
	void setChunkBeginOffset(uint32 offset) { _beginOffset = offset; }

	bool isVideoLoaded() const { return _fd != 0; }
	const Graphics::Surface *decodeNextFrame();
	bool endOfVideo() const;
	uint32 getElapsedTime() const;
	uint32 getTimeToNextFrame() const;
	Graphics::PixelFormat getPixelFormat() const;

	// SeekableVideoDecoder API
	void seekToFrame(uint32 frame);
	void seekToTime(Audio::Timestamp time);
	uint32 getDuration() const { return _duration * 1000 / _timeScale; }

private:
	// This is the file handle from which data is read from. It can be the actual file handle or a decompressed stream.
	Common::SeekableReadStream *_fd;

	struct MOVatom {
		uint32 type;
		uint32 offset;
		uint32 size;
	};

	struct ParseTable {
		int (QuickTimeDecoder::*func)(MOVatom atom);
		uint32 type;
	};

	struct MOVstts {
		int count;
		int duration;
	};

	struct MOVstsc {
		uint32 first;
		uint32 count;
		uint32 id;
	};

	struct EditListEntry {
		uint32 trackDuration;
		int32 mediaTime;
		Common::Rational mediaRate;
	};

	struct STSDEntry {
		STSDEntry();
		~STSDEntry();

		uint32 codecTag;
		uint16 bitsPerSample;

		// Video
		char codecName[32];
		uint16 colorTableId;
		byte *palette;
		Codec *videoCodec;

		// Audio
		uint16 channels;
		uint32 sampleRate;
		uint32 samplesPerFrame;
		uint32 bytesPerFrame;
	};

	enum CodecType {
		CODEC_TYPE_MOV_OTHER,
		CODEC_TYPE_VIDEO,
		CODEC_TYPE_AUDIO
	};

	struct MOVStreamContext {
		MOVStreamContext();
		~MOVStreamContext();

		uint32 chunk_count;
		uint32 *chunk_offsets;
		int stts_count;
		MOVstts *stts_data;
		uint32 sample_to_chunk_sz;
		MOVstsc *sample_to_chunk;
		uint32 sample_size;
		uint32 sample_count;
		uint32 *sample_sizes;
		uint32 keyframe_count;
		uint32 *keyframes;
		int32 time_scale;
		int time_rate;

		uint16 width;
		uint16 height;
		CodecType codec_type;

		uint32 stsdEntryCount;
		STSDEntry *stsdEntries;

		uint32 editCount;
		EditListEntry *editList;

		Common::SeekableReadStream *extradata;

		uint32 nb_frames;
		uint32 duration;
		uint32 start_time;
		Common::Rational scaleFactorX;
		Common::Rational scaleFactorY;
	};

	const ParseTable *_parseTable;
	bool _foundMOOV;
	uint32 _timeScale;
	uint32 _duration;
	uint32 _numStreams;
	Common::Rational _scaleFactorX;
	Common::Rational _scaleFactorY;
	MOVStreamContext *_streams[20];
	const byte *_palette;
	bool _dirtyPalette;
	uint32 _beginOffset;
	Common::MacResManager *_resFork;

	void initParseTable();
	Audio::AudioStream *createAudioStream(Common::SeekableReadStream *stream);
	bool checkAudioCodecSupport(uint32 tag);
	Common::SeekableReadStream *getNextFramePacket(uint32 &descId);
	uint32 getFrameDuration();
	void init();

	Audio::QueuingAudioStream *_audStream;
	void startAudio();
	void stopAudio();
	void updateAudioBuffer();
	void readNextAudioChunk();
	uint32 getAudioChunkSampleCount(uint chunk);
	int8 _audioStreamIndex;
	uint _curAudioChunk;
	Audio::SoundHandle _audHandle;
	Audio::Timestamp _audioStartOffset;

	Codec *createCodec(uint32 codecTag, byte bitsPerPixel);
	Codec *findDefaultVideoCodec() const;
	uint32 _nextFrameStartTime;
	int8 _videoStreamIndex;
	uint32 findKeyFrame(uint32 frame) const;

	Graphics::Surface *_scaledSurface;
	const Graphics::Surface *scaleSurface(const Graphics::Surface *frame);
	Common::Rational getScaleFactorX() const;
	Common::Rational getScaleFactorY() const;

	void pauseVideoIntern(bool pause);

	int readDefault(MOVatom atom);
	int readLeaf(MOVatom atom);
	int readELST(MOVatom atom);
	int readHDLR(MOVatom atom);
	int readMDHD(MOVatom atom);
	int readMOOV(MOVatom atom);
	int readMVHD(MOVatom atom);
	int readTKHD(MOVatom atom);
	int readTRAK(MOVatom atom);
	int readSTCO(MOVatom atom);
	int readSTSC(MOVatom atom);
	int readSTSD(MOVatom atom);
	int readSTSS(MOVatom atom);
	int readSTSZ(MOVatom atom);
	int readSTTS(MOVatom atom);
	int readCMOV(MOVatom atom);
	int readWAVE(MOVatom atom);
};

} // End of namespace Video

#endif
