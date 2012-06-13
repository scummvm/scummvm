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
#include "video/video_decoder.h"
#include "common/stream.h"
//#include <theora/theora.h>

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
	vorbis_comment   m_VorbisComment;

	int _theoraStreams;
	int _vorbisStreams;*/

	//ogg_int64_t m_AudiobufGranulepos; //time position of last sample


	// external objects
	Common::SeekableReadStream *_file;
	Common::String _filename;

	//CBSoundTheora *_sound;
	//ogg_int16_t *_audioBuf;
	/*int _audioBufSize;
	int _audioBufFill;*/

	CBSurface *_texture;
	//CVidSubtitler *_subtitler;

	// control methods
	HRESULT initialize(const Common::String &filename, const Common::String &subtitleFile = NULL);
	HRESULT initializeSimple();
	HRESULT update();
	HRESULT play(TVideoPlayback Type = VID_PLAY_CENTER, int X = 0, int Y = 0, bool FreezeGame = false, bool FreezeMusic = true, bool Looping = false, uint32 StartTime = 0, float ForceZoom = -1.0f, int Volume = -1);
	HRESULT stop();
	HRESULT display(uint32 Alpha = 0xFFFFFFFF);
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
	int getMovieFrame();

	CBSurface *getTexture();

	int _state;
	uint32 _startTime;

	int _savedState;
	uint32 _savedPos;


	// alpha related
	CBImage *_alphaImage;
	Common::String _alphaFilename;
	HRESULT setAlphaImage(const Common::String &filename);
	__inline byte getAlphaAt(int X, int Y);
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
