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

#ifndef VOYEUR_ANIMATION_H
#define VOYEUR_ANIMATION_H

#include "video/video_decoder.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/stream.h"

namespace Video {

/**
 * Decoder for RL2 videos.
 *
 * Video decoder used in engines:
 *  - voyeur
 */
class RL2Decoder : public VideoDecoder {
public:
	RL2Decoder();
	virtual ~RL2Decoder();

	bool loadStream(Common::SeekableReadStream *stream);

	const Common::List<Common::Rect> *getDirtyRects() const;
	void clearDirtyRects();
	void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);

private:
	class RL2VideoTrack : public VideoTrack {
	public:
		RL2VideoTrack(Common::SeekableReadStream *stream);
		~RL2VideoTrack();

		bool endOfTrack() const;
		bool isRewindable() const { return true; }
		bool rewind();

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		uint32 getNextFrameStartTime() const { return _nextFrameStartTime; }
		const Graphics::Surface *decodeNextFrame();
		const byte *getPalette() const { _dirtyPalette = false; return _palette; }
		bool hasDirtyPalette() const { return _dirtyPalette; }

		const Common::List<Common::Rect> *getDirtyRects() const { return &_dirtyRects; }
		void clearDirtyRects() { _dirtyRects.clear(); }
		void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);

	private:
		Common::SeekableReadStream *_fileStream;
		Graphics::Surface *_surface;
		Graphics::Surface *_backSurface;

		int _curFrame;

		byte *_palette;
		mutable bool _dirtyPalette;

		uint32 _frameCount;
		uint32 _videoBase;
		uint32 *_frameOffset;
		uint32 _frameDelay;
		uint32 _nextFrameStartTime;

		Common::List<Common::Rect> _dirtyRects;

		void copyFrame(uint8 *data);
		void rl2DecodeFrameWithBackground();
		void rl2DecodeFrameWithoutBackground(int screenOffset = -1);
	};
};

} // End of namespace Video

#endif /* VOYEUR_ANIMATION_H */
