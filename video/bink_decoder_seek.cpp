/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "common/bitstream.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/system.h"

#include "graphics/surface.h"
#include "graphics/yuva_to_rgba.h"

#include "video/bink_decoder_seek.h"

static const uint32 kBIKiID = MKTAG('B', 'I', 'K', 'i');

namespace Video {

void SeekableBinkDecoder::videoPacket(VideoFrame &video) {
	assert(video.bits);

	if (_hasAlpha) {
		if (_id == kBIKiID)
			video.bits->skip(32);

		decodePlane(video, 3, false);
	}

	if (_id == kBIKiID)
		video.bits->skip(32);

	for (int i = 0; i < 3; i++) {
		int planeIdx = ((i == 0) || !_swapPlanes) ? i : (i ^ 3);

		decodePlane(video, planeIdx, i != 0);

		if (video.bits->pos() >= video.bits->size())
			break;
	}

	// Convert the YUV data we have to our format
	assert(_curPlanes[0] && _curPlanes[1] && _curPlanes[2] && _curPlanes[3]);
	Graphics::convertYUVA420ToRGBA(&_surface, _curPlanes[0], _curPlanes[1], _curPlanes[2], _curPlanes[3],
			_surface.w, _surface.h, _surface.w, _surface.w >> 1);

	// And swap the planes with the reference planes
	for (int i = 0; i < 4; i++)
		SWAP(_curPlanes[i], _oldPlanes[i]);
}

uint32 SeekableBinkDecoder::getDuration() const
{
	Common::Rational duration = getFrameCount() * 1000 / getFrameRate();
	return duration.toInt();
}

uint32 SeekableBinkDecoder::findKeyFrame(uint32 frame) const {
	for (int i = frame; i >= 0; i--) {
		if (_frames[i].keyFrame)
				return i;
	}

	// If none found, we'll assume the requested frame is a key frame
	return frame;
}

void SeekableBinkDecoder::seekToFrame(uint32 frame) {
	assert(frame < _frames.size());

	// Fast path
	if ((int32)frame == _curFrame + 1)
		return;

	// Stop all audio (for now)
	stopAudio();

	// Track down the keyframe
	_curFrame = findKeyFrame(frame) - 1;
	while (_curFrame < (int32)frame - 1)
		skipNextFrame();

	// Map out the starting point
	Common::Rational startTime = frame * 1000 / getFrameRate();
	_startTime = g_system->getMillis() - startTime.toInt();
	resetPauseStartTime();

	// Adjust the audio starting point
	if (_audioTrack < _audioTracks.size()) {
		Common::Rational audioStartTime = (frame + 1) * 1000 / getFrameRate();
		_audioStartOffset = audioStartTime.toInt();
	}

	// Restart the audio
	startAudio();
}

void SeekableBinkDecoder::seekToTime(const Audio::Timestamp &time) {
	// Try to find the last frame that should have been decoded
	Common::Rational frame = time.msecs() * getFrameRate() / 1000;
	seekToFrame(frame.toInt());
}

void SeekableBinkDecoder::skipNextFrame() {
	if (endOfVideo())
		return;

	VideoFrame &frame = _frames[_curFrame + 1];

	if (!_bink->seek(frame.offset))
		error("Bad bink seek");

	uint32 frameSize = frame.size;

	for (uint32 i = 0; i < _audioTracks.size(); i++) {
		uint32 audioPacketLength = _bink->readUint32LE();

		frameSize -= 4;

		if (frameSize < audioPacketLength)
			error("Audio packet too big for the frame");

		if (audioPacketLength >= 4) {
			// Skip audio data
			_bink->seek(audioPacketLength, SEEK_CUR);

			frameSize -= audioPacketLength;
		}
	}

	uint32 videoPacketStart = _bink->pos();
	uint32 videoPacketEnd   = _bink->pos() + frameSize;

	frame.bits =
		new Common::BitStream32LELSB(new Common::SeekableSubReadStream(_bink,
		    videoPacketStart, videoPacketEnd), true);

	videoPacket(frame);

	delete frame.bits;
	frame.bits = 0;

	_curFrame++;
	if (_curFrame == 0)
		_startTime = g_system->getMillis();
}

void SeekableBinkDecoder::setAudioTrack(uint32 track) {
	if (_audioTracks.size() > 1 && track < _audioTracks.size())
		_audioTrack = track;
}

} /* namespace Video */
