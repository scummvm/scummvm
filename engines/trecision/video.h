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

class NightlongSmackerDecoder : public Video::SmackerDecoder {
public:
	bool loadStream(Common::SeekableReadStream *stream) override;
	void muteTrack(uint track, bool mute);
	void setMute(bool mute);
	bool forceSeekToFrame(uint frame);
	bool endOfFrames() const;
};

class NightlongVideoDecoder {
public:
	NightlongVideoDecoder(bool isAmiga);
	~NightlongVideoDecoder();
	bool loadStream(Common::SeekableReadStream *stream);
	void muteTrack(uint track, bool mute);
	void setMute(bool mute);
	bool forceSeekToFrame(uint frame);
	bool endOfFrames() const;

	// VideoDecoder functions
	int getCurFrame() const;
	uint16 getWidth() const;
	uint16 getHeight() const;
	const Graphics::Surface *decodeNextFrame();
	uint32 getFrameCount() const;
	const byte *getPalette();
	void start();
	void rewind();
	bool needsUpdate() const;
	void setEndFrame(uint frame);
	bool endOfVideo() const;

	bool loadFile(const Common::Path &filename);
	const Common::Rect *getNextDirtyRect();

private:
	bool _isAmiga;
	NightlongSmackerDecoder *_smkDecoder;
	Audio::SoundHandle _amigaSoundHandle;
	Audio::Mixer *_mixer;
};

} // End of namespace Trecision
#endif
