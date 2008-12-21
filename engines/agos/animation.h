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
 * $URL$
 * $Id$
 *
 */

#ifndef AGOS_ANIMATION_H
#define AGOS_ANIMATION_H

#include "common/file.h"
#include "common/stream.h"

#include "graphics/dxa_player.h"
#include "graphics/smk_player.h"
#include "sound/mixer.h"

namespace AGOS {

class AGOSEngine;

class MoviePlayer {
	friend class MoviePlayerDXA;
	friend class MoviePlayerSMK;

	AGOSEngine *_vm;

	Audio::Mixer *_mixer;

	Audio::SoundHandle _bgSound;
	Audio::AudioStream *_bgSoundStream;

	bool _leftButtonDown;
	bool _rightButtonDown;
	bool _skipMovie;
	uint32 _ticks;
	uint16 _frameSkipped;

	char baseName[40];
public:
	MoviePlayer(AGOSEngine *vm);
	virtual ~MoviePlayer();

	virtual bool load() = 0;
	virtual void play();
	virtual void playVideo() {};
	virtual void nextFrame() {};
	virtual void stopVideo() {};

private:
	virtual void handleNextFrame();
	virtual bool processFrame() = 0;
	virtual void startSound() {};
};

class MoviePlayerDXA : public MoviePlayer, ::Graphics::DXAPlayer {
	static const char *_sequenceList[90];
	uint8 _sequenceNum;
public:
	MoviePlayerDXA(AGOSEngine *vm, const char *name);

	bool load();
	void playVideo();
	void nextFrame();
	virtual void stopVideo();
protected:
	void setPalette(byte *pal);

private:
	void handleNextFrame();
	bool processFrame();
	void startSound();
};

class MoviePlayerSMK : public MoviePlayer, ::Graphics::SMKPlayer {
public:
	MoviePlayerSMK(AGOSEngine *vm, const char *name);

	bool load();
	void playVideo();
	void nextFrame();
	virtual void stopVideo();
protected:
	void setPalette(byte *pal);
private:
	void handleNextFrame();
	bool processFrame();
	void startSound();
};

MoviePlayer *makeMoviePlayer(AGOSEngine *vm, const char *name);

} // End of namespace AGOS

#endif
