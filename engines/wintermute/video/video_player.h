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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_VIDPLAYER_H
#define WINTERMUTE_VIDPLAYER_H

#include "video/avi_decoder.h"

#include "engines/wintermute/dctypes.h"    // Added by ClassView
#include "engines/wintermute/base/base.h"
#include "engines/wintermute/video/video_subtitler.h"
#include "graphics/surface.h"

#define MAX_AUDIO_STREAMS 5
#define MAX_VIDEO_STREAMS 5


namespace Wintermute {
class BaseSurface;
// AVI-Video-player
class VideoPlayer : public BaseClass {
public:
	bool isPlaying();
	// external object
	Common::String _filename;
	bool stop();
	bool play(TVideoPlayback Type = VID_PLAY_CENTER, int x = 0, int y = 0, bool freezeMusic = true);
	Video::AVIDecoder *_aviDecoder;
	bool setDefaults();
	bool _playing;
	bool display();
	bool update();
	bool initialize(const Common::String &filename, const Common::String &subtitleFile = Common::String());
	bool cleanup();
	VideoPlayer(BaseGame *inGame);
	~VideoPlayer() override;

	BaseSurface *_texture;
	VideoSubtitler *_subtitler;

	int32 _playPosX;
	int32 _playPosY;
	float _playZoom;

private:
	bool _foundSubtitles;
	bool _videoFrameReady;
	bool _playbackStarted;
	bool _freezeGame;
};

} // End of namespace Wintermute

#endif
