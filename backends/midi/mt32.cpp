/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * YM2612 tone generation code written by Tomoaki Hayasaka.
 * Used under the terms of the GNU General Public License.
 * Adpated to ScummVM by Jamieson Christian.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "common/scummsys.h"

#ifdef USE_MT32EMU

#include "backends/midi/mt32/mt32emu.h"

#include "backends/midi/emumidi.h"
#include "sound/mpu401.h"

#include "common/util.h"
#include "common/file.h"
#include "common/config-manager.h"

class MidiDriver_MT32 : public MidiDriver_Emulated {
private:
	MidiChannel_MPU401 _midi_channels[16];
	uint16 _channel_mask;
	MT32Emu::Synth *_synth;

	int _outputRate;

protected:
	void generate_samples(int16 *buf, int len);

public:
	MidiDriver_MT32(SoundMixer *mixer);
	virtual ~MidiDriver_MT32();

	int open();
	void close();
	void send(uint32 b);
	void sysEx(byte *msg, uint16 length);

	uint32 property(int prop, uint32 param);
	MidiChannel *allocateChannel();
	MidiChannel *getPercussionChannel();

	// AudioStream API
	bool isStereo() const { return true; }
	int getRate() const { return _outputRate; }
};

typedef File SFile;

class MT32File: public MT32Emu::File {
	SFile file;
public:
	bool open(const char *filename, OpenMode mode) {
		SFile::AccessMode accessMode = mode == OpenMode_read ? SFile::kFileReadMode : SFile::kFileWriteMode;
		return file.open(filename, accessMode);
	}
	void close() {
		return file.close();
	}
	size_t read(void *ptr, size_t size) {
		return file.read(ptr, size);
	}
	bool readLine(char *ptr, size_t size) {
		return file.gets(ptr, size) != NULL;
	}
	size_t write(const void *ptr, size_t size) {
		return file.write(ptr, size);
	}
	int readByte() {
		byte b = file.readByte();
		if (file.eof())
			return -1;
		return b;
	}
	bool writeByte(unsigned char out) {
		file.writeByte(out);
		if (file.ioFailed())
			return false;
		return true;
	}
	bool isEOF() {
		return file.eof();
	}
};

MT32Emu::File *MT32_OpenFile(void *userData, const char *filename, MT32Emu::File::OpenMode mode) {
	MT32File *file = new MT32File();
	if (!file->open(filename, mode)) {
		delete file;
		return NULL;
	}
	return file;
}

////////////////////////////////////////
//
// MidiDriver_MT32
//
////////////////////////////////////////

static void report(void *userData, MT32Emu::ReportType type, void *reportData) {
	switch(type) {
	case MT32Emu::ReportType_lcdMessage:
		g_system->displayMessageOnOSD((char *)reportData);
		break;
	case MT32Emu::ReportType_errorPreset1:
		error("Couldn't open Preset1.syx file");
		break;
	case MT32Emu::ReportType_errorPreset2:
		error("Couldn't open Preset2.syx file");
		break;
	case MT32Emu::ReportType_errorDrumpat:
		error("Couldn't open drumpat.rom file");
		break;
	case MT32Emu::ReportType_errorPatchlog:
		error("Couldn't open patchlog.cfg file");
		break;
	case MT32Emu::ReportType_errorMT32ROM:
		error("Couldn't open MT32_PCM.ROM file");
		break;
	default:
		break;
	}
}

MidiDriver_MT32::MidiDriver_MT32(SoundMixer *mixer) : MidiDriver_Emulated(mixer) {
	_channel_mask = 0xFFFF; // Permit all 16 channels by default
	uint i;
	for (i = 0; i < ARRAYSIZE(_midi_channels); ++i) {
		_midi_channels [i].init (this, i);
	}
	_synth = NULL;

	_baseFreq = 1000;

	_outputRate = 44100;
}

MidiDriver_MT32::~MidiDriver_MT32() {
	if (_synth != NULL)
		delete _synth;
}

static void vdebug(void *data, const char *fmt, va_list list) {
	// do nothing here now
}

int MidiDriver_MT32::open() {
	MT32Emu::SynthProperties prop;

	if (_isOpen)
		return MERR_ALREADY_OPEN;

	MidiDriver_Emulated::open();
	
	memset(&prop, 0, sizeof(prop));
	prop.SampleRate = getRate();
	prop.UseReverb = true;
	prop.UseDefault = false;
	prop.RevType = 0;
	prop.RevTime = 5;
	prop.RevLevel = 3;
	prop.userData = (void *)1;
	prop.printDebug = &vdebug;
	prop.report = &report;
	prop.openFile = MT32_OpenFile;
	_synth = new MT32Emu::Synth();
	if (!_synth->open(prop))
		return MERR_DEVICE_NOT_AVAILABLE;

	_mixer->setupPremix(this);

	return 0;
}

void MidiDriver_MT32::send(uint32 b) {
	_synth->playMsg(b);
}

void MidiDriver_MT32::sysEx(byte *msg, uint16 length) {
	if (msg[0] == 0xf0) {
		_synth->playSysex(msg, length);
	} else {
		_synth->playSysexWithoutFraming(msg, length);
	}
}

void MidiDriver_MT32::close() {
	if (!_isOpen)
		return;
	_isOpen = false;

	// Detach the premix callback handler
	_mixer->setupPremix(0);

	_synth->close();
	delete _synth;
	_synth = NULL;
}

void MidiDriver_MT32::generate_samples(int16 *data, int len) {
	_synth->render(data, len);
}

uint32 MidiDriver_MT32::property (int prop, uint32 param) {
	switch (prop) {
		case PROP_CHANNEL_MASK:
			_channel_mask = param & 0xFFFF;
			return 1;
	}

	return 0;
}

MidiChannel *MidiDriver_MT32::allocateChannel() {
	MidiChannel_MPU401 *chan;
	uint i;

	for (i = 0; i < ARRAYSIZE(_midi_channels); ++i) {
		if (i == 9 || !(_channel_mask & (1 << i)))
			continue;
		chan = &_midi_channels[i];
		if (chan->allocate()) {
			return chan;
		}
	}
	return NULL;
}

MidiChannel *MidiDriver_MT32::getPercussionChannel() {
	return &_midi_channels [9];
}

////////////////////////////////////////
//
// MidiDriver_MT32 factory
//
////////////////////////////////////////

MidiDriver *MidiDriver_MT32_create(SoundMixer *mixer) {
	// HACK: It will stay here until engine plugin loader overhaul
	if (ConfMan.hasKey("extrapath"))                                        
		File::addDefaultDirectory(ConfMan.get("extrapath"));
	return new MidiDriver_MT32(mixer);
}

#endif
