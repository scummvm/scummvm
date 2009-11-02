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

#ifndef SWORD1_ANIMATION_H
#define SWORD1_ANIMATION_H

#include "graphics/video/dxa_decoder.h"
#include "graphics/video/smk_decoder.h"
#include "graphics/video/video_player.h"

#include "sword1/screen.h"
#include "sword1/sound.h"
#include "sound/audiostream.h"

namespace Sword1 {

enum DecoderType {
	kVideoDecoderDXA = 0,
	kVideoDecoderSMK = 1
};

class MovieText {
public:
	uint16 _startFrame;
	uint16 _endFrame;
	char *_text;
	MovieText(int startFrame, int endFrame, const char *text) {
		_startFrame = startFrame;
		_endFrame = endFrame;
		_text = strdup(text);
	}
	~MovieText() {
		free(_text);
	}
};

class DXADecoderWithSound : public Graphics::DXADecoder {
public:
	DXADecoderWithSound(Audio::Mixer *mixer, Audio::SoundHandle *bgSoundHandle);
	~DXADecoderWithSound() {}

	int32 getAudioLag();
private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle *_bgSoundHandle;
};

class MoviePlayer : public Graphics::VideoPlayer {
public:
	MoviePlayer(SwordEngine *vm, Text *textMan, Audio::Mixer *snd, OSystem *system, Audio::SoundHandle *bgSoundHandle, Graphics::VideoDecoder *decoder, DecoderType decoderType);
	virtual ~MoviePlayer();
	bool load(uint32 id);
	void play();
protected:
	SwordEngine *_vm;
	Text *_textMan;
	Audio::Mixer *_snd;
	OSystem *_system;
	Common::Array<MovieText *> _movieTexts;
	int _textX, _textY, _textWidth, _textHeight;
	DecoderType _decoderType;

	Audio::SoundHandle *_bgSoundHandle;
	Audio::AudioStream *_bgSoundStream;

	void performPostProcessing(byte *screen);
};

MoviePlayer *makeMoviePlayer(uint32 id, SwordEngine *vm, Text *textMan, Audio::Mixer *snd, OSystem *system);

} // End of namespace Sword1

#endif
