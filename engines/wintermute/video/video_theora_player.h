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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_VIDTHEORAPLAYER_H
#define WINTERMUTE_VIDTHEORAPLAYER_H

#include "engines/wintermute/base/base.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/video/video_subtitler.h"
#include "video/video_decoder.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace Wintermute {
class BaseSurface;
class BaseImage;
class VideoTheoraPlayer : public BaseClass {
private:
	enum {
		THEORA_STATE_NONE = 0,
		THEORA_STATE_PLAYING = 1,
		THEORA_STATE_PAUSED = 2,
		THEORA_STATE_FINISHED = 3
	};
	Video::VideoDecoder *_theoraDecoder;
	Graphics::Surface _surface;
public:
	DECLARE_PERSISTENT(VideoTheoraPlayer, BaseClass)

	VideoTheoraPlayer(BaseGame *inGame);
	~VideoTheoraPlayer(void) override;

	// external objects
	Common::SeekableReadStream *_file;
	Common::String _filename;

	BaseSurface *_texture;
	VideoSubtitler *_subtitler;

	// control methods
	bool initialize(const Common::String &filename, const Common::String &subtitleFile = Common::String());
	bool initializeSimple();
	bool update();
	bool play(TVideoPlayback type = VID_PLAY_CENTER, int x = 0, int y = 0, bool freezeGame = false, bool freezeMusic = true, bool looping = false, uint32 startTime = 0, float forceZoom = -1.0f, int volume = -1);
	bool stop();
	bool display(uint32 alpha = 0xFFFFFFFF);

	bool pause();
	bool resume();

	bool isPlaying() const {
		return _state == THEORA_STATE_PLAYING;
	};
	bool isFinished() const {
		return _state == THEORA_STATE_FINISHED;
	};
	bool isPaused() const {
		return _state == THEORA_STATE_PAUSED;
	};

	uint32 getMovieTime() const;

	BaseSurface *getTexture() const;

	// alpha related
	BaseImage *_alphaImage;
	Common::String _alphaFilename;
	bool setAlphaImage(const Common::String &filename);
	byte getAlphaAt(int x, int y) const;
	void writeAlpha();

	bool seekToTime(uint32 Time);

	void cleanup();
	bool resetStream();

	// video properties
	int32 _posX;
	int32 _posY;

	bool _dontDropFrames;
private:
	int32 _state;
	uint32 _startTime;

	int32 _savedState;
	uint32 _savedPos;

	// video properties
	TVideoPlayback _playbackType;
	bool _looping;
	float _playZoom;
	int32 _volume;

	bool _freezeGame;
	uint32 _currentTime;

	// seeking support
	bool _seekingKeyframe;
	float _timeOffset;

	bool _frameRendered;

	bool getIsFrameReady() const {
		return _videoFrameReady;
	}

	bool _audioFrameReady;
	bool _videoFrameReady;
	float _videobufTime;

	bool writeVideo();

	bool _playbackStarted;

	bool _foundSubtitles;

	// helpers
	void SetDefaults();
};

} // End of namespace Wintermute

#endif
