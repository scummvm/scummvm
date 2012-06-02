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


#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/video/vidtheoraplayer.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/Base/BSurfaceSDL.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/video/decoders/theora_decoder.h"
#include "common/system.h"
//#pragma comment(lib, "libtheora.lib")

namespace WinterMute {

//IMPLEMENT_PERSISTENT(CVidTheoraPlayer, false);

//////////////////////////////////////////////////////////////////////////
CVidTheoraPlayer::CVidTheoraPlayer(CBGame *inGame): CBBase(inGame) {
	SetDefaults();
}

//////////////////////////////////////////////////////////////////////////
void CVidTheoraPlayer::SetDefaults() {

	_file = NULL;
	_filename = "";
	_startTime = 0;
	_looping = false;

	_freezeGame = false;
	_currentTime = 0;

	_state = THEORA_STATE_NONE;

	_videoFrameReady = false;
	_audioFrameReady = false;
	_videobufTime = 0;

	_playbackStarted = false;
	_dontDropFrames = false;

	_texture = NULL;
	_alphaImage = NULL;
	_alphaFilename = NULL;

	_frameRendered = false;

	_seekingKeyframe = false;
	_timeOffset = 0.0f;
	
	_posX = _posY = 0;
	_playbackType = VID_PLAY_CENTER;
	_playZoom = 0.0f;
	
	_savedState = THEORA_STATE_NONE;
	_savedPos = 0;
	_volume = 100;
#if 0
	_vorbisStreams = _theoraStreams = 0;

	GenLookupTables();

	_subtitler = NULL;
#endif
}

//////////////////////////////////////////////////////////////////////////
CVidTheoraPlayer::~CVidTheoraPlayer(void) {
	cleanup();

/*	SAFE_DELETE_ARRAY(_filename);
	SAFE_DELETE_ARRAY(_alphaFilename);
	SAFE_DELETE(_texture);
	SAFE_DELETE(_alphaImage);*/
//	SAFE_DELETE(_subtitler);
}

//////////////////////////////////////////////////////////////////////////
void CVidTheoraPlayer::cleanup() {
#if 0
	if (m_Sound) {
		Game->m_SoundMgr->RemoveSound(m_Sound);
		m_Sound = NULL;
	}

	SAFE_DELETE_ARRAY(m_AudioBuf);
	m_AudioBufFill = 0;
	m_AudioBufSize = 0;
#endif
}

//////////////////////////////////////////////////////////////////////////
HRESULT CVidTheoraPlayer::initialize(const Common::String &filename, const Common::String &subtitleFile) {
	cleanup();
	
	_file = Game->_fileManager->OpenFile(filename, true, false);
	if (!_file) return E_FAIL;
	
	//if (Filename != _filename) CBUtils::SetString(&_filename, Filename);
	_theoraDecoder = new TheoraDecoder();
	_theoraDecoder->loadStream(_file);
	
	if (!_theoraDecoder->isVideoLoaded())
		return E_FAIL;
	
	_state = THEORA_STATE_PAUSED;

	// Additional setup.
	_surface.create(_theoraDecoder->getWidth(), _theoraDecoder->getHeight(), _theoraDecoder->getPixelFormat());
	_texture = new CBSurfaceSDL(Game);
	_texture->Create(_theoraDecoder->getWidth(), _theoraDecoder->getHeight());
	_state = THEORA_STATE_PLAYING;
	_playZoom = 100;
	
	return S_OK;
#if 0
	Cleanup();

	_file = Game->_fileManager->OpenFile(Filename);
	if (!_file) return E_FAIL;

	if (Filename != _filename) CBUtils::SetString(&_filename, Filename);

	// start up Ogg stream synchronization layer
	ogg_sync_init(&m_OggSyncState);

	// init supporting Vorbis structures needed in header parsing
	vorbis_comment_init(&m_VorbisComment);
	vorbis_info_init(&m_VorbisInfo);

	// init supporting Theora structures needed in header parsing
	theora_comment_init(&m_TheoraComment);
	theora_info_init(&m_TheoraInfo);



	// Ogg file open; parse the headers
	// Only interested in Vorbis/Theora streams
	ogg_packet TempOggPacket;
	bool IsDone = false;
	while (!IsDone) {
		int BytesRead = BufferData(&m_OggSyncState);
		if (BytesRead == 0) break;

		while (ogg_sync_pageout(&m_OggSyncState, &m_OggPage) > 0) {
			ogg_stream_state OggStateTest;

			// is this a mandated initial header? If not, stop parsing
			if (!ogg_page_bos(&m_OggPage)) {
				// don't leak the page; get it into the appropriate stream
				if (m_TheoraStreams)
					ogg_stream_pagein(&m_TheoraStreamState, &m_OggPage);
				if (m_VorbisStreams)
					ogg_stream_pagein(&m_VorbisStreamState, &m_OggPage);

				IsDone = true;
				break;
			}

			ogg_stream_init(&OggStateTest, ogg_page_serialno(&m_OggPage));
			ogg_stream_pagein(&OggStateTest, &m_OggPage);
			ogg_stream_packetout(&OggStateTest, &TempOggPacket);

			// identify the codec: try theora
			if (!m_TheoraStreams && theora_decode_header(&m_TheoraInfo, &m_TheoraComment, &TempOggPacket) >= 0) {
				// it is theora
				memcpy(&m_TheoraStreamState, &OggStateTest, sizeof(OggStateTest));
				m_TheoraStreams = 1;
			} else if (!m_VorbisStreams && vorbis_synthesis_headerin(&m_VorbisInfo, &m_VorbisComment, &TempOggPacket) >= 0) {
				// it is vorbis
				memcpy(&m_VorbisStreamState, &OggStateTest, sizeof(OggStateTest));
				m_VorbisStreams = 1;
			} else {
				// whatever it is, we don't care about it
				ogg_stream_clear(&OggStateTest);
			}
		}
	}

	// we're expecting more header packets
	while ((m_TheoraStreams && m_TheoraStreams < 3) || (m_VorbisStreams && m_VorbisStreams < 3)) {
		int Ret;

		// look for further theora headers
		while (m_TheoraStreams && (m_TheoraStreams < 3) && (Ret = ogg_stream_packetout(&m_TheoraStreamState, &TempOggPacket))) {
			if (Ret < 0) {
				Game->LOG(0, "Error parsing Theora stream headers; corrupt stream?");
				return E_FAIL;
			}
			if (theora_decode_header(&m_TheoraInfo, &m_TheoraComment, &TempOggPacket)) {
				Game->LOG(0, "Error parsing Theora stream headers; corrupt stream?");
				return E_FAIL;
			}
			m_TheoraStreams++;
			if (m_TheoraStreams == 3) break;
		}

		/* look for more vorbis header packets */
		while (m_VorbisStreams && (m_VorbisStreams < 3) && (Ret = ogg_stream_packetout(&m_VorbisStreamState, &TempOggPacket))) {
			if (Ret < 0) {
				Game->LOG(0, "Error parsing Vorbis stream headers; corrupt stream?");
				return E_FAIL;
			}
			if (vorbis_synthesis_headerin(&m_VorbisInfo, &m_VorbisComment, &TempOggPacket)) {
				Game->LOG(0, "Error parsing Vorbis stream headers; corrupt stream?");
				return E_FAIL;
			}
			m_VorbisStreams++;
			if (m_VorbisStreams == 3) break;
		}

		// The header pages/packets will arrive before anything else we
		// care about, or the stream is not obeying spec
		if (ogg_sync_pageout(&m_OggSyncState, &m_OggPage) > 0) {
			if (m_TheoraStreams)
				ogg_stream_pagein(&m_TheoraStreamState, &m_OggPage);
			if (m_VorbisStreams)
				ogg_stream_pagein(&m_VorbisStreamState, &m_OggPage);
		} else {
			int Ret = BufferData(&m_OggSyncState); // someone needs more data
			if (Ret == 0) {
				Game->LOG(0, "End of file while searching for codec headers");
				return E_FAIL;
			}
		}
	}



	// and now we have it all.  initialize decoders
	if (m_TheoraStreams) {
		theora_decode_init(&m_TheoraState, &m_TheoraInfo);
	} else {
		// tear down the partial theora setup
		theora_info_clear(&m_TheoraInfo);
		theora_comment_clear(&m_TheoraComment);
	}

	if (m_VorbisStreams) {
		vorbis_synthesis_init(&m_VorbisDSPState, &m_VorbisInfo);
		vorbis_block_init(&m_VorbisDSPState, &m_VorbisBlock);

	} else {
		// tear down the partial vorbis setup
		vorbis_info_clear(&m_VorbisInfo);
		vorbis_comment_clear(&m_VorbisComment);
	}

	HRESULT Res = S_OK;

	// create sound buffer
	if (m_VorbisStreams && Game->m_SoundMgr->m_SoundAvailable) {
		m_Sound = new CBSoundTheora(Game);
		Game->m_SoundMgr->AddSound(m_Sound);
		if (FAILED(Res = m_Sound->InitializeBuffer(this))) {
			Game->m_SoundMgr->RemoveSound(m_Sound);
			m_Sound = NULL;
			Game->LOG(Res, "Error initializing sound buffer for Theora file '%s'", Filename);
		} else {
			SAFE_DELETE_ARRAY(m_AudioBuf);
			m_AudioBufSize = m_Sound->m_StreamBlockSize;
			m_AudioBuf = new ogg_int16_t[m_AudioBufSize];
		}
	}

	// create texture
	if (m_TheoraStreams && !m_Texture) {
		if (Game->m_UseD3D)
			m_Texture = new CBSurfaceD3D(Game);
		else
			m_Texture = new CBSurfaceDD(Game);

		if (!m_Texture || FAILED(Res = m_Texture->Create(m_TheoraInfo.width, m_TheoraInfo.height))) {
			SAFE_DELETE(m_Texture);
		}
	}


	if (!m_Subtitler) m_Subtitler = new CVidSubtitler(Game);
	if (m_Subtitler && Game->m_VideoSubtitles) m_Subtitler->LoadSubtitles(Filename, SubtitleFile);

	return Res;
#endif
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CVidTheoraPlayer::resetStream() {
#if 0
	if (_sound) _sound->Stop();

	m_TimeOffset = 0.0f;
	Initialize(m_Filename);
	Play(m_PlaybackType, m_PosX, m_PosY, false, false, m_Looping, 0, m_PlayZoom);
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CVidTheoraPlayer::play(TVideoPlayback Type, int X, int Y, bool FreezeGame, bool FreezeMusic, bool Looping, uint32 StartTime, float ForceZoom, int Volume) {
	if (_theoraDecoder) {
		_surface.copyFrom(*_theoraDecoder->decodeNextFrame());
		_state = THEORA_STATE_PLAYING;
		return S_OK;
	}
#if 0
	if (ForceZoom < 0.0f) ForceZoom = 100.0f;
	if (Volume < 0) m_Volume = Game->m_SoundMgr->GetVolumePercent(SOUND_SFX);
	else m_Volume = Volume;

	m_FreezeGame = FreezeGame;
	if (!m_PlaybackStarted && m_FreezeGame) Game->Freeze(FreezeMusic);

	m_PlaybackStarted = false;
	m_State = THEORA_STATE_PLAYING;

	m_Looping = Looping;
	m_PlaybackType = Type;

	float Width, Height;
	if (m_TheoraStreams) {
		Width = (float)m_TheoraInfo.width;
		Height = (float)m_TheoraInfo.height;
	} else {
		Width = (float)Game->m_Renderer->m_Width;
		Height = (float)Game->m_Renderer->m_Height;
	}

	switch (Type) {
	case VID_PLAY_POS:
		m_PlayZoom = ForceZoom;
		m_PosX = X;
		m_PosY = Y;
		break;

	case VID_PLAY_STRETCH: {
		float ZoomX = (float)((float)Game->m_Renderer->m_Width / Width * 100);
		float ZoomY = (float)((float)Game->m_Renderer->m_Height / Height * 100);
		m_PlayZoom = min(ZoomX, ZoomY);
		m_PosX = (Game->m_Renderer->m_Width - Width * (m_PlayZoom / 100)) / 2;
		m_PosY = (Game->m_Renderer->m_Height - Height * (m_PlayZoom / 100)) / 2;
	}
	break;

	case VID_PLAY_CENTER:
		m_PlayZoom = 100.0f;
		m_PosX = (Game->m_Renderer->m_Width - Width) / 2;
		m_PosY = (Game->m_Renderer->m_Height - Height) / 2;
		break;
	}


	if (StartTime) SeekToTime(StartTime);

	Update();
#endif
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CVidTheoraPlayer::stop() {
#if 0
	if (m_Sound) m_Sound->Stop();
	m_State = THEORA_STATE_FINISHED;
	if (m_FreezeGame) Game->Unfreeze();
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CVidTheoraPlayer::update() {
	if (_theoraDecoder) {
		if (_theoraDecoder->endOfVideo()) {
			warning("Finished movie");
			_state = THEORA_STATE_FINISHED;
		}
		if (_state == THEORA_STATE_PLAYING) {
			if (_theoraDecoder->getTimeToNextFrame() == 0) {
				_surface.copyFrom(*_theoraDecoder->decodeNextFrame());
				_videoFrameReady = true;
			}
			if (_texture && _videoFrameReady) {
				WriteVideo();
			}
			return S_OK;
		}
	}

#if 0
	m_CurrentTime = m_FreezeGame ? Game->m_LiveTimer : Game->m_Timer;

	if (!IsPlaying()) return S_OK;

	if (m_PlaybackStarted && m_Sound && !m_Sound->IsPlaying()) return S_OK;

	if (m_PlaybackStarted && !m_FreezeGame && Game->m_State == GAME_FROZEN) return S_OK;

	int Counter = 0;
	while (true) {
		if (m_Sound) DecodeVorbis();
		else m_AudioFrameReady = true;

		if (m_Texture) DecodeTheora();
		else m_VideoFrameReady = true;

		if ((!m_Sound || !m_AudioFrameReady) && (!m_Texture || !m_VideoFrameReady) && m_File->IsEOF()) {
			// end playback
			if (!m_Looping) {
				m_State = THEORA_STATE_FINISHED;
				if (m_Sound) m_Sound->Stop();
				if (m_FreezeGame) Game->Unfreeze();
				break;
			} else {
				ResetStream();
				return S_OK;
			}
		}


		if (!m_VideoFrameReady || !m_AudioFrameReady) {
			Counter++;
			if (StreamInData() == 0) break;
		} else break;
	}


	// If playback has begun, top audio buffer off immediately.
	//if(m_Sound) WriteAudio();

	// are we at or past time for this video frame?
	if (m_PlaybackStarted && m_VideoFrameReady && (!m_FrameRendered || m_VideobufTime <= GetMovieTime())) {
		//Game->LOG(0, "%f %f", m_VideobufTime, GetMovieTime());
		if (m_Texture) WriteVideo();
		m_VideoFrameReady = false;

		if (m_SavedState == THEORA_STATE_PAUSED) {
			Pause();
			m_SavedState = THEORA_STATE_NONE;
		}
	}

	// if our buffers either don't exist or are ready to go,
	// we can begin playback
	bool StartNow = false;
	if ((!m_TheoraStreams || m_VideoFrameReady) &&
	        (!m_VorbisStreams || m_AudioFrameReady)) StartNow = true;
	// same if we've run out of input
	if (m_File->IsEOF()) StartNow = true;


	if (m_Sound) WriteAudio();


	if (!m_PlaybackStarted && StartNow && !m_SeekingKeyframe) {
		//m_StartTime = timeGetTime();
		m_StartTime = m_CurrentTime;
		if (m_Sound) {
			m_Sound->SetPrivateVolume(m_Volume);
			m_Sound->Play();
		}
		m_PlaybackStarted = true;
	}

	if (m_Subtitler && Game->m_VideoSubtitles) m_Subtitler->Update(GetMovieFrame());
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
float CVidTheoraPlayer::getMovieTime() {
	if (!_playbackStarted) {
		return 0.0f;
	} else {
		return _theoraDecoder->getTime();
	}
}


//////////////////////////////////////////////////////////////////////////
int CVidTheoraPlayer::getMovieFrame() {
#if 0
	if (!m_TheoraStreams) return 0;
	float Time = GetMovieTime();

	return Time / ((double)m_TheoraInfo.fps_denominator / m_TheoraInfo.fps_numerator);
#endif
	return 0;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CVidTheoraPlayer::WriteVideo() {
	if (!_texture) return E_FAIL;

	_texture->StartPixelOp();
	//RenderFrame(_texture, &yuv);
	_texture->PutSurface(_surface);
	_texture->EndPixelOp();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CVidTheoraPlayer::display(uint32 Alpha) {

	RECT rc;
	HRESULT Res;

	if (_texture) {
		CBPlatform::SetRect(&rc, 0, 0, _texture->GetWidth(), _texture->GetHeight());
		if (_playZoom == 100.0f) Res = _texture->DisplayTrans(_posX, _posY, rc, Alpha);
		else Res = _texture->DisplayTransZoom(_posX, _posY, rc, _playZoom, _playZoom, Alpha);
	} else Res = E_FAIL;
#if 0
	if (m_Subtitler && Game->m_VideoSubtitles) m_Subtitler->Display();
#endif
	return Res;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CVidTheoraPlayer::setAlphaImage(const char *filename) {
	warning("CVidTheoraPlayer::SetAlphaImage(%s) - Not implemented", filename);
#if 0
	SAFE_DELETE(m_AlphaImage);
	m_AlphaImage = new CBImage(Game);
	if (!m_AlphaImage || FAILED(m_AlphaImage->LoadFile(Filename))) {
		SAFE_DELETE(m_AlphaImage);
		SAFE_DELETE_ARRAY(m_AlphaFilename);
		return E_FAIL;
	}
	if (m_AlphaFilename != Filename) CBUtils::SetString(&m_AlphaFilename, Filename);
	m_AlphaImage->Convert(IMG_TRUECOLOR);
#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
byte CVidTheoraPlayer::getAlphaAt(int X, int Y) {
#if 0
	if (_alphaImage) return _alphaImage->GetAlphaAt(X, Y);
	else return 0xFF;
#endif
	return 0;
}


//////////////////////////////////////////////////////////////////////////
inline int intlog(int num) {
	int r = 0;
	while (num > 0) {
		num = num / 2;
		r = r + 1;
	}

	return r;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CVidTheoraPlayer::SeekToTime(uint32 time) {
	warning("CVidTheoraPlayer::SeekToTime(%d) - not supported", time);
#if 0
	if (!m_TheoraStreams) return E_FAIL;


	float TargetTime = Time / 1000.0f;


	ogg_page page;
	int read = 1;
	ogg_int64_t gran;
	float movieLength = 0;
	DWORD LastPos = 0;

	int keyframe_granule_shift = intlog(m_TheoraInfo.keyframe_frequency_force - 1);

	while (!m_File->IsEOF() && read != 0) {
		read = BufferData(&m_OggSyncState);

		while (ogg_sync_pageout(&m_OggSyncState, &page) > 0) {
			int serno = ogg_page_serialno(&page);
			//This is theora stream we were searching for
			if (m_TheoraStreamState.serialno == serno) {
				//Calculate a rough time estimate
				gran = ogg_page_granulepos(&page);
				if (gran >= 0) {
					ogg_int64_t iframe = gran >> keyframe_granule_shift;
					ogg_int64_t pframe = gran   - (iframe << keyframe_granule_shift);
					movieLength = (iframe + pframe) *
					              ((double)m_TheoraInfo.fps_denominator / m_TheoraInfo.fps_numerator);

					if (movieLength >= TargetTime) {
						m_TimeOffset = movieLength;
						//m_TimeOffset = TargetTime;
						//m_File->Seek(LastPos);

						goto finish;
					}
					LastPos = m_File->GetPos();
				}
			}
		}
	}

finish:
	ogg_sync_reset(&m_OggSyncState);

	ogg_stream_reset(&m_TheoraStreamState);
	ogg_stream_reset(&m_VorbisStreamState);

	theora_clear(&m_TheoraState);
	theora_decode_init(&m_TheoraState, &m_TheoraInfo);
	vorbis_synthesis_restart(&m_VorbisDSPState);

	m_SeekingKeyframe = true;

	//theora_packet_iskeyframe

#endif
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CVidTheoraPlayer::pause() {
	if (_state == THEORA_STATE_PLAYING) {
		_state = THEORA_STATE_PAUSED;
		_theoraDecoder->pauseVideo(true);
		return S_OK;
	} else {
		return E_FAIL;
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CVidTheoraPlayer::resume() {
	if (_state == THEORA_STATE_PAUSED) {
		_state = THEORA_STATE_PLAYING;
		_theoraDecoder->pauseVideo(false);
		return S_OK;
	} else {
		return E_FAIL;
	}
}

//////////////////////////////////////////////////////////////////////////
/*HRESULT CVidTheoraPlayer::Persist(CBPersistMgr *PersistMgr) {
	//CBBase::Persist(PersistMgr);
#if 0
	if (PersistMgr->m_Saving) {
		m_SavedPos = GetMovieTime() * 1000;
		m_SavedState = m_State;
	} else {
		SetDefaults();
	}

	PersistMgr->Transfer(TMEMBER(Game));
	PersistMgr->Transfer(TMEMBER(m_SavedPos));
	PersistMgr->Transfer(TMEMBER(m_SavedState));
	PersistMgr->Transfer(TMEMBER(m_Filename));
	PersistMgr->Transfer(TMEMBER(m_AlphaFilename));
	PersistMgr->Transfer(TMEMBER(m_PosX));
	PersistMgr->Transfer(TMEMBER(m_PosY));
	PersistMgr->Transfer(TMEMBER(m_PlayZoom));
	PersistMgr->Transfer(TMEMBER_INT(m_PlaybackType));
	PersistMgr->Transfer(TMEMBER(m_Looping));

	if (PersistMgr->CheckVersion(1, 7, 3)) {
		PersistMgr->Transfer(TMEMBER(m_Volume));
	} else {
		m_Volume = 100;
	}
#endif
	return S_OK;
}
*/
//////////////////////////////////////////////////////////////////////////
HRESULT CVidTheoraPlayer::initializeSimple() {
	if (SUCCEEDED(initialize(_filename))) {
		if (_alphaFilename) setAlphaImage(_alphaFilename);
		play(_playbackType, _posX, _posY, false, false, _looping, _savedPos, _playZoom);
	} else _state = THEORA_STATE_FINISHED;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CBSurface *CVidTheoraPlayer::getTexture() {
	return _texture;
}

} // end of namespace WinterMute
