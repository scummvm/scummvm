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

#ifndef WINTERMUTE_VIDPLAYER_H
#define WINTERMUTE_VIDPLAYER_H

#include "engines/wintermute/dctypes.h"    // Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "engines/wintermute/Base/BBase.h"

#define MAX_AUDIO_STREAMS 5
#define MAX_VIDEO_STREAMS 5


namespace WinterMute {

class CVidPlayer : public CBBase {
public:
	bool _showSubtitle;
	int _currentSubtitle;
	HRESULT loadSubtitles(const char *Filename, const char *SubtitleFile);
	bool _slowRendering;
	bool isPlaying();
	char *_filename;
	HRESULT stop();
	HRESULT play(TVideoPlayback Type = VID_PLAY_CENTER, int X = 0, int Y = 0, bool FreezeMusic = true);
	uint32 _totalVideoTime;
	uint32 _startTime;
	//CVidRenderer *_vidRenderer;
	//CBSoundAVI *_sound;
	bool _soundAvailable;
	HRESULT SetDefaults();
	bool _playing;
	HRESULT display();
	HRESULT update();
	HRESULT initialize(const char *inFilename, const char *SubtitleFile = NULL);
	HRESULT cleanup();
	CVidPlayer(CBGame *inGame);
	virtual ~CVidPlayer();

	/*PAVIFILE _aviFile;

	LONG _lastSample;

	PAVISTREAM _audioStream;
	PAVISTREAM _videoStream;

	LPWAVEFORMAT _audioFormat;

	LPBITMAPINFO _videoFormat;
	PGETFRAME _videoPGF;*/
	uint32 _videoEndTime;

	int _playPosX;
	int _playPosY;
	float _playZoom;

/*	LPBITMAPV4HEADER _targetFormat;

	CBArray<CVidSubtitle *, CVidSubtitle *> _subtitles;*/
};

} // end of namespace WinterMute

#endif
