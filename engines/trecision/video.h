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

#ifndef TRECISION_VIDEO_H
#define TRECISION_VIDEO_H

#include "common/stream.h"
#include "common/serializer.h"
#include "video/smk_decoder.h"

#include "trecision/struct.h"

namespace Trecision {

class TrecisionEngine;

class NightlongVideoDecoder : public Video::SmackerDecoder {
public:
	void muteTrack(uint track, bool mute);
	void setMute(bool mute);
	virtual bool forceSeekToFrame(uint frame) { return false; }
	virtual bool endOfFrames() const { return false; }
};

class NightlongSmackerDecoder : public NightlongVideoDecoder {
public:
	bool loadStream(Common::SeekableReadStream *stream) override;
	bool forceSeekToFrame(uint frame) override;
	bool endOfFrames() const override;
};

class NightlongAmigaDecoder : public NightlongVideoDecoder {
public:
	bool loadStream(Common::SeekableReadStream *stream) override;
	bool forceSeekToFrame(uint frame) override;
	bool endOfFrames() const override;
	const Common::Rect *getNextDirtyRect() override;

private:
	Common::Rect _lastDirtyRect;

	void readNextPacket() override;

	class AmigaVideoTrack : public VideoTrack {
	public:
		AmigaVideoTrack(const Common::String &fileName);

	private:
		byte _palette[3 * 256];
		int _curFrame;
		uint32 _frameCount;

		uint16 getWidth() const override;
		uint16 getHeight() const override;
		Graphics::PixelFormat getPixelFormat() const override;
		int getCurFrame() const override { return _curFrame; }
		uint32 getNextFrameStartTime() const override;
		const Graphics::Surface *decodeNextFrame() override;
		int getFrameCount() const override { return _frameCount; }
		const byte *getPalette() const override { return _palette; }
		bool hasDirtyPalette() const override { return true; }
	};

	class AmigaAudioTrack : public AudioTrack {
	public:
		AmigaAudioTrack(const Common::String &fileName);
	private:
		Audio::AudioStream *getAudioStream() const override { return _audioStream; }
		Audio::AudioStream *_audioStream;
	};
};

} // End of namespace Trecision
#endif
