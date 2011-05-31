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
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#ifdef MACOSX

// HACK to disable deprecated warnings under Mac OS X 10.5.
// Apple depracted the AUGraphNewNode & AUGraphGetNodeInfo APIs
// in favor of the new AUGraphAddNode & AUGraphNodeInfo APIs.
// While it would be trivial to switch to those, this would break
// binary compatibility with all pre-10.5 systems, so we don't want
// to do that just now. Maybe when 10.6 comes... :)
#include <AvailabilityMacros.h>
#undef DEPRECATED_ATTRIBUTE
#define DEPRECATED_ATTRIBUTE


#include "common/config-manager.h"
#include "common/error.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include <CoreServices/CoreServices.h>
#include <AudioToolbox/AUGraph.h>


// Activating the following switch disables reverb support in the CoreAudio
// midi backend. Reverb will suck away a *lot* of CPU time, so on slower
// systems, you may want to turn it off completely.
// TODO: Maybe make this a config option?
//#define COREAUDIO_DISABLE_REVERB


// A macro to simplify error handling a bit.
#define RequireNoErr(error)                                         \
do {                                                                \
	err = error;                                                    \
	if (err != noErr)                                               \
		goto bail;                                                  \
} while (false)


/* CoreAudio MIDI driver
 * By Max Horn / Fingolfin
 * Based on code by Benjamin W. Zale
 */
class MidiDriver_CORE : public MidiDriver_MPU401 {
public:
	MidiDriver_CORE();
	~MidiDriver_CORE();
	int open();
	bool isOpen() const { return _auGraph != 0; }
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	AUGraph _auGraph;
	AudioUnit _synth;
};

MidiDriver_CORE::MidiDriver_CORE()
	: _auGraph(0) {
}

MidiDriver_CORE::~MidiDriver_CORE() {
	if (_auGraph) {
		AUGraphStop(_auGraph);
		DisposeAUGraph(_auGraph);
		_auGraph = 0;
	}
}

int MidiDriver_CORE::open() {
	OSStatus err = 0;

	if (isOpen())
		return MERR_ALREADY_OPEN;

	// Open the Music Device.
	RequireNoErr(NewAUGraph(&_auGraph));

	AUNode outputNode, synthNode;
	ComponentDescription desc;

	// The default output device
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	RequireNoErr(AUGraphNewNode(_auGraph, &desc, 0, NULL, &outputNode));

	// The built-in default (softsynth) music device
	desc.componentType = kAudioUnitType_MusicDevice;
	desc.componentSubType = kAudioUnitSubType_DLSSynth;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
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
			warning("Failed loading custom sound font '%s' (error %ld)\n", soundfont, (long)err);
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

	return 0;

bail:
	if (_auGraph) {
		AUGraphStop(_auGraph);
		DisposeAUGraph(_auGraph);
		_auGraph = 0;
	}
	return MERR_CANNOT_CONNECT;
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
	assert(isOpen());

	byte status_byte = (b & 0x000000FF);
	byte first_byte = (b & 0x0000FF00) >> 8;
	byte second_byte = (b & 0x00FF0000) >> 16;

	MusicDeviceMIDIEvent(_synth, status_byte, first_byte, second_byte, 0);
}

void MidiDriver_CORE::sysEx(const byte *msg, uint16 length) {
	unsigned char buf[266];

	assert(length + 2 <= ARRAYSIZE(buf));
	assert(isOpen());

	// Add SysEx frame
	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	// Send it
	MusicDeviceSysEx(_synth, buf, length+2);
}


// Plugin interface

class CoreAudioMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "CoreAudio";
	}

	const char *getId() const {
		return "core";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices CoreAudioMusicPlugin::getDevices() const {
	MusicDevices devices;
	// TODO: Return a different music type depending on the configuration
	// TODO: List the available devices
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error CoreAudioMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_CORE();

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(COREAUDIO)
	//REGISTER_PLUGIN_DYNAMIC(COREAUDIO, PLUGIN_TYPE_MUSIC, CoreAudioMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(COREAUDIO, PLUGIN_TYPE_MUSIC, CoreAudioMusicPlugin);
//#endif

#endif // MACOSX
