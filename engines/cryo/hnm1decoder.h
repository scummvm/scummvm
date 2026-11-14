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

#ifndef CRYO_HNM1DECODER_H
#define CRYO_HNM1DECODER_H

#include "common/array.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"

namespace Audio {
class SeekableAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace Cryo {

/**
 * Decoder for the untagged predecessor of Cryo's HNM4 video format. Lost Eden
 * stores a number of movies in it (the Cryo and Virgin Interactive logos,
 * several world flyovers, ...) alongside its regular HNM4 ones.
 *
 * The container has no magic tag and no superchunk framing:
 *
 *   uint16 LE dataOffset   - offset at which the frame records begin
 *   palette                - start/count/RGB triples, 6 bits per component,
 *                            terminated by 0xFF,0xFF
 *   byte 0xFF              - marker
 *   uint32 LE offsets[N+1] - offset of each of the N frame records, relative
 *                            to dataOffset. offsets[N] is the number of bytes
 *                            left in the stream, i.e. an end marker.
 *
 * A frame record is a list of chunks:
 *
 *   uint16 LE recordSize   - size of the whole record
 *   then, repeatedly:
 *     uint16 tag
 *       "sd", "pl": uint16 LE length (counting the four byte header), data.
 *                   "sd" holds a slice of one VOC file spanning the whole
 *                   movie, "pl" a palette update in the format used above.
 *       anything else: an image chunk, always last in the record:
 *         uint16 LE flags/width, uint16 LE (mode << 8 | height),
 *         six byte compression header, compressed data.
 *
 * Images decompress to width * height bytes, optionally preceded by up to
 * four bytes of padding, and are drawn at the top of the frame; rows past
 * the chunk's height keep what the previous frame left there. Only mode 0xFE,
 * a complete picture, is handled: mode 0xFF renders into an off screen buffer
 * instead, so movies using it are rejected rather than shown incorrectly.
 * One of two compression schemes is used, selected by a checksum over the six
 * byte compression header:
 *
 *   0xAB - the LZ77 variant also used by EdenGame::expandHSQ() (resource.cpp)
 *   0xAD - two stages: a byte oriented LZ77 pass expands the data into a
 *          scratch area at the end of the output buffer, then a bit driven
 *          RLE pass expands that into the image
 */
class HNM1Decoder : public Video::VideoDecoder {
public:
	HNM1Decoder();
	~HNM1Decoder() override;

	bool loadStream(Common::SeekableReadStream *stream) override;
	void close() override;

	/**
	 * The range of colours the movie names, the rest belonging to whatever else
	 * is on screen. Both zero when there is no picture to speak of.
	 */
	void getPaletteRange(uint16 &first, uint16 &last) const;

private:
	enum {
		kWidth = 320,
		kMaxHeight = 200,
		// Neither the container nor the chunks encode a frame delay. This is
		// the rate the movies with sound play back at, and matches the delay
		// the regular HNM decoder uses for soundless HNM4 clips.
		kFrameDelayMs = 80
	};

	class HNM1VideoTrack : public VideoTrack {
	public:
		HNM1VideoTrack(Common::SeekableReadStream *stream, uint32 dataOffset,
		               const Common::Array<uint32> &frameOffsets, const byte *palette,
		               uint16 height, uint16 palFirst, uint16 palLast);
		~HNM1VideoTrack() override;

		bool endOfTrack() const override { return _curFrame + 1 >= getFrameCount(); }
		uint16 getWidth() const override { return kWidth; }
		uint16 getHeight() const override { return _height; }
		Graphics::PixelFormat getPixelFormat() const override { return _surface.format; }
		int getCurFrame() const override { return _curFrame; }
		int getFrameCount() const override { return _frameOffsets.size() - 1; }
		uint32 getNextFrameStartTime() const override {
			return (uint32)(_curFrame + 1) * kFrameDelayMs;
		}
		const Graphics::Surface *decodeNextFrame() override;
		const byte *getPalette() const override { _dirtyPalette = false; return _palette.data(); }
		bool hasDirtyPalette() const override { return _dirtyPalette; }

		/** Which colours the movie names. The rest are not its to give away. */
		uint16 getPaletteFirst() const { return _palFirst; }
		uint16 getPaletteLast() const { return _palLast; }

		uint16 _palFirst;
		uint16 _palLast;

	private:
		/** Decode an image chunk into the surface. */
		bool decodeImage(const byte *chunk, uint32 size);
		/** Apply a palette update chunk. */
		void updatePalette(const byte *data, uint32 size);

		Common::SeekableReadStream *_stream; // Not owned
		uint32 _dataOffset;
		Common::Array<uint32> _frameOffsets;
		int _curFrame;
		uint16 _height;

		Graphics::Surface _surface;
		Graphics::Palette _palette;
		mutable bool _dirtyPalette;

		byte *_record;
		uint32 _recordAlloc;
		byte *_decodeBuffer;
	};

	class HNM1AudioTrack : public AudioTrack {
	public:
		HNM1AudioTrack(Audio::SeekableAudioStream *stream, Audio::Mixer::SoundType soundType);
		~HNM1AudioTrack() override;

	protected:
		Audio::AudioStream *getAudioStream() const override;

	private:
		Audio::SeekableAudioStream *_stream;
	};

	Common::SeekableReadStream *_stream;
	HNM1VideoTrack *_videoTrack;
};

} // End of namespace Cryo

#endif
