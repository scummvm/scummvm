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
#include "sword1/credits.h"
#include "sword1/text.h"
#include "sound/vorbis.h"

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/str.h"
#include "common/events.h"
#include "common/system.h"

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

MoviePlayer::MoviePlayer(Screen *screen, Text *textMan, Audio::Mixer *snd, OSystem *system)
	: _screen(screen), _textMan(textMan), _snd(snd), _system(system) {
	_bgSoundStream = NULL;
	_ticks = 0;
	_textSpriteBuf = NULL;
	_black = 1;
	_white = 255;
	_currentFrame = 0;
	_forceFrame = false;
	_framesSkipped = 0;
}

MoviePlayer::~MoviePlayer(void) {
}

void MoviePlayer::updatePalette(byte *pal, bool packed) {
	byte palette[4 * 256];
	byte *p = palette;

	uint32 maxWeight = 0;
	uint32 minWeight = 0xFFFFFFFF;

	for (int i = 0; i < 256; i++) {
		int r = *pal++;
		int g = *pal++;
		int b = *pal++;

		if (!packed)
			pal++;

		uint32 weight = 3 * r * r + 6 * g * g + 2 * b * b;

		if (weight >= maxWeight) {
			_white = i;
			maxWeight = weight;
		}

		if (weight <= minWeight) {
			_black = i;
			minWeight = i;
		}

		*p++ = r;
		*p++ = g;
		*p++ = b;
		*p++ = 0;
	}

	_system->setPalette(palette, 0, 256);
	_forceFrame = true;
}

void MoviePlayer::handleScreenChanged(void) {
}

bool MoviePlayer::initOverlays(uint32 id) {
	return true;
}

bool MoviePlayer::checkSkipFrame(void) {
	if (_forceFrame) {
		_forceFrame = false;
		return false;
	}
	if (_framesSkipped > 10) {
		warning("Forced frame %d to be displayed", _currentFrame);
		_framesSkipped = 0;
		return false;
	}
	if (_bgSoundStream) {
		if ((_snd->getSoundElapsedTime(_bgSoundHandle) * 12) / 1000 < _currentFrame + 1)
			return false;
	} else {
		if (_system->getMillis() <= _ticks)
			return false;
	}
	_framesSkipped++;
	return true;
}

bool MoviePlayer::syncFrame(void) {
	_ticks += 83;
	if (checkSkipFrame()) {
		warning("Skipped frame %d", _currentFrame);
		return false;
	}
	if (_bgSoundStream) {
		while (_snd->isSoundHandleActive(_bgSoundHandle) && (_snd->getSoundElapsedTime(_bgSoundHandle) * 12) / 1000 < _currentFrame) {
			_system->delayMillis(10);
		}

		// In case the background sound ends prematurely, update _ticks
		// so that we can still fall back on the no-sound sync case for
		// the subsequent frames.

		_ticks = _system->getMillis();
	} else {
		while (_system->getMillis() < _ticks) {
			_system->delayMillis(10);
		}
	}
	return true;
}

/**
 * Plays an animated cutscene.
 * @param id the id of the file
 */
bool MoviePlayer::load(uint32 id) {
	Common::File f;
	char fileName[20];

	_id = id;
	_bgSoundStream = NULL;

	if (SwordEngine::_systemVars.showText) {
		sprintf(fileName, "%s.txt", sequenceList[id]);
		if (f.open(fileName)) {
			char line[120];
			int lineNo = 0;
			int lastEnd = -1;

			_movieTexts.clear();
			while (f.readLine(line, sizeof(line))) {
				lineNo++;
				if (line[0] == '#' || line[0] == 0) {
					continue;
				}

				char *ptr = line;

				// TODO: Better error handling
				int startFrame = strtoul(ptr, &ptr, 10);
				int endFrame = strtoul(ptr, &ptr, 10);

				while (*ptr && isspace(*ptr))
					ptr++;

				if (startFrame > endFrame) {
					warning("%s:%d: startFrame (%d) > endFrame (%d)", fileName, lineNo, startFrame, endFrame);
					continue;
				}

				if (startFrame <= lastEnd) {
					warning("%s:%d startFrame (%d) <= lastEnd (%d)", fileName, lineNo, startFrame, lastEnd);
					continue;
				}

				_movieTexts.push_back(new MovieText(startFrame, endFrame, ptr));
				lastEnd = endFrame;
			}
		}
	}

	if (SwordEngine::_systemVars.cutscenePackVersion == 1) {
		if ((id == SEQ_INTRO) || (id == SEQ_FINALE) || (id == SEQ_HISTORY) || (id == SEQ_FERRARI)) {
#ifdef USE_VORBIS
			// these sequences are language specific
			sprintf(fileName, "%s.snd", sequenceList[id]);
			Common::File *oggSource = new Common::File();
			if (oggSource->open(fileName)) {
				SplittedAudioStream *sStream = new SplittedAudioStream();
				uint32 numSegs = oggSource->readUint32LE(); // number of audio segments, either 1 or 2.
				// for each segment and each of the 7 languages, we've got fileoffset and size
				uint32 *header = (uint32*)malloc(numSegs * 7 * 2 * 4);
				for (uint32 cnt = 0; cnt < numSegs * 7 * 2; cnt++)
					header[cnt] = oggSource->readUint32LE();
				for (uint32 segCnt = 0; segCnt < numSegs; segCnt++) {
					oggSource->seek( header[SwordEngine::_systemVars.language * 2 + 0 + segCnt * 14]);
					uint32 segSize = header[SwordEngine::_systemVars.language * 2 + 1 + segCnt * 14];
					Common::MemoryReadStream *stream = oggSource->readStream(segSize);
					Audio::AudioStream *apStream = Audio::makeVorbisStream(stream, true);
					if (!apStream)
						error("Can't create Vorbis Stream from file %s", fileName);
					sStream->appendStream(apStream);
				}
				free(header);
				_bgSoundStream = sStream;
			} else
				warning("Sound file \"%s\" not found", fileName);
			delete oggSource;
#endif
			initOverlays(id);
		}
	}
	return true;
}

void MoviePlayer::play(void) {
	_screen->clearScreen();
	_framesSkipped = 0;
	_ticks = _system->getMillis();
	_bgSoundStream = Audio::AudioStream::openStreamFile(sequenceList[_id]);
	if (_bgSoundStream) {
		_snd->playInputStream(Audio::Mixer::kSFXSoundType, &_bgSoundHandle, _bgSoundStream);
	}
	_currentFrame = 0;
	bool terminated = false;
	Common::EventManager *eventMan = _system->getEventManager();
	while (!terminated && decodeFrame()) {
		if (!_movieTexts.empty()) {
			if (_currentFrame == _movieTexts[0]->_startFrame) {
				_textMan->makeTextSprite(2, (uint8 *)_movieTexts[0]->_text, 600, LETTER_COL);

				FrameHeader *frame = _textMan->giveSpriteData(2);
				_textWidth = frame->width;
				_textHeight = frame->height;
				_textX = 320 - _textWidth / 2;
				_textY = 420 - _textHeight;
				_textSpriteBuf = (byte *)calloc(_textHeight, _textWidth);
			}
			if (_currentFrame == _movieTexts[0]->_endFrame) {
				_textMan->releaseText(2);
				free(_textSpriteBuf);
				_textSpriteBuf = NULL;
				delete _movieTexts.remove_at(0);
			}
		}
		processFrame();
		if (syncFrame())
			updateScreen();
		_currentFrame++;
		Common::Event event;
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_SCREEN_CHANGED:
				handleScreenChanged();
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_snd->stopHandle(_bgSoundHandle);
					terminated = true;
				}
				break;
			default:
				break;
			}
		}
	}

	while (!_movieTexts.empty()) {
		delete _movieTexts.remove_at(_movieTexts.size() - 1);
	}

	while (_snd->isSoundHandleActive(_bgSoundHandle))
		_system->delayMillis(100);

	// It's tempting to call _screen->fullRefresh() here to restore the old
	// palette. However, that causes glitches with DXA movies, here the
	// previous location would be momentarily drawn, before switching to
	// the new one. Work around this by setting the palette to black.

	byte pal[3 * 256];
	memset(pal, 0, sizeof(pal));
	updatePalette(pal, true);
}

SplittedAudioStream::SplittedAudioStream(void) {
	_queue = NULL;
}

SplittedAudioStream::~SplittedAudioStream(void) {
	while (_queue) {
		delete _queue->stream;
		FileQueue *que = _queue->next;
		delete _queue;
		_queue = que;
	}
}

int SplittedAudioStream::getRate(void) const {
	if (_queue)
		return _queue->stream->getRate();
	else
		return 22050;
}

void SplittedAudioStream::appendStream(Audio::AudioStream *stream) {
	FileQueue **que = &_queue;
	while (*que)
		que = &((*que)->next);
	*que = new FileQueue;
	(*que)->stream = stream;
	(*que)->next = NULL;
}

bool SplittedAudioStream::endOfData(void) const {
	if (_queue)
		return _queue->stream->endOfData();
	else
		return true;
}

bool SplittedAudioStream::isStereo(void) const {
	if (_queue)
		return _queue->stream->isStereo();
	else
		return false; // all the BS1 files are mono, anyways.
}

int SplittedAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int retVal = 0;
	int needSamples = numSamples;
	while (needSamples && _queue) {
		int retSmp = _queue->stream->readBuffer(buffer, needSamples);
		needSamples -= retSmp;
		retVal += retSmp;
		buffer += retSmp;
		if (_queue->stream->endOfData()) {
			delete _queue->stream;
			FileQueue *que = _queue->next;
			delete _queue;
			_queue = que;
		}
	}
	return retVal;
}

#ifdef USE_ZLIB

///////////////////////////////////////////////////////////////////////////////
// Movie player for the new DXA movies
///////////////////////////////////////////////////////////////////////////////

MoviePlayerDXA::MoviePlayerDXA(Screen *screen, Text *textMan, Audio::Mixer *snd, OSystem *system)
	: MoviePlayer(screen, textMan, snd, system) {
	debug(0, "Creating DXA cutscene player");
}

MoviePlayerDXA::~MoviePlayerDXA(void) {
	closeFile();
}

bool MoviePlayerDXA::load(uint32 id) {
	if (!MoviePlayer::load(id))
		return false;

	char filename[20];
	snprintf(filename, sizeof(filename), "%s.dxa", sequenceList[id]);
	if (loadFile(filename)) {
		// The Broken Sword games always use external audio tracks.
		if (_fd->readUint32BE() != MKID_BE('NULL'))
			return false;
		_frameWidth = getWidth();
		_frameHeight = getHeight();
		_frameX = (640 - _frameWidth) / 2;
		_frameY = (480 - _frameHeight) / 2;
		return true;
	}
	return false;
}

bool MoviePlayerDXA::initOverlays(uint32 id) {
	// TODO
	return true;
}

void MoviePlayerDXA::setPalette(byte *pal) {
	updatePalette(pal, true);
}

bool MoviePlayerDXA::decodeFrame(void) {
	if (_currentFrame < _framesCount) {
		decodeNextFrame();
		return true;
	}
	return false;
}

void MoviePlayerDXA::processFrame(void) {
	// TODO: Handle the advanced cutscene packs. Do they really exist?

	// We cannot draw the text to _drawBuffer, sinzce ethat's one of the
	// decoder's internal buffers. Instead, we copy part of _drawBuffer
	// to the text sprite.

	if (_textSpriteBuf) {
		memset(_textSpriteBuf, 0, _textWidth * _textHeight);

		// FIXME: This is inefficient
		int x, y;

		for (y = _textY; y < _textY + _textHeight; y++) {
			for (x = _textX; x < _textX + _textWidth; x++) {
				if (x >= _frameX && x <= _frameX + _frameWidth && y >= _frameY && y <= _frameY + _frameHeight) {
					_textSpriteBuf[(y - _textY) * _textWidth + x - _textX] = _drawBuffer[(y - _frameY) * _frameWidth + x - _frameX];
				}
			}
		}

		byte *src = (byte *)_textMan->giveSpriteData(2) + sizeof(FrameHeader);
		byte *dst = _textSpriteBuf;

		for (y = 0; y < _textHeight; y++) {
			for (x = 0; x < _textWidth; x++) {
				switch (src[x]) {
				case BORDER_COL:
					dst[x] = _black;
					break;
				case LETTER_COL:
					dst[x] = _white;
					break;
				}
			}
			src += _textWidth;
			dst += _textWidth;
		}
	}
}

void MoviePlayerDXA::updateScreen(void) {
	_system->copyRectToScreen(_drawBuffer, _frameWidth, _frameX, _frameY, _frameWidth, _frameHeight);
	if (_textSpriteBuf) {
		_system->copyRectToScreen(_textSpriteBuf, _textWidth, _textX, _textY, _textWidth, _textHeight);
	}
	_system->updateScreen();
}

#endif

#ifdef USE_MPEG2

///////////////////////////////////////////////////////////////////////////////
// Movie player for the old MPEG movies
///////////////////////////////////////////////////////////////////////////////

MoviePlayerMPEG::MoviePlayerMPEG(Screen *screen, Text *textMan, Audio::Mixer *snd, OSystem *system)
	: MoviePlayer(screen, textMan, snd, system) {
#ifdef BACKEND_8BIT
	debug(0, "Creating MPEG cutscene player (8-bit)");
#else
	debug(0, "Creating MPEG cutscene player (16-bit)");
#endif
	for (uint8 cnt = 0; cnt < INTRO_LOGO_OVLS; cnt++)
		_logoOvls[cnt] = NULL;
	_introPal = NULL;
	_anim = NULL;
}

MoviePlayerMPEG::~MoviePlayerMPEG(void) {
	free(_introPal);
	for (uint8 cnt = 0; cnt < INTRO_LOGO_OVLS; cnt++)
		free(_logoOvls[cnt]);
	delete _anim;
}

void MoviePlayerMPEG::handleScreenChanged(void) {
	_anim->handleScreenChanged();
}

void MoviePlayerMPEG::insertOverlay(OverlayColor *buf, uint8 *ovl, OverlayColor *pal) {
	if (ovl != NULL)
		for (uint32 cnt = 0; cnt < 640 * 400; cnt++)
			if (ovl[cnt])
				buf[cnt] = pal[ovl[cnt]];
}

bool MoviePlayerMPEG::load(uint32 id) {
	if (MoviePlayer::load(id)) {
		_anim = new AnimationState(this, _screen, _system);
		return _anim->init(sequenceList[id]);
	}
	return false;
}

bool MoviePlayerMPEG::initOverlays(uint32 id) {
	if (id == SEQ_INTRO) {
		ArcFile ovlFile;
		if (!ovlFile.open("intro.dat")) {
			warning("\"intro.dat\" not found");
			return false;
		}
		ovlFile.enterPath(SwordEngine::_systemVars.language);
		for (uint8 fcnt = 0; fcnt < 12; fcnt++) {
			_logoOvls[fcnt] = ovlFile.decompressFile(fcnt);
			if (fcnt > 0)
				for (uint32 cnt = 0; cnt < 640 * 400; cnt++)
					if (_logoOvls[fcnt - 1][cnt] && !_logoOvls[fcnt][cnt])
						_logoOvls[fcnt][cnt] = _logoOvls[fcnt - 1][cnt];
		}
		uint8 *pal = ovlFile.fetchFile(12);
		_introPal = (OverlayColor*)malloc(256 * sizeof(OverlayColor));
		for (uint16 cnt = 0; cnt < 256; cnt++)
			_introPal[cnt] = _system->RGBToColor(pal[cnt * 3 + 0], pal[cnt * 3 + 1], pal[cnt * 3 + 2]);
	}

	return true;
}

bool MoviePlayerMPEG::decodeFrame(void) {
	return _anim->decodeFrame();
}

void MoviePlayerMPEG::updateScreen(void) {
	_anim->updateScreen();
}

void MoviePlayerMPEG::processFrame(void) {
#ifndef BACKEND_8BIT
	if ((_id != 4) || (SwordEngine::_systemVars.cutscenePackVersion == 0))
		return;
	OverlayColor *buf = _anim->giveRgbBuffer();
	if ((_currentFrame > 397) && (_currentFrame < 444)) { // Broken Sword Logo
		if (_currentFrame <= 403)
			insertOverlay(buf, _logoOvls[_currentFrame - 398], _introPal); // fade up
		else if (_currentFrame <= 437)
			insertOverlay(buf, _logoOvls[(_currentFrame - 404) % 6 + 6], _introPal); // animation
		else {
			insertOverlay(buf, _logoOvls[5 - (_currentFrame - 438)], _introPal); // fade down
		}
	}
#endif
}

AnimationState::AnimationState(MoviePlayer *player, Screen *screen, OSystem *system)
	: BaseAnimationState(system, 640, 400), _player(player), _screen(screen) {
}

AnimationState::~AnimationState(void) {
}

#ifdef BACKEND_8BIT
void AnimationState::setPalette(byte *pal) {
	_player->updatePalette(pal, false);
}
#endif

void AnimationState::drawYUV(int width, int height, byte *const *dat) {
	_frameWidth = width;
	_frameHeight = height;

#ifdef BACKEND_8BIT
	_screen->plotYUV(_lut, width, height, dat);
#else
	plotYUV(width, height, dat);
#endif
}

OverlayColor *AnimationState::giveRgbBuffer(void) {
#ifdef BACKEND_8BIT
	return NULL;
#else
	return _overlay;
#endif
}

Audio::AudioStream *AnimationState::createAudioStream(const char *name, void *arg) {
	if (arg)
		return (Audio::AudioStream*)arg;
	else
		return Audio::AudioStream::openStreamFile(name);
}

#endif

///////////////////////////////////////////////////////////////////////////////
// Factory function for creating the appropriate cutscene player
///////////////////////////////////////////////////////////////////////////////

MoviePlayer *makeMoviePlayer(uint32 id, Screen *screen, Text *textMan, Audio::Mixer *snd, OSystem *system) {
#if defined(USE_ZLIB) || defined(USE_MPEG2)
	char filename[20];
#endif

#ifdef USE_ZLIB
	snprintf(filename, sizeof(filename), "%s.dxa", sequenceList[id]);

	if (Common::File::exists(filename)) {
		return new MoviePlayerDXA(screen, textMan, snd, system);
	}
#endif

#ifdef USE_MPEG2
	snprintf(filename, sizeof(filename), "%s.mp2", sequenceList[id]);

	if (Common::File::exists(filename)) {
		return new MoviePlayerMPEG(screen, textMan, snd, system);
	}
#endif

	return NULL;
}

} // End of namespace Sword1
