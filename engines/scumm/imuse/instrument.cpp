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

#include "scumm/scumm.h"
#include "scumm/imuse/instrument.h"
#include "audio/mididrv.h"

namespace Scumm {

static struct {
	const char *name;
	byte program;
}

roland_to_gm_map[] = {
	// Monkey Island 2 instruments
	// TODO: Complete
	{ "badspit   ",  62 },
	{ "Big Drum  ", 116 },
	{ "burp      ",  58 },
//	{ "dinkfall  ", ??? },
//	{ "Fire Pit  ", ??? },
	{ "foghorn   ",  60 },
	{ "glop      ",  39 },
//	{ "jacob's la", ??? },
	{ "LeshBass  ",  33 },
//	{ "lowsnort  ", ??? },
	{ "ML explosn", 127 },
	{ "ReggaeBass",  32 },
//	{ "rope fall ", ??? },
	{ "rumble    ",  89 },
	{ "SdTrk Bend",  97 },
//	{ "snort     ", ??? },
	{ "spitting  ",  62 },
	{ "Swell 1   ",  95 },
	{ "Swell 2   ",  95 },
	{ "thnderclap", 127 }

	// Fate of Atlantis instruments
	// TODO: Build
//	{ "*aah!     ", ??? },
//	{ "*ooh!     ", ??? },
//	{ "*ShotFar4 ", ??? },
//	{ "*splash3  ", ??? },
//	{ "*torpedo5 ", ??? },
//	{ "*whip3    ", ??? },
//	{ "*woodknock", ??? },
//	{ "35 lavabub", ??? },
//	{ "49 bzzt!  ", ??? },
//	{ "applause  ", ??? },
//	{ "Arabongo  ", ??? },
//	{ "Big Drum  ", ??? }, // DUPLICATE (todo: confirm)
//	{ "bodythud1 ", ??? },
//	{ "boneKLOK2 ", ??? },
//	{ "boom10    ", ??? },
//	{ "boom11    ", ??? },
//	{ "boom15    ", ??? },
//	{ "boxclik1a ", ??? },
//	{ "brassbonk3", ??? },
//	{ "carstart  ", ??? },
//	{ "cb tpt 2  ", ??? },
//	{ "cell door ", ??? },
//	{ "chains    ", ??? },
//	{ "crash     ", ??? },
//	{ "crsrt/idl3", ??? },
//	{ "Fire Pit  ", ??? }, // DUPLICATE (todo: confirm)
//	{ "Fzooom    ", ??? },
//	{ "Fzooom 2  ", ??? },
//	{ "ghostwhosh", ??? },
//	{ "glasssmash", ??? },
//	{ "gloop2    ", ??? },
//	{ "gunShotNea", ??? },
//	{ "idoorclse ", ??? },
//	{ "knife     ", ??? },
//	{ "lavacmbl4 ", ??? },
//	{ "Mellow Str", ??? },
//	{ "mtlheater1", ??? },
//	{ "pachinko5 ", ??? },
//	{ "Ping1     ", ??? },
//	{ "rockcrunch", ??? },
//	{ "rumble    ", ??? }, // DUPLICATE (todo: confirm)
//	{ "runngwatr ", ??? },
//	{ "scrape2   ", ??? },
//	{ "snakeHiss ", ??? },
//	{ "snort     ", ??? }, // DUPLICATE (todo: confirm)
//	{ "spindle4  ", ??? },
//	{ "splash2   ", ??? },
//	{ "squirel   ", ??? },
//	{ "steam3    ", ??? },
//	{ "stonwheel6", ??? },
//	{ "street    ", ??? },
//	{ "trickle4  ", ??? }
};

// This emulates the percussion bank setup LEC used with the MT-32,
// where notes 24 - 34 were assigned instruments without reverb.
// It also fixes problems on GS devices that map sounds to these
// notes by default.
const byte Instrument::_gmRhythmMap[35] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0, 36, 37, 38, 39, 40, 41, 66, 47,
	65, 48, 56
};

class Instrument_Program : public InstrumentInternal {
private:
	byte _program;
	byte _bank;
	bool _soundTypeMT32;
	bool _nativeMT32Device;

public:
	Instrument_Program(byte program, byte bank, bool soundTypeMT32, bool nativeMT32Device);
	Instrument_Program(Common::Serializer &s, bool nativeMT32Device);
	void saveLoadWithSerializer(Common::Serializer &s) override;
	void send(MidiChannel *mc) override;
	void copy_to(Instrument *dest) override { dest->program(_program, _bank, _soundTypeMT32); }
	bool is_valid() override {
		return (_program < 128) &&
		       ((_nativeMT32Device == _soundTypeMT32) || (_nativeMT32Device
		        ? (MidiDriver::_gmToMt32[_program] < 128)
		        : (MidiDriver::_mt32ToGm[_program] < 128)));
	}
};

class Instrument_AdLib : public InstrumentInternal {
private:

#include "common/pack-start.h"  // START STRUCT PACKING

	struct AdLibInstrument {
		byte flags_1;
		byte oplvl_1;
		byte atdec_1;
		byte sustrel_1;
		byte waveform_1;
		byte flags_2;
		byte oplvl_2;
		byte atdec_2;
		byte sustrel_2;
		byte waveform_2;
		byte feedback;
		byte flags_a;
		struct {
			byte a, b, c, d, e, f, g, h;
		} extra_a;
		byte flags_b;
		struct {
			byte a, b, c, d, e, f, g, h;
		} extra_b;
		byte duration;
	} PACKED_STRUCT;

#include "common/pack-end.h"    // END STRUCT PACKING

	AdLibInstrument _instrument;

public:
	Instrument_AdLib(const byte *data);
	Instrument_AdLib(Common::Serializer &s);
	void saveLoadWithSerializer(Common::Serializer &s) override;
	void send(MidiChannel *mc) override;
	void copy_to(Instrument *dest) override { dest->adlib((byte *)&_instrument); }
	bool is_valid() override { return true; }
};

class Instrument_Roland : public InstrumentInternal {
private:

#include "common/pack-start.h"  // START STRUCT PACKING

	struct RolandInstrument {
		byte roland_id;
		byte device_id;
		byte model_id;
		byte command;
		byte address[3];
		struct {
			byte name[10];
			byte partial_struct12;
			byte partial_struct34;
			byte partial_mute;
			byte env_mode;
		} common;
		struct {
			byte wg_pitch_coarse;
			byte wg_pitch_fine;
			byte wg_pitch_keyfollow;
			byte wg_pitch_bender_sw;
			byte wg_waveform_pcm_bank;
			byte wg_pcm_wave_num;
			byte wg_pulse_width;
			byte wg_pw_velo_sens;
			byte p_env_depth;
			byte p_evn_velo_sens;
			byte p_env_time_keyf;
			byte p_env_time[4];
			byte p_env_level[3];
			byte p_env_sustain_level;
			byte end_level;
			byte p_lfo_rate;
			byte p_lfo_depth;
			byte p_lfo_mod_sens;
			byte tvf_cutoff_freq;
			byte tvf_resonance;
			byte tvf_keyfollow;
			byte tvf_bias_point_dir;
			byte tvf_bias_level;
			byte tvf_env_depth;
			byte tvf_env_velo_sens;
			byte tvf_env_depth_keyf;
			byte tvf_env_time_keyf;
			byte tvf_env_time[5];
			byte tvf_env_level[3];
			byte tvf_env_sustain_level;
			byte tva_level;
			byte tva_velo_sens;
			byte tva_bias_point_1;
			byte tva_bias_level_1;
			byte tva_bias_point_2;
			byte tva_bias_level_2;
			byte tva_env_time_keyf;
			byte tva_env_time_v_follow;
			byte tva_env_time[5];
			byte tva_env_level[3];
			byte tva_env_sustain_level;
		} partial[4];
		byte checksum;
	} PACKED_STRUCT;

#include "common/pack-end.h"    // END STRUCT PACKING

	RolandInstrument _instrument;

	char _instrument_name[11];
	bool _nativeMT32Device;

	uint8 getEquivalentGM();

public:
	Instrument_Roland(const byte *data, bool nativeMT32Device);
	Instrument_Roland(Common::Serializer &s, bool nativeMT32Device);
	void saveLoadWithSerializer(Common::Serializer &s) override;
	void send(MidiChannel *mc) override;
	void copy_to(Instrument *dest) override { dest->roland((byte *)&_instrument); }
	bool is_valid() override { return (_nativeMT32Device ? true : (_instrument_name[0] != '\0')); }
};

class Instrument_PcSpk : public InstrumentInternal {
public:
	Instrument_PcSpk(const byte *data);
	Instrument_PcSpk(Common::Serializer &s);
	void saveLoadWithSerializer(Common::Serializer &s) override;
	void send(MidiChannel *mc) override;
	void copy_to(Instrument *dest) override { dest->pcspk((byte *)&_instrument); }
	bool is_valid() override { return true; }

private:
	byte _instrument[23];
};

////////////////////////////////////////
//
// Instrument class members
//
////////////////////////////////////////

void Instrument::clear() {
	delete _instrument;
	_instrument = nullptr;
	_type = itNone;
}

void Instrument::program(byte prog, byte bank, bool mt32SoundType) {
	clear();
	if (prog > 127)
		return;
	_type = itProgram;
	_instrument = new Instrument_Program(prog, bank, mt32SoundType, _nativeMT32Device);
}

void Instrument::adlib(const byte *instrument) {
	clear();
	if (!instrument)
		return;
	_type = itAdLib;
	_instrument = new Instrument_AdLib(instrument);
}

void Instrument::roland(const byte *instrument) {
	clear();
	if (!instrument)
		return;
	_type = itRoland;
	_instrument = new Instrument_Roland(instrument, _nativeMT32Device);
}

void Instrument::pcspk(const byte *instrument) {
	clear();
	if (!instrument)
		return;
	_type = itPcSpk;
	_instrument = new Instrument_PcSpk(instrument);
}

void Instrument::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.isSaving()) {
		s.syncAsByte(_type);
		if (_instrument)
			_instrument->saveLoadWithSerializer(s);
	} else {
		clear();
		s.syncAsByte(_type);
		switch (_type) {
		case itNone:
			break;
		case itProgram:
			_instrument = new Instrument_Program(s, _nativeMT32Device);
			break;
		case itAdLib:
			_instrument = new Instrument_AdLib(s);
			break;
		case itRoland:
			_instrument = new Instrument_Roland(s, _nativeMT32Device);
			break;
		case itPcSpk:
			_instrument = new Instrument_PcSpk(s);
			break;
		case itMacDeprecated: {
			byte prog = 255;
			s.syncAsByte(prog);
			_instrument = new Instrument_Program(prog, 1, false, false);
			} break;
		default:
			warning("No known instrument classification #%d", (int)_type);
			_type = itNone;
		}
	}
}

////////////////////////////////////////
//
// Instrument_Program class members
//
////////////////////////////////////////

Instrument_Program::Instrument_Program(byte program, byte bank, bool soundTypeMT32, bool nativeMT32Device) :
	_program(program),
	_bank(bank),
	_soundTypeMT32(soundTypeMT32),
	_nativeMT32Device(nativeMT32Device) {
	if (program > 127)
		_program = 255;
}

Instrument_Program::Instrument_Program(Common::Serializer &s, bool nativeMT32Device) :
	_nativeMT32Device(nativeMT32Device) {
	_program = 255;
	_bank = 0;
	_soundTypeMT32 = false;
	if (!s.isSaving())
		saveLoadWithSerializer(s);
}

void Instrument_Program::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsByte(_program);
	s.syncAsByte(_bank, VER(123));
	if (s.isSaving()) {
		s.syncAsByte(_soundTypeMT32);
		s.syncAsByte(_nativeMT32Device);
	} else {
		byte tmp;
		s.syncAsByte(tmp);
		_soundTypeMT32 = (tmp > 0);
		s.syncAsByte(tmp, VER(122));
		_nativeMT32Device = (tmp > 0);
	}
}

void Instrument_Program::send(MidiChannel *mc) {
	if (_program > 127)
		return;

	byte program = _program;
	if (!_nativeMT32Device && _soundTypeMT32)
		program =  MidiDriver::_mt32ToGm[program];

	if (_bank)
		mc->bankSelect(_bank);
	if (program < 128)
		mc->programChange(program);
	if (_bank)
		mc->bankSelect(0);
}

////////////////////////////////////////
//
// Instrument_AdLib class members
//
////////////////////////////////////////

Instrument_AdLib::Instrument_AdLib(const byte *data) {
	memcpy(&_instrument, data, sizeof(_instrument));
}

Instrument_AdLib::Instrument_AdLib(Common::Serializer &s) {
	if (!s.isSaving())
		saveLoadWithSerializer(s);
	else
		memset(&_instrument, 0, sizeof(_instrument));
}

void Instrument_AdLib::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncBytes((byte *)(&_instrument), sizeof(_instrument));
}

void Instrument_AdLib::send(MidiChannel *mc) {
	mc->sysEx_customInstrument('ADL ', (byte *)&_instrument, sizeof(_instrument));
}

////////////////////////////////////////
//
// Instrument_Roland class members
//
////////////////////////////////////////

Instrument_Roland::Instrument_Roland(const byte *data, bool nativeMT32Device) : _nativeMT32Device(nativeMT32Device) {
	memcpy(&_instrument, data, sizeof(_instrument));
	memcpy(&_instrument_name, &_instrument.common.name, sizeof(_instrument.common.name));
	_instrument_name[10] = '\0';
	if (!_nativeMT32Device && getEquivalentGM() >= 128) {
		debug(0, "MT-32 instrument \"%s\" not supported yet", _instrument_name);
		_instrument_name[0] = '\0';
	}
}

Instrument_Roland::Instrument_Roland(Common::Serializer &s, bool nativeMT32Device) : _nativeMT32Device(nativeMT32Device) {
	_instrument_name[0] = '\0';
	if (!s.isSaving())
		saveLoadWithSerializer(s);
	else
		memset(&_instrument, 0, sizeof(_instrument));
}

void Instrument_Roland::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncBytes((byte *)(&_instrument), sizeof(_instrument));
	if (!s.isSaving()) {
		memcpy(&_instrument_name, &_instrument.common.name, sizeof(_instrument.common.name));
		_instrument_name[10] = '\0';
		if (!_nativeMT32Device && getEquivalentGM() >= 128) {
			debug(2, "MT-32 custom instrument \"%s\" not supported", _instrument_name);
			_instrument_name[0] = '\0';
		}
	}
}

void Instrument_Roland::send(MidiChannel *mc) {
	if (_nativeMT32Device) {
		mc->sysEx_customInstrument('ROL ', (byte *)&_instrument, sizeof(_instrument));
	} else {
		// Convert to a GM program change.
		byte program = getEquivalentGM();
		if (program < 128)
			mc->programChange(program);
	}
}

uint8 Instrument_Roland::getEquivalentGM() {
	byte i;
	for (i = 0; i != ARRAYSIZE(roland_to_gm_map); ++i) {
		if (!memcmp(roland_to_gm_map[i].name, _instrument.common.name, 10))
			return roland_to_gm_map[i].program;
	}
	return 255;
}

////////////////////////////////////////
//
// Instrument_PcSpk class members
//
////////////////////////////////////////

Instrument_PcSpk::Instrument_PcSpk(const byte *data) {
	memcpy(_instrument, data, sizeof(_instrument));
}

Instrument_PcSpk::Instrument_PcSpk(Common::Serializer &s) {
	if (!s.isSaving())
		saveLoadWithSerializer(s);
	else
		memset(_instrument, 0, sizeof(_instrument));
}

void Instrument_PcSpk::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncBytes(_instrument, sizeof(_instrument));
}

void Instrument_PcSpk::send(MidiChannel *mc) {
	mc->sysEx_customInstrument('SPK ', (byte *)&_instrument, sizeof(_instrument));
}

} // End of namespace Scumm
