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


#include "dcgf.h"
#include "VidPlayer.h"

//#pragma comment(lib, "vfw32.lib")

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CVidPlayer::CVidPlayer(CBGame *inGame): CBBase(inGame) {
	SetDefaults();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CVidPlayer::SetDefaults() {
	_playing = false;

/*	_aviFile = NULL;

	_audioStream = NULL;
	_audioFormat = NULL;

	_videoStream = NULL;
	_videoFormat = NULL;
	_videoPGF = NULL;*/
	_videoEndTime = 0;

	//_sound = NULL;
	_soundAvailable = false;

	//_vidRenderer = NULL;

	_startTime = 0;
	_totalVideoTime = 0;

	//_lastSample = -1;

	//_targetFormat = NULL;

	_playPosX = _playPosY = 0;
	_playZoom = 0.0f;

	_filename = NULL;

	_slowRendering = false;

	_currentSubtitle = 0;
	_showSubtitle = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CVidPlayer::~CVidPlayer() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CVidPlayer::Cleanup() {
#if 0
	if (_sound) _sound->Stop();
	if (_videoPGF) AVIStreamGetFrameClose(_videoPGF);
	_videoPGF = NULL;

	_playing = false;


	if (_aviFile) AVIFileRelease(m_AviFile);

	if (_audioStream) AVIStreamRelease(m_AudioStream);
	if (_videoStream) AVIStreamRelease(m_VideoStream);

	if (_audioFormat)  delete [](byte *)m_AudioFormat;
	if (_videoFormat)  delete [](byte *)m_VideoFormat;
	if (_targetFormat) delete [](byte *)m_TargetFormat;

	SAFE_DELETE(_sound);
	SAFE_DELETE(_vidRenderer);

	SAFE_DELETE_ARRAY(_filename);

	for (int i = 0; i < _subtitles.GetSize(); i++) delete _subtitles[i];
	_subtitles.RemoveAll();

	return SetDefaults();
#endif
	return 0;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CVidPlayer::Initialize(char *inFilename, char *SubtitleFile) {
#if 0
	Cleanup();

	char Filename[MAX_PATH];
	Game->_fileManager->GetFullPath(inFilename, Filename);

	// open file
	if (AVIFileOpen(&_aviFile, Filename, OF_READ, NULL) != 0) {
		Game->LOG(0, "Error opening AVI file '%s'", Filename);
		return E_FAIL;
	}

	// get video stream
	if (AVIFileGetStream(_aviFile, &_videoStream, streamtypeVIDEO, 0) != 0) {
		Game->LOG(0, "Error finding video stream in AVI file '%s'", Filename);
		return E_FAIL;
	}
	_totalVideoTime = AVIStreamEndTime(_videoStream);

	// get audio stream
	if (Game->m_SoundMgr->_soundAvailable && AVIFileGetStream(_aviFile, &_audioStream, streamtypeAUDIO, 0) == 0)
		_soundAvailable = true;
	else
		_soundAvailable = false;


	LONG Size;

	// get video format
	if (AVIStreamReadFormat(m_VideoStream, 0, NULL, &Size)) {
		Game->LOG(0, "Error obtaining video stream format in AVI file '%s'", Filename);
		return E_FAIL;
	}
	_videoFormat = (LPBITMAPINFO)new BYTE[Size];
	AVIStreamReadFormat(m_VideoStream, 0, m_VideoFormat, &Size);

	// initialize optimal target format
	m_TargetFormat = (LPBITMAPV4HEADER)new BYTE[max(Size, sizeof(BITMAPV4HEADER))];
	memset(m_TargetFormat, 0, sizeof(BITMAPV4HEADER));
	memcpy(m_TargetFormat, m_VideoFormat, Size);
	m_TargetFormat->bV4Size = max(Size, sizeof(BITMAPV4HEADER));

	m_TargetFormat->bV4BitCount = 24;
	m_TargetFormat->bV4V4Compression = BI_RGB;

	if (Game->m_UseD3D)
		m_VidRenderer = new CVidRendererD3D(Game);
	else
		m_VidRenderer = new CVidRendererDD(Game);

	if (!m_VidRenderer || FAILED(m_VidRenderer->Initialize(m_VideoFormat, m_TargetFormat))) {
		Game->LOG(0, "Error initializing video renderer for AVI file '%s'", Filename);
		SAFE_DELETE(m_VidRenderer);
		return E_FAIL;
	}


	// create sound buffer
	HRESULT res;

	if (_soundAvailable) {
		_sound = new CBSoundAVI(Game);
		if (FAILED(res = _sound->InitializeBuffer(_audioStream))) {
			SAFE_DELETE(_sound);
			_soundAvailable = false;
			Game->LOG(res, "Error initializing sound buffer for AVI file '%s'", Filename);
		}
	}

	if (Game->_videoSubtitles) LoadSubtitles(inFilename, SubtitleFile);

	_filename = new char[strlen(Filename) + 1];
	if (_filename) strcpy(_filename, Filename);
#endif
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CVidPlayer::Update() {
#if 0
	if (!m_Playing) return S_OK;

	HRESULT res;

	if (_soundAvailable && m_Sound) {
		res = _sound->Update();
		if (FAILED(res)) return res;
	}


	DWORD CurrentTime; // current playing time (in ms)
	/*
	 if(m_SoundAvailable && m_Sound){
	 CurrentTime = m_Sound->GetPosition(); // in samples
	 CurrentTime /= (m_Sound->m_Format.wf.nSamplesPerSec / 1000);

	 if(!m_Sound->IsPlaying()) CurrentTime = m_TotalVideoTime;
	 }
	 else
	 CurrentTime = timeGetTime() - m_StartTime;
	 */
	CurrentTime = timeGetTime() - _startTime;

	if (CurrentTime >= _totalVideoTime) {
		Stop();
		return S_OK;
	}


	// get and render frame
	DWORD sample = AVIStreamTimeToSample(_videoStream, CurrentTime);
	if (sample != _lastSample) {
		_lastSample = sample;

		// process subtitles
		_showSubtitle = false;
		while (_currentSubtitle < _subtitles.GetSize()) {
			int End = _subtitles[_currentSubtitle]->m_EndFrame;

			bool NextFrameOK = (_currentSubtitle < _subtitles.GetSize() - 1 && _subtitles[_currentSubtitle + 1]->_startFrame <= sample);

			if (sample > End) {
				if (NextFrameOK) {
					_currentSubtitle++;
				} else {
					_showSubtitle = (End == 0);
					break;
				}
			} else {
				_showSubtitle = true;
				break;
			}
		}


		// render frame
		LPBITMAPINFOHEADER FrameData = (LPBITMAPINFOHEADER)AVIStreamGetFrame(m_VideoPGF, sample);
		if (FrameData) {
			if (_slowRendering) return _vidRenderer->ProcessFrameSlow(FrameData);
			else return _vidRenderer->ProcessFrame(FrameData);
		} else return E_FAIL;
	} else return S_OK;
#endif
	return 0;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CVidPlayer::Display() {
#if 0
	if (!m_Playing) return S_OK;

	HRESULT res;
	if (_vidRenderer) res = _vidRenderer->Display(m_PlayPosX, m_PlayPosY, m_PlayZoom);
	else res = E_FAIL;

	// display subtitle
	if (m_ShowSubtitle) {
		CBFont *font = Game->_videoFont ? Game->_videoFont : Game->_systemFont;
		int Height = font->GetTextHeight((BYTE *)m_Subtitles[_currentSubtitle]->_text, Game->_renderer->_width);
		font->DrawText((byte *)_subtitles[m_CurrentSubtitle]->_text, 0, Game->_renderer->_height - Height - 5, Game->_renderer->_width, TAL_CENTER);
	}

	return res;
#endif
	return 0;
}




//////////////////////////////////////////////////////////////////////////
HRESULT CVidPlayer::Play(TVideoPlayback Type, int X, int Y, bool FreezeMusic) {
#if 0
	if (!_videoStream || !_vidRenderer) return E_FAIL;

	switch (Type) {
	case VID_PLAY_POS:
		_playZoom = 100.0f;
		_playPosX = X;
		_playPosY = Y;
		break;

	case VID_PLAY_STRETCH: {
		float ZoomX = (float)((float)Game->_renderer->m_Width / (float)_videoFormat->bmiHeader.biWidth * 100);
		float ZoomY = (float)((float)Game->_renderer->m_Height / (float)_videoFormat->bmiHeader.biHeight * 100);
		_playZoom = min(ZoomX, ZoomY);
		_playPosX = (Game->_renderer->_width - _videoFormat->bmiHeader.biWidth * (_playZoom / 100)) / 2;
		_playPosY = (Game->_renderer->_height - _videoFormat->bmiHeader.biHeight * (_playZoom / 100)) / 2;
	}
	break;

	case VID_PLAY_CENTER:
		_playZoom = 100.0f;
		_playPosX = (Game->_renderer->_width - _videoFormat->bmiHeader.biWidth) / 2;
		_playPosY = (Game->_renderer->_height - _videoFormat->bmiHeader.biHeight) / 2;
		break;
	}

	_targetFormat->bV4BitCount = 24;
	_targetFormat->bV4V4Compression = BI_RGB;


	_videoPGF = AVIStreamGetFrameOpen(_videoStream, (LPBITMAPINFOHEADER)m_TargetFormat);
	if (!_videoPGF) {
		_videoPGF = AVIStreamGetFrameOpen(_videoStream, NULL);
		if (!_videoPGF) {
			Game->LOG(0, "Error: Unsupported AVI format (file '%s')", m_Filename);
			Cleanup();
			return E_FAIL;
		} else {
			Game->LOG(0, "Performance warning: non-optimal AVI format, using generic (i.e. slow) rendering routines (file '%s')", m_Filename);
			_slowRendering = true;
		}
	} else _slowRendering = false;

	// HACK!!!
	_slowRendering = true;


	_currentSubtitle = 0;

	Game->Freeze(FreezeMusic);

	_playing = true;
	if (_sound) _sound->Play();
	_startTime = timeGetTime();
#endif
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CVidPlayer::Stop() {
#if 0
	if (!_playing) return S_OK;

	Cleanup();

	Game->Unfreeze();
#endif
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CVidPlayer::IsPlaying() {
	return _playing;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CVidPlayer::LoadSubtitles(char *Filename, char *SubtitleFile) {
#if 0
	if (!Filename) return S_OK;

	char NewFile[MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];

	if (SubtitleFile) {
		strcpy(NewFile, SubtitleFile);
	} else {
		_splitpath(Filename, drive, dir, fname, NULL);
		_makepath(NewFile, drive, dir, fname, ".SUB");
	}

	DWORD Size;
	BYTE *Buffer = Game->m_FileManager->ReadWholeFile(NewFile, &Size, false);
	if (Buffer == NULL) return S_OK; // no subtitles


	LONG Start, End;
	bool InToken;
	char *TokenStart;
	int TokenLength;
	int TokenPos;
	int TextLength;

	int Pos = 0;
	int LineLength = 0;
	while (Pos < Size) {
		Start = End = -1;
		InToken = false;
		TokenPos = -1;
		TextLength = 0;

		LineLength = 0;
		while (Pos + LineLength < Size && Buffer[Pos + LineLength] != '\n' && Buffer[Pos + LineLength] != '\0') LineLength++;

		int RealLength = LineLength - (Pos + LineLength >= Size ? 0 : 1);
		char *Text = new char[RealLength + 1];
		char *line = (char *)&Buffer[Pos];

		for (int i = 0; i < RealLength; i++) {
			if (line[i] == '{') {
				if (!InToken) {
					InToken = true;
					TokenStart = line + i + 1;
					TokenLength = 0;
					TokenPos++;
				} else TokenLength++;
			} else if (line[i] == '}') {
				if (InToken) {
					InToken = false;
					char *Token = new char[TokenLength + 1];
					strncpy(Token, TokenStart, TokenLength);
					Token[TokenLength] = '\0';
					if (TokenPos == 0) Start = atoi(Token);
					else if (TokenPos == 1) End = atoi(Token);

					delete [] Token;
				} else {
					Text[TextLength] = line[i];
					TextLength++;
				}
			} else {
				if (InToken) {
					TokenLength++;
				} else {
					Text[TextLength] = line[i];
					if (Text[TextLength] == '|') Text[TextLength] = '\n';
					TextLength++;
				}
			}
		}
		Text[TextLength] = '\0';

		if (Start != -1 && TextLength > 0) _subtitles.Add(new CVidSubtitle(Game, Text, Start, End));

		delete [] Text;

		Pos += LineLength + 1;
	}

	delete [] Buffer;
#endif
	return S_OK;
}

} // end of namespace WinterMute
