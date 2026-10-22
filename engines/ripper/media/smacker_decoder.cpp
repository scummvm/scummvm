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

#include "ripper/media/smacker_decoder.h"

#include "common/debug.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "ripper/detection.h"

namespace Ripper {

class RipperSmackerDecoder::RipperSmackerVideoTrack : public SmackerVideoTrack {
public:
	RipperSmackerVideoTrack(uint32 width, uint32 height, uint32 frameCount,
			const Common::Rational &frameRate, uint32 flags, uint32 version) :
		SmackerVideoTrack(width, height, frameCount, frameRate, flags, version) {}

	void clearSurface() {
		_surface->fillRect(_surface->getRect(), 0);
	}
};

bool RipperSmackerDecoder::loadStreamWithDataFixes(Common::SeekableReadStream *stream,
		const Common::String &name) {
	if (!Video::SmackerDecoder::loadStream(stream))
		return false;

	repairEfw4FrameTable(name);
	return true;
}

const Graphics::Surface *RipperSmackerDecoder::restartAtFrame(uint frame) {
	if (!isVideoLoaded() || frame >= getFrameCount() || !rewind())
		return nullptr;

	stopAudio();
	RipperSmackerVideoTrack *videoTrack =
		static_cast<RipperSmackerVideoTrack *>(getTrack(0));
	if (!videoTrack)
		return nullptr;
	videoTrack->clearSurface();

	const Graphics::Surface *surface = nullptr;
	while (getCurFrame() < (int)frame && !endOfVideo())
		surface = decodeNextFrame();
	if (!surface || getCurFrame() != (int)frame)
		return nullptr;

	_lastTimeChange = videoTrack->getFrameTime(frame);
	if (isPlaying())
		_startTime = g_system->getMillis() -
			(_lastTimeChange.msecs() / getRate()).toInt();
	resetPauseStartTime();
	return surface;
}

void RipperSmackerDecoder::repairEfw4FrameTable(const Common::String &name) {
	// EFW4.SMK's damaged frame-size entry makes the declared payloads exceed EOF.
	const uint kFrame = 62;
	const uint32 kDamagedSize = 0x2000a070;
	const uint32 kRepairedSize = 0x8070;
	const uint32 kFrameCount = 110;
	const uint32 kFileSize = 0x2ce97c;
	if (!name.hasSuffixIgnoreCase("efw4.smk") ||
			getFrameCount() != kFrameCount || !_frameSizes ||
			_frameSizes[kFrame] != kDamagedSize ||
			_fileStream->size() != kFileSize)
		return;

	uint64 repairedEnd = _fileStream->pos();
	for (uint i = 0; i < kFrameCount; ++i)
		repairedEnd += (i == kFrame ? kRepairedSize : _frameSizes[i]) & ~3U;
	if (repairedEnd != kFileSize) {
		warning("Ripper: EFW4 Smacker frame-table repair rejected repairedEnd=%u fileSize=%u",
			(uint)repairedEnd, kFileSize);
		return;
	}

	_frameSizes[kFrame] = kRepairedSize;
	debugC(1, kDebugVideo,
		"Ripper: repaired EFW4 Smacker frame table frame=%u raw=0x%08x size=0x%04x fileSize=%u",
		kFrame, kDamagedSize, kRepairedSize, kFileSize);
}

RipperSmackerDecoder::SmackerVideoTrack *RipperSmackerDecoder::createVideoTrack(uint32 width, uint32 height, uint32 frameCount,
		const Common::Rational &frameRate, uint32 flags, uint32 version) const {
	return new RipperSmackerVideoTrack(width, height, frameCount, frameRate,
		flags, version);
}

} // End of namespace Ripper
