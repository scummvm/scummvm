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

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/util.h"
#include "common/debug.h"

#include "sound/mods/maxtrax.h"

namespace Audio {

MaxTrax::MaxTrax(int rate, bool stereo)
	: Paula(stereo, rate, rate/50), _patch(), _scores(), _numScores(), _microtonal() {
}

MaxTrax::~MaxTrax() {
	stopMusic();
	freePatches();
	freeScores();
}

void MaxTrax::interrupt() {
}

void MaxTrax::stopMusic() {
}

void MaxTrax::freeScores() {
	if (_scores) {
		for (int i = 0; i < _numScores; ++i)
			delete _scores[i].events;
		delete _scores;
		_scores = 0;
	}
	_numScores = 0;
	memset(_microtonal, 0, sizeof(_microtonal));
}

void MaxTrax::freePatches() {
	for (int i = 0; i < ARRAYSIZE(_patch); ++i) {
		delete[] _patch[i].samplePtr;
		delete[] _patch[i].attackPtr;
	}
	memset(_patch, 0, sizeof(_patch));
}

bool MaxTrax::load(Common::SeekableReadStream &musicData, bool loadScores, bool loadSamples) {
	bool res = false;
	stopMusic();
	if (loadSamples)
		freePatches();
	if (loadScores)
		freeScores();
	// 0x0000: 4 Bytes Header "MXTX"
	// 0x0004: uint16 tempo
	// 0x0006: uint16 flags. bit0 = lowpassfilter, bit1 = attackvolume, bit15 = microtonal	
	if (musicData.readUint32BE() != 0x4D585458) {
		warning("Maxtrax: File is not a Maxtrax Module");
		return false;
	}
	_playerCtx.tempo = musicData.readUint16BE();
	const uint16 flags = musicData.readUint16BE();
	_playerCtx.filterOn = (flags & 1) != 0;
	_playerCtx.handleVolume = (flags & 2) != 0;
	debug("Header: MXTX %02X %02X", _playerCtx.tempo, flags);

	if (loadScores && flags & (1 << 15)) {
		debug("Song has microtonal");
		for (int i = 0; i < ARRAYSIZE(_microtonal); ++i)
			_microtonal[i] = musicData.readUint16BE();
	}

	int scoresLoaded = 0;
	// uint16 number of Scores
	const uint16 scoresInFile = musicData.readUint16BE();

	if (loadScores) {
		const uint16 scoremax = 128; // some variable which is set upon initialisation of player
		const uint16 tempScores = MIN(scoresInFile, scoremax);
		debug("#Scores: %d, loading # of scores: %d", scoresInFile, tempScores);
		Score *curScore =_scores = new Score[tempScores];
		
		for (int i = tempScores; i > 0; --i, ++curScore) {
			const uint32 numEvents = musicData.readUint32BE();
			Event *curEvent = curScore->events = new Event[numEvents];
			for (int j = numEvents; j > 0; --j, ++curEvent) {
				curEvent->command = musicData.readByte();
				curEvent->parameter = musicData.readByte();
				curEvent->startTime = musicData.readUint16BE();
				curEvent->stopTime = musicData.readUint16BE();
			}
			curScore->numEvents = numEvents;
		}
		_numScores = scoresLoaded = tempScores;
	}

	if (false && !loadSamples)
		return true;

	// skip over remaining scores in file
	for (int i = scoresInFile - scoresLoaded; i > 0; --i)
		musicData.skip(musicData.readUint32BE() * 6);

	for (int i = 0; i < _numScores; ++i)
		outPutScore(_scores[i], i);

	debug("samples start at filepos %08X", musicData.pos());
	// uint16 number of Samples
	const uint16 wavesInFile = musicData.readUint16BE();
	if (loadSamples) {
		for (int i = wavesInFile; i > 0; --i) {
			// load disksample structure
			const uint16 number = musicData.readUint16BE();
			assert(number < ARRAYSIZE(_patch));
			// pointer to samples needed?
			Patch &curPatch = _patch[number];

			curPatch.tune = musicData.readUint16BE();
			curPatch.volume = musicData.readUint16BE();
			curPatch.sampleOctaves = musicData.readUint16BE();
			curPatch.sampleAttack = musicData.readUint32BE();
			curPatch.sampleSustain = musicData.readUint32BE();
			// each octave the number of samples doubles.
			const uint32 totalSamples = (curPatch.sampleAttack + curPatch.sampleSustain) * ((1 << curPatch.sampleOctaves) - 1);
			curPatch.attackLen = musicData.readUint16BE();
			curPatch.releaseLen = musicData.readUint16BE();
			const uint32 totalEnvs = curPatch.attackLen + curPatch.releaseLen;

			debug("wave nr %d at %08X - %d octaves", number, musicData.pos(), curPatch.sampleOctaves);
			// Allocate space for both attack and release Segment.
			Envelope *envPtr = new Envelope[totalEnvs];
			// Attack Segment
			curPatch.attackPtr = envPtr;
			// Release Segment
			// curPatch.releasePtr = envPtr + curPatch.attackLen;

			// Read Attack and Release Segments
			for (int j = totalEnvs; j > 0; --j, ++envPtr) {
				envPtr->duration = musicData.readUint16BE();
				envPtr->volume = musicData.readUint16BE();
			}

			// read Samples
			curPatch.samplePtr = new int8[totalSamples];
			musicData.read(curPatch.samplePtr, totalSamples);
		}
	} else if (wavesInFile > 0){
		uint32 skipLen = 3 * 2;
		for (int i = wavesInFile; i > 0; --i) {
			musicData.skip(skipLen);
			const uint16 octaves = musicData.readUint16BE();
			const uint32 attackLen = musicData.readUint32BE();
			const uint32 sustainLen = musicData.readUint32BE();
			const uint16 attackCount = musicData.readUint16BE();
			const uint16 releaseCount = musicData.readUint16BE();
			debug("wave nr %d at %08X", 0, musicData.pos());
			
			skipLen = attackCount * 4 + releaseCount * 4 
				+ (attackLen + sustainLen) * ((1 << octaves) - 1)
				+ 3 * 2;
		}
		musicData.skip(skipLen - 3 * 2);
	}
	debug("endpos %08X", musicData.pos());
	return res;
}

}	// End of namespace Audio