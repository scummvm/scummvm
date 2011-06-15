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

#ifndef SWORD1_ANIMATION_H
#define SWORD1_ANIMATION_H

#include "video/dxa_decoder.h"
#include "video/smk_decoder.h"
#include "video/video_decoder.h"

#include "common/list.h"

#include "audio/audiostream.h"

#include "sword1/screen.h"
#include "sword1/sound.h"

namespace Sword1 {

enum DecoderType {
	kVideoDecoderDXA = 0,
	kVideoDecoderSMK = 1
};

class MovieText {
public:
	uint16 _startFrame;
	uint16 _endFrame;
	Common::String _text;
	MovieText(int startFrame, int endFrame, const Common::String &text) {
		_startFrame = startFrame;
		_endFrame = endFrame;
		_text = text;
	}
};

class DXADecoderWithSound : public Video::DXADecoder {
public:
	DXADecoderWithSound(Audio::Mixer *mixer, Audio::SoundHandle *bgSoundHandle);
	~DXADecoderWithSound() {}

	uint32 getElapsedTime() const;

private:
	Audio::Mixer *_mixer;
	Audio::SoundHandle *_bgSoundHandle;
};

class MoviePlayer {
public:
	MoviePlayer(SwordEngine *vm, Text *textMan, Audio::Mixer *snd, OSystem *system, Audio::SoundHandle *bgSoundHandle, Video::VideoDecoder *decoder, DecoderType decoderType);
	virtual ~MoviePlayer();
	bool load(uint32 id);
	void play();

protected:
	SwordEngine *_vm;
	Text *_textMan;
	Audio::Mixer *_snd;
	OSystem *_system;
	Common::List<MovieText> _movieTexts;
	int _textX, _textY, _textWidth, _textHeight;
	byte _white, _black;
	DecoderType _decoderType;

	Video::VideoDecoder *_decoder;
	Audio::SoundHandle *_bgSoundHandle;
	Audio::AudioStream *_bgSoundStream;

	bool playVideo();
	void performPostProcessing(byte *screen);

	byte findBlackPalIndex();
	byte findWhitePalIndex();
};

MoviePlayer *makeMoviePlayer(uint32 id, SwordEngine *vm, Text *textMan, Audio::Mixer *snd, OSystem *system);

} // End of namespace Sword1

#endif
