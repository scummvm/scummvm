// Copyright 2009, 2010 Jan Nedoma
//
// This file is part of Wintermute Engine.
//
// Wintermute Engine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Wintermute Engine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Wintermute Engine.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////


#ifndef WINTERMUTE_VIDTHEORAPLAYER_H
#define WINTERMUTE_VIDTHEORAPLAYER_H

#include "engines/wintermute/Base/BBase.h"
#include "engines/wintermute/Base/file/BFile.h"
#include "engines/wintermute/Base/BSurface.h"
#include "engines/wintermute/Base/BImage.h"
//#include <theora/theora.h>

namespace WinterMute {

class CVidTheoraPlayer : public CBBase {
private:
	enum { THEORA_STATE_NONE = 0, THEORA_STATE_PLAYING = 1, THEORA_STATE_PAUSED = 2, THEORA_STATE_FINISHED = 3 };
public:
	//DECLARE_PERSISTENT(CVidTheoraPlayer, CBBase);

	CVidTheoraPlayer(CBGame *inGame);
	virtual ~CVidTheoraPlayer(void);

	// Vorbis/Theora structs
	/*ogg_sync_state   m_OggSyncState;
	ogg_page         m_OggPage;
	ogg_stream_state m_VorbisStreamState;
	ogg_stream_state m_TheoraStreamState;

	theora_info      m_TheoraInfo;
	theora_comment   m_TheoraComment;
	theora_state     m_TheoraState;

	vorbis_info      m_VorbisInfo;
	vorbis_dsp_state m_VorbisDSPState;
	vorbis_block     m_VorbisBlock;
	vorbis_comment   m_VorbisComment;*/

	int _theoraStreams;
	int _vorbisStreams;

	//ogg_int64_t m_AudiobufGranulepos; //time position of last sample


	// external objects
	CBFile *_file;
	char *_filename;

	//CBSoundTheora *_sound;
	//ogg_int16_t *_audioBuf;
	int _audioBufSize;
	int _audioBufFill;

	CBSurface *_texture;
	//CVidSubtitler *_subtitler;

	// control methods
	HRESULT Initialize(char *Filename, char *SubtitleFile = NULL);
	HRESULT InitializeSimple();
	HRESULT Update();
	HRESULT Play(TVideoPlayback Type = VID_PLAY_CENTER, int X = 0, int Y = 0, bool FreezeGame = false, bool FreezeMusic = true, bool Looping = false, uint32 StartTime = 0, float ForceZoom = -1.0f, int Volume = -1);
	HRESULT Stop();
	HRESULT Display(uint32 Alpha = 0xFFFFFFFF);
	//HRESULT RenderFrame(CBSurface *Texture, yuv_buffer *yuv);

	HRESULT Pause();
	HRESULT Resume();

	bool IsPlaying()  {
		return _state == THEORA_STATE_PLAYING;
	};
	bool IsFinished() {
		return _state == THEORA_STATE_FINISHED;
	};
	bool IsPaused()   {
		return _state == THEORA_STATE_PAUSED;
	};

	float GetMovieTime();
	int GetMovieFrame();

	CBSurface *GetTexture();

	int _state;
	uint32 _startTime;

	int _savedState;
	uint32 _savedPos;


	// alpha related
	CBImage *_alphaImage;
	char *_alphaFilename;
	HRESULT SetAlphaImage(const char *Filename);
	__inline byte GetAlphaAt(int X, int Y);

	HRESULT SeekToTime(uint32 Time);


	void Cleanup();
	HRESULT ResetStream();

	// video properties
	TVideoPlayback m_PlaybackType;
	int _posX;
	int _posY;
	float _playZoom;
	int _volume;

	bool _looping;
	bool _dontDropFrames;
	bool _freezeGame;
	uint32 _currentTime;


private:
	// data streaming
	//int BufferData(ogg_sync_state *OggSyncState);
	//int StreamInData();


	// lookup tables
	unsigned int _yTable[256];
	unsigned int _bUTable[256];
	unsigned int _gUTable[256];
	unsigned int _gVTable[256];
	unsigned int _rVTable[256];

	void GenLookupTables();


	// seeking support
	bool _seekingKeyframe;
	float _timeOffset;

	bool _frameRendered;


	// decoding
	void DecodeVorbis();
	void DecodeTheora();

	bool _audioFrameReady;
	bool _videoFrameReady;
	float _videobufTime;

	HRESULT WriteAudio();
	HRESULT WriteVideo();

	bool _playbackStarted;

	// helpers
	void SetDefaults();

};

} // end of namespace WinterMute

#endif
