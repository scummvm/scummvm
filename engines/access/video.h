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

#ifndef ACCESS_VIDEO_H
#define ACCESS_VIDEO_H

#include "common/scummsys.h"
#include "common/memstream.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "access/data.h"
#include "access/asurface.h"
#include "access/files.h"

namespace Access {

enum VideoFlags { VIDEOFLAG_NONE = 0, VIDEOFLAG_BG = 1 };

class VideoPlayer : public Manager {
public:
	VideoPlayer(AccessEngine *vm);
	virtual ~VideoPlayer();

	/**
	 * Start up a video
	 */
	void setVideo(BaseSurface *vidSurface, const Common::Point &pt, const FileIdent &videoFile, int rate);
	void setVideo(BaseSurface *vidSurface, const Common::Point &pt, const Common::Path &filename, int rate);

	/**
	 * Decodes a frame of the video
	 */
	virtual void playVideo() = 0;

	virtual void copyVideo() = 0;
	/**
	 * Frees the data for a previously loaded video
	 */
	virtual void closeVideo();

protected:
	virtual void setVideo(const Common::Point &pt, int rate) = 0;

	Resource *_videoData;
	BaseSurface *_vidSurface;

public:
	int _videoFrame;
	bool _soundFlag;
	int _soundFrame;
	bool _videoEnd;

};


class VideoPlayer_v1 : public VideoPlayer {
	struct VideoHeader {
		int _frameCount;
		int _width, _height;
		VideoFlags _flags;
	};
private:
	VideoHeader _header;
	byte *_startCoord;
	int _frameCount;
	int _xCount;
	int _scanCount;
	int _frameSize;
	Common::Rect _videoBounds;

	void getFrame();
protected:
	void setVideo(const Common::Point &pt, int rate) override;

public:
	VideoPlayer_v1(AccessEngine *vm);

	void playVideo() override;

	void copyVideo() override;
};

class VideoPlayer_v2 : public VideoPlayer {
	struct VideoHeader {
		uint32 _id;
		byte _version;
		int _frameCount;
		int _width, _height;
		int _frameIncr;
		uint16 _unk;
		VideoFlags _flags;
	};
private:
	VideoHeader _header;
	BaseSurface *_frame;
	byte _palette[768];

	Audio::QueuingAudioStream *_audioStream;
	Audio::SoundHandle _audioStreamHandle;

	void handlePaletteChunk();
	void handleFrameChunk(bool delta, bool skipLines);
	void handleSoundChunk(bool init);

protected:
	void setVideo(const Common::Point &pt, int rate) override;

public:
	VideoPlayer_v2(AccessEngine *vm);

	void playVideo() override;

	void copyVideo() override {};

	void closeVideo() override;
};


} // End of namespace Access

#endif /* ACCESS_VIDEO_H */
