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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

#include "engines/wintermute/Base/BBase.h"
#include "engines/wintermute/Base/file/BFile.h"
#include "engines/wintermute/Base/BSurface.h"
#include "engines/wintermute/Base/BImage.h"
#include "engines/wintermute/persistent.h"
#include "video/video_decoder.h"
#include "common/stream.h"

namespace WinterMute {

class CVidTheoraPlayer : public CBBase {
private:
	enum {
	    THEORA_STATE_NONE = 0,
	    THEORA_STATE_PLAYING = 1,
	    THEORA_STATE_PAUSED = 2,
	    THEORA_STATE_FINISHED = 3
	};
	Video::RewindableVideoDecoder *_theoraDecoder;
	Graphics::Surface _surface;
public:
	DECLARE_PERSISTENT(CVidTheoraPlayer, CBBase)

	CVidTheoraPlayer(CBGame *inGame);
	virtual ~CVidTheoraPlayer(void);

	// external objects
	Common::SeekableReadStream *_file;
	Common::String _filename;

	CBSurface *_texture;
	//CVidSubtitler *_subtitler;

	// control methods
	HRESULT initialize(const Common::String &filename, const Common::String &subtitleFile = NULL);
	HRESULT initializeSimple();
	HRESULT update();
	HRESULT play(TVideoPlayback type = VID_PLAY_CENTER, int x = 0, int y = 0, bool freezeGame = false, bool freezeMusic = true, bool Looping = false, uint32 startTime = 0, float forceZoom = -1.0f, int volume = -1);
	HRESULT stop();
	HRESULT display(uint32 alpha = 0xFFFFFFFF);
	//HRESULT RenderFrame(CBSurface *Texture, yuv_buffer *yuv);

	HRESULT pause();
	HRESULT resume();

	bool isPlaying()  {
		return _state == THEORA_STATE_PLAYING;
	};
	bool isFinished() {
		return _state == THEORA_STATE_FINISHED;
	};
	bool isPaused()   {
		return _state == THEORA_STATE_PAUSED;
	};

	float getMovieTime();
	uint32 getMovieFrame();

	CBSurface *getTexture();

	int _state;
	uint32 _startTime;

	int _savedState;
	uint32 _savedPos;


	// alpha related
	CBImage *_alphaImage;
	Common::String _alphaFilename;
	HRESULT setAlphaImage(const Common::String &filename);
	__inline byte getAlphaAt(int x, int y);
	void writeAlpha();

	HRESULT SeekToTime(uint32 Time);


	void cleanup();
	HRESULT resetStream();

	// video properties
	TVideoPlayback _playbackType;
	int _posX;
	int _posY;
	float _playZoom;
	int _volume;

	bool _looping;
	bool _dontDropFrames;
	bool _freezeGame;
	uint32 _currentTime;


private:
	// seeking support
	bool _seekingKeyframe;
	float _timeOffset;

	bool _frameRendered;

	bool getIsFrameReady() {
		return _videoFrameReady;
	}
private:
	bool _audioFrameReady;
	bool _videoFrameReady;
	float _videobufTime;

	HRESULT WriteVideo();

	bool _playbackStarted;

	// helpers
	void SetDefaults();

};

} // end of namespace WinterMute

#endif
