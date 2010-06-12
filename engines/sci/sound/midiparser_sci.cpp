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
#include "sci/sound/midiparser_sci.h"
#include "sci/sound/drivers/mididriver.h"

namespace Sci {

static const int nMidiParams[] = { 2, 2, 2, 2, 1, 1, 2, 0 };

enum SciMidiCommands {
	kSetSignalLoop = 0x7F,
	kEndOfTrack = 0xFC,
	kSetReverb = 0x50,
	kMidiHold = 0x52,
	kUpdateCue = 0x60,
	kResetOnPause = 0x4C
};

//  MidiParser_SCI
//
MidiParser_SCI::MidiParser_SCI(SciVersion soundVersion) :
	MidiParser() {
	_soundVersion = soundVersion;
	_mixedData = NULL;
	// mididata contains delta in 1/60th second
	// values of ppqn and tempo are found experimentally and may be wrong
	_ppqn = 1;
	setTempo(16667);

	_volume = 0;

	_signalSet = false;
	_signalToSet = 0;
	_dataincAdd = false;
	_dataincToAdd = 0;
	_resetOnPause = false;
	_channelsUsed = 0;

	for (int i = 0; i < 16; i++)
		_channelRemap[i] = i;
}

MidiParser_SCI::~MidiParser_SCI() {
	unloadMusic();
}

bool MidiParser_SCI::loadMusic(SoundResource::Track *track, MusicEntry *psnd, int channelFilterMask, SciVersion soundVersion) {
	unloadMusic();
	_track = track;
	_pSnd = psnd;
	_soundVersion = soundVersion;

	if (_pSnd)
		setVolume(psnd->volume);

	if (channelFilterMask) {
		// SCI0 only has 1 data stream, but we need to filter out channels depending on music hardware selection
		midiFilterChannels(channelFilterMask);
	} else {
		midiMixChannels();
	}

	_num_tracks = 1;
	_tracks[0] = _mixedData;
	if (_pSnd)
		setTrack(0);
	_loopTick = 0;

	if (_pSnd) {
		if (_soundVersion <= SCI_VERSION_0_LATE) {
			// Set initial voice count
			for (int i = 0; i < 16; ++i) {
				byte voiceCount = 0;
				if (channelFilterMask & (1 << i))
					voiceCount = psnd->soundRes->getInitialVoiceCount(i);
				_driver->send(0xB0 | i, 0x4B, voiceCount);
			}
		}

		// Send a velocity off signal to all channels
		for (int i = 0; i < 16; ++i) {
			_driver->send(0xB0 | i, 0x4E, 0);	// Reset velocity
		}
	}

	return true;
}

void MidiParser_SCI::unloadMusic() {
	if (_pSnd) {
		resetTracking();
		allNotesOff();
	}
	_num_tracks = 0;
	_active_track = 255;
	_resetOnPause = false;

	if (_mixedData) {
		delete[] _mixedData;
		_mixedData = NULL;
	}

	// Center the pitch wheels and hold pedal in preparation for the next piece of music
	if (_driver && _pSnd) {
		for (int i = 0; i < 16; ++i) {
			if (isChannelUsed(i)) {
				_driver->send(0xE0 | i, 0, 0x40);	// Reset pitch wheel
				_driver->send(0xB0 | i, 0x40, 0);	// Reset hold pedal
			}
		}
	}

	for (int i = 0; i < 16; i++)
		_channelRemap[i] = i;
}

void MidiParser_SCI::parseNextEvent(EventInfo &info) {
	// Monitor which channels are used by this song
	setChannelUsed(info.channel());

	// Set signal AFTER waiting for delta, otherwise we would set signal too soon resulting in all sorts of bugs
	if (_dataincAdd) {
		_dataincAdd = false;
		_pSnd->dataInc += _dataincToAdd;
		_pSnd->signal = 0x7f + _pSnd->dataInc;
		debugC(4, kDebugLevelSound, "datainc %04x", _dataincToAdd);
	}
	if (_signalSet) {
		_signalSet = false;
		_pSnd->signal = _signalToSet;
		debugC(4, kDebugLevelSound, "signal %04x", _signalToSet);
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
			if (info.basic.param1 != kSetSignalLoop) {
				_signalSet = true;
				_signalToSet = info.basic.param1;
			} else {
				_loopTick = _position._play_tick + info.delta;
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
			// Reference for some events:
			// http://wiki.scummvm.org/index.php/SCI/Specifications/Sound/SCI0_Resource_Format#Status_Reference
			// Also, sci/sound/iterator/iterator.cpp, function BaseSongIterator::parseMidiCommand()
			switch (info.basic.param1) {
			case kSetReverb:
				((MidiPlayer *)_driver)->setReverb(info.basic.param2);
				break;
			case kMidiHold:
				// Check if the hold ID marker is the same as the hold ID
				// marker set for that song by cmdSetSoundHold.
				// If it is, loop back, but don't stop notes when jumping.
				if (info.basic.param2 == _pSnd->hold)
					jumpToTick(_loopTick, false, false);
				break;
			case kUpdateCue:
				_dataincAdd = true;
				switch (_soundVersion) {
				case SCI_VERSION_0_EARLY:
				case SCI_VERSION_0_LATE:
					_dataincToAdd = info.basic.param2;
					break;
				case SCI_VERSION_1_EARLY:
				case SCI_VERSION_1_LATE:
					_dataincToAdd = 1;
					break;
				default:
					break;
				}
				break;
			case kResetOnPause:
				_resetOnPause = info.basic.param2;
				break;
			// Unhandled SCI commands
			case 0x46: // LSL3 - binoculars
			case 0x61: // Iceman (AdLib?)
			case 0x73: // Hoyle
			case 0xD1: // KQ4, when riding the unicorn
				// Obscure SCI commands - ignored
				break;
			// Standard MIDI commands
			case 0x01:	// mod wheel
			case 0x04:	// foot controller
			case 0x07:	// channel volume
			case 0x0A:	// pan
			case 0x0B:	// expression
			case 0x40:	// sustain
			case 0x4E:	// velocity control
			case 0x79:	// reset all
			case 0x7B:	// notes off
				// These are all handled by the music driver, so ignore them
				break;
			case 0x4B:	// voice mapping
				// TODO: is any support for this needed at the MIDI parser level?
				warning("Unhanded SCI MIDI command 0x%x - voice mapping (parameter %d)", info.basic.param1, info.basic.param2);
				break;
			default:
				warning("Unhandled SCI MIDI command 0x%x (parameter %d)", info.basic.param1, info.basic.param2);
				break;
			}
		}
		if (info.basic.param1 == 7) // channel volume change -scale it
			info.basic.param2 = info.basic.param2 * _volume / MUSIC_VOLUME_MAX;
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
				if (_pSnd->loop)
					_pSnd->loop--;
				if (_pSnd->loop) {
					// We need to play it again...
					jumpToTick(_loopTick);
				} else {
					_pSnd->status = kSoundStopped;
					_pSnd->signal = SIGNAL_OFFSET;

					debugC(4, kDebugLevelSound, "signal EOT");
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


byte MidiParser_SCI::midiGetNextChannel(long ticker) {
	byte curr = 0xFF;
	long closest = ticker + 1000000, next = 0;

	for (int i = 0; i < _track->channelCount; i++) {
		if (_track->channels[i].time == -1) // channel ended
			continue;
		next = *_track->channels[i].data; // when the next event should occur
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

byte *MidiParser_SCI::midiMixChannels() {
	int totalSize = 0;
	byte **dataPtr = new byte *[_track->channelCount];

	for (int i = 0; i < _track->channelCount; i++) {
		dataPtr[i] = _track->channels[i].data;
		_track->channels[i].time = 0;
		_track->channels[i].prev = 0;
		totalSize += _track->channels[i].size;
	}

	byte *outData = new byte[totalSize * 2]; // FIXME: creates overhead and still may be not enough to hold all data
	_mixedData = outData;
	long ticker = 0;
	byte curr, curDelta;
	byte command = 0, par1, global_prev = 0;
	long new_delta;
	SoundResource::Channel *channel;

	while ((curr = midiGetNextChannel(ticker)) != 0xFF) { // there is still an active channel
		channel = &_track->channels[curr];
		curDelta = *channel->data++;
		channel->time += (curDelta == 0xF8 ? 240 : curDelta); // when the command is supposed to occur
		if (curDelta == 0xF8)
			continue;
		new_delta = channel->time - ticker;
		ticker += new_delta;

		command = *channel->data++;
		if (command != kEndOfTrack) {
			debugC(4, kDebugLevelSound, "\nDELTA ");
			// Write delta
			while (new_delta > 240) {
				*outData++ = 0xF8;
				debugC(4, kDebugLevelSound, "F8 ");
				new_delta -= 240;
			}
			*outData++ = (byte)new_delta;
			debugC(4, kDebugLevelSound, "%02X ", (uint32)new_delta);
		}
		// Write command
		switch (command) {
		case 0xF0: // sysEx
			*outData++ = command;
			debugC(4, kDebugLevelSound, "%02X ", command);
			do {
				par1 = *channel->data++;
				*outData++ = par1; // out
			} while (par1 != 0xF7);
			break;
		case kEndOfTrack: // end of channel
			channel->time = -1; // FIXME
			break;
		default: // MIDI command
			if (command & 0x80) {
				par1 = *channel->data++;

				// TODO: Fix remapping

#if 0
				// Remap channel. Keep the upper 4 bits (command code) and change
				// the lower 4 bits (channel)
				byte remappedChannel = _channelRemap[par1 & 0xF];
				par1 = (par1 & 0xF0) | (remappedChannel & 0xF);
#endif
			} else {// running status
				par1 = command;
				command = channel->prev;
			}
			if (command != global_prev)
				*outData++ = command; // out command
			*outData++ = par1;// pout par1
			if (nMidiParams[(command >> 4) - 8] == 2)
				*outData++ = *channel->data++; // out par2
			channel->prev = command;
			global_prev = command;
		}// switch(command)
	}// while (curr)

	// Insert stop event
	*outData++ = 0;    // Delta
	*outData++ = 0xFF; // Meta event
	*outData++ = 0x2F; // End of track (EOT)
	*outData++ = 0x00;
	*outData++ = 0x00;

	for (int channelNr = 0; channelNr < _track->channelCount; channelNr++)
		_track->channels[channelNr].data = dataPtr[channelNr];

	delete[] dataPtr;
	return _mixedData;
}

// This is used for SCI0 sound-data. SCI0 only has one stream that may
// contain several channels and according to output device we remove
// certain channels from that data.
byte *MidiParser_SCI::midiFilterChannels(int channelMask) {
	SoundResource::Channel *channel = &_track->channels[0];
	byte *channelData = channel->data;
	byte *channelDataEnd = channel->data + channel->size;
	byte *outData = new byte[channel->size + 5];
	byte curChannel = 15, curByte, curDelta;
	byte command = 0, lastCommand = 0;
	int delta = 0;
	int midiParamCount = 0;

	_mixedData = outData;

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
		case kEndOfTrack: // end of channel
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
			if (command != kEndOfTrack) {
				debugC(4, kDebugLevelSound, "\nDELTA ");
				// Write delta
				while (delta > 240) {
					*outData++ = 0xF8;
					debugC(4, kDebugLevelSound, "F8 ");
					delta -= 240;
				}
				*outData++ = (byte)delta;
				debugC(4, kDebugLevelSound, "%02X ", delta);
				delta = 0;
			}
			// Write command
			switch (command) {
			case 0xF0: // sysEx
				*outData++ = command;
				debugC(4, kDebugLevelSound, "%02X ", command);
				do {
					curByte = *channelData++;
					*outData++ = curByte; // out
				} while (curByte != 0xF7);
				lastCommand = command;
				break;

			case kEndOfTrack: // end of channel
				break;

			default: // MIDI command
				if (lastCommand != command) {
					*outData++ = command;
					debugC(4, kDebugLevelSound, "%02X ", command);
					lastCommand = command;
				}
				if (midiParamCount > 0) {
					if (curByte & 0x80) {
						debugC(4, kDebugLevelSound, "%02X ", *channelData);
						*outData++ = *channelData++;
					} else {
						debugC(4, kDebugLevelSound, "%02X ", curByte);
						*outData++ = curByte;
					}
				}
				if (midiParamCount > 1) {
					debugC(4, kDebugLevelSound, "%02X ", *channelData);
					*outData++ = *channelData++;
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

	// Insert stop event
	*outData++ = 0;    // Delta
	*outData++ = 0xFF; // Meta event
	*outData++ = 0x2F; // End of track (EOT)
	*outData++ = 0x00;
	*outData++ = 0x00;

	return _mixedData;
}

void MidiParser_SCI::setVolume(byte volume) {
	// FIXME: This receives values > 127... throw a warning for now and clip the variable
	if (volume > MUSIC_VOLUME_MAX) {
		warning("attempted to set an invalid volume(%d)", volume);
		volume = MUSIC_VOLUME_MAX;	// reset
	}

	assert(volume <= MUSIC_VOLUME_MAX);
	if (_volume != volume) {
		_volume = volume;

		switch (_soundVersion) {
		case SCI_VERSION_0_EARLY:
		case SCI_VERSION_0_LATE: {
			int16 globalVolume = _volume * 15 / 127;
			((MidiPlayer *)_driver)->setVolume(globalVolume);
			break;
		}

		case SCI_VERSION_1_EARLY:
		case SCI_VERSION_1_LATE:
			// sending volume change to all active channels
			for (int i = 0; i < _track->channelCount; i++)
				if (_track->channels[i].number <= 0xF)
					_driver->send(0xB0 + _track->channels[i].number, 7, _volume);
			break;

		default:
			error("MidiParser_SCI::setVolume: Unsupported soundVersion");
		}
	}
}

} // End of namespace Sci
