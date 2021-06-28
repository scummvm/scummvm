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

#include "saga2/rect.h"
#include "saga2/queues.h"
#include "saga2/idtypes.h"
#include "saga2/audiosmp.h"
#include "saga2/audqueue.h"
#include "saga2/audiosys.h"

#include "saga2/audiodec.h"
#include "saga2/audiofnc.h"

namespace Saga2 {

audioInterface *audio;

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
	return (!audio->queue.isPlaying() && audio->queue.getSize() > 0);
}

void audioInterface::playMe(void) {
	warning("STUB: audioInterface::PlayMe()");
	audio->queue.playNext();
}

void audioInterface::queueMusic(soundSegment s, decoderSet *, int16 loopFactor, sampleLocation where) {
	warning("STUB: audioInterface::queueMusic()");
}

void audioInterface::stopMusic(void) {
	warning("STUB: audioInterface::stopMusic()");
}

bool audioInterface::goodMIDICard(void) {
	warning("STUB: audioInterface::goodMIDICard()");
	return false;
}

void audioInterface::queueSound(soundSegment s, decoderSet *, int16 loopFactor, sampleLocation where) {
	warning("STUB: audioInterface::queueSound(%d,  @%d,%d)", s, where.x, where.y);
	audio->queue.pushSound(s);
}

void audioInterface::queueLoop(soundSegment s, decoderSet *sDec, int16 loopFactor, sampleLocation where) {
	warning("STUB: audioInterface::queueLoop()");
}

void audioInterface::stopLoop(void) {
	warning("STUB: audioInterface::stopLoop()");
}

void audioInterface::setLoopPosition(sampleLocation newLoc) {
	warning("STUB: audioInterface::setLoopPosition(%d,%d)", newLoc.x, newLoc.y);
}

void audioInterface::queueVoice(soundSegment s, decoderSet *, sampleLocation where) {
	warning("STUB: audioInterface::queueVoice(soundSegment, decoderSet *, sampleLocation)");
	audio->queue.pushVoice(s);
}

void audioInterface::queueVoice(soundSegment s[], decoderSet *, sampleLocation where) {
	warning("STUB: audioInterface::queueVoice(soundSegment [], decoderSet *, sampleLocation)");
	audio->queue.pushVoice(s);
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

soundDecoder::soundDecoder() {
	warning("STUB: soundDecoder::soundDecoder()");
}

soundDecoder::~soundDecoder() {
	warning("STUB: soundDecoder::~soundDecoder()");
}

soundDecoder::soundDecoder(INPLACEDECODER *, INPLACEDECODER *, INPLACEDECODER *) {
	warning("STUB: soundDecoder::soundDecoder(INPLACEDECODER *, INPLACEDECODER *, INPLACEDECODER *)");
}

soundDecoder::soundDecoder(BUFFERDECODER *, BUFFERDECODER *, BUFFERDECODER *, int16, audioInterface *, int16) {
	warning("STUB: soundDecoder::soundDecoder(BUFFERDECODER *, BUFFERDECODER *, BUFFERDECODER *, int16, audioInterface *, int16)");
}

soundDecoder::soundDecoder(BUFFERLOADER *, BUFFERLOADER *, BUFFERLOADER *) {
	warning("STUB: soundDecoder::soundDecoder(BUFFERLOADER *, BUFFERLOADER *, BUFFERLOADER *)");
}

void decoderSet::addDecoder(soundDecoder *sodec) {
	warning("STUB: soundDecoder::addDecoder()");
}

BUFFERDEC(flushDecompress) {
	warning("STUB: flushDecompress()");
	return 0;
}

BUFFERDEC(seekDecompress) {
	warning("STUB: seekDecompress()");
	return 0;
}

BUFFERDEC(readDecompress) {
	warning("STUB: readDecompress()");
	return 0;
}

} // end of namespace Saga2
