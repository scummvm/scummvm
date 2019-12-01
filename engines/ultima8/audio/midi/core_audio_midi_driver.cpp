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

#include "ultima8/misc/pent_include.h"
#include "CoreAudioMidiDriver.h"
#include "ultima8/conf/setting_manager.h"

#ifdef USE_CORE_AUDIO_MIDI

#include <pthread.h>
#include <sched.h>
#include <iostream>

namespace Ultima8 {

// With the release of Mac OS X 10.5 in October 2007, Apple deprecated the
// AUGraphNewNode & AUGraphGetNodeInfo APIs in favor of the new AUGraphAddNode &
// AUGraphNodeInfo APIs. While it is easy to switch to those, it breaks
// compatibility with all pre-10.5 systems.
//
// Since 10.5 was the last system to support PowerPC, we use the old, deprecated
// APIs on PowerPC based systems by default. On all other systems (such as Mac
// OS X running on Intel hardware, or iOS running on ARM), we use the new API by
// default.
//
// This leaves Mac OS X 10.4 running on x86 processors as the only system
// combination that this code will not support by default. It seems quite
// reasonable to assume that anybody with an Intel system has since then moved
// on to a newer Mac OS X release. But if for some reason you absolutely need to
// build an x86 version of this code using the old, deprecated API, you can
// simply do so by manually enable the USE_DEPRECATED_COREAUDIO_API switch (e.g.
// by adding setting it suitably in CPPFLAGS).
#if !defined(USE_DEPRECATED_COREAUDIO_API)
#   if TARGET_CPU_PPC || TARGET_CPU_PPC64
#      define USE_DEPRECATED_COREAUDIO_API 1
#   else
#      define USE_DEPRECATED_COREAUDIO_API 0
#   endif
#endif


// A macro to simplify error handling a bit.
#define RequireNoErr(error) \
	do { \
		err = error; \
		if (err != noErr) \
			goto bail; \
	} while (false)

const MidiDriver::MidiDriverDesc CoreAudioMidiDriver::desc =
    MidiDriver::MidiDriverDesc("CoreAudio", createInstance);

CoreAudioMidiDriver::CoreAudioMidiDriver() :
	_auGraph(0) {
}

int CoreAudioMidiDriver::open() {
	OSStatus err = noErr;
	//getting the soundfont config entry
	std::string soundfont = getConfigSetting("soundfont", "");

	if (_auGraph)
		return 1;

	// Open the Music Device.
	RequireNoErr(NewAUGraph(&_auGraph));
	AUNode outputNode, synthNode;
	// OS X 10.5 SDK doesn't know AudioComponentDescription desc;
#if USE_DEPRECATED_COREAUDIO_API || (MAC_OS_X_VERSION_MAX_ALLOWED <= 1050)
	ComponentDescription desc;
#else
	AudioComponentDescription desc;
#endif

	// The default output device
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
#if USE_DEPRECATED_COREAUDIO_API
	RequireNoErr(AUGraphNewNode(_auGraph, &desc, 0, NULL, &outputNode));
#else
	RequireNoErr(AUGraphAddNode(_auGraph, &desc, &outputNode));
#endif

	// The built-in default (softsynth) music device
	desc.componentType = kAudioUnitType_MusicDevice;
	desc.componentSubType = kAudioUnitSubType_DLSSynth;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
#if USE_DEPRECATED_COREAUDIO_API
	RequireNoErr(AUGraphNewNode(_auGraph, &desc, 0, NULL, &synthNode));
#else
	RequireNoErr(AUGraphAddNode(_auGraph, &desc, &synthNode));
#endif

	// Connect the softsynth to the default output
	RequireNoErr(AUGraphConnectNodeInput(_auGraph, synthNode, 0, outputNode, 0));

	// Open and initialize the whole graph
	RequireNoErr(AUGraphOpen(_auGraph));
	RequireNoErr(AUGraphInitialize(_auGraph));

	// Get the music device from the graph.
#if USE_DEPRECATED_COREAUDIO_API
	RequireNoErr(AUGraphGetNodeInfo(_auGraph, synthNode, NULL, NULL, NULL, &_synth));
#else
	RequireNoErr(AUGraphNodeInfo(_auGraph, synthNode, NULL, &_synth));
#endif

	// Load custom soundfont, if specified
	if (soundfont != "") {
		pout << "Loading CoreAudio SoundFont '" << soundfont << "'" << std::endl;
		OSErr err;
#if USE_DEPRECATED_COREAUDIO_API
		FSRef   fsref;
		err = FSPathMakeRef(reinterpret_cast<const UInt8 *>(soundfont.c_str()), &fsref, NULL);
		if (!err) {
			err = AudioUnitSetProperty(
			          _synth, kMusicDeviceProperty_SoundBankFSRef,
			          kAudioUnitScope_Global, 0, &fsref, sizeof(fsref));
		}
#else
		// kMusicDeviceProperty_SoundBankFSSpec is present on 10.6+, but broken
		// kMusicDeviceProperty_SoundBankURL was added in 10.5 as a replacement
		CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault,
		               reinterpret_cast<const UInt8 *>(soundfont.c_str()),
		               soundfont.size(), false);
		if (url) {
			err = AudioUnitSetProperty(
			          _synth, kMusicDeviceProperty_SoundBankURL,
			          kAudioUnitScope_Global, 0, &url, sizeof(url));
			CFRelease(url);
		} else {
			pout << "Failed to allocate CFURLRef from '" << soundfont << "'" << std::endl;
		}
#endif
		if (!err) {
			pout << "Loaded CoreAudio SoundFont!" << std::endl;
		} else {
			pout << "Error loading CoreAudio SoundFont '" << soundfont << "'" << std::endl;
			// after trying and failing to load a soundfont it's better
			// to fail initializing the CoreAudio driver or it might crash
			return 1;
		}
	} else {
		pout << "No soundfont in the soundfont config entry!" << std::endl;
		pout << "Continuing with CoreAudio default." << std::endl;
	}

	// Finally: Start the graph!
	RequireNoErr(AUGraphStart(_auGraph));

	return 0;

bail:
	if (_auGraph) {
		AUGraphStop(_auGraph);
		DisposeAUGraph(_auGraph);
		_auGraph = 0;
	}
	return 0;
}

void CoreAudioMidiDriver::close() {
	// Stop the output
	if (_auGraph) {
		AUGraphStop(_auGraph);
		DisposeAUGraph(_auGraph);
		_auGraph = 0;
	}
}

void CoreAudioMidiDriver::send(uint32 message) {
	uint8 status_byte = (message & 0x000000FF);
	uint8 first_byte = (message & 0x0000FF00) >> 8;
	uint8 second_byte = (message & 0x00FF0000) >> 16;

	assert(_auGraph != NULL);
	MusicDeviceMIDIEvent(_synth, status_byte, first_byte, second_byte, 0);
}


void CoreAudioMidiDriver::send_sysex(uint8 status, const uint8 *msg, uint16 length) {
	uint8 buf[384];

	assert(sizeof(buf) >= static_cast<size_t>(length) + 2);
	assert(_auGraph != NULL);

	// Add SysEx frame
	buf[0] = status;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	MusicDeviceSysEx(_synth, buf, length + 2);
}


void CoreAudioMidiDriver::increaseThreadPriority() {
	pthread_t self;
	int policy;
	struct sched_param param;

	self = pthread_self();
	pthread_getschedparam(self, &policy, &param);
	param.sched_priority = sched_get_priority_max(policy);
	pthread_setschedparam(self, policy, &param);
}

void CoreAudioMidiDriver::yield() {
	sched_yield();
}

} // End of namespace Ultima8

#endif
