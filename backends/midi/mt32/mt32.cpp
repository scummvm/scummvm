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

#include "backends/midi/emumidi.h"

#include "common/util.h"
#include "common/file.h"

#include "backends/midi/mt32/synth.h"

class MidiDriver_MT32 : public MidiDriver_Emulated {
private:
	CSynthMT32 *_synth;

	const char *rom_path;

protected:
	void generate_samples(int16 *buf, int len);

public:
	MidiDriver_MT32(SoundMixer *mixer, const char *path);
	virtual ~MidiDriver_MT32();

	int open();
	void close();
	void send(uint32 b);
	uint32 property(int prop, uint32 param) { return 0; }

	void setPitchBendRange(byte channel, uint range) { }
	void sysEx(byte *msg, uint16 length);

	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }


	// AudioStream API
	bool isStereo() const { return true; }
	int getRate() const { return 32000; }
};


////////////////////////////////////////
//
// MidiDriver_MT32
//
////////////////////////////////////////


MidiDriver_MT32::MidiDriver_MT32(SoundMixer *mixer, const char *path)
	: MidiDriver_Emulated(mixer) {
	_synth = new CSynthMT32();
	rom_path = path;
	File::addDefaultDirectory(path);
}

MidiDriver_MT32::~MidiDriver_MT32() {
	delete _synth;
}

int MidiDriver_MT32::open() {
	SynthProperties prop;

	if (_isOpen)
		return MERR_ALREADY_OPEN;
	
	MidiDriver_Emulated::open();
	
	prop.SampleRate = getRate(); // 32000;
	prop.UseReverb = true;
	prop.UseDefault = true;
	//prop.RevType = 0;
	//prop.RevTime = 5;
	//prop.RevLevel = 3;

	_synth->ClassicOpen(rom_path, prop);

	_mixer->setupPremix(this);

	return 0;
}

void MidiDriver_MT32::send(uint32 b) {
	_synth->PlayMsg(b);
}

void MidiDriver_MT32::sysEx(byte *msg, uint16 length) {
	_synth->PlaySysex(msg, length);
}

void MidiDriver_MT32::close() {
	if (!_isOpen)
		return;
	_isOpen = false;

	// Detach the premix callback handler
	_mixer->setupPremix(0);

	_synth->Close();
}

void MidiDriver_MT32::generate_samples(int16 *data, int len) {
	_synth->MT32_CallBack((uint8 *)data, len, _mixer->getMusicVolume());
}


////////////////////////////////////////
//
// MidiDriver_MT32 factory
//
////////////////////////////////////////

MidiDriver *MidiDriver_MT32_create(SoundMixer *mixer, const char *path) {
	return new MidiDriver_MT32(mixer, path);
}
