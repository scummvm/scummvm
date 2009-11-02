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


#include "common/file.h"
#include "sword1/sword1.h"
#include "sword1/animation.h"
#include "sword1/text.h"
#include "sound/vorbis.h"

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/str.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "gui/message.h"

namespace Sword1 {

static const char *sequenceList[20] = {
	"ferrari",  // 0  CD2   ferrari running down fitz in sc19
	"ladder",   // 1  CD2   george walking down ladder to dig sc24->sc$
	"steps",    // 2  CD2   george walking down steps sc23->sc24
	"sewer",    // 3  CD1   george entering sewer sc2->sc6
	"intro",    // 4  CD1   intro sequence ->sc1
	"river",    // 5  CD1   george being thrown into river by flap & g$
	"truck",    // 6  CD2   truck arriving at bull's head sc45->sc53/4
	"grave",    // 7  BOTH  george's grave in scotland, from sc73 + from sc38 $
	"montfcon", // 8  CD2   monfaucon clue in ireland dig, sc25
	"tapestry", // 9  CD2   tapestry room beyond spain well, sc61
	"ireland",  // 10 CD2   ireland establishing shot europe_map->sc19
	"finale",   // 11 CD2   grand finale at very end, from sc73
	"history",  // 12 CD1   George's history lesson from Nico, in sc10
	"spanish",  // 13 CD2   establishing shot for 1st visit to Spain, europe_m$
	"well",     // 14 CD2   first time being lowered down well in Spai$
	"candle",   // 15 CD2   Candle burning down in Spain mausoleum sc59
	"geodrop",  // 16 CD2   from sc54, George jumping down onto truck
	"vulture",  // 17 CD2   from sc54, vultures circling George's dead body
	"enddemo",  // 18 ---   for end of single CD demo
	"credits",  // 19 CD2   credits, to follow "finale" sequence
};

///////////////////////////////////////////////////////////////////////////////
// Basic movie player
///////////////////////////////////////////////////////////////////////////////

MoviePlayer::MoviePlayer(SwordEngine *vm, Text *textMan, Audio::Mixer *snd, OSystem *system, Audio::SoundHandle *bgSoundHandle, Graphics::VideoDecoder *decoder, DecoderType decoderType)
	: _vm(vm), _textMan(textMan), _snd(snd), _bgSoundHandle(bgSoundHandle), _system(system), VideoPlayer(decoder) {
	_bgSoundStream = NULL;
	_decoderType = decoderType;
}

MoviePlayer::~MoviePlayer() {
	delete _decoder;
	delete _bgSoundHandle;
}

/**
 * Plays an animated cutscene.
 * @param id the id of the file
 */
bool MoviePlayer::load(uint32 id) {
	Common::File f;
	char filename[20];

	if (_decoderType == kVideoDecoderDXA) {
		_bgSoundStream = Audio::AudioStream::openStreamFile(sequenceList[id]);
	} else {
		_bgSoundStream = NULL;
	}

	if (SwordEngine::_systemVars.showText) {
		sprintf(filename, "%s.txt", sequenceList[id]);
		if (f.open(filename)) {
			Common::String line;
			int lineNo = 0;
			int lastEnd = -1;

			_movieTexts.clear();
			while (!f.eos() && !f.err()) {
				line = f.readLine();
				lineNo++;
				if (line.empty() || line[0] == '#') {
					continue;
				}

				const char *ptr = line.c_str();

				// TODO: Better error handling
				int startFrame = strtoul(ptr, const_cast<char **>(&ptr), 10);
				int endFrame = strtoul(ptr, const_cast<char **>(&ptr), 10);

				while (*ptr && isspace(*ptr))
					ptr++;

				if (startFrame > endFrame) {
					warning("%s:%d: startFrame (%d) > endFrame (%d)", filename, lineNo, startFrame, endFrame);
					continue;
				}

				if (startFrame <= lastEnd) {
					warning("%s:%d startFrame (%d) <= lastEnd (%d)", filename, lineNo, startFrame, lastEnd);
					continue;
				}

				_movieTexts.push_back(new MovieText(startFrame, endFrame, ptr));
				lastEnd = endFrame;
			}
			f.close();
		}
	}

	switch (_decoderType) {
	case kVideoDecoderDXA:
		snprintf(filename, sizeof(filename), "%s.dxa", sequenceList[id]);
		break;
	case kVideoDecoderSMK:
		snprintf(filename, sizeof(filename), "%s.smk", sequenceList[id]);
		break;
	}

	return _decoder->loadFile(filename);
}

void MoviePlayer::play() {
	if (_bgSoundStream) {
		_snd->playInputStream(Audio::Mixer::kSFXSoundType, _bgSoundHandle, _bgSoundStream);
	}
	bool terminated = false;

	_textX = 0;
	_textY = 0;

	terminated = !playVideo();

	if (terminated)
		_snd->stopHandle(*_bgSoundHandle);

	_textMan->releaseText(2, false);

	while (!_movieTexts.empty())
		delete _movieTexts.remove_at(_movieTexts.size() - 1);

	while (_snd->isSoundHandleActive(*_bgSoundHandle))
		_system->delayMillis(100);

	// It's tempting to call _screen->fullRefresh() here to restore the old
	// palette. However, that causes glitches with DXA movies, where the
	// previous location would be momentarily drawn, before switching to
	// the new one. Work around this by setting the palette to black.

	byte pal[4 * 256];
	memset(pal, 0, sizeof(pal));
	_system->setPalette(pal, 0, 256);
}

void MoviePlayer::performPostProcessing(byte *screen) {
	if (!_movieTexts.empty()) {
		if (_decoder->getCurFrame() == _movieTexts[0]->_startFrame) {
			_textMan->makeTextSprite(2, (uint8 *)_movieTexts[0]->_text, 600, LETTER_COL);

			FrameHeader *frame = _textMan->giveSpriteData(2);
			_textWidth = frame->width;
			_textHeight = frame->height;
			_textX = 320 - _textWidth / 2;
			_textY = 420 - _textHeight;
		}
		if (_decoder->getCurFrame() == _movieTexts[0]->_endFrame) {
			_textMan->releaseText(2, false);
			delete _movieTexts.remove_at(0);
		}
	}

	byte *src, *dst;
	int x, y;

	if (_textMan->giveSpriteData(2)) {
		src = (byte *)_textMan->giveSpriteData(2) + sizeof(FrameHeader);
		dst = screen + _textY * SCREEN_WIDTH + _textX * 1;

		for (y = 0; y < _textHeight; y++) {
			for (x = 0; x < _textWidth; x++) {
				switch (src[x]) {
				case BORDER_COL:
					dst[x] = _decoder->getBlack();
					break;
				case LETTER_COL:
					dst[x] = _decoder->getWhite();
					break;
				}
			}
			src += _textWidth;
			dst += SCREEN_WIDTH;
		}
	} else if (_textX && _textY) {
		// If the frame doesn't cover the entire screen, we have to
		// erase the subtitles manually.

		int frameWidth = _decoder->getWidth();
		int frameHeight = _decoder->getHeight();
		int frameX = (_system->getWidth() - frameWidth) / 2;
		int frameY = (_system->getHeight() - frameHeight) / 2;

		dst = screen + _textY * _system->getWidth();

		for (y = 0; y < _textHeight; y++) {
			if (_textY + y < frameY || _textY + y >= frameY + frameHeight) {
				memset(dst + _textX, _decoder->getBlack(), _textWidth);
			} else {
				if (frameX > _textX)
					memset(dst + _textX, _decoder->getBlack(), frameX - _textX);
				if (frameX + frameWidth < _textX + _textWidth)
					memset(dst + frameX + frameWidth, _decoder->getBlack(), _textX + _textWidth - (frameX + frameWidth));
			}

			dst += _system->getWidth();
		}

		_textX = 0;
		_textY = 0;
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

	audioTime = (((int32) _mixer->getSoundElapsedTime(*_bgSoundHandle)) * 100);

	return videoTime - audioTime;
}

///////////////////////////////////////////////////////////////////////////////
// Factory function for creating the appropriate cutscene player
///////////////////////////////////////////////////////////////////////////////

MoviePlayer *makeMoviePlayer(uint32 id, SwordEngine *vm, Text *textMan, Audio::Mixer *snd, OSystem *system) {
	char filename[20];
	char buf[60];
	Audio::SoundHandle *bgSoundHandle = new Audio::SoundHandle;

	snprintf(filename, sizeof(filename), "%s.smk", sequenceList[id]);

	if (Common::File::exists(filename)) {
		Graphics::SmackerDecoder *smkDecoder = new Graphics::SmackerDecoder(snd);
		return new MoviePlayer(vm, textMan, snd, system, bgSoundHandle, smkDecoder, kVideoDecoderSMK);
	}

	snprintf(filename, sizeof(filename), "%s.dxa", sequenceList[id]);

	if (Common::File::exists(filename)) {
#ifdef USE_ZLIB
		DXADecoderWithSound *dxaDecoder = new DXADecoderWithSound(snd, bgSoundHandle);
		return new MoviePlayer(vm, textMan, snd, system, bgSoundHandle, dxaDecoder, kVideoDecoderDXA);
#else
		GUI::MessageDialog dialog("DXA cutscenes found but ScummVM has been built without zlib support", "OK");
		dialog.runModal();
		return NULL;
#endif
	}

	// Old MPEG2 cutscenes
	snprintf(filename, sizeof(filename), "%s.mp2", sequenceList[id]);

	if (Common::File::exists(filename)) {
		GUI::MessageDialog dialog("MPEG2 cutscenes are no longer supported", "OK");
		dialog.runModal();
		return NULL;
	}

	sprintf(buf, "Cutscene '%s' not found", sequenceList[id]);
	GUI::MessageDialog dialog(buf, "OK");
	dialog.runModal();

	return NULL;
}

} // End of namespace Sword1
