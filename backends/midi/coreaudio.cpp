/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#ifdef MACOSX

#include "common/stdafx.h"
#include "common/config-manager.h"
#include "common/util.h"
#include "sound/mpu401.h"

#include <AudioToolbox/AUGraph.h>



// Activating the following switch disables reverb support in the CoreAudio
// midi backend. Reverb will suck away a *lot* of CPU time, so on slower
// systems, you may want to turn it off completely.
// TODO: Maybe make this a config option?
//#define COREAUDIO_DISABLE_REVERB


// A macro to simplify error handling a bit.
#define RequireNoErr(error)                                         \
do {                                                                \
	OSStatus localError = error;                                    \
	if (localError != noErr)                                        \
		throw localError;                                           \
} while (false)


/* CoreAudio MIDI driver
 * By Max Horn / Fingolfin
 * Based on code by Benjamin W. Zale
 */
class MidiDriver_CORE : public MidiDriver_MPU401 {
public:
	MidiDriver_CORE();
	int open();
	void close();
	void send(uint32 b);
	void sysEx(byte *msg, uint16 length);

private:
	AUGraph _auGraph;
	AudioUnit _synth;
};

MidiDriver_CORE::MidiDriver_CORE()
	: _auGraph(0) {
}

int MidiDriver_CORE::open() {
	if (_auGraph)
		return MERR_ALREADY_OPEN;

	// Open the Music Device.
	// We use the AudioUnit v1 API, even though it is deprecated, because
	// this way we stay compatible with older OS X versions.
	// For v2, we'd use kAudioUnitType_MusicDevice and kAudioUnitSubType_DLSSynth
	try {
		RequireNoErr(NewAUGraph(&_auGraph));

		AUNode outputNode, synthNode;
		ComponentDescription desc;

		// The default output device
		desc.componentType = kAudioUnitComponentType;
		desc.componentSubType = kAudioUnitSubType_Output;
		desc.componentManufacturer = kAudioUnitID_DefaultOutput;
		desc.componentFlags = 0;
		desc.componentFlagsMask = 0;
		RequireNoErr(AUGraphNewNode(_auGraph, &desc, 0, NULL, &outputNode));

		// The built-in default (softsynth) music device
		desc.componentSubType = kAudioUnitSubType_MusicDevice;
		desc.componentManufacturer = kAudioUnitID_DLSSynth;
		RequireNoErr(AUGraphNewNode(_auGraph, &desc, 0, NULL, &synthNode));

		// Connect the softsynth to the default output
		RequireNoErr(AUGraphConnectNodeInput(_auGraph, synthNode, 0, outputNode, 0));

		// Open and initialize the whole graph
		RequireNoErr(AUGraphOpen(_auGraph));
		RequireNoErr(AUGraphInitialize(_auGraph));

		// Get the music device from the graph.
		RequireNoErr(AUGraphGetNodeInfo(_auGraph, synthNode, NULL, NULL, NULL, &_synth));


		// Load custom soundfont, if specified
		if (ConfMan.hasKey("soundfont")) {
			OSErr	err;
			FSRef	fsref;
			FSSpec	fsSpec;
			const char *soundfont = ConfMan.get("soundfont").c_str();

			err = FSPathMakeRef ((const byte *)soundfont, &fsref, NULL);

			if (err == noErr) {
				err = FSGetCatalogInfo (&fsref, kFSCatInfoNone, NULL, NULL, &fsSpec, NULL);
			}

			if (err == noErr) {
				// TODO: We should really check here whether the file contains an
				// actual soundfont...
				err = AudioUnitSetProperty (
					_synth,
					kMusicDeviceProperty_SoundBankFSSpec, kAudioUnitScope_Global,
					0,
					&fsSpec, sizeof(fsSpec)
				);
			}

			if (err != noErr)
				warning("Failed loading custom sound font '%s' (error %d)\n", soundfont, err);
		}

#ifdef COREAUDIO_DISABLE_REVERB
		// Disable reverb mode, as that sucks up a lot of CPU power, which can
		// be painful on low end machines.
		// TODO: Make this customizable via a config key?
		UInt32 usesReverb = 0;
		AudioUnitSetProperty (_synth, kMusicDeviceProperty_UsesInternalReverb,
			kAudioUnitScope_Global, 0, &usesReverb, sizeof (usesReverb));
#endif


		// Finally: Start the graph!
		RequireNoErr(AUGraphStart(_auGraph));

	} catch (OSStatus err) {
		if (_auGraph) {
			AUGraphStop(_auGraph);
			DisposeAUGraph(_auGraph);
			_auGraph = 0;
		}
		return MERR_CANNOT_CONNECT;
	}
	return 0;
}

void MidiDriver_CORE::close() {
	MidiDriver_MPU401::close();

	if (_auGraph) {
		AUGraphStop(_auGraph);
		DisposeAUGraph(_auGraph);
		_auGraph = 0;
	}
}

void MidiDriver_CORE::send(uint32 b) {
	assert(_auGraph != NULL);

	byte status_byte = (b & 0x000000FF);
	byte first_byte = (b & 0x0000FF00) >> 8;
	byte second_byte = (b & 0x00FF0000) >> 16;

	MusicDeviceMIDIEvent(_synth, status_byte, first_byte, second_byte, 0);
}

void MidiDriver_CORE::sysEx(byte *msg, uint16 length) {
	assert(_auGraph != NULL);

	// Add SysEx frame if missing
	byte *buf = 0;
	if (*msg != 0xF0) {
		buf = (byte *)malloc(length + 2);
		buf[0] = 0xF0;
		memcpy(buf+1, msg, length);
		buf[length+1] = 0xF7;
		msg = buf;
		length += 2;
	}

	MusicDeviceSysEx(_synth, msg, length);

	free(buf);
}

MidiDriver *MidiDriver_CORE_create() {
	return new MidiDriver_CORE();
}

#endif // MACOSX
