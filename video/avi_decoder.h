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
 */

#ifndef VIDEO_AVI_PLAYER_H
#define VIDEO_AVI_PLAYER_H

#include "common/endian.h"
#include "common/rational.h"
#include "common/rect.h"
#include "common/str.h"

#include "video/video_decoder.h"
#include "audio/mixer.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct PixelFormat;
}

namespace Graphics {
struct Surface;
}

namespace Video {

class Codec;

#define UNKNOWN_HEADER(a) error("Unknown header found -- \'%s\'", tag2str(a))

// IDs used throughout the AVI files
// that will be handled by this player
#define ID_RIFF MKTAG('R','I','F','F')
#define ID_AVI  MKTAG('A','V','I',' ')
#define ID_LIST MKTAG('L','I','S','T')
#define ID_HDRL MKTAG('h','d','r','l')
#define ID_AVIH MKTAG('a','v','i','h')
#define ID_STRL MKTAG('s','t','r','l')
#define ID_STRH MKTAG('s','t','r','h')
#define ID_VIDS MKTAG('v','i','d','s')
#define ID_AUDS MKTAG('a','u','d','s')
#define ID_MIDS MKTAG('m','i','d','s')
#define ID_TXTS MKTAG('t','x','t','s')
#define ID_JUNK MKTAG('J','U','N','K')
#define ID_STRF MKTAG('s','t','r','f')
#define ID_MOVI MKTAG('m','o','v','i')
#define ID_REC  MKTAG('r','e','c',' ')
#define ID_VEDT MKTAG('v','e','d','t')
#define ID_IDX1 MKTAG('i','d','x','1')
#define ID_STRD MKTAG('s','t','r','d')
#define ID_00AM MKTAG('0','0','A','M')
//#define ID_INFO MKTAG('I','N','F','O')

// Codec tags
#define ID_RLE  MKTAG('R','L','E',' ')
#define ID_CRAM MKTAG('C','R','A','M')
#define ID_MSVC MKTAG('m','s','v','c')
#define ID_WHAM MKTAG('W','H','A','M')
#define ID_CVID MKTAG('c','v','i','d')
#define ID_IV32 MKTAG('i','v','3','2')
#define ID_DUCK MKTAG('D','U','C','K')

struct BITMAPINFOHEADER {
	uint32 size;
	uint32 width;
	uint32 height;
	uint16 planes;
	uint16 bitCount;
	uint32 compression;
	uint32 sizeImage;
	uint32 xPelsPerMeter;
	uint32 yPelsPerMeter;
	uint32 clrUsed;
	uint32 clrImportant;
};

struct WAVEFORMAT {
	uint16 tag;
	uint16 channels;
	uint32 samplesPerSec;
	uint32 avgBytesPerSec;
	uint16 blockAlign;
};

struct PCMWAVEFORMAT : public WAVEFORMAT {
	uint16 size;
};

struct WAVEFORMATEX : public WAVEFORMAT {
	uint16 bitsPerSample;
	uint16 size;
};

struct AVIOLDINDEX {
	uint32 size;
	struct Index {
		uint32 id;
		uint32 flags;
		uint32 offset;
		uint32 size;
	} *indices;
};

// Index Flags
enum IndexFlags {
	AVIIF_INDEX = 0x10
};

// Audio Codecs
enum {
	kWaveFormatNone = 0,
	kWaveFormatPCM = 1,
	kWaveFormatDK3 = 98
};

struct AVIHeader {
	uint32 size;
	uint32 microSecondsPerFrame;
	uint32 maxBytesPerSecond;
	uint32 padding;
	uint32 flags;
	uint32 totalFrames;
	uint32 initialFrames;
	uint32 streams;
	uint32 bufferSize;
	uint32 width;
	uint32 height;
};

// Flags from the AVIHeader
enum AviFlags {
	AVIF_HASINDEX = 0x00000010,
	AVIF_MUSTUSEINDEX = 0x00000020,
	AVIF_ISINTERLEAVED = 0x00000100,
	AVIF_TRUSTCKTYPE = 0x00000800,
	AVIF_WASCAPTUREFILE = 0x00010000,
	AVIF_WASCOPYRIGHTED = 0x00020000
};

struct AVIStreamHeader {
	uint32 size;
	uint32 streamType;
	uint32 streamHandler;
	uint32 flags;
	uint16 priority;
	uint16 language;
	uint32 initialFrames;
	uint32 scale;
	uint32 rate;
	uint32 start;
	uint32 length;
	uint32 bufferSize;
	uint32 quality;
	uint32 sampleSize;
	Common::Rect frame;
};

/**
 * Decoder for AVI videos.
 *
 * Video decoder used in engines:
 *  - sci
 */
class AviDecoder : public FixedRateVideoDecoder {
public:
	AviDecoder(Audio::Mixer *mixer,
			Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	virtual ~AviDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

	bool isVideoLoaded() const { return _fileStream != 0; }
	uint16 getWidth() const { return _header.width; }
	uint16 getHeight() const { return _header.height; }
	uint32 getFrameCount() const { return _header.totalFrames; }
	uint32 getElapsedTime() const;
	const Graphics::Surface *decodeNextFrame();
	Graphics::PixelFormat getPixelFormat() const;
	const byte *getPalette() { _dirtyPalette = false; return _palette; }
	bool hasDirtyPalette() const { return _dirtyPalette; }

protected:
	Common::Rational getFrameRate() const { return Common::Rational(_vidsHeader.rate, _vidsHeader.scale); }

private:
	Audio::Mixer *_mixer;
	BITMAPINFOHEADER _bmInfo;
	PCMWAVEFORMAT _wvInfo;
	AVIOLDINDEX _ixInfo;
	AVIHeader _header;
	AVIStreamHeader _vidsHeader;
	AVIStreamHeader _audsHeader;
	byte _palette[3 * 256];
	bool _dirtyPalette;

	Common::SeekableReadStream *_fileStream;
	bool _decodedHeader;

	Codec *_videoCodec;
	Codec *createCodec();

	Audio::Mixer::SoundType _soundType;

	void runHandle(uint32 tag);
	void handleList();
	void handleStreamHeader();
	void handlePalChange();

	Audio::SoundHandle *_audHandle;
	Audio::QueuingAudioStream *_audStream;
	Audio::QueuingAudioStream *createAudioStream();
	void queueAudioBuffer(uint32 chunkSize);
};

} // End of namespace Video

#endif
