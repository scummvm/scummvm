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
 * $URL$
 * $Id$
 */

#include "common/scummsys.h"

#if defined(__amigaos4__)

#include "sound/mpu401.h"
#include "common/util.h"
#include "common/endian.h"

#include <proto/camd.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>

/*
 * CAMD sequencer driver
 * Mostly cut'n'pasted from FreeSCI by Christoph Reichenbach
 */

class MidiDriver_CAMD : public MidiDriver_MPU401 {
public:
	MidiDriver_CAMD();
	int open();
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	bool _isOpen;
	struct Library *_CamdBase;
	struct CamdIFace *_ICamd;
	struct MidiLink *_midi_link;
};

MidiDriver_CAMD::MidiDriver_CAMD()
	: _isOpen(false), _CamdBase(NULL), _ICamd(NULL), _midi_link(NULL) {
}

int MidiDriver_CAMD::open() {
	const char *devicename = "via686.out.0";

	if (_isOpen)
		return MERR_ALREADY_OPEN;

	_CamdBase = IExec->OpenLibrary("camd.library", 36L);
	if (!_CamdBase) {
		error("Could not open 'camd.library'");
		return -1;
	}

	_ICamd = (struct CamdIFace *) IExec->GetInterface(_CamdBase, "main", 1, NULL);
	if (!_ICamd) {
		IExec->CloseLibrary(_CamdBase);
		error("Error while retrieving CAMD interface");
		return -1;
	}

	struct MidiNode *midi_node;
	midi_node = _ICamd->CreateMidi(MIDI_MsgQueue, 0L, MIDI_SysExSize, 4096L, MIDI_Name, "scummvm", TAG_END);
	if (!midi_node) {
		IExec->CloseLibrary(_CamdBase);
		error("Could not create CAMD MIDI node");
		return -1;
	}

	_midi_link = _ICamd->AddMidiLink(midi_node, MLTYPE_Sender, MLINK_Location, devicename, TAG_END);
	if (!_midi_link) {
		IExec->CloseLibrary(_CamdBase);
		error("Could not create CAMD MIDI link to '%s'", devicename);
		return -1;
	}

	_isOpen = true;
	return 0;
}

void MidiDriver_CAMD::close() {
	_isOpen = false;
	MidiDriver_MPU401::close();

	if (_CamdBase)
		IExec->CloseLibrary(_CamdBase);
}

void MidiDriver_CAMD::send(uint32 b) {
	ULONG data = READ_LE_UINT32(&b);
	_ICamd->PutMidi(_midi_link, data);
}

void MidiDriver_CAMD::sysEx(const byte *msg, uint16 length) {
	unsigned char buf[256];

	assert(length + 2 <= 256);

	// Add SysEx frame
	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	// Send it
	_ICamd->PutSysEx(_midi_link, buf);
}

MidiDriver *MidiDriver_CAMD_create() {
	return new MidiDriver_CAMD();
}

#endif
