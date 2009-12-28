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

#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/sfx/midiparser.h"

namespace Sci {

static const int nMidiParams[] = { 2, 2, 2, 2, 1, 1, 2, 0 };

//---------------------------------------------
//  MidiParser_SCI
//
MidiParser_SCI::MidiParser_SCI() :
	MidiParser() {
	_mixedData = NULL;
	// mididata contains delta in 1/60th second
	// values of ppqn and tempo are found experimentally and may be wrong
	_ppqn = 1;
	setTempo(16667);

	_signalSet = false;
	_signalToSet = 0;
}
//---------------------------------------------
MidiParser_SCI::~MidiParser_SCI() {
	unloadMusic();
}
//---------------------------------------------
bool MidiParser_SCI::loadMusic(SoundResource::Track *track, MusicEntry *psnd, int channelFilterMask, SciVersion soundVersion) {
	unloadMusic();
	_track = track;
	_pSnd = psnd;
	_soundVersion = soundVersion;

	setVolume(psnd->volume);

	if (channelFilterMask) {
		// SCI0 only has 1 data stream, but we need to filter out channels depending on music hardware selection
		midiFilterChannels(channelFilterMask);
	} else {
		midiMixChannels();
	}

	_num_tracks = 1;
	_tracks[0] = _mixedData;
	setTrack(0);
	_loopTick = 0;
	return true;
}

void MidiParser_SCI::unloadMusic() {
	allNotesOff();
	resetTracking();
	_num_tracks = 0;
	if (_mixedData) {
		delete[] _mixedData;
		_mixedData = NULL;
	}
}

void MidiParser_SCI::parseNextEvent(EventInfo &info) {
	SegManager *segMan = ((SciEngine *)g_engine)->getEngineState()->_segMan;	// HACK

	// Set signal AFTER waiting for delta, otherwise we would set signal too soon resulting in all sorts of bugs
	if (_signalSet) {
		_signalSet = false;
		PUT_SEL32V(segMan, _pSnd->soundObj, signal, _signalToSet);
		debugC(2, kDebugLevelSound, "signal %04x", _signalToSet);
	}

	info.start = _position._play_pos;
	info.delta = 0;
	while (*_position._play_pos == 0xF8) {
		info.delta += 240;
		_position._play_pos++;
	}
	info.delta += *(_position._play_pos++);

	// Process the next info.
	if ((_position._play_pos[0] & 0xF0) >= 0x80)
		info.event = *(_position._play_pos++);
	else
		info.event = _position._running_status;
	if (info.event < 0x80)
		return;

	_position._running_status = info.event;
	switch (info.command()) {
	case 0xC:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = 0;
		if (info.channel() == 0xF) {// SCI special case
			if (info.basic.param1 != 0x7F) {
				_signalSet = true;
				_signalToSet = info.basic.param1;
			} else {
				_loopTick = _position._play_tick;
			}
		}
		break;
	case 0xD:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = 0;
		break;

	case 0xB:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = *(_position._play_pos++);
		if (info.channel() == 0xF) {// SCI special
			if (info.basic.param1 == 0x60) {
				switch (_soundVersion) {
				case SCI_VERSION_0_EARLY:
				case SCI_VERSION_0_LATE:
					_pSnd->dataInc += info.basic.param2;
					_signalSet = true;
					_signalToSet = 0x7f + _pSnd->dataInc;
					break;
				case SCI_VERSION_1_EARLY:
				case SCI_VERSION_1_LATE:
					_pSnd->dataInc++;
					break;
				default:
					break;
				}
			}
			// BF 50 x - set reverb to x
			// BF 60 x - dataInc++
			// BF 52 x - bHold=x
		}
		if (info.basic.param1 == 7) // channel volume change -scale it
			info.basic.param2 = info.basic.param2 * _volume / 0x7F;
		info.length = 0;
		break;

	case 0x8:
	case 0x9:
	case 0xA:
	case 0xE:
		info.basic.param1 = *(_position._play_pos++);
		info.basic.param2 = *(_position._play_pos++);
		if (info.command() == 0x9 && info.basic.param2 == 0)
			info.event = info.channel() | 0x80;
		info.length = 0;
		break;

	case 0xF: // System Common, Meta or SysEx event
		switch (info.event & 0x0F) {
		case 0x2: // Song Position Pointer
			info.basic.param1 = *(_position._play_pos++);
			info.basic.param2 = *(_position._play_pos++);
			break;

		case 0x3: // Song Select
			info.basic.param1 = *(_position._play_pos++);
			info.basic.param2 = 0;
			break;

		case 0x6:
		case 0x8:
		case 0xA:
		case 0xB:
		case 0xC:
		case 0xE:
			info.basic.param1 = info.basic.param2 = 0;
			break;

		case 0x0: // SysEx
			info.length = readVLQ(_position._play_pos);
			info.ext.data = _position._play_pos;
			_position._play_pos += info.length;
			break;

		case 0xF: // META event
			info.ext.type = *(_position._play_pos++);
			info.length = readVLQ(_position._play_pos);
			info.ext.data = _position._play_pos;
			_position._play_pos += info.length;
			if (info.ext.type == 0x2F) {// end of track reached
				if (_pSnd->loop) {
					jumpToTick(_loopTick);
					_pSnd->loop--;
				} else {
					_pSnd->status = kSoundStopped;
					PUT_SEL32V(segMan, _pSnd->soundObj, signal, 0xFFFF);
					if (_soundVersion <= SCI_VERSION_0_LATE)
						PUT_SEL32V(segMan, _pSnd->soundObj, state, kSoundStopped);
					debugC(2, kDebugLevelSound, "signal EOT");
				}
			}
			break;
		default:
			warning(
					"MidiParser_SCI::parseNextEvent: Unsupported event code %x",
					info.event);
		} // // System Common, Meta or SysEx event
	}// switch (info.command())
}

//----------------------------------------
byte MidiParser_SCI::midiGetNextChannel(long ticker) {
	byte curr = 0xFF;
	long closest = ticker + 1000000, next = 0;

	for (int i = 0; i < _track->channelCount; i++) {
		if (_track->channels[i].time == -1) // channel ended
			continue;
		next = *_track->channels[i].data; // when the next event shoudl occur
		if (next == 0xF8) // 0xF8 means 240 ticks delay
			next = 240;
		next += _track->channels[i].time;
		if (next < closest) {
			curr = i;
			closest = next;
		}
	}

	return curr;
}
//----------------------------------------
byte *MidiParser_SCI::midiMixChannels() {
	int totalSize = 0;
	byte **dataPtr = new byte *[_track->channelCount];

	for (int i = 0; i < _track->channelCount; i++) {
		dataPtr[i] = _track->channels[i].data;
		_track->channels[i].time = 0;
		_track->channels[i].prev = 0;
		totalSize += _track->channels[i].size;
	}

	byte *mixedData = new byte[totalSize * 2]; // FIXME: creates overhead and still may be not enough to hold all data
	_mixedData = mixedData;
	long ticker = 0;
	byte curr, delta;
	byte cmd, par1, global_prev = 0;
	long new_delta;
	SoundResource::Channel *channel;
	while ((curr = midiGetNextChannel(ticker)) != 0xFF) { // there is still active channel
		channel = &_track->channels[curr];
		delta = *channel->data++;
		channel->time += (delta == 0xF8 ? 240 : delta); // when the comamnd is supposed to occur
		if (delta == 0xF8)
			continue;
		new_delta = channel->time - ticker;
		ticker += new_delta;

		cmd = *channel->data++;
		if (cmd != 0xFC) {
			// output new delta
			while (new_delta > 240) {
				*mixedData++ = 0xF8;
				new_delta -= 240;
			}
			*mixedData++ = (byte)new_delta;
		}
		switch (cmd) {
		case 0xF0: // sysEx
			*mixedData++ = cmd;
			do {
				par1 = *channel->data++;
				*mixedData++ = par1; // out
			} while (par1 != 0xF7);
			break;
		case 0xFC: // end channel
			channel->time = -1; // FIXME
			break;
		default: // MIDI command
			if (cmd & 0x80)
				par1 = *channel->data++;
			else {// running status
				par1 = cmd;
				cmd = channel->prev;
			}
			if (cmd != global_prev)
				*mixedData++ = cmd; // out cmd
			*mixedData++ = par1;// pout par1
			if (nMidiParams[(cmd >> 4) - 8] == 2)
				*mixedData++ = *channel->data++; // out par2
			channel->prev = cmd;
			global_prev = cmd;
		}// switch(cmd)
	}// while (curr)
	// mixing finished. inserting stop event
	*mixedData++ = 0;
	*mixedData++ = 0xFF;
	*mixedData++ = 0x2F;
	*mixedData++ = 0x00;
	*mixedData++ = 0x00;

	for (int channelNr = 0; channelNr < _track->channelCount; channelNr++)
		_track->channels[channelNr].data = dataPtr[channelNr];

	delete[] dataPtr;
	return _mixedData;
}

// This is used for SCI0 sound-data. SCI0 only has one stream that may contain several channels and according to output
//  device we remove certain channels from that data
byte *MidiParser_SCI::midiFilterChannels(int channelMask) {
	SoundResource::Channel *channel = &_track->channels[0];
	byte *channelData = channel->data;
	byte *channelDataEnd = channel->data + channel->size;
	byte *filterData = new byte[channel->size + 5];
	byte curChannel, curByte, curDelta;
	byte command, lastCommand;
	int delta = 0;
	//int dataLeft = channel->size;
	int midiParamCount;

	_mixedData = filterData;
	command = 0;
	midiParamCount = 0;
	lastCommand = 0;
	curChannel = 15;

	while (channelData < channelDataEnd) {
		curDelta = *channelData++;
		if (curDelta == 0xF8) {
			delta += 240;
			continue;
		}
		delta += curDelta;
		curByte = *channelData++;

		switch (curByte) {
		case 0xF0: // sysEx
		case 0xFC: // end of channel
			command = curByte;
			curChannel = 15;
			break;
		default:
			if (curByte & 0x80) {
				command = curByte;
				curChannel = command & 0x0F;
				midiParamCount = nMidiParams[(command >> 4) - 8];
			}
		}
		if ((1 << curChannel) & channelMask) {
			if (command != 0xFC) {
				debugC(2, kDebugLevelSound, "\nDELTA ");
				// Write delta
				while (delta > 240) {
					*filterData++ = 0xF8;
					debugC(2, kDebugLevelSound, "F8 ");
					delta -= 240;
				}
				*filterData++ = (byte)delta;
				debugC(2, kDebugLevelSound, "%02X ", delta);
				delta = 0;
			}
			// Write command
			switch (command) {
			case 0xF0: // sysEx
				*filterData++ = command;
				debugC(2, kDebugLevelSound, "%02X ", command);
				do {
					curByte = *channelData++;
					*filterData++ = curByte; // out
				} while (curByte != 0xF7);
				lastCommand = command;
				break;

			case 0xFC: // end of channel
				break;

			default: // MIDI command
				if (lastCommand != command) {
					*filterData++ = command;
					debugC(2, kDebugLevelSound, "%02X ", command);
					lastCommand = command;
				}
				if (midiParamCount > 0) {
					if (curByte & 0x80) {
						debugC(2, kDebugLevelSound, "%02X ", *channelData);
						*filterData++ = *channelData++;
					} else {
						debugC(2, kDebugLevelSound, "%02X ", curByte);
						*filterData++ = curByte;
					}
				}
				if (midiParamCount > 1) {
					debugC(2, kDebugLevelSound, "%02X ", *channelData);
					*filterData++ = *channelData++;
				}
			}
		} else {
			if (curByte & 0x80) {
				channelData += midiParamCount;
			} else {
				channelData += midiParamCount - 1;
			}
		}
	}
	// Stop event
	*filterData++ = 0;    // delta
	*filterData++ = 0xFF; // Meta-Event
	*filterData++ = 0x2F; // End-Of-Track
	*filterData++ = 0x00;
	*filterData++ = 0x00;

	return _mixedData;
}

void MidiParser_SCI::setVolume(byte bVolume) {
	if (bVolume > 0x7F)
		bVolume = 0x7F;
	if (_volume != bVolume) {
		_volume = bVolume;

		// sending volume change to all active channels
		for (int i = 0; i < _track->channelCount; i++)
			if (_track->channels[i].number <= 0xF)
				_driver->send(0xB0 + _track->channels[i].number, 7, _volume);
	}
}
} // end of namespace SCI
