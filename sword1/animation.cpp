/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "common/file.h"
#include "sword1/sword1.h"
#include "sword1/animation.h"
#include "sword1/credits.h"
#include "sound/vorbis.h"

#include "common/config-manager.h"
#include "common/str.h"
#include "common/system.h"

namespace Sword1 {

AnimationState::AnimationState(Screen *scr, Audio::Mixer *snd, OSystem *sys)
	: BaseAnimationState(snd, sys, 640, 400), _scr(scr) {
}

AnimationState::~AnimationState() {
}


#ifdef BACKEND_8BIT
void AnimationState::setPalette(byte *pal) {
	_sys->setPalette(pal, 0, 256);
}
#endif

void AnimationState::drawYUV(int width, int height, byte *const *dat) {
#ifdef BACKEND_8BIT
	_scr->plotYUV(_lut, width, height, dat);
#else
	plotYUV(width, height, dat);
#endif
}

void AnimationState::updateScreen(void) {
#ifndef BACKEND_8BIT
	_sys->copyRectToOverlay(_overlay, _movieWidth, 0, 40, _movieWidth, _movieHeight);
#endif
	_sys->updateScreen();
}

OverlayColor *AnimationState::giveRgbBuffer(void) {
#ifdef BACKEND_8BIT
	return NULL;
#else
	return _overlay;
#endif
}

bool AnimationState::soundFinished(void) {
	return !_snd->isSoundHandleActive(_bgSound);
}

AudioStream *AnimationState::createAudioStream(const char *name, void *arg) {
	if (arg)
		return (AudioStream*)arg;
	else
		return AudioStream::openStreamFile(name);
}

MoviePlayer::MoviePlayer(Screen *scr, Audio::Mixer *snd, OSystem *sys)
	: _scr(scr), _snd(snd), _sys(sys) {
		for (uint8 cnt = 0; cnt < INTRO_LOGO_OVLS; cnt++)
			_logoOvls[cnt] = NULL;
		_introPal = NULL;
}

MoviePlayer::~MoviePlayer(void) {
	if (_introPal)
		free(_introPal);
	for (uint8 cnt = 0; cnt < INTRO_LOGO_OVLS; cnt++)
		if (_logoOvls[cnt])
			free(_logoOvls[cnt]);
}

/**
 * Plays an animated cutscene.
 * @param id the id of the file
 */
void MoviePlayer::play(uint32 id) {
#if defined(USE_MPEG2) && (defined(USE_VORBIS) || defined(USE_TREMOR))
	AnimationState *anim = new AnimationState(_scr, _snd, _sys);
	AudioStream *stream = NULL;
	if (SwordEngine::_systemVars.cutscenePackVersion == 1) {
		if ((id == SEQ_INTRO) || (id == SEQ_FINALE) || (id == SEQ_HISTORY) || (id == SEQ_FERRARI)) {
			// these sequences are language specific
			char sndName[20];
			sprintf(sndName, "%s.snd", _sequenceList[id]);
			Common::File *oggSource = new Common::File();
			if (oggSource->open(sndName)) {
				SplittedAudioStream *sStream = new SplittedAudioStream();
				uint32 numSegs = oggSource->readUint32LE(); // number of audio segments, either 1 or 2.
				// for each segment and each of the 7 languages, we've got fileoffset and size
				uint32 *header = (uint32*)malloc(numSegs * 7 * 2 * 4);
				for (uint32 cnt = 0; cnt < numSegs * 7 * 2; cnt++)
					header[cnt] = oggSource->readUint32LE();
				for (uint32 segCnt = 0; segCnt < numSegs; segCnt++) {
					oggSource->seek( header[SwordEngine::_systemVars.language * 2 + 0 + segCnt * 14]);
					uint32 segSize = header[SwordEngine::_systemVars.language * 2 + 1 + segCnt * 14];
					AudioStream *apStream = makeVorbisStream(oggSource, segSize);
					if (!apStream)
						error("Can't create Vorbis Stream from file %s", sndName);
					sStream->appendStream(apStream);
				}
				free(header);
				stream = sStream;
			} else
				warning("Sound file \"%s\" not found", sndName);
			initOverlays(id);
			oggSource->decRef();
		}
	}
	bool initOK = anim->init(_sequenceList[id], stream);

	uint32 frameCount = 0;
	if (initOK) {
		while (anim->decodeFrame()) {
			processFrame(id, anim, frameCount);
			anim->updateScreen();
			frameCount++;
			OSystem::Event event;
			while (_sys->pollEvent(event)) {
				switch (event.type) {
#ifndef BACKEND_8BIT
				case OSystem::EVENT_SCREEN_CHANGED:
					anim->buildLookup();
					break;
#endif
				case OSystem::EVENT_KEYDOWN:
					if (event.kbd.keycode == 27) {
						delete anim;
						return;
					}
					break;
				case OSystem::EVENT_QUIT:
					_sys->quit();
					break;
				default:
					break;
				}
			}
		}
	}
	while (!anim->soundFinished())
		_sys->delayMillis(100);
	delete anim;
#endif // USE_MPEG2 && (USE_VORBIS || USE_TREMOR)
}

void MoviePlayer::insertOverlay(OverlayColor *buf, uint8 *ovl, OverlayColor *pal) {
	if (ovl != NULL)
		for (uint32 cnt = 0; cnt < 640 * 400; cnt++)
			if (ovl[cnt])
				buf[cnt] = pal[ovl[cnt]];
}

void MoviePlayer::processFrame(uint32 animId, AnimationState *anim, uint32 frameNo) {
#if defined(USE_MPEG2) && !defined(BACKEND_8BIT)
	if ((animId != 4) || (SwordEngine::_systemVars.cutscenePackVersion == 0))
		return;
	OverlayColor *buf = anim->giveRgbBuffer();
	if ((frameNo > 397) && (frameNo < 444)) { // Broken Sword Logo
		if (frameNo <= 403)
			insertOverlay(buf, _logoOvls[frameNo - 398], _introPal); // fade up
		else if (frameNo <= 437)
			insertOverlay(buf, _logoOvls[(frameNo - 404) % 6 + 6], _introPal); // animation
		else {
			insertOverlay(buf, _logoOvls[5 - (frameNo - 438)], _introPal); // fade down
		}
	}
#endif
}

bool MoviePlayer::initOverlays(uint32 id) {
#if defined(USE_MPEG2) && !defined(BACKEND_8BIT)
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
			_introPal[cnt] = _sys->RGBToColor(pal[cnt * 3 + 0], pal[cnt * 3 + 1], pal[cnt * 3 + 2]);
	}
#endif

	return true;
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

void SplittedAudioStream::appendStream(AudioStream *stream) {
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

const char * MoviePlayer::_sequenceList[20] = {
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

} // End of namespace Sword1
