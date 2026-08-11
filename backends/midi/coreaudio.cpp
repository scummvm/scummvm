/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#ifdef MACOSX

#include "backends/platform/sdl/macosx/macosx-compat.h"

// With the release of Mac OS X 10.5 in October 2007, Apple deprecated the
// AUGraphNewNode & AUGraphGetNodeInfo APIs in favor of the new AUGraphAddNode &
// AUGraphNodeInfo APIs. The newer APIs are used by default, but we do need to
// use the old ones when building for 10.4.
#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
#define COREAUDIO_USE_PRE_LEOPARD_API
#endif

#include "common/config-manager.h"
#include "common/error.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include <CoreServices/CoreServices.h>
#include <AudioToolbox/AUGraph.h>

#ifdef COREAUDIO_USE_PRE_LEOPARD_API
#define AUGraphAddNode(a, b, c)      AUGraphNewNode(a, b, 0, nullptr, c)
#define AUGraphNodeInfo(a, b, c, d)  AUGraphGetNodeInfo(a, b, c, nullptr, nullptr, d)
#endif

#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_6
typedef ComponentDescription AudioComponentDescription;
#endif

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
	int open() override;
	bool isOpen() const override { return _auGraph != nullptr; }
	void close() override;
	void send(uint32 b) override;
	void sysEx(const byte *msg, uint16 length) override;

private:
	void loadSoundFont(const char *soundfont);
	AUGraph _auGraph;
	AudioUnit _synth;
};

MidiDriver_CORE::MidiDriver_CORE()
	: _auGraph(nullptr) {
}

MidiDriver_CORE::~MidiDriver_CORE() {
	if (_auGraph) {
		AUGraphStop(_auGraph);
		DisposeAUGraph(_auGraph);
		_auGraph = nullptr;
	}
}

int MidiDriver_CORE::open() {
	OSStatus err = noErr;

	if (isOpen())
		return MERR_ALREADY_OPEN;

	// Open the Music Device.
	RequireNoErr(NewAUGraph(&_auGraph));

	AUNode outputNode, synthNode;
	AudioComponentDescription desc;

	// The default output device
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
	RequireNoErr(AUGraphAddNode(_auGraph, &desc, &outputNode));

	// The built-in default (softsynth) music device
	desc.componentType = kAudioUnitType_MusicDevice;
	desc.componentSubType = kAudioUnitSubType_DLSSynth;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	RequireNoErr(AUGraphAddNode(_auGraph, &desc, &synthNode));

	// Connect the softsynth to the default output
	RequireNoErr(AUGraphConnectNodeInput(_auGraph, synthNode, 0, outputNode, 0));

	// Open and initialize the whole graph
	RequireNoErr(AUGraphOpen(_auGraph));
	RequireNoErr(AUGraphInitialize(_auGraph));

	// Get the music device from the graph.
	RequireNoErr(AUGraphNodeInfo(_auGraph, synthNode, nullptr, &_synth));

	// Load custom soundfont, if specified
	if (ConfMan.hasKey("soundfont"))
		loadSoundFont(ConfMan.getPath("soundfont").toString(Common::Path::kNativeSeparator).c_str());

#ifdef COREAUDIO_DISABLE_REVERB
	// Disable reverb mode, as that sucks up a lot of CPU power, which can
	// be painful on low end machines.
	// TODO: Make this customizable via a config key?
	{
		UInt32 usesReverb = 0;
		// XXX: iOS might need kAudioUnitProperty_UsesInternalReverb instead?
		AudioUnitSetProperty(_synth, kMusicDeviceProperty_UsesInternalReverb,
			kAudioUnitScope_Global, 0, &usesReverb, sizeof(usesReverb));
	}
#endif

	// Finally: Start the graph!
	RequireNoErr(AUGraphStart(_auGraph));

	return 0;

bail:
	if (_auGraph) {
		AUGraphStop(_auGraph);
		DisposeAUGraph(_auGraph);
		_auGraph = nullptr;
	}
	return MERR_CANNOT_CONNECT;
}

void MidiDriver_CORE::loadSoundFont(const char *soundfont) {
	// TODO: We should really check whether the file contains an
	// actual soundfont...

	OSStatus err = noErr;

#ifdef COREAUDIO_USE_PRE_LEOPARD_API
	FSRef fsref;
	err = FSPathMakeRef((const UInt8 *)soundfont, &fsref, nullptr);

	if (err == noErr) {
		err = AudioUnitSetProperty(
			_synth,
			kMusicDeviceProperty_SoundBankFSRef, kAudioUnitScope_Global,
			0,
			&fsref, sizeof(fsref)
		);
	}
#else
	// kMusicDeviceProperty_SoundBankURL was added in 10.5 as a replacement
	// In addition, the File Manager API became deprecated starting in 10.8
	CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8 *)soundfont, strlen(soundfont), false);

	if (url) {
		err = AudioUnitSetProperty(
			_synth,
			kMusicDeviceProperty_SoundBankURL, kAudioUnitScope_Global,
			0,
			&url, sizeof(url)
		);

		CFRelease(url);
	} else {
		warning("Failed to allocate CFURLRef from '%s'", soundfont);
	}
#endif // COREAUDIO_USE_PRE_LEOPARD_API

	if (err != noErr)
		error("Failed loading custom SoundFont '%s' (error %ld)", soundfont, (long)err);
}

void MidiDriver_CORE::close() {
	MidiDriver_MPU401::close();
	if (_auGraph) {
		AUGraphStop(_auGraph);
		DisposeAUGraph(_auGraph);
		_auGraph = nullptr;
	}
}

void MidiDriver_CORE::send(uint32 b) {
	assert(isOpen());

	midiDriverCommonSend(b);

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
		return "Apple DLS Software Synthesizer";
	}

	const char *getId() const {
		return "core";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices CoreAudioMusicPlugin::getDevices() const {
	MusicDevices devices;
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
