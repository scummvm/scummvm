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

#ifndef WINTERMUTE_BSOUNDBUFFER_H
#define WINTERMUTE_BSOUNDBUFFER_H


#include "engines/wintermute/Base/BBase.h"
#include "common/stream.h"

namespace Audio {
class SeekableAudioStream;
class SoundHandle;
}

namespace WinterMute {

class CBFile;
class CBSoundBuffer : public CBBase {
public:

	CBSoundBuffer(CBGame *inGame);
	virtual ~CBSoundBuffer();


	HRESULT pause();
	HRESULT play(bool Looping = false, uint32 StartSample = 0);
	HRESULT resume();
	HRESULT stop();
	bool isPlaying();

	void setLooping(bool looping);

	uint32 getPosition();
	HRESULT setPosition(uint32 Pos);
	uint32 getLength();

	HRESULT setLoopStart(uint32 Pos);
	uint32 getLoopStart() const {
		return _loopStart;
	}

	HRESULT setPan(float Pan);
	HRESULT setPrivateVolume(int Volume);
	HRESULT setVolume(int Volume);

	void setType(TSoundType Type);

	HRESULT loadFromFile(const char *filename, bool ForceReload = false);
	void setStreaming(bool Streamed, uint32 NumBlocks = 0, uint32 BlockSize = 0);
	HRESULT applyFX(TSFXType Type, float Param1, float Param2, float Param3, float Param4);

	//HSTREAM _stream;
	//HSYNC _sync;
	Audio::SeekableAudioStream *_stream;
	Audio::SoundHandle *_handle;

	bool _freezePaused;
	uint32 _loopStart;
	TSoundType _type;
	bool _looping;
	Common::SeekableReadStream *_file;
	char *_filename;
	bool _streamed;
	int _privateVolume;

	/*static void CALLBACK LoopSyncProc(HSYNC handle, uint32 channel, uint32 data, void *user);

	static void CALLBACK FileCloseProc(void *user);
	static QWORD CALLBACK FileLenProc(void *user);
	static uint32 CALLBACK FileReadProc(void *buffer, uint32 length, void *user);
	static BOOL CALLBACK FileSeekProc(QWORD offset, void *user);*/
};

} // end of namespace WinterMute

#endif
