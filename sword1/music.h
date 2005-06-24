/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef BSMUSIC_H
#define BSMUSIC_H

#include "common/scummsys.h"
#include "common/mutex.h"
#include "common/file.h"
#include "sound/audiostream.h"
#include "sound/rate.h"

namespace Audio {
	class Mixer;
}

namespace Sword1 {

#define TOTAL_TUNES 270

enum MusicMode {
	MusicNone = 0,
	MusicWave,
	MusicMp3,
	MusicVorbis
};

class WaveAudioStream : public AudioStream {
public:
	WaveAudioStream(Common::File *source, uint32 pSize);
	virtual ~WaveAudioStream();
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo(void) const { return _isStereo; };
	virtual bool endOfData(void) const;
	virtual int getRate(void) const { return _rate; };
private:
	Common::File	*_sourceFile;
	uint8	*_sampleBuf;
	uint32	 _rate;
	bool	 _isStereo;
	uint32   _samplesLeft;
	uint16	 _bitsPerSample;
};

class MusicHandle : public AudioStream {
private:
	Common::File _file;
	bool _looping;
	int32 _fading;
	int32 _fadeSamples;
	MusicMode _musicMode;
	AudioStream *_audioSource;
	AudioStream *createAudioSource(void);
public:
	MusicHandle() : _looping(false), _fading(0), _audioSource(NULL) {}
	virtual int readBuffer(int16 *buffer, const int numSamples);
	bool play(const char *filename, bool loop);
	void stop();
	void fadeUp();
	void fadeDown();
	bool streaming() const;
	int32 fading() { return _fading; }
	bool endOfData() const;
	bool endOfStream() const { return false; }
	bool isStereo() const;
	int getRate() const;
};

class Music : public AudioStream {
public:
	Music(Audio::Mixer *pMixer);
	~Music();
	void startMusic(int32 tuneId, int32 loopFlag);
	void fadeDown();
	void setVolume(uint8 volL, uint8 volR);
	void giveVolume(uint8 *volL, uint8 *volR);

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) {
		mixer(buffer, numSamples / 2);
		return numSamples;
	}
	bool isStereo() const { return true; }
	bool endOfData() const { return false; }
	int getRate() const { return _sampleRate; }

private:
	Audio::st_volume_t _volumeL, _volumeR;
	MusicHandle _handles[2];
	Audio::RateConverter *_converter[2];
	Audio::Mixer *_mixer;
	uint32 _sampleRate;
	Common::Mutex _mutex;

	static void passMixerFunc(void *param, int16 *buf, uint len);
	void mixer(int16 *buf, uint32 len);

	static const char _tuneList[TOTAL_TUNES][8]; // in staticres.cpp
};

} // End of namespace Sword1 

#endif // BSMUSIC_H
