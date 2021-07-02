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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/audio.h"
#include "saga2/fta.h"
#include "saga2/shorten.h"
#include "saga2/hresmgr.h"

#include "saga2/rect.h"
#include "saga2/idtypes.h"

namespace Saga2 {

audioInterface *audio;

extern hResContext *voiceRes;
extern hResContext *soundRes;
extern hResContext *musicRes;

bool initAudio() {
	warning("STUB: initAudio()");
	audio = new audioInterface();
	return true;
}

void cleanupAudio() {
	warning("STUB: cleanupAudio()");
	delete audio;
}

void *audioAlloc(size_t s, const char desc[]) {
	warning("STUB: audioAlloc()");
	return malloc(s);
}

void audioFree(void *mem) {
	warning("STUB: audioFree()");
	//delete mem;
}

audioInterface::audioInterface(const char *driver_path, const char *undriver_path) {
	warning("STUB: audioInteraface::audioInterface()");
}

audioInterface::~audioInterface() {
	warning("STUB: audioInteraface::~audioInterface()");
}

void audioInterface::initAudioInterface(audioInterfaceSettings &ais) {
	warning("STUB: audioInterface::initAudioInterface()");
}

void audioInterface::cleanupAudioInterface(void) {
	warning("STUB: audioInterface::cleanupAudioInterface()");
}

void audioInterface::suspendGameClock(void) {
	warning("STUB: audioInterace::suspendGameClock()");
}

void audioInterface::resumeGameClock(void) {
	warning("STUB: audioInterface::resumeGameClock()");
}

bool audioInterface::playFlag(void) {
	debugC(5, kDebugSound, "STUB: audioInterface::playFlag()");
	bool isSoundActive = g_system->getMixer()->isSoundHandleActive(audio->_speechSoundHandle);
	return !isSoundActive && audio->_speechQueue.size() > 0;
}

void audioInterface::playMe(void) {
	warning("STUB: audioInterface::PlayMe()");
	SoundInstance si = audio->_speechQueue.pop();

	Common::SeekableReadStream *stream = loadResourceToStream(voiceRes, si.seg, "voice data");

	Audio::AudioStream *aud = makeShortenStream(*stream);

	g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &audio->_speechSoundHandle, aud);

	delete stream;
}

void audioInterface::playMusic(soundSegment s, int16 loopFactor, sampleLocation where) {
	warning("STUB: audioInterface::queueMusic()");
}

void audioInterface::stopMusic(void) {
	warning("STUB: audioInterface::stopMusic()");
}

bool audioInterface::goodMIDICard(void) {
	warning("STUB: audioInterface::goodMIDICard()");
	return false;
}

void audioInterface::queueSound(soundSegment s, int16 loopFactor, sampleLocation where) {
	warning("STUB: audioInterface::queueSound(%d,  @%d,%d)", s, where.x, where.y);
	SoundInstance si;

	si.seg = s;
	si.loop = loopFactor;
	si.loc = where;

	audio->_sfxQueue.push(si);
}

void audioInterface::queueLoop(soundSegment s, int16 loopFactor, sampleLocation where) {
	warning("STUB: audioInterface::queueLoop()");
}

void audioInterface::stopLoop(void) {
	warning("STUB: audioInterface::stopLoop()");
}

void audioInterface::setLoopPosition(sampleLocation newLoc) {
	warning("STUB: audioInterface::setLoopPosition(%d,%d)", newLoc.x, newLoc.y);
}

void audioInterface::queueVoice(soundSegment s, sampleLocation where) {
	warning("STUB: audioInterface::queueVoice(soundSegment, sampleLocation)");
	SoundInstance si;

	si.seg = s;
	si.loop = false;
	si.loc = where;

	audio->_speechQueue.push(si);
}

void audioInterface::queueVoice(soundSegment s[], sampleLocation where) {
	warning("STUB: audioInterface::queueVoice(soundSegment [], sampleLocation)");
	SoundInstance si;

	soundSegment *p = s;
	while (*p) {
		si.seg = *p;
		si.loop = false;
		si.loc = where;

		audio->_speechQueue.push(si);
		p++;
	}
}

void audioInterface::stopVoice(void) {
	warning("STUB: audioInterface::stopVoice()");
}

bool audioInterface::talking(void) {
	warning("STUB: audioInterface::talking()");
	return false;
}

bool audioInterface::saying(soundSegment s) {
	warning("STUB: audioInterface::saying()");
	return false;
}

bool audioInterface::active(void) {
	warning("STUB: audioInterface::active()");
	return true;
}

void audioInterface::enable(volumeTarget i, bool onOff) {
	warning("STUB: audioInterface::enable()");
}

void audioInterface::setVolume(volumeTarget targ, volumeMode op, Volume val) {
	warning("STUB: audioInterface::setVolume()");
}

Volume audioInterface::getVolume(volumeTarget src) {
	warning("STUB: audioInterface::getVolume()");
	return 0;
}

void audioInterface::setMusicFadeStyle(int16 tOut, int16 tIn, int16 tOver) {
	warning("STUB: audioInterface::setMusicFadeStyle()");
}

void audioInterface::suspend(void) {
	warning("STUB: audioInterface::suspend()");
}

void audioInterface::resume(void) {
	warning("STUB: audioInterface::resume()");
}

audioAttenuationFunction audioInterface::setAttenuator(audioAttenuationFunction newAF) {
	warning("STUB: audioInterface::setAttenuator()");
	return nullptr;
}

bool bufCheckResID(hResContext *hrc, uint32 s) {
	return s != 0;
}

bool hResCheckResID(hResContext *hrc, uint32 s) {
	if (hrc != NULL)
		return hrc->seek(s);
	return false;
}

bool hResCheckResID(hResContext *hrc, uint32 s[]) {
	if (s != NULL) {
		if (s[0] == 0)
			return false;

		for (int i = 0; s[i]; i++) {
			if (!hResCheckResID(hrc, s[i]))
				return false;
		}
	}
	return true;
}

} // end of namespace Saga2
