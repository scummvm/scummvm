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

// Based on the Bink 2 decoder written for FFmpeg by
// Konstantin Shishkov and Paul B Mahol, which was never merged there,
// with what it lacked taken from RAD's own decoder.

#include "common/scummsys.h"

#ifdef USE_BINK

#ifndef VIDEO_BINK2_DECODER_H
#define VIDEO_BINK2_DECODER_H

#include "common/array.h"
#include "common/bitstream.h"
#include "common/rational.h"

#include "graphics/pixelformat.h"

#include "video/bink_decoder.h"
#include "video/video_decoder.h"

namespace Common {
class SeekableReadStream;
template <class BITSTREAM>
class Huffman;
}

namespace Graphics {
struct Surface;
}

namespace Video {

/**
 * Decoder for Bink 2 videos of revisions 'f' and 'g' (KB2f, KB2g).
 *
 * Video decoder used in engines:
 *  - nancy
 */
class Bink2Decoder : public VideoDecoder {
public:
	/**
	 * @param skipOutsideBlocks  Play revision 'f' as RAD's older decoders do
	 *                           (Nancy29): a motion block whose source starts
	 *                           outside the frame is skipped, where newer ones
	 *                           move its source back into the frame.
	 */
	explicit Bink2Decoder(bool skipOutsideBlocks = false);
	~Bink2Decoder() override;

	bool loadStream(Common::SeekableReadStream *stream) override;
	void close() override;

	/** Whether the stream starts like a file this decoder plays. */
	static bool isBink2(Common::SeekableReadStream &stream);

protected:
	void readNextPacket() override;
	bool supportsAudioTrackSwitching() const override { return true; }
	AudioTrack *getAudioTrack(int index) override;
	bool seekIntern(const Audio::Timestamp &time) override;

private:
	struct Frame {
		uint32 offset;
		uint32 size;
		bool isKeyFrame;
	};

	/**
	 * A frame is made of 32x32 macroblocks of YUV 4:2:0 and optional alpha,
	 * coded with a float DCT and half pixel (luma) or quarter pixel (chroma)
	 * motion compensation. Once decoded, its blocks with only a DC are blurred
	 * into their neighbors and the edges between blocks smoothed, as RAD's own
	 * decoder does.
	 */
	class Bink2VideoTrack : public FixedRateVideoTrack {
	public:
		Bink2VideoTrack(char revision, bool skipOutsideBlocks, uint32 width, uint32 height, uint32 flags, uint32 frameCount, const Common::Rational &frameRate);
		~Bink2VideoTrack() override;

		uint16 getWidth() const override { return _width; }
		uint16 getHeight() const override { return _height; }
		Graphics::PixelFormat getPixelFormat() const override { return _pixelFormat; }
		bool setOutputPixelFormat(const Graphics::PixelFormat &format) override;
		int getCurFrame() const override { return _curFrameNumber; }
		int getFrameCount() const override { return _frameCount; }
		const Graphics::Surface *decodeNextFrame() override { return _surface; }
		bool isSeekable() const override { return true; }
		bool seek(const Audio::Timestamp &) override { return true; }
		bool rewind() override;

		void setCurFrame(int frame) { _curFrameNumber = frame; }

		/** Decode the next size bytes of the stream, a video packet, into the next frame. */
		void decodePacket(Common::SeekableReadStream &stream, uint32 size, bool isKeyFrame);

	protected:
		Common::Rational getFrameRate() const override { return _frameRate; }

	private:
		typedef Common::BitStreamMemory32LELSB BitStream;
		typedef Common::Huffman<BitStream> Huffman;

		enum BlockType {
			kBlockIntra   = 0,
			kBlockSkip    = 1,
			kBlockMotion  = 2,
			kBlockResidue = 3
		};

		// Macroblock edge flags
		enum {
			kFlagLeft     = 0x20, // first macroblock of a row
			kFlagTop      = 0x80  // first row of a slice
		};

		struct DCPredict {
			float dc[4][16];
			int blockType;
		};

		struct MotionVectors {
			int v[4][2];
		};

		// Revision 'g' predicts integer DCs, and the quantizers from the
		// macroblocks around
		struct IntDCPredict {
			int dc[4][16];
			int blockType;
		};

		struct QuantPredict {
			int intraQ;
			int interQ;
		};

		char _revision;
		bool _skipOutsideBlocks;

		uint32 _width;
		uint32 _height;
		uint32 _alignedWidth;
		uint32 _alignedHeight;
		bool _hasAlpha;

		int _frameCount;
		int _curFrameNumber;
		Common::Rational _frameRate;

		Graphics::PixelFormat _pixelFormat;
		Graphics::Surface *_surface;

		// Two frames with a border around each plane, for motion vectors
		// that reach past the edges
		byte *_buffers[2][4];
		byte *_planes[2][4];
		int _pitches[4];
		uint32 _planeSizes[4];
		int _numPlanes;
		int _curFrame;

		Common::Array<byte> _packet;

		// Per macroblock column, for the current and the previous row
		Common::Array<DCPredict> _dcRows[2];
		Common::Array<MotionVectors> _mvRows[2];
		DCPredict *_currentDC;
		DCPredict *_prevDC;
		MotionVectors *_currentMV;
		MotionVectors *_prevMV;

		int _comp;
		int _mbPos;

		float _block[4][64];
		int _coefCounts[4];

		// Per 8x8 block of each plane: the kind of block in the top three bits
		// (0 not intra, then 4 to 7 for intra ones with fewer AC coefficients),
		// and its DC in the others. Per 16x16 luma block: the motion vector.
		Common::Array<uint16> _blockWords[4];
		int _blockColumns[4];
		Common::Array<int> _blockVectors;
		int _mbX;
		int _mbY;
		uint32 _frameFlags;

		Huffman *_quantHuffman;
		Huffman *_acValueHuffman[2];
		Huffman *_acSkipHuffman[2];

		// Revision 'g'
		Common::Array<IntDCPredict> _intDCRows[2];
		Common::Array<QuantPredict> _quantRows[2];
		IntDCPredict *_currentIntDC;
		IntDCPredict *_prevIntDC;
		QuantPredict *_currentQuant;
		QuantPredict *_prevQuant;
		int16 _intBlock[4][64];
		int16 _residue[32 * 32];
		Huffman *_gACSkipHuffman[2];
		Huffman *_gMotionHuffman;

		/** Clear both frames to black. */
		void reset();

		void decodeFrame(Common::SeekableReadStream &stream, uint32 size, bool isKeyFrame);
		void decodeSlice(BitStream &bits, bool isKeyFrame, int start, int end);

		void skipBlockFlags(BitStream &bits, int size);

		int decodeDeltaQ(BitStream &bits);
		uint32 decodeLumaCBP(BitStream &bits, uint32 prevCBP);
		uint32 decodeChromaCBP(BitStream &bits, uint32 prevCBP);

		void predictDC(bool isLuma, float minDC, float maxDC, int flags, float base, const float tdc[16]);
		void decodeDC(BitStream &bits, bool isLuma, int q, int minDC, int maxDC, int flags);
		void decodeAC(BitStream &bits, const byte *scan, uint32 cbp, float q, const float *qmat);

		bool decodeLuma(BitStream &bits, bool isIntra, uint32 &prevCBP, int &prevQ, byte *dst, int pitch, int flags);
		bool decodeChroma(BitStream &bits, bool isIntra, uint32 &prevCBP, int &prevQ, byte *dst, int pitch, int flags);

		void decodeMotionVectors(BitStream &bits, int flags, MotionVectors &mv);
		void predictMotionVectors(int flags, const MotionVectors &mv);
		// x and y are in the plane; dst points at the row of the macroblock
		void getSourcePosition(int x, int y, int block, int &posX, int &posY) const;
		void compensateLuma(int x, int y, byte *dst, const byte *src, int pitch);
		void compensateChroma(int x, int y, byte *dst, const byte *src, int pitch);

		// The DCs of a macroblock that was not coded as intra, from its pixels.
		// x is in luma pixels, y in luma rows from the current row.
		void averageNeighbor(int x, int y, byte *const dst[4], DCPredict &predict);

		void setBlockWord(int plane, int x, int y, uint16 word);
		void storeDeblockInfo(int type);

		// Revision 'g'
		void decodeSliceG(BitStream &bits, bool isKeyFrame, int start, int end);
		int decodeBlockTypeG(BitStream &bits, int lru[4]);
		int decodeDeltaQG(BitStream &bits);
		uint32 decodeLumaCBPG(BitStream &bits, uint32 prevCBP);
		uint32 decodeChromaCBPG(BitStream &bits, uint32 prevCBP);
		void predictDCG(bool isLuma, int minDC, int maxDC, int flags, const int tdc[16]);
		void decodeDCG(BitStream &bits, bool isLuma, int q, int minDC, int maxDC, int flags);
		void decodeACG(BitStream &bits, uint32 cbp, int q, const uint16 qmat[4][64]);
		void decodeLumaG(BitStream &bits, bool isIntra, uint32 &prevCBP, int q, byte *dst, int pitch, int flags);
		void decodeChromaG(BitStream &bits, bool isIntra, uint32 &prevCBP, int q, byte *dst, int pitch, int flags);
		int decodeMotionVectorsG(BitStream &bits, MotionVectors &mv);
		void predictMotionVectorsG(int flags, const MotionVectors &mv, int count);
		void averageNeighborG(int x, int y, byte *const dst[4], IntDCPredict &predict);
		int predictQuant(bool isIntra, int flags) const;

		// After a frame is decoded
		void deblock();
		void blurBlock(int plane, int x, int y, int threshold);
		void blurBlockG(int plane, int x, int y, const uint16 words[3][3], int threshold);
		void filterEdges(int plane, bool isVertical);

		// Into the output surface
		void convertFrame();
	};

	Common::SeekableReadStream *_stream;
	Common::Array<Frame> _frames;
	uint32 _audioTrackCount;
	bool _skipOutsideBlocks;

	uint32 findKeyFrame(uint32 frame) const;
};

} // End of namespace Video

#endif // VIDEO_BINK2_DECODER_H

#endif // USE_BINK
