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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_MEDIA_SMACKER_DECODER_H
#define RIPPER_MEDIA_SMACKER_DECODER_H

#include "common/array.h"
#include "common/str.h"
#include "video/smk_decoder.h"

namespace Ripper {

struct BitmapAssetFrame;

void cacheSmackerFrames(Video::SmackerDecoder &decoder,
	Common::Array<BitmapAssetFrame> &frames);

class RipperSmackerDecoder : public Video::SmackerDecoder {
public:
	bool loadStreamWithDataFixes(Common::SeekableReadStream *stream,
		const Common::String &name);
	const Graphics::Surface *restartAtFrame(uint frame);
	void setTimelineStartMillis(uint32 *timelineStartMillis);

protected:
	void repairEfw4FrameTable(const Common::String &name);
	class RipperSmackerVideoTrack;
	SmackerVideoTrack *createVideoTrack(uint32 width, uint32 height, uint32 frameCount,
		const Common::Rational &frameRate, uint32 flags, uint32 version) const override;
};

} // End of namespace Ripper

#endif // RIPPER_MEDIA_SMACKER_DECODER_H
