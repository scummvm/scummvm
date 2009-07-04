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

#ifndef SOUND_MODS_MAXTRAX_H
#define SOUND_MODS_MAXTRAX_H

#include "sound/mods/paula.h"

namespace Audio {

class MaxTrax : public Paula {
public:
	MaxTrax(int rate, bool stereo);
	virtual ~MaxTrax();

protected:
	void interrupt();

private:
public:

	uint16	_microtonal[128];

	struct PlayerContext {
		uint16	tempo;
		bool	filterOn;
		bool	handleVolume;

	} _playerCtx;

	struct Envelope {
		uint16	duration;
		uint16	volume;
	};

	struct Patch {
		Envelope *attackPtr;
		//Envelope *releasePtr;
		uint16	attackLen;
		uint16	releaseLen;

		uint16	tune;
		uint16	volume;

		// this was the SampleData struct in the assembler source
		int8	*samplePtr;
		uint32	sampleAttack;
		uint32	sampleSustain;
		uint16	sampleOctaves;
	} _patch[64];

	struct Event {
		uint16	startTime;
		uint16	stopTime;
		byte	command;
		byte	parameter;
	};

	struct Score {
		Event	*events;
		uint32	numEvents;
	} *_scores;

	int _numScores;



	bool load(Common::SeekableReadStream &musicData, bool loadScores = true, bool loadSamples = true);

	void stopMusic();
	void freePatches();
	void freeScores();

	static void outPutEvent(const Event &ev, int num = -1) {
		struct {
			byte cmd;
			char *name;
			char *param;
		} COMMANDS[] = {
			{0x80, "TEMPO   ", "TEMPO, N/A      "},
			{0xa0, "SPECIAL ", "CHAN, SPEC # | VAL"},
			{0xb0, "CONTROL ", "CHAN, CTRL # | VAL"},
			{0xc0, "PROGRAM ", "CHANNEL, PROG # "},
			{0xe0, "BEND    ", "CHANNEL, BEND VALUE"},
			{0xf0, "SYSEX   ", "TYPE, SIZE      "},
			{0xf8, "REALTIME", "REALTIME, N/A   "},
			{0xff, "END     ", "N/A, N/A        "},
			{0xff, "NOTE    ", "VOL | CHAN, STOP"},
		};

		int i = 0;
		for (; i < ARRAYSIZE(COMMANDS) - 1 && ev.command != COMMANDS[i].cmd; ++i)
			;

		if (num == -1)
			debug("Event    : %02X %s %s %02X %04X %04X", ev.command, COMMANDS[i].name, COMMANDS[i].param, ev.parameter, ev.startTime, ev.stopTime);
		else
			debug("Event %3d: %02X %s %s %02X %04X %04X", num, ev.command, COMMANDS[i].name, COMMANDS[i].param, ev.parameter, ev.startTime, ev.stopTime);
	}

	static void outPutScore(const Score &sc, int num = -1) {
		if (num == -1)
			debug("score   : %i Events", sc.numEvents);
		else
			debug("score %2d: %i Events", num, sc.numEvents);
		for (uint i = 0; i < sc.numEvents; ++i)
			outPutEvent(sc.events[i], i);
		debug("");
	}
};
}	// End of namespace Audio

#endif