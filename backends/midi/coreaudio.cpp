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
 * $Header$
 */

#ifdef MACOSX

#include "common/stdafx.h"
#include "common/config-manager.h"
#include "common/util.h"
#include "sound/mpu401.h"

#include <AudioUnit/AudioUnit.h>
#include <CoreMIDI/CoreMIDI.h>7


// Activating the following switch disables reverb support in the CoreAudio
// midi backend. Reverb will suck away a *lot* of CPU time, so on slower
// systems, you may want to turn it off completely.
// TODO: Maybe make this a config option?
//#define COREAUDIO_DISABLE_REVERB


// Enable the following switch to make ScummVM try to use native MIDI hardware
// on your computer for MIDI output. This is currently quite hackish, in
// particular you have no way to specify which device is used (it just always
// uses the first output device it can find), nor is there a switch to
// force it to use the soft synth instead of the MIDI HW.
//#define ENABLE_HACKISH_NATIVE_MIDI_SUPPORT 1

/* CoreAudio MIDI driver
 * Based on code by Benjamin W. Zale
 * Extended by Max Horn
 */
class MidiDriver_CORE : public MidiDriver_MPU401 {
public:
	MidiDriver_CORE();
	~MidiDriver_CORE();
	int open();
	void close();
	void send(uint32 b);
	void sysEx(byte *msg, uint16 length);

private:
	AudioUnit au_MusicDevice;
	AudioUnit au_output;

	MIDIClientRef	mClient;
	MIDIPortRef		mOutPort;
	MIDIEndpointRef	mDest;
};

MidiDriver_CORE::MidiDriver_CORE()
	: au_MusicDevice(0), au_output(0), mClient(0), mOutPort(0), mDest(0) {

	OSStatus err;
	err = MIDIClientCreate(CFSTR("ScummVM MIDI Driver for OS X"), NULL, NULL, &mClient);
}

MidiDriver_CORE::~MidiDriver_CORE() {
	if (mClient)
		MIDIClientDispose(mClient);
	mClient = 0;
}

int MidiDriver_CORE::open() {
	if (au_output || mDest)
		return MERR_ALREADY_OPEN;

	OSStatus err = noErr;

	mOutPort = 0;
#ifdef ENABLE_HACKISH_NATIVE_MIDI_SUPPORT
	int dests = MIDIGetNumberOfDestinations();
	if (dests > 0 && mClient) {
		mDest = MIDIGetDestination(0);
		err = MIDIOutputPortCreate( mClient,
									CFSTR("scummvm_output_port"),
									&mOutPort);
	}
#endif

	if (err != noErr || !mOutPort) {
		AudioUnitConnection auconnect;
		ComponentDescription compdesc;
		Component compid;

		// Open the Music Device.
		// We use the AudioUnit v1 API, even though it is deprecated, because
		// this way we stay compatible with older OS X versions.
		// For v2, we'd use kAudioUnitType_MusicDevice and kAudioUnitSubType_DLSSynth
		compdesc.componentType = kAudioUnitComponentType;
		compdesc.componentSubType = kAudioUnitSubType_MusicDevice;
		compdesc.componentManufacturer = kAudioUnitID_DLSSynth;
		compdesc.componentFlags = 0;
		compdesc.componentFlagsMask = 0;
		compid = FindNextComponent(NULL, &compdesc);
		au_MusicDevice = static_cast<AudioUnit>(OpenComponent(compid));

		if (au_MusicDevice == 0)
			error("Failed opening CoreAudio music device");

		// Load custom soundfont, if specified
		// FIXME: This is kind of a temporary hack. Better (IMO) would be to
		// query QuickTime for whatever custom soundfont was set in the
		// QuickTime Preferences, and use that automatically.
		if (ConfMan.hasKey("soundfont")) {
			FSRef	fsref;
			FSSpec	fsSpec;
			const char *soundfont = ConfMan.get("soundfont").c_str();

			err = FSPathMakeRef ((const byte *)soundfont, &fsref, NULL);

			if (err == noErr) {
				err = FSGetCatalogInfo (&fsref, kFSCatInfoNone, NULL, NULL, &fsSpec, NULL);
			}

			if (err == noErr) {
				err = AudioUnitSetProperty (
					au_MusicDevice,
					kMusicDeviceProperty_SoundBankFSSpec, kAudioUnitScope_Global,
					0,
					&fsSpec, sizeof(fsSpec)
				);
			}

			if (err != noErr)
				warning("Failed loading custom sound font '%s' (error %d)\n", soundfont, err);
		}

		// open the output unit
		au_output = (AudioUnit) OpenDefaultComponent(kAudioUnitComponentType, kAudioUnitSubType_Output);
		if (au_output == 0)
			error("Failed opening output audio unit");

		// connect the units
		auconnect.sourceAudioUnit = au_MusicDevice;
		auconnect.sourceOutputNumber = 0;
		auconnect.destInputNumber = 0;
		err =
			AudioUnitSetProperty(au_output, kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input, 0,
													 (void *)&auconnect, sizeof(AudioUnitConnection));

	#ifdef COREAUDIO_DISABLE_REVERB
		UInt32 usesReverb = 0;
		AudioUnitSetProperty (au_MusicDevice, kMusicDeviceProperty_UsesInternalReverb,
			kAudioUnitScope_Global,    0, &usesReverb, sizeof (usesReverb));
	#endif

		// initialize the units
		AudioUnitInitialize(au_MusicDevice);
		AudioUnitInitialize(au_output);

		// start the output
		AudioOutputUnitStart(au_output);

	}

	return 0;
}

void MidiDriver_CORE::close() {
	MidiDriver_MPU401::close();

	if (mOutPort && mDest) {
		MIDIPortDispose(mOutPort);
		mOutPort = 0;
		mDest = 0;
	} else {
		// Stop the output
		AudioOutputUnitStop(au_output);

		// Cleanup
		CloseComponent(au_output);
		au_output = 0;
		CloseComponent(au_MusicDevice);
		au_MusicDevice = 0;
	}
}

void MidiDriver_CORE::send(uint32 b) {
	byte status_byte = (b & 0x000000FF);
	byte first_byte = (b & 0x0000FF00) >> 8;
	byte second_byte = (b & 0x00FF0000) >> 16;

	if (mOutPort && mDest) {
		MIDIPacketList packetList;
		MIDIPacket *packet = &packetList.packet[0];

		packetList.numPackets = 1;

		packet->timeStamp = 0;
		packet->length = 3;
		packet->data[0] = status_byte;
		packet->data[1] = first_byte;
		packet->data[2] = second_byte;

		MIDISend(mOutPort, mDest, &packetList);
	} else {
		assert(au_output != NULL);
		assert(au_MusicDevice != NULL);
		MusicDeviceMIDIEvent(au_MusicDevice, status_byte, first_byte, second_byte, 0);
	}
}

void MidiDriver_CORE::sysEx(byte *msg, uint16 length) {

	if (mOutPort && mDest) {
		byte buf[384];
		MIDIPacketList *packetList = (MIDIPacketList *)buf;
		MIDIPacket *packet = packetList->packet;

		assert(sizeof(buf) >= sizeof(UInt32) + sizeof(MIDITimeStamp) + sizeof(UInt16) + length + 2);

		packetList->numPackets = 1;

		packet->timeStamp = 0;

		// Add SysEx frame if missing
		if (*msg != 0xF0) {
			packet->length = length + 2;
			packet->data[0] = 0xF0;
			memcpy(packet->data + 1, msg, length);
			packet->data[length + 1] = 0xF7;
		} else {
			packet->length = length;
			memcpy(packet->data, msg, length);
		}

		MIDISend(mOutPort, mDest, packetList);
	} else {
		assert(au_output != NULL);
		assert(au_MusicDevice != NULL);

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

		MusicDeviceSysEx(au_MusicDevice, msg, length);

		free(buf);
	}
}

MidiDriver *MidiDriver_CORE_create() {
	return new MidiDriver_CORE();
}

#endif // MACOSX
