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

#ifndef VIDEO_ASF_DECODER_H
#define VIDEO_ASF_DECODER_H

#include "video/video_decoder.h"
#include "video/asf.h"
#include "common/queue.h"

namespace Video {

class ASFDecoder : public VideoDecoder {
public:
	ASFDecoder();
	~ASFDecoder() override;
	bool loadStream(Common::SeekableReadStream *stream) override;
	void close() override;
protected:
	void readNextPacket() override;
private:
	class ASFVideoTrack;
	class ASFAudioTrack;
	bool prepareFrame();
	bool readObject(ASFDemuxer::Object &object);
	void bufferPackets();
	Common::SeekableReadStream *_input;
	ASFDemuxer _demux;
	Common::Queue<ASFDemuxer::Object> _videoObjects;
	uint32 _videoQueueBytes;
	ASFVideoTrack *_video;
	ASFAudioTrack *_audio;
	uint8 _videoID, _audioID;
	bool _ended;
};

} // End of namespace Video
#endif
