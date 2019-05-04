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
 */

#ifndef KYRA_SOUND_MIDIDRIVER_H
#define KYRA_SOUND_MIDIDRIVER_H

#include "kyra/sound/sound_intern.h"

namespace Kyra {

class MidiOutput : public MidiDriver_BASE {
public:
	MidiOutput(OSystem *system, MidiDriver *output, bool isMT32, bool defaultMT32);
	~MidiOutput();

	void setSourceVolume(int source, int volume, bool apply=false);

	void initSource(int source);
	void deinitSource(int source);
	void stopNotesOnChannel(int channel);

	void setSoundSource(int source) { _curSource = source; }

	// MidiDriver_BASE interface
	virtual void send(uint32 b);
	virtual void sysEx(const byte *msg, uint16 length);
	virtual void metaEvent(byte type, byte *data, uint16 length);

	// TODO: Get rid of the following two methods
	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { _output->setTimerCallback(timerParam, timerProc); }
	uint32 getBaseTempo() { return _output->getBaseTempo(); }


private:
	void sendIntern(const byte event, const byte channel, byte param1, const byte param2);
	void sendSysEx(const byte p1, const byte p2, const byte p3, const byte *buffer, const int size);

	OSystem *_system;
	MidiDriver *_output;

	bool _isMT32;
	bool _defaultMT32;

	struct Controller {
		byte controller;
		byte value;
	};

	enum {
		kChannelLocked = 0x80,
		kChannelProtected = 0x40
	};

	struct Channel {
		byte flags;

		byte program;
		int16 pitchWheel;

		byte noteCount;

		Controller controllers[9];
	} _channels[16];

	int lockChannel();
	void unlockChannel(int channel);

	int _curSource;

	struct SoundSource {
		int volume;

		int8 channelMap[16];
		byte channelProgram[16];
		int16 channelPW[16];
		Controller controllers[16][9];

		struct Note {
			byte channel;
			byte note;
		};

		Note notes[32];
	} _sources[4];
};

} // End of namespace Kyra

#endif
