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

// Based on eos' Bink decoder which is in turn
// based quite heavily on the Bink decoder found in FFmpeg.
// Many thanks to Kostya Shishkov for doing the hard work.

#include "common/scummsys.h"

#ifdef USE_BINK

#ifndef VIDEO_BINK_DECODER_H
#define VIDEO_BINK_DECODER_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/array.h"
#include "common/rational.h"

#include "graphics/surface.h"

#include "video/video_decoder.h"

namespace Common {
	class SeekableReadStream;
	class BitStream;
	class Huffman;

	class RDFT;
	class DCT;
}

namespace Video {

/**
 * Decoder for Bink videos.
 *
 * Video decoder used in engines:
 *  - scumm (he)
 */
class BinkDecoder : public FixedRateVideoDecoder {
public:
	BinkDecoder();
	~BinkDecoder();

	// VideoDecoder API
	bool loadStream(Common::SeekableReadStream *stream);
	void close();
	bool isVideoLoaded() const { return _bink != 0; }
	uint16 getWidth() const { return _surface.w; }
	uint16 getHeight() const { return _surface.h; }
	Graphics::PixelFormat getPixelFormat() const { return _surface.format; }
	uint32 getFrameCount() const { return _frames.size(); }
	uint32 getElapsedTime() const;
	const Graphics::Surface *decodeNextFrame();

	// FixedRateVideoDecoder
	Common::Rational getFrameRate() const { return _frameRate; }

private:
	static const int kAudioChannelsMax  = 2;
	static const int kAudioBlockSizeMax = (kAudioChannelsMax << 11);

	/** IDs for different data types used in Bink video codec. */
	enum Source {
		kSourceBlockTypes    = 0, ///< 8x8 block types.
		kSourceSubBlockTypes    , ///< 16x16 block types (a subset of 8x8 block types).
		kSourceColors           , ///< Pixel values used for different block types.
		kSourcePattern          , ///< 8-bit values for 2-color pattern fill.
		kSourceXOff             , ///< X components of motion value.
		kSourceYOff             , ///< Y components of motion value.
		kSourceIntraDC          , ///< DC values for intrablocks with DCT.
		kSourceInterDC          , ///< DC values for interblocks with DCT.
		kSourceRun              , ///< Run lengths for special fill block.

		kSourceMAX
	};

	/** Bink video block types. */
	enum BlockType {
		kBlockSkip    = 0,  ///< Skipped block.
		kBlockScaled     ,  ///< Block has size 16x16.
		kBlockMotion     ,  ///< Block is copied from previous frame with some offset.
		kBlockRun        ,  ///< Block is composed from runs of colors with custom scan order.
		kBlockResidue    ,  ///< Motion block with some difference added.
		kBlockIntra      ,  ///< Intra DCT block.
		kBlockFill       ,  ///< Block is filled with single color.
		kBlockInter      ,  ///< Motion block with DCT applied to the difference.
		kBlockPattern    ,  ///< Block is filled with two colors following custom pattern.
		kBlockRaw           ///< Uncoded 8x8 block.
	};

	/** Data structure for decoding and tranlating Huffman'd data. */
	struct Huffman {
		int  index;       ///< Index of the Huffman codebook to use.
		byte symbols[16]; ///< Huffman symbol => Bink symbol tranlation list.
	};

	/** Data structure used for decoding a single Bink data type. */
	struct Bundle {
		int countLengths[2]; ///< Lengths of number of entries to decode (in bits).
		int countLength;     ///< Length of number of entries to decode (in bits) for the current plane.

		Huffman huffman; ///< Huffman codebook.

		byte *data;    ///< Buffer for decoded symbols.
		byte *dataEnd; ///< Buffer end.

		byte *curDec; ///< Pointer to the data that wasn't yet decoded.
		byte *curPtr; ///< Pointer to the data that wasn't yet read.
	};

	enum AudioCodec {
		kAudioCodecDCT,
		kAudioCodecRDFT
	};

	/** An audio track. */
	struct AudioTrack {
		uint16 flags;

		uint32 sampleRate;
		uint8  channels;

		uint32 outSampleRate;
		uint8  outChannels;

		AudioCodec codec;

		uint32 sampleCount;

		Common::BitStream *bits;

		bool first;

		uint32 frameLen;
		uint32 overlapLen;

		uint32 blockSize;

		uint32  bandCount;
		uint32 *bands;

		float root;

		float coeffs[16 * kAudioBlockSizeMax];
		int16 prevCoeffs[kAudioBlockSizeMax];

		float *coeffsPtr[kAudioChannelsMax];

		Common::RDFT *rdft;
		Common::DCT  *dct;

		AudioTrack();
		~AudioTrack();
	};

	/** A video frame. */
	struct VideoFrame {
		bool keyFrame;

		uint32 offset;
		uint32 size;

		Common::BitStream *bits;

		VideoFrame();
		~VideoFrame();
	};

	/** A decoder state. */
	struct DecodeContext {
		VideoFrame *video;

		uint32 planeIdx;

		uint32 blockX;
		uint32 blockY;

		byte *dest;
		byte *prev;

		byte *destStart, *destEnd;
		byte *prevStart, *prevEnd;

		uint32 pitch;

		int coordMap[64];
		int coordScaledMap1[64];
		int coordScaledMap2[64];
		int coordScaledMap3[64];
		int coordScaledMap4[64];
	};

	Common::SeekableReadStream *_bink;

	uint32 _id; ///< The BIK FourCC.

	Common::Rational _frameRate;

	Graphics::Surface _surface;

	Audio::SoundHandle _audioHandle;
	Audio::QueuingAudioStream *_audioStream;
	bool _audioStarted;

	uint32 _videoFlags; ///< Video frame features.

	bool _hasAlpha;   ///< Do video frames have alpha?
	bool _swapPlanes; ///< Are the planes ordered (A)YVU instead of (A)YUV?

	uint32 _audioFrame;

	Common::Array<AudioTrack> _audioTracks; ///< All audio tracks.
	Common::Array<VideoFrame> _frames;      ///< All video frames.

	uint32 _audioTrack; ///< Audio track to use.

	Common::Huffman *_huffman[16]; ///< The 16 Huffman codebooks used in Bink decoding.

	Bundle _bundles[kSourceMAX]; ///< Bundles for decoding all data types.

	/** Huffman codebooks to use for decoding high nibbles in color data types. */
	Huffman _colHighHuffman[16];
	/** Value of the last decoded high nibble in color data types. */
	int _colLastVal;

	byte *_curPlanes[4]; ///< The 4 color planes, YUVA, current frame.
	byte *_oldPlanes[4]; ///< The 4 color planes, YUVA, last frame.


	/** Initialize the bundles. */
	void initBundles();
	/** Deinitialize the bundles. */
	void deinitBundles();

	/** Initialize the Huffman decoders. */
	void initHuffman();

	/** Decode an audio packet. */
	void audioPacket(AudioTrack &audio);
	/** Decode a video packet. */
	void videoPacket(VideoFrame &video);

	/** Decode a plane. */
	void decodePlane(VideoFrame &video, int planeIdx, bool isChroma);

	/** Read/Initialize a bundle for decoding a plane. */
	void readBundle(VideoFrame &video, Source source);

	/** Read the symbols for a Huffman code. */
	void readHuffman(VideoFrame &video, Huffman &huffman);
	/** Merge two Huffman symbol lists. */
	void mergeHuffmanSymbols(VideoFrame &video, byte *dst, const byte *src, int size);

	/** Read and translate a symbol out of a Huffman code. */
	byte getHuffmanSymbol(VideoFrame &video, Huffman &huffman);

	/** Get a direct value out of a bundle. */
	int32 getBundleValue(Source source);
	/** Read a count value out of a bundle. */
	uint32 readBundleCount(VideoFrame &video, Bundle &bundle);

	// Handle the block types
	void blockSkip         (DecodeContext &ctx);
	void blockScaledSkip   (DecodeContext &ctx);
	void blockScaledRun    (DecodeContext &ctx);
	void blockScaledIntra  (DecodeContext &ctx);
	void blockScaledFill   (DecodeContext &ctx);
	void blockScaledPattern(DecodeContext &ctx);
	void blockScaledRaw    (DecodeContext &ctx);
	void blockScaled       (DecodeContext &ctx);
	void blockMotion       (DecodeContext &ctx);
	void blockRun          (DecodeContext &ctx);
	void blockResidue      (DecodeContext &ctx);
	void blockIntra        (DecodeContext &ctx);
	void blockFill         (DecodeContext &ctx);
	void blockInter        (DecodeContext &ctx);
	void blockPattern      (DecodeContext &ctx);
	void blockRaw          (DecodeContext &ctx);

	// Read the bundles
	void readRuns        (VideoFrame &video, Bundle &bundle);
	void readMotionValues(VideoFrame &video, Bundle &bundle);
	void readBlockTypes  (VideoFrame &video, Bundle &bundle);
	void readPatterns    (VideoFrame &video, Bundle &bundle);
	void readColors      (VideoFrame &video, Bundle &bundle);
	void readDCS         (VideoFrame &video, Bundle &bundle, int startBits, bool hasSign);
	void readDCTCoeffs   (VideoFrame &video, int16 *block, bool isIntra);
	void readResidue     (VideoFrame &video, int16 *block, int masksCount);

	void initAudioTrack(AudioTrack &audio);

	float getFloat(AudioTrack &audio);

	/** Decode an audio block. */
	void audioBlock    (AudioTrack &audio, int16 *out);
	/** Decode a DCT'd audio block. */
	void audioBlockDCT (AudioTrack &audio);
	/** Decode a RDFT'd audio block. */
	void audioBlockRDFT(AudioTrack &audio);

	void readAudioCoeffs(AudioTrack &audio, float *coeffs);

	void floatToInt16Interleave(int16 *dst, const float **src, uint32 length, uint8 channels);

	// Bink video IDCT
	void IDCT(int16 *block);
	void IDCTPut(DecodeContext &ctx, int16 *block);
	void IDCTAdd(DecodeContext &ctx, int16 *block);
};

} // End of namespace Video

#endif // VIDEO_BINK_DECODER_H

#endif // USE_BINK
