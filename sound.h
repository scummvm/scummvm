// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef SOUND_H
#define SOUND_H

#include "bits.h"
#include "resource.h"
#include <list>

class Sound : public Resource {
public:
	Sound(const char *filename, const char *data, int len);
	~Sound();

	bool done() const { return currPos_ >= numSamples_; }

private:
	int numSamples_, numChannels_, currPos_;
	int16 *samples_;

	static void init();

	void reset();
	void mix(int16 *data, int samples);

	friend class Mixer;
};

class Mixer {
public:
	static Mixer *instance();

	void start();

	void playVoice(Sound *s);
	void playSfx(Sound *s);
	void stopSfx(Sound *s);
	void stopVoice(Sound *s);
	void setImuseState(int state);
	void setImuseSeq(int seq);

	Sound *findSfx(const char *filename);
	bool voicePlaying() const;

	void getAudio(int16 *data, int numSamples);

private:
	Mixer();
	~Mixer();

	static Mixer *instance_;
	typedef std::list<ResPtr<Sound> > sound_list;
	sound_list voiceSounds_, sfxSounds_;
	ResPtr<Sound> musicSound_, seqSound_;

	friend void mixerCallback(void *userdata, uint8 *stream, int len);
};

#endif
