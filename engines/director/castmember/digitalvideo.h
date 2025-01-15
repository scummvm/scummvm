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

#ifndef DIRECTOR_CASTMEMBER_DIGITALVIDEO_H
#define DIRECTOR_CASTMEMBER_DIGITALVIDEO_H

#include "director/castmember/castmember.h"

namespace Video {
class VideoDecoder;
}

namespace Director {

enum DigitalVideoType {
	kDVQuickTime,
	kDVVideoForWindows,
	kDVUnknown = -1,
};

class DigitalVideoCastMember : public CastMember {
public:
	DigitalVideoCastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream, uint16 version);
	DigitalVideoCastMember(Cast *cast, uint16 castId, DigitalVideoCastMember &source);
	~DigitalVideoCastMember();

	bool isModified() override;
	Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) override;

	bool loadVideoFromCast();
	bool loadVideo(Common::String path);
	void setChannel(Channel *channel) { _channel = channel; }
	void startVideo();
	void stopVideo();
	void rewindVideo();

	uint getMovieCurrentTime();
	uint getDuration();
	uint getMovieTotalTime();
	void seekMovie(int stamp);
	void setStopTime(int stamp);
	void setMovieRate(double rate);
	void setFrameRate(int rate);

	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

	Common::String formatInfo() override;

	Common::Point getRegistrationOffset() override;
	Common::Point getRegistrationOffset(int16 width, int16 height) override;

	Common::String _filename;

	uint32 _vflags;
	bool _looping;
	bool _pausedAtStart;
	bool _enableVideo;
	bool _enableSound;
	bool _crop;
	bool _center;
	bool _preload;
	bool _showControls;
	bool _directToStage;
	bool _avimovie, _qtmovie;
	bool _dirty;
	FrameRateType _frameRateType;
	DigitalVideoType _videoType;

	uint16 _frameRate;
	bool _getFirstFrame;
	int _duration;

	Video::VideoDecoder *_video;
	Graphics::Surface *_lastFrame;

	Channel *_channel;
};

} // End of namespace Director

#endif
