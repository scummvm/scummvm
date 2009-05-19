/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "common/file.h"
#include "common/system.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/resman.h"
#include "sword2/sound.h"
#include "sword2/screen.h"
#include "sword2/animation.h"

#include "gui/message.h"

namespace Sword2 {

///////////////////////////////////////////////////////////////////////////////
// Basic movie player
///////////////////////////////////////////////////////////////////////////////

static const MovieInfo sequenceList[19] = {
	{ "carib",    222, false },
	{ "escape",   187, false },
	{ "eye",      248, false },
	{ "finale",  1485, false },
	{ "guard",     75, false },
	{ "intro",   1800, false },
	{ "jungle",   186, false },
	{ "museum",   167, false },
	{ "pablo",     75, false },
	{ "pyramid",   60, false },
	{ "quaram",   184, false },
	{ "river",    656, false },
	{ "sailing",  138, false },
	{ "shaman",   788, true  },
	{ "stone1",    34, true  },
	{ "stone2",   282, false },
	{ "stone3",    65, true  },
	{ "demo",      60, false },
	{ "enddemo",  110, false }
};

MoviePlayer::MoviePlayer(Sword2Engine *vm, Audio::Mixer *snd, OSystem *system, Audio::SoundHandle *bgSoundHandle, Graphics::VideoDecoder *decoder, DecoderType decoderType)
	: _vm(vm), _snd(snd), _bgSoundHandle(bgSoundHandle), _system(system), VideoPlayer(decoder) {
	_bgSoundStream = NULL;
	_decoderType = decoderType;
}

MoviePlayer:: ~MoviePlayer(void) {
	delete _bgSoundHandle;
	delete _decoder;
}

/**
 * Plays an animated cutscene.
 * @param id the id of the file
 */
bool MoviePlayer::load(const char *name) {
	_id = -1;

	for (int i = 0; i < ARRAYSIZE(sequenceList); i++) {
		if (scumm_stricmp(name, sequenceList[i].name) == 0) {
			_id = i;
			break;
		}
	}

	if (_decoderType == kVideoDecoderDXA) {
		_bgSoundStream = Audio::AudioStream::openStreamFile(name);
	} else {
		_bgSoundStream = NULL;
	}

	_textSurface = NULL;

	char filename[20];
	switch (_decoderType) {
	case kVideoDecoderDXA:
		snprintf(filename, sizeof(filename), "%s.dxa", name);
		break;
	case kVideoDecoderSMK:
		snprintf(filename, sizeof(filename), "%s.smk", name);
		break;
	}

	if (_decoder->loadFile(filename)) {
		// The DXA animations in the Broken Sword games always use external audio tracks,
		// if they have any sound at all.
		if (_decoderType == kVideoDecoderDXA && _decoder->readSoundHeader() != MKID_BE('NULL'))
			return false;
	} else {
		return false;
	}

	return true;
}

void MoviePlayer::play(MovieText *movieTexts, uint32 numMovieTexts, uint32 leadIn, uint32 leadOut) {
	// This happens when quitting during the "eye" cutscene.
	if (_vm->shouldQuit())
		return;

	bool seamless = false;

	if (_id >= 0) {
		seamless = sequenceList[_id].seamless;
		_numFrames = sequenceList[_id].frames;
		if (_numFrames > 60)
			_leadOutFrame = _numFrames - 60;
	}

	_movieTexts = movieTexts;
	_numMovieTexts = numMovieTexts;
	_currentMovieText = 0;
	_leadOut = leadOut;

	if (leadIn) {
		_vm->_sound->playMovieSound(leadIn, kLeadInSound);
	}

	if (_bgSoundStream) {
		_snd->playInputStream(Audio::Mixer::kSFXSoundType, _bgSoundHandle, _bgSoundStream);
	}

	bool terminated = false;

	Common::List<Common::Event> stopEvents;
	Common::Event stopEvent;
	stopEvents.clear();
	stopEvent.type = Common::EVENT_KEYDOWN;
	stopEvent.kbd = Common::KEYCODE_ESCAPE;
	stopEvents.push_back(stopEvent);

	terminated = !playVideo(stopEvents);

	closeTextObject(_currentMovieText);

	if (terminated) {
		_snd->stopHandle(*_bgSoundHandle);
		_vm->_sound->stopMovieSounds();
		_vm->_sound->stopSpeech();
	}

	while (_snd->isSoundHandleActive(*_bgSoundHandle))
		_system->delayMillis(100);
}

void MoviePlayer::openTextObject(uint32 index) {
	MovieText *text = &_movieTexts[index];

	// Pull out the text line to get the official text number (for WAV id)

	uint32 res = text->_textNumber / SIZE;
	uint32 localText = text->_textNumber & 0xffff;

	// Open text resource and get the line

	byte *textData = _vm->fetchTextLine(_vm->_resman->openResource(res), localText);

	text->_speechId = READ_LE_UINT16(textData);

	// Is it speech or subtitles, or both?

	// If we want subtitles, or there was no sound

	if (_vm->getSubtitles() || !text->_speechId) {
		text->_textMem = _vm->_fontRenderer->makeTextSprite(textData + 2, 600, 255, _vm->_speechFontId, 1);
	}

	_vm->_resman->closeResource(res);

	if (text->_textMem) {
		FrameHeader frame;

		frame.read(text->_textMem);

		text->_textSprite.x = 320 - frame.width / 2;
		text->_textSprite.y = 440 - frame.height;
		text->_textSprite.w = frame.width;
		text->_textSprite.h = frame.height;
		text->_textSprite.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION;
		text->_textSprite.data = text->_textMem + FrameHeader::size();
		text->_textSprite.isText = true;
		_vm->_screen->createSurface(&text->_textSprite, &_textSurface);

		_textX = 320 - text->_textSprite.w / 2;
		_textY = 420 - text->_textSprite.h;
	}
}

void MoviePlayer::closeTextObject(uint32 index) {
	if (index < _numMovieTexts) {
		MovieText *text = &_movieTexts[index];

		free(text->_textMem);
		text->_textMem = NULL;

		if (_textSurface) {
			_vm->_screen->deleteSurface(_textSurface);
			_textSurface = NULL;
		}
	}
}

void MoviePlayer::drawTextObject(uint32 index, byte *screen) {
	MovieText *text = &_movieTexts[index];

	byte white = _decoder->getWhite();
	byte black = _decoder->getBlack();

	if (text->_textMem && _textSurface) {
		byte *src = text->_textSprite.data;
		uint16 width = text->_textSprite.w;
		uint16 height = text->_textSprite.h;

		// Resize text sprites for PSX version
		if (Sword2Engine::isPsx()) { 
			height *= 2;
			byte *buffer = (byte *)malloc(width * height);
			Screen::resizePsxSprite(buffer, src, width, height);
			src = buffer;
		}

		byte *dst = screen + _textY * _decoder->getWidth() + _textX;

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (src[x] == 1)
					dst[x] = black;
				else if (src[x] == 255)
					dst[x] = white;
			}
			src += width;
			dst += _decoder->getWidth();
		}
	}
}

// FIXME: This assumes that the subtitles always fit within the frame of the
// movie. In Broken Sword 2, that's a fairly safe assumption, but not
// necessarily in all other games.

void MoviePlayer::performPostProcessing(byte *screen) {
	MovieText *text;
	int frame = _decoder->getCurFrame();

	if (_currentMovieText < _numMovieTexts) {
		text = &_movieTexts[_currentMovieText];
	} else {
		text = NULL;
	}

	if (text && frame == text->_startFrame) {
		if ((_vm->getSubtitles() || !text->_speechId) && _currentMovieText < _numMovieTexts) {
			openTextObject(_currentMovieText);
		}
	}

	if (text && frame >= text->_startFrame) {
		if (text->_speechId && !text->_played && _vm->_sound->amISpeaking() == RDSE_QUIET) {
			text->_played = true;
			_vm->_sound->playCompSpeech(text->_speechId, 16, 0);
		}
		if (frame <= text->_endFrame) {
			drawTextObject(_currentMovieText, screen);
		} else {
			_currentMovieText++;
		}
	}

	if (_leadOut && _decoder->getCurFrame() == _leadOutFrame) {
		_vm->_sound->playMovieSound(_leadOut, kLeadOutSound);
	}
}

DXADecoderWithSound::DXADecoderWithSound(Audio::Mixer *mixer, Audio::SoundHandle *bgSoundHandle)
	: _mixer(mixer), _bgSoundHandle(bgSoundHandle)  {
}

int32 DXADecoderWithSound::getAudioLag() {
	if (!_fileStream)
		return 0;

	if (!_mixer->isSoundHandleActive(*_bgSoundHandle))
		return 0;

	int32 frameDelay = getFrameDelay();
	int32 videoTime = _videoInfo.currentFrame * frameDelay;
	int32 audioTime;

	audioTime = (int32) _mixer->getSoundElapsedTime(*_bgSoundHandle);

	return videoTime - audioTime;
}

///////////////////////////////////////////////////////////////////////////////
// Factory function for creating the appropriate cutscene player
///////////////////////////////////////////////////////////////////////////////

MoviePlayer *makeMoviePlayer(const char *name, Sword2Engine *vm, Audio::Mixer *snd, OSystem *system) {
	char filename[20];
	char buf[60];
	Audio::SoundHandle *bgSoundHandle = new Audio::SoundHandle;

	snprintf(filename, sizeof(filename), "%s.smk", name);

	if (Common::File::exists(filename)) {
		Graphics::SmackerDecoder *smkDecoder = new Graphics::SmackerDecoder(snd);
		return new MoviePlayer(vm, snd, system, bgSoundHandle, smkDecoder, kVideoDecoderSMK);
	}

	snprintf(filename, sizeof(filename), "%s.dxa", name);

	if (Common::File::exists(filename)) {
#ifdef USE_ZLIB
		DXADecoderWithSound *dxaDecoder = new DXADecoderWithSound(snd, bgSoundHandle);
		return new MoviePlayer(vm, snd, system, bgSoundHandle, dxaDecoder, kVideoDecoderDXA);
#else
		GUI::MessageDialog dialog("DXA cutscenes found but ScummVM has been built without zlib support", "OK");
		dialog.runModal();
		return NULL;
#endif
	}

	// Old MPEG2 cutscenes
	snprintf(filename, sizeof(filename), "%s.mp2", name);

	if (Common::File::exists(filename)) {
		GUI::MessageDialog dialog("MPEG2 cutscenes are no longer supported", "OK");
		dialog.runModal();
		return NULL;
	}

	// The demo tries to play some cutscenes that aren't there, so make
	// those warnings more discreet.
	if (!vm->_logic->readVar(DEMO)) {
		sprintf(buf, "Cutscene '%s' not found", name);
		GUI::MessageDialog dialog(buf, "OK");
		dialog.runModal();
	} else
		warning("Cutscene '%s' not found", name);

	return NULL;
}

void MoviePlayer::pauseMovie(bool pause) {
	if (_bgSoundHandle) {
		_snd->pauseHandle(*_bgSoundHandle, pause);
	}
}

} // End of namespace Sword2
