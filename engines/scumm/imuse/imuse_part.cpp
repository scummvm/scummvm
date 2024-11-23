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



#include "common/debug.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "scumm/scumm.h"
#include "scumm/imuse/imuse_internal.h"

namespace Scumm {

////////////////////////////////////////
//
//  IMuse Part implementation
//
////////////////////////////////////////

Part::Part() {
	_slot = 0;
	_next = nullptr;
	_prev = nullptr;
	_mc = nullptr;
	_player = nullptr;
	_pitchbend = 0;
	_pitchbend_factor = 0;
	_volControlSensitivity = 127;
	_transpose = 0;
	_transpose_eff = 0;
	_vol = 0;
	_vol_eff = 0;
	_detune = 0;
	_detune_eff = 0;
	_pan = 0;
	_pan_eff = 0;
	_polyphony = 0;
	_on = false;
	_modwheel = 0;
	_pedal = false;
	_pri = 0;
	_pri_eff = 0;
	_chan = 0;
	_effect_level = 0;
	_chorus = 0;
	_percussion = 0;
	_bank = 0;
	_unassigned_instrument = false;
	_se = nullptr;
}

void Part::saveLoadWithSerializer(Common::Serializer &ser) {
	int num;
	if (ser.isSaving()) {
		num = (_next ? (_next - _se->_parts + 1) : 0);
		ser.syncAsUint16LE(num);

		num = (_prev ? (_prev - _se->_parts + 1) : 0);
		ser.syncAsUint16LE(num);

		num = (_player ? (_player - _se->_players + 1) : 0);
		ser.syncAsUint16LE(num);
	} else {
		ser.syncAsUint16LE(num);
		_next = (num ? &_se->_parts[num - 1] : nullptr);

		ser.syncAsUint16LE(num);
		_prev = (num ? &_se->_parts[num - 1] : nullptr);

		ser.syncAsUint16LE(num);
		_player = (num ? &_se->_players[num - 1] : nullptr);
	}

	ser.syncAsSint16LE(_pitchbend, VER(8));
	ser.syncAsByte(_pitchbend_factor, VER(8));
	ser.syncAsSByte(_transpose, VER(8));
	ser.syncAsByte(_vol, VER(8));
	ser.syncAsSByte(_detune, VER(8));
	ser.syncAsSByte(_pan, VER(8));
	ser.syncAsByte(_on, VER(8));
	ser.syncAsByte(_modwheel, VER(8));
	ser.syncAsByte(_pedal, VER(8));
	ser.skip(1, VER(8), VER(16)); // _program
	ser.syncAsByte(_pri, VER(8));
	ser.syncAsByte(_chan, VER(8));
	ser.syncAsByte(_effect_level, VER(8));
	ser.syncAsByte(_chorus, VER(8));
	ser.syncAsByte(_percussion, VER(8));
	ser.syncAsByte(_bank, VER(8));
	ser.syncAsByte(_polyphony, VER(116));
	ser.syncAsByte(_volControlSensitivity, VER(116));
}

void Part::set_detune(int8 detune) {
	// Sam&Max does not have detune except for the parameter faders, so the argument
	// here will always be 0 and the only relevant part will be the detune from the player.
	_detune_eff = _se->_newSystem ? _player->getDetune() : clamp((_detune = detune) + _player->getDetune(), -128, 127);
	sendDetune();
}

void Part::pitchBend(int16 value) {
	_pitchbend = value;
	sendPitchBend();
}

void Part::volume(byte value) {
	_vol = value;
	sendVolume(0);
}

void Part::volControlSensitivity(byte value) {
	if (value > 127)
		return;
	_volControlSensitivity = value;
	sendVolume(0);
}

void Part::set_pri(int8 pri) {
	_pri_eff = clamp((_pri = pri) + _player->getPriority(), 0, 255);
	if (_mc)
		_mc->priority(_pri_eff);
}

void Part::set_pan(int8 pan) {
	_pan_eff = clamp((_pan = pan) + _player->getPan(), -64, 63);
	sendPanPosition(_pan_eff + 0x40);
}

void Part::set_polyphony(byte val) {
	if (!_se->_newSystem)
		return;
	_polyphony = val;
	if (_mc)
		_mc->controlChange(17, val);
}

void Part::set_transpose(int8 transpose, int8 clipRangeLow, int8 clipRangeHi)  {
	if (_se->_game_id == GID_TENTACLE && (transpose > 24 || transpose < -24))
		return;

	_transpose = transpose;
	// The Amiga versions have a signed/unsigned bug which makes the check for _transpose == -128 impossible. They actually check for
	// a value of 128 with a signed int8 (a signed int8 can never be 128). The playback depends on this being implemented exactly
	// like in the original driver. I found this bug with the WinUAE debugger. The DOS versions do not have that bug.
	_transpose_eff = (_se->_soundType != MDT_AMIGA && _transpose == -128) ? 0 : transpose_clamp(_transpose + _player->getTranspose(), clipRangeLow, clipRangeHi);
	sendTranspose();
}

void Part::sustain(bool value) {
	_pedal = value;
	if (_mc)
		_mc->sustain(value);
}

void Part::modulationWheel(byte value) {
	_modwheel = value;
	if (_mc)
		_mc->modulationWheel(value);
}

void Part::chorusLevel(byte value) {
	_chorus = value;
	if (_mc)
		_mc->chorusLevel(value);
}

void Part::effectLevel(byte value) {
	_effect_level = value;
	sendEffectLevel(value);
}

void Part::fix_after_load() {
	int lim = (_se->_game_id == GID_TENTACLE || _se->_soundType == MDT_AMIGA|| _se->isNativeMT32()) ? 12 : 24;
	set_transpose(_transpose, -lim, lim);
	volume(_vol);
	set_detune(_detune);
	set_pri(_pri);
	set_pan(_pan);

	if (!_se->_dynamicChanAllocation && !_mc && !_percussion) {
		_mc = _se->allocateChannel(_player->getMidiDriver(), _pri_eff);
		if (!_mc)
			_se->suspendPart(this);
	}

	sendAll();
}

void Part::pitchBendFactor(byte value) {
	if (value > 12)
		return;
	pitchBend(0);
	_pitchbend_factor = value;
	if (_mc)
		_mc->pitchBendFactor(value);
}

void Part::set_onoff(bool on) {
	if (_on != on) {
		_on = on;
		if (!on)
			off();
		if (!_percussion)
			_player->_se->reallocateMidiChannels(_player->getMidiDriver());
	}
}

void Part::set_instrument(byte *data) {
	if (_se->_soundType == MDT_PCSPK)
		_instrument.pcspk(data);
	else
		_instrument.adlib(data);

	if (clearToTransmit())
		_instrument.send(_mc);
}

void Part::load_global_instrument(byte slot) {
	_player->_se->copyGlobalInstrument(slot, &_instrument);
	if (clearToTransmit())
		_instrument.send(_mc);
}

void Part::noteOn(byte note, byte velocity) {
	if (!_on)
		return;

	MidiChannel *mc = _mc;

	// DEBUG
	if (_unassigned_instrument && !_percussion) {
		_unassigned_instrument = false;
		if (!_instrument.isValid()) {
			debug(0, "[%02d] No instrument specified", (int)_chan);
			return;
		}
	}

	if (mc && _instrument.isValid()) {
		mc->noteOn(note, velocity);
	} else if (_percussion) {
		mc = _player->getMidiDriver()->getPercussionChannel();
		if (!mc)
			return;

		if (_vol_eff != _se->_rhyState.vol)
			mc->volume(_vol_eff);

		if (_se->_newSystem) {
			if (_pri_eff != _se->_rhyState.prio)
				mc->priority(_pri_eff);
			if (_polyphony != _se->_rhyState.poly)
				mc->controlChange(17, _polyphony);

		} else if ((note < 35) && (!_player->_se->isNativeMT32())) {
			note = Instrument::_gmRhythmMap[note];
		}

		_se->_rhyState = IMuseInternal::RhyState(_vol_eff, _polyphony, _pri_eff);

		mc->noteOn(note, velocity);
	}
}

void Part::noteOff(byte note) {
	if (!_on)
		return;

	MidiChannel *mc = _mc;
	if (mc) {
		mc->noteOff(note);
	} else if (_percussion) {
		mc = _player->getMidiDriver()->getPercussionChannel();
		if (mc)
			mc->noteOff(note);
	}
}

void Part::init(bool useNativeMT32) {
	_player = nullptr;
	_next = nullptr;
	_prev = nullptr;
	_mc = nullptr;
	_instrument.setNativeMT32Mode(useNativeMT32);
}

void Part::setup(Player *player) {
	_player = player;

	_percussion = (player->isMIDI() && _chan == 9); // true;
	_on = true;
	_pri_eff = player->getPriority();
	_pri = 0;
	_vol = 127;
	_vol_eff = player->getEffectiveVolume();
	_pan = clamp(player->getPan(), -64, 63);
	_transpose_eff = player->getTranspose();
	_transpose = 0;
	_detune = 0;
	_detune_eff = player->getDetune();
	_pitchbend_factor = 2;
	_volControlSensitivity = 127;
	_polyphony = 1;
	_pitchbend = 0;
	_effect_level = player->_se->isNativeMT32() ? 127 : 64;
	_instrument.clear();
	_unassigned_instrument = true;
	_chorus = 0;
	_modwheel = 0;
	_bank = 0;
	_pedal = false;
	_mc = nullptr;
}

void Part::uninit() {
	if (!_player)
		return;
	off();
	_player->removePart(this);
	_se->removeSuspendedPart(this);
	_player = nullptr;
}

void Part::off() {
	if (_mc) {
		_mc->sustain(false);
		_mc->allNotesOff();
		if (!_se->reassignChannelAndResumePart(_mc))
			_mc->release();
		_mc = nullptr;
	}
}

bool Part::clearToTransmit() {
	if (_mc)
		return true;
	if (_instrument.isValid())
		_player->_se->reallocateMidiChannels(_player->getMidiDriver());
	return false;
}

void Part::sendAll() {
	if (!clearToTransmit())
		return;

	_mc->pitchBendFactor(_pitchbend_factor);
	sendTranspose();
	sendDetune();
	sendPitchBend();
	_mc->volume(_vol_eff);
	_mc->sustain(_pedal);
	_mc->modulationWheel(_modwheel);
	sendPanPosition(_pan_eff + 0x40);
	sendPolyphony();

	if (_instrument.isValid())
		_instrument.send(_mc);

	// We need to send the effect level after setting up the instrument
	// otherwise the reverb setting for MT-32 will be overwritten.
	sendEffectLevel(_effect_level);

	_mc->chorusLevel(_chorus);
	_mc->priority(_pri_eff);
}

void Part::sendPitchBend() {
	if (_se->_newSystem && !_pitchbend_factor) {
		sendVolumeFade();
		return;
	}

	if (_mc)
		_mc->pitchBend(_pitchbend);
}

void Part::sendVolume(int8 fadeModifier) {
	uint16 vol = (_vol + fadeModifier + 1) * _player->getEffectiveVolume();

	if (_se->_newSystem)
		vol = (vol * (_volControlSensitivity + 1)) >> 7;

	_vol_eff = vol >> 7;

	if (_mc)
		_mc->volume(_vol_eff);
}

void Part::sendVolumeFade() {
	int16 fadeModifier = ((((_pitchbend >= 0) ? 127 - _vol : _vol) + 1) * _pitchbend) >> 7;
	sendVolume(fadeModifier);
}

void Part::sendTranspose() {
	if (!_mc)
		return;

	_mc->transpose(_transpose_eff);
}

void Part::sendDetune() {
	if (!_mc)
		return;

	_mc->detune(_detune_eff);
}

void Part::programChange(byte value) {
	_bank = 0;
	_instrument.program(value, 0, _player->isMT32());
	if (clearToTransmit())
		_instrument.send(_mc);
}

void Part::set_instrument(uint b) {
	_bank = (byte)(b >> 8);

	// Indy4 and Monkey2 Macintosh versions always use the second bank for sound effects here.
	if (_se->_soundType == MDT_MACINTOSH && (_se->_game_id == GID_MONKEY2 || _se->_game_id == GID_INDY4))
		_bank = 1;

	_instrument.program((byte)b, _bank, _player->isMT32());

	if (clearToTransmit())
		_instrument.send(_mc);
}

void Part::allNotesOff() {
	if (!_mc)
		return;
	_mc->allNotesOff();
}

void Part::sendPanPosition(uint8 value) {
	if (!_mc)
		return;

	// As described in bug report #1849 "MI2: Minor problems in native MT-32 mode"
	// the original iMuse MT-32 driver did revert the panning. So we do the same
	// here in our code to have correctly panned sound output.
	if (_player->_se->isNativeMT32())
		value = 127 - value;

	_mc->panPosition(value);
}

void Part::sendEffectLevel(uint8 value) {
	if (!_mc)
		return;
	_mc->effectLevel(value);
}

void Part::sendPolyphony() {
	if (!_mc || !_se->_newSystem)
		return;
	_mc->controlChange(17, _polyphony);
}

} // End of namespace Scumm
