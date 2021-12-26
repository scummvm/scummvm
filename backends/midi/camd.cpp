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

#if defined(__amigaos4__) || defined(__MORPHOS__)

#include "common/textconsole.h"
#include "common/error.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/str.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include <proto/camd.h>
#include <proto/exec.h>
#include <proto/dos.h>

/*
 * CAMD sequencer driver
 * Mostly cut'n'pasted from FreeSCI by Christoph Reichenbach
 */

class MidiDriver_CAMD : public MidiDriver_MPU401 {
public:
	MidiDriver_CAMD();
	int open();
	bool isOpen() const { return _isOpen; }
	void close();
	void send(uint32 b) override;
	void sysEx(const byte *msg, uint16 length);

private:
	bool _isOpen;
	struct Library *_CamdBase;
#if defined(__amigaos4__)
	struct CamdIFace *_ICamd;
#endif
	struct MidiNode *_midi_node;
	struct MidiLink *_midi_link;
	char _outport[128];

	char *getDevice();
	void closeAll();
};

MidiDriver_CAMD::MidiDriver_CAMD()
#if defined(__amigaos4__)
	: _isOpen(false), _CamdBase(NULL), _ICamd(NULL), _midi_link(NULL) {
#else
	: _isOpen(false), _CamdBase(NULL), _midi_link(NULL) {
#endif
}

int MidiDriver_CAMD::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

#if defined(__amigaos4__)
	_CamdBase = IExec->OpenLibrary("camd.library", 36L);
#else
	_CamdBase = OpenLibrary("camd.library", 36L);
#endif
	if (!_CamdBase) {
		error("Could not open 'camd.library'");
		return -1;
	}

#if defined(__amigaos4__)
	_ICamd = (struct CamdIFace *) IExec->GetInterface(_CamdBase, "main", 1, NULL);
	if (!_ICamd) {
		closeAll();
		error("Error while retrieving CAMD interface");
		return -1;
	}
#endif
#if defined(__amigaos4__)
	_midi_node = _ICamd->CreateMidi(MIDI_MsgQueue, 0L, MIDI_SysExSize, 4096L, MIDI_Name, "scummvm", TAG_END);
#else
	TagItem tags[] = { MIDI_MsgQueue, 0L, MIDI_SysExSize, 4096L, MIDI_Name, (ULONG)"scummvm", TAG_END, 0};
	_midi_node = CreateMidiA(tags);
#endif
	if (!_midi_node) {
		closeAll();
		error("Could not create CAMD MIDI node");
		return -1;
	}

	char *devicename = getDevice();
	if (!devicename) {
		closeAll();
		error("Could not find an output device");
		return MERR_DEVICE_NOT_AVAILABLE;
	}

#if defined(__amigaos4__)
	_midi_link = _ICamd->AddMidiLink(_midi_node, MLTYPE_Sender, MLINK_Location, devicename, TAG_END);
#else
	TagItem tagsLink[] = { MLINK_Location, (ULONG)devicename, TAG_END, 0};
	_midi_link = AddMidiLinkA(_midi_node, MLTYPE_Sender, tagsLink);
#endif
	if (!_midi_link) {
		closeAll();
		error("Could not create CAMD MIDI link to '%s'", devicename);
		return MERR_CANNOT_CONNECT;
	}

	_isOpen = true;
	return 0;
}

void MidiDriver_CAMD::close() {
	MidiDriver_MPU401::close();
	closeAll();
}

void MidiDriver_CAMD::send(uint32 b) {
	if (!_isOpen) {
		warning("MidiDriver_CAMD: Got event while not open");
		return;
	}

	midiDriverCommonSend(b);

	ULONG data = READ_LE_UINT32(&b);
	#if defined(__amigaos4__)
	_ICamd->PutMidi(_midi_link, data);
	#else
	PutMidi(_midi_link, data);
	#endif
}

void MidiDriver_CAMD::sysEx(const byte *msg, uint16 length) {
	if (!_isOpen) {
		warning("MidiDriver_CAMD: Got SysEx while not open");
		return;
	}

	unsigned char buf[266];

	assert(length + 2 <= ARRAYSIZE(buf));

	// Add SysEx frame
	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	// Send it
#if defined(__amigaos4__)
	_ICamd->PutSysEx(_midi_link, buf);
#else
	PutSysEx(_midi_link, buf);
#endif
}

char *MidiDriver_CAMD::getDevice() {
	char *retname = NULL;
#if defined(__amigaos4__)
	APTR key = _ICamd->LockCAMD(CD_Linkages);
#else
	APTR key = LockCAMD(CD_Linkages);
#endif
	if (key != NULL) {
#if defined(__amigaos4__)
		struct MidiCluster *cluster = _ICamd->NextCluster(NULL);
#else
		struct MidiCluster *cluster = NextCluster(NULL);
#endif
		while (cluster && !retname) {
			// Get the current cluster name
			char *dev = cluster->mcl_Node.ln_Name;

			if (strstr(dev, "out") != NULL) {
				// This is an output device, return this
				Common::strlcpy(_outport, dev, sizeof(_outport));
				retname = _outport;
			} else {
				// Search the next one
#if defined(__amigaos4__)
				cluster = _ICamd->NextCluster(cluster);
#else
				cluster = NextCluster(cluster);
#endif
			}
		}

		// If the user has a preference outport set, use this instead
#if defined(__amigaos4__)
		if(IDOS->GetVar("DefMidiOut", _outport, 128, 0))
#else
		if (GetVar("DefMidiOut", _outport, 128, 0))
#endif
			retname = _outport;

#if defined(__amigaos4__)
		_ICamd->UnlockCAMD(key);
#else
		UnlockCAMD(key);
#endif
	}

	return retname;
}

void MidiDriver_CAMD::closeAll() {
	if (_CamdBase) {

#if defined(__amigaos4__)
		if (_ICamd)
			IExec->DropInterface((struct Interface *)_ICamd);

		IExec->CloseLibrary(_CamdBase);
		_CamdBase = NULL;
#else

		FlushMidi(_midi_node);
		if (_midi_link) {
			RemoveMidiLink(_midi_link);
			_midi_link = NULL;
		}
		if (_midi_node) {
			DeleteMidi(_midi_node);
			_midi_node = NULL;
		}

		CloseLibrary(_CamdBase);
		_CamdBase = NULL;
#endif
	}
	_isOpen = false;
}


// Plugin interface

class CamdMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "CAMD";
	}

	const char *getId() const {
		return "camd";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices CamdMusicPlugin::getDevices() const {
	MusicDevices devices;
	// TODO: Return a different music type depending on the configuration
	// TODO: List the available devices
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error CamdMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_CAMD();

	return Common::kNoError;
}

REGISTER_PLUGIN_STATIC(CAMD, PLUGIN_TYPE_MUSIC, CamdMusicPlugin);

#endif
