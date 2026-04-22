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

#include "common/endian.h"
#include "common/md5.h"
#include "common/textconsole.h"
#include "mads/madsv2/phantom/sound_phantom.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

void PhantomSoundManager::validate() {
	Common::File f;
	static const char *const MD5[] = {
		"8edcb79a8c3514eac0835496326a72ae",
		"4b81a46440f8404d9eda1ce5ae2c5579",
		"11d8d441e47ad1ccd8faafd6572a17d0",
		"4cd5c4d45126e60ca701690489ab8afa",
		"588357d711bbcdabdf7d7e5d96013ce5",
		nullptr,
		nullptr,
		nullptr,
		"3d4843074c1dcbfd7919179c58aec9bc"
	};

	for (int i = 1; i <= 9; ++i) {
		if (i >= 6 && i <= 8)
			continue;
		Common::Path filename(Common::String::format("asound.ph%d", i));
		if (!f.open(filename))
			error("Could not process - %s", filename.toString().c_str());
		Common::String md5str = Common::computeStreamMD5AsString(f, 8192);
		f.close();

		if (md5str != MD5[i - 1])
			error("Invalid sound file - %s", filename.toString().c_str());
	}
}

void PhantomSoundManager::loadDriver(int sectionNumber) {
	switch (sectionNumber) {
	case 1:
		_driver = new ASound1(_mixer, _opl);
		break;
	case 2:
		_driver = new ASound2(_mixer, _opl);
		break;
	case 3:
		_driver = new ASound3(_mixer, _opl);
		break;
	case 4:
		_driver = new ASound4(_mixer, _opl);
		break;
	case 5:
		_driver = new ASound5(_mixer, _opl);
		break;
	case 9:
		_driver = new ASound9(_mixer, _opl);
		break;
	default:
		_driver = nullptr;
		return;
	}
}

/*-----------------------------------------------------------------------*/

PhantomASound::PhantomASound(Audio::Mixer *mixer, OPL::OPL *opl, const Common::Path &filename, int dataOffset) :
		ASound(mixer, opl, filename, dataOffset) {
	_chanCommandCount = 66;
	memset(_scratchArr, 0, sizeof(_scratchArr));

	// Load sound samples - all the asound drivers have an identical set of 120
	// samples starting at offset 1d4h in their data segment
	_soundFile.seek(_dataOffset + 0x1d4);
	for (int i = 0; i < 120; ++i)
		_samples.push_back(AdlibSample(_soundFile));
}

void PhantomASound::channelCommand(byte *&pSrc, bool &updateFlag) {
	AdlibChannel *chan = _activeChannelPtr;
	int cmdNum = (int8)*pSrc;   // -1 .. -66

	switch (cmdNum) {

	case -1:
		// Single-level repeat loop (uses _field17 / _ptr3)
		if (!chan->_field17) {
			if (pSrc[1] == 0) {
				chan->_pSrc += 2;
				chan->_ptr3 = chan->_pSrc;
				chan->_field17 = 0;
			} else {
				chan->_field17 = (int8)pSrc[1];
				chan->_pSrc = chan->_ptr3;
			}
		} else {
			--chan->_field17;
			if (!chan->_field17) {
				chan->_pSrc += 2;
				chan->_ptr3 = chan->_pSrc;
			} else {
				chan->_pSrc = chan->_ptr3;
			}
		}
		break;

	case -2:
		// Double-level repeat loop (uses _field19/_ptr4 outer, _field17/_ptr3 inner)
		if (!chan->_field19) {
			if (pSrc[1] == 0) {
				chan->_pSrc += 2;
				chan->_ptr4 = chan->_pSrc;
				chan->_ptr3 = chan->_pSrc;
				chan->_field17 = 0;
				chan->_field19 = 0;
			} else {
				chan->_field19 = (int8)pSrc[1];
				chan->_pSrc = chan->_ptr4;
				chan->_ptr3 = chan->_ptr4;
			}
		} else {
			--chan->_field19;
			if (!chan->_field19) {
				chan->_pSrc += 2;
				chan->_ptr4 = chan->_pSrc;
				chan->_ptr3 = chan->_pSrc;
			} else {
				chan->_pSrc = chan->_ptr4;
				chan->_ptr3 = chan->_ptr4;
			}
		}
		break;

	case -3:
		// Reset channel to soundData start
		chan->_fieldE = 0;      // original stored soundData near ptr here
		chan->_pSrc     = chan->_soundData;
		chan->_ptr3     = chan->_soundData;
		chan->_ptr4     = chan->_soundData;
		chan->_field1   = 0;
		chan->_field2   = 0;
		chan->_field3   = 0;
		chan->_field26  = 0;
		chan->_volumeOffset = 0;
		chan->_field28  = 0;
		chan->_volume   = 0;
		chan->_field9   = 0;
		chan->_fieldB   = 0;
		chan->_field17  = 0;
		chan->_field19  = 0;
		chan->_field7   = 0;
		break;

	case -4: {
		// Jump: set all four data pointers from a near-pointer word argument
		++pSrc;
		int nearPtr = (int16)READ_LE_UINT16(pSrc);
		byte *p = getDataPtr(nearPtr);
		chan->_fieldE    = nearPtr;
		chan->_pSrc      = p;
		chan->_ptr3      = p;
		chan->_ptr4      = p;
		chan->_soundData = p;
		break;
	}

	case -5: {
		// Unconditional jump to near-pointer target (no return address saved)
		++pSrc;
		int nearPtr = (int16)READ_LE_UINT16(pSrc);
		chan->_pSrc = getDataPtr(nearPtr);
		break;
	}

	case -6: {
		// Unconditional call to near-pointer target (saves return address in _ptrEnd)
		++pSrc;
		int nearPtr = (int16)READ_LE_UINT16(pSrc);
		chan->_ptrEnd = chan->_pSrc + 3;
		chan->_pSrc   = getDataPtr(nearPtr);
		break;
	}

	case -7:
		// Return from call (restores _pSrc from _ptrEnd)
		if (chan->_ptrEnd) {
			chan->_pSrc   = chan->_ptrEnd;
			chan->_ptrEnd = nullptr;
		} else {
			chan->_pSrc += 1;
		}
		break;

	case -8:
		// Load sample: byte argument is sample index
		chan->_sampleIndex = pSrc[1];
		chan->_pSrc += 2;
		loadSample(chan->_sampleIndex);
		break;

	case -9:
		// Set _field7 (pitch source), clear _field2C (frequency counter)
		chan->_field7  = pSrc[1];
		chan->_field2C = 0;
		chan->_pSrc += 2;
		break;

	case -10:
		// Set _field2C (frequency counter), clear _field7
		chan->_field2C = pSrc[1];
		chan->_field7  = 0;
		chan->_pSrc += 2;
		break;

	case -11:
		// Set _field1 (envelope / modulation)
		chan->_field1 = pSrc[1];
		chan->_pSrc += 2;
		break;

	case -12:
		// Set volume, capped downward if _field2B (mute flag) is active
		{
			int val = (int8)pSrc[1];
			if (chan->_field2B) {
				if (chan->_volume > val)
					chan->_volume = val;
			} else {
				chan->_volume = val;
			}
			updateFlag = true;
			chan->_pSrc += 2;
		}
		break;

	case -13:
		// Set _fieldA and _field2 (vibrato params), or skip if muted
		if (chan->_field2B) {
			chan->_pSrc += 3;
		} else {
			chan->_fieldA = pSrc[1];
			chan->_field2 = pSrc[2];
			chan->_field9 = 1;
			chan->_pSrc += 3;
		}
		break;

	case -14:
		// Set _field26 (pitch delta)
		chan->_field26 = pSrc[1];
		chan->_pSrc += 2;
		break;

	case -15:
		// Set _volumeOffset, capped downward if _field2B (mute flag) is active
		{
			int val = (int8)pSrc[1];
			if (chan->_field2B) {
				if ((int8)chan->_volumeOffset > val)
					chan->_volumeOffset = val;
			} else {
				chan->_volumeOffset = val;
			}
			updateFlag = true;
			chan->_pSrc += 2;
		}
		break;

	case -16:
		// Set _fieldD (volume increment step)
		chan->_fieldD = pSrc[1];
		updateFlag = true;
		chan->_pSrc += 2;
		break;

	case -17:
		// Set _fieldC (period) and _field3 (volume slide), enable slide (_fieldB=1)
		chan->_fieldC = pSrc[1];
		chan->_field3 = pSrc[2];
		chan->_fieldB = 1;
		chan->_pSrc += 3;
		break;

	case -18:
		// Set _field2A
		chan->_field2A = pSrc[1];
		chan->_pSrc += 2;
		break;

	case -19:
		// Relative skip: advance by (signed byte arg) + 3
		chan->_pSrc += (int8)pSrc[1] + 3;
		break;

	case -20: {
		// Pick random element from array[0..N-1], write chosen value to a
		// self-modifying offset within the sound data
		int n    = (byte)pSrc[1];
		byte *base = pSrc + 2;
		int rnd  = getRandomNumber() & 0x7FFF;
		int idx  = rnd % n;
		byte chosen = base[idx];
		int destIdx = (int8)base[n];
		base[destIdx + n + 1] = chosen;
		chan->_pSrc += n + 3;
		break;
	}

	case -21: {
		// Pick random value in [lower..upper], write to a self-modifying offset
		int lower = (int8)pSrc[1];
		int upper = (int8)pSrc[2];
		int range = upper - lower + 1;
		int rnd   = getRandomNumber() & 0x7FFF;
		byte *base = pSrc + 3;
		int destIdx = (int8)base[0];
		base[destIdx + 1] = (byte)(rnd % range + lower);
		chan->_pSrc += 4;
		break;
	}

	case -22: {
		// Like -20 but array index is read from _scratchArr[regIdx]
		int regIdx = (byte)pSrc[1];
		int n      = (byte)pSrc[2];
		byte *base = pSrc + 3;
		int arrVal = _scratchArr[regIdx];
		byte chosen = base[arrVal];
		int destOffset = (int8)base[n];
		base[destOffset + n + 1] = chosen;
		chan->_pSrc += n + 4;
		break;
	}

	case -23:
		// arr[dest] = literal
		_scratchArr[(byte)pSrc[1]] = (byte)pSrc[2];
		chan->_pSrc += 3;
		break;

	case -24:
		// arr[dest] = arr[src]
		_scratchArr[(byte)pSrc[1]] = _scratchArr[(byte)pSrc[2]];
		chan->_pSrc += 3;
		break;

	case -25:
		// Write arr[regIdx] into sound data at pSrc[offset+3]
		pSrc[(int8)pSrc[2] + 3] = _scratchArr[(byte)pSrc[1]];
		chan->_pSrc += 3;
		break;

	case -26:
		// arr[idx]++
		++_scratchArr[(byte)pSrc[1]];
		chan->_pSrc += 2;
		break;

	case -27:
		// arr[idx]--
		--_scratchArr[(byte)pSrc[1]];
		chan->_pSrc += 2;
		break;

	case -28:
		// arr[dest] += literal
		_scratchArr[(byte)pSrc[1]] += (byte)pSrc[2];
		chan->_pSrc += 3;
		break;

	case -29:
		// arr[dest] += arr[src]
		_scratchArr[(byte)pSrc[1]] += _scratchArr[(byte)pSrc[2]];
		chan->_pSrc += 3;
		break;

	case -30:
		// arr[dest] -= literal
		_scratchArr[(byte)pSrc[1]] -= (byte)pSrc[2];
		chan->_pSrc += 3;
		break;

	case -31:
		// arr[dest] -= arr[src]
		_scratchArr[(byte)pSrc[1]] -= _scratchArr[(byte)pSrc[2]];
		chan->_pSrc += 3;
		break;

	case -32:
		// arr[dest] = literal * arr[dest]  (byte multiply, low byte)
		_scratchArr[(byte)pSrc[1]] = (byte)((byte)pSrc[2] * _scratchArr[(byte)pSrc[1]]);
		chan->_pSrc += 3;
		break;

	case -33:
		// arr[dest] = arr[src] * arr[dest]  (byte multiply, low byte)
		_scratchArr[(byte)pSrc[1]] = (byte)(_scratchArr[(byte)pSrc[2]] * _scratchArr[(byte)pSrc[1]]);
		chan->_pSrc += 3;
		break;

	case -34:
		// arr[dest] /= literal  (quotient)
		_scratchArr[(byte)pSrc[1]] = (byte)(_scratchArr[(byte)pSrc[1]] / (int8)pSrc[2]);
		chan->_pSrc += 3;
		break;

	case -35:
		// arr[dest] /= arr[src]  (quotient, unsigned)
		_scratchArr[(byte)pSrc[1]] = (byte)(_scratchArr[(byte)pSrc[1]] / _scratchArr[(byte)pSrc[2]]);
		chan->_pSrc += 3;
		break;

	case -36:
		// arr[dest] %= literal  (remainder)
		_scratchArr[(byte)pSrc[1]] = (byte)(_scratchArr[(byte)pSrc[1]] % (int8)pSrc[2]);
		chan->_pSrc += 3;
		break;

	case -37:
		// arr[dest] %= arr[src]  (remainder, unsigned)
		_scratchArr[(byte)pSrc[1]] = (byte)(_scratchArr[(byte)pSrc[1]] % _scratchArr[(byte)pSrc[2]]);
		chan->_pSrc += 3;
		break;

	case -38:
		// arr[dest] &= literal
		_scratchArr[(byte)pSrc[1]] &= (byte)pSrc[2];
		chan->_pSrc += 3;
		break;

	case -39:
		// arr[dest] &= arr[src]
		_scratchArr[(byte)pSrc[1]] &= _scratchArr[(byte)pSrc[2]];
		chan->_pSrc += 3;
		break;

	case -40:
		// arr[dest] |= literal
		_scratchArr[(byte)pSrc[1]] |= (byte)pSrc[2];
		chan->_pSrc += 3;
		break;

	case -41:
		// arr[dest] |= arr[src]
		_scratchArr[(byte)pSrc[1]] |= _scratchArr[(byte)pSrc[2]];
		chan->_pSrc += 3;
		break;

	case -42:
		// arr[dest] ^= literal
		_scratchArr[(byte)pSrc[1]] ^= (byte)pSrc[2];
		chan->_pSrc += 3;
		break;

	case -43:
		// arr[dest] ^= arr[src]
		_scratchArr[(byte)pSrc[1]] ^= _scratchArr[(byte)pSrc[2]];
		chan->_pSrc += 3;
		break;

	// Cases -44 to -51: conditional jumps (5-byte: cmd, idx, literal/idx2, ptr_lo, ptr_hi)
	// Branch taken  → jump to near-pointer target (bytes 3-4)
	// Branch not taken → skip 5 bytes

#define PHANTOM_COND_JUMP(cond) \
	do { \
		if (cond) { \
			int nearPtr = (int16)READ_LE_UINT16(pSrc + 3); \
			chan->_pSrc = getDataPtr(nearPtr); \
		} else { \
			chan->_pSrc += 5; \
		} \
	} while (0)

	case -44:
		PHANTOM_COND_JUMP(_scratchArr[(byte)pSrc[1]] == (byte)pSrc[2]);
		break;

	case -45:
		PHANTOM_COND_JUMP(_scratchArr[(byte)pSrc[1]] != (byte)pSrc[2]);
		break;

	case -46:
		// arr[idx] < literal  (jmp if below the cap)
		PHANTOM_COND_JUMP(_scratchArr[(byte)pSrc[1]] < (byte)pSrc[2]);
		break;

	case -47:
		// arr[idx] > literal
		PHANTOM_COND_JUMP(_scratchArr[(byte)pSrc[1]] > (byte)pSrc[2]);
		break;

	case -48:
		PHANTOM_COND_JUMP(_scratchArr[(byte)pSrc[2]] == _scratchArr[(byte)pSrc[1]]);
		break;

	case -49:
		PHANTOM_COND_JUMP(_scratchArr[(byte)pSrc[2]] != _scratchArr[(byte)pSrc[1]]);
		break;

	case -50:
		// arr[idx2] > arr[idx1]  (unsigned)
		PHANTOM_COND_JUMP(_scratchArr[(byte)pSrc[2]] > _scratchArr[(byte)pSrc[1]]);
		break;

	case -51:
		// arr[idx2] < arr[idx1]  (unsigned)
		PHANTOM_COND_JUMP(_scratchArr[(byte)pSrc[2]] < _scratchArr[(byte)pSrc[1]]);
		break;

#undef PHANTOM_COND_JUMP

	// Cases -52 to -59: conditional calls — same as -44..-51 but save return address
	// Branch taken  → save (chan->_pSrc + 5) in _ptrEnd, then jump to target
	// Branch not taken → skip 5 bytes

#define PHANTOM_COND_CALL(cond) \
	do { \
		if (cond) { \
			int nearPtr = (int16)READ_LE_UINT16(pSrc + 3); \
			chan->_ptrEnd = chan->_pSrc + 5; \
			chan->_pSrc   = getDataPtr(nearPtr); \
		} else { \
			chan->_pSrc += 5; \
		} \
	} while (0)

	case -52:
		PHANTOM_COND_CALL(_scratchArr[(byte)pSrc[1]] == (byte)pSrc[2]);
		break;

	case -53:
		PHANTOM_COND_CALL(_scratchArr[(byte)pSrc[1]] != (byte)pSrc[2]);
		break;

	case -54:
		// arr[idx] < literal
		PHANTOM_COND_CALL(_scratchArr[(byte)pSrc[1]] < (byte)pSrc[2]);
		break;

	case -55:
		// arr[idx] > literal
		PHANTOM_COND_CALL(_scratchArr[(byte)pSrc[1]] > (byte)pSrc[2]);
		break;

	case -56:
		PHANTOM_COND_CALL(_scratchArr[(byte)pSrc[2]] == _scratchArr[(byte)pSrc[1]]);
		break;

	case -57:
		PHANTOM_COND_CALL(_scratchArr[(byte)pSrc[2]] != _scratchArr[(byte)pSrc[1]]);
		break;

	case -58:
		// arr[idx2] > arr[idx1]  (unsigned)
		PHANTOM_COND_CALL(_scratchArr[(byte)pSrc[2]] > _scratchArr[(byte)pSrc[1]]);
		break;

	case -59:
		// arr[idx2] < arr[idx1]  (unsigned)
		PHANTOM_COND_CALL(_scratchArr[(byte)pSrc[2]] < _scratchArr[(byte)pSrc[1]]);
		break;

#undef PHANTOM_COND_CALL

	case -60:
		// Call near function pointer — not translatable to C++; skip 3-byte command
		chan->_pSrc += 3;
		break;

	case -61:
		// No-op with 2-byte command (original called a null stub)
		chan->_pSrc += 2;
		break;

	case -62:
		// Skip 4 bytes (advance past this 4-byte command)
		chan->_pSrc += 4;
		break;

	case -63:
		// Store signed byte into _w11F50 music state variable
		_w11F50 = (int8)pSrc[1];
		chan->_pSrc += 2;
		break;

	case -64:
		// Store signed byte into _w11F46; if _w11F44 == 0, also copy to _w11F4E
		_w11F46 = (int8)pSrc[1];
		chan->_pSrc += 2;
		if (!_w11F44)
			_w11F4E = _w11F46;
		break;

	case -65: {
		// Store 16-bit word into _w11F48; signal music sync (_w11F32/_w11F42 = 1)
		++pSrc;
		_w11F48 = (int16)READ_LE_UINT16(pSrc);
		chan->_pSrc = pSrc + 2;     // advance 3 bytes total from command
		if (!_w11F44)
			_w11F4C = _w11F48;
		_w11F32 = 1;
		_w11F42 = 1;
		break;
	}

	case -66:
		// Store signed byte into _w11F4A music state variable
		_w11F4A = (int8)pSrc[1];
		chan->_pSrc += 2;
		break;

	default:
		break;
	}
}

/*-----------------------------------------------------------------------*/
/* ASound1  (asound.ph1)                                                  *
 *                                                                        *
 * Channel-load helpers in the original:                                  *
 *   sub_103FF=ch0, sub_10404=ch1, sub_10409=ch2                         *
 *   sub_1040E=ch3, sub_10413=ch4, sub_10418=ch5                         *
 *   loc_1041D=ch6, loc_10422=ch7, loc_10427=ch8                         *
 *                                                                        *
 * sub_106DF = isSoundActive guard (non-zero BX → already playing)       *
 * sub_1039C = playSoundData(pData, ADLIB_CHANNEL_MIDWAY)  (upper pool)  *
 * sub_10352 = playSoundData(pData, 0)                     (lower pool)  *
 *-----------------------------------------------------------------------*/

const ASound1::CommandPtr ASound1::_commandList[40] = {
	&ASound1::command0,  &ASound1::command1,  &ASound1::command2,  &ASound1::command3,
	&ASound1::command4,  &ASound1::command5,  &ASound1::command6,  &ASound1::command7,
	&ASound1::command8,  nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound1::command16, nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound1::command24, &ASound1::command25, &ASound1::command26, &ASound1::command27,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound1::command32, &ASound1::command33, &ASound1::command34, &ASound1::command35,
	&ASound1::command36, &ASound1::command37, &ASound1::command38, &ASound1::command39
};

ASound1::ASound1(Audio::Mixer *mixer, OPL::OPL *opl)
		: PhantomASound(mixer, opl, "asound.ph1", 0x21e0) {
}

int ASound1::command(int commandId, int param) {
	// The original dispatcher also handles commands 64–76 via a raw-data
	// near-pointer table (unk_13C3E) that cannot be safely reconstructed.
	if (commandId > 39 || !_commandList[commandId])
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

// commands 0–8: delegate to base ASound
int ASound1::command0() { return ASound::command0(); }
int ASound1::command1() { return ASound::command1(); }
int ASound1::command2() { return ASound::command2(); }
int ASound1::command3() { return ASound::command3(); }
int ASound1::command4() { return ASound::command4(); }
int ASound1::command5() { return ASound::command5(); }
int ASound1::command6() { return ASound::command6(); }
int ASound1::command7() { return ASound::command7(); }
int ASound1::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// Background-music loaders (sub_11D84, sub_11EE6, sub_11F0E, sub_11F36)
// Each calls command1() then loads six channels.
// The five cx values that command16 checks against _channels[0]._field17
// are the starting offsets of each piece: 0x1ECA, 0x21C4, 0x3418, 0x3688,
// 0x3D52.  (0x21C4 is the start of dead code following commandMusic0 that
// was never reached by any dispatch table entry.)
// ---------------------------------------------------------------------------

int ASound1::commandMusic0() {
	ASound::command1();
	_channels[0].load(loadData(0x1ECA, 245));
	_channels[1].load(loadData(0x1FBF, 120));
	_channels[2].load(loadData(0x2037, 183));
	_channels[3].load(loadData(0x20EE, 173));
	_channels[4].load(loadData(0x219B,  20));
	_channels[5].load(loadData(0x21AF,  21));
	return 0;
}

int ASound1::commandMusic1() {
	ASound::command1();
	_channels[0].load(loadData(0x3418, 211));
	_channels[1].load(loadData(0x34EB, 176));
	_channels[2].load(loadData(0x359B, 189));
	_channels[3].load(loadData(0x3658,  15));
	_channels[4].load(loadData(0x3667,  16));
	_channels[5].load(loadData(0x3677,  17));
	return 0;
}

int ASound1::commandMusic2() {
	ASound::command1();
	_channels[0].load(loadData(0x3688, 499));
	_channels[1].load(loadData(0x387B, 390));
	_channels[2].load(loadData(0x3A01, 453));
	_channels[3].load(loadData(0x3BC6, 363));
	_channels[4].load(loadData(0x3D31,  16));
	_channels[5].load(loadData(0x3D41,  17));
	return 0;
}

int ASound1::commandMusic3() {
	ASound::command1();
	_channels[0].load(loadData(0x3D52, 641));
	_channels[1].load(loadData(0x3FD3, 556));
	_channels[2].load(loadData(0x41FF,  13));
	_channels[3].load(loadData(0x420C,  13));
	_channels[4].load(loadData(0x4219,  16));
	_channels[5].load(loadData(0x4229,  17));
	return 0;
}

// ---------------------------------------------------------------------------
// command16 (sub_11F70) – random background music
//
// If channel 0 is active and already playing one of the five known music
// pieces (identified by their starting offset in field_17), leave it alone.
// Otherwise pick a piece at random: the original uses getRandomNumber() & 7,
// discarding 0 and indexing a four-entry table repeated twice (entries 1–3
// → pieces 1–3, entries 4–7 → same pieces again with entry 4 wrapping to
// piece 0).  We reproduce this with a modulo-4 on a non-zero value.
// ---------------------------------------------------------------------------
int ASound1::command16() {
	if (_channels[0]._activeCount) {
		int f = _channels[0]._field17;
		if (f == 0x1ECA || f == 0x21C4 ||
		    f == 0x3418 || f == 0x3688 || f == 0x3D52)
			return 0;
	}

	int idx;
	do {
		idx = getRandomNumber() & 3;
	} while (idx == 0);
	_musicIndex = idx;

	typedef int (ASound1::*MusicPtr)();
	static const MusicPtr musicTable[4] = {
		&ASound1::commandMusic0,
		&ASound1::commandMusic1,
		&ASound1::commandMusic2,
		&ASound1::commandMusic3
	};
	return (this->*musicTable[idx])();
}

// ---------------------------------------------------------------------------
// commands 24–27 (off_11C46) – upper channel pool (sub_1039C per entry)
// ---------------------------------------------------------------------------

int ASound1::command24() {
	playSound(0x173A, 51);
	playSound(0x176D, 46);
	return 0;
}

int ASound1::command25() {
	playSound(0x179B, 44);
	playSound(0x17C7, 46);
	return 0;
}

int ASound1::command26() {
	playSound(0x17F5, 12);
	return 0;
}

int ASound1::command27() {
	playSound(0x1801, 81);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 32–39 (off_11C4E, entries 0–7)
// ---------------------------------------------------------------------------

// command32 (sub_11DD4) – no guard, no fade, load ch0–5
int ASound1::command32() {
	_channels[0].load(loadData(0x2522, 59));
	_channels[1].load(loadData(0x255D, 52));
	_channels[2].load(loadData(0x2591, 42));
	_channels[3].load(loadData(0x25BB, 44));
	_channels[4].load(loadData(0x25E7, 44));
	_channels[5].load(loadData(0x2613, 89));
	return 0;
}

// command33 (sub_11E02) – isSoundActive guard, command1, load ch0–5
int ASound1::command33() {
	byte *pData = loadData(0x266C, 701);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x2929, 500));
		_channels[2].load(loadData(0x2B1D, 538));
		_channels[3].load(loadData(0x2D37, 396));
		_channels[4].load(loadData(0x2EC3, 368));
		_channels[5].load(loadData(0x3033, 493));
	}
	return 0;
}

// command34 (sub_11D54) – isSoundActive guard, stop(), load ch0–5
int ASound1::command34() {
	byte *pData = loadData(0x1852, 599);
	if (!isSoundActive(pData)) {
		stop();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x1AA9, 283));
		_channels[2].load(loadData(0x1BC4, 301));
		_channels[3].load(loadData(0x1CF1, 257));
		_channels[4].load(loadData(0x1DF2, 204));
		_channels[5].load(loadData(0x1EBE,  12));
	}
	return 0;
}

// command35 (sub_11CCC) – isSoundActive guard, command2 (lower-bank fade),
// load ch0–5
int ASound1::command35() {
	byte *pData = loadData(0x0C36, 329);
	if (!isSoundActive(pData)) {
		ASound::command2();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x0D7F, 201));
		_channels[2].load(loadData(0x0E48, 200));
		_channels[3].load(loadData(0x0F10, 162));
		_channels[4].load(loadData(0x0FB2, 228));
		_channels[5].load(loadData(0x1096, 250));
	}
	return 0;
}

// command36 (sub_11CFC) – isSoundActive guard, command2 (lower-bank fade),
// load ch0–5
int ASound1::command36() {
	byte *pData = loadData(0x1190, 327);
	if (!isSoundActive(pData)) {
		ASound::command2();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x12D7, 211));
		_channels[2].load(loadData(0x13AA, 204));
		_channels[3].load(loadData(0x1476, 178));
		_channels[4].load(loadData(0x1528, 236));
		_channels[5].load(loadData(0x1614, 294));
	}
	return 0;
}

// command37 (sub_11E32) – isSoundActive guard, command1, four loadAny
// calls starting from channel 0 (sub_10352 = playSoundData(pData, 0))
int ASound1::command37() {
	byte *pData = loadData(0x3220, 74);
	if (!isSoundActive(pData)) {
		ASound::command1();
		playSoundData(pData, 0);
		playSoundData(loadData(0x326A, 41), 0);
		playSoundData(loadData(0x3293, 25), 0);
		playSoundData(loadData(0x32AC, 14), 0);
	}
	return 0;
}

// command38 (sub_11D84) – alias for commandMusic0; also the direct dispatch
// target for command 38.
int ASound1::command38() {
	return commandMusic0();
}

// command39 (sub_11FAE) – isSoundActive guard, command1, load ch0–5
int ASound1::command39() {
	byte *pData = loadData(0x423A, 421);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x43DF, 280));
		_channels[2].load(loadData(0x44F7, 246));
		_channels[3].load(loadData(0x45ED, 268));
		_channels[4].load(loadData(0x46F9, 438));
		_channels[5].load(loadData(0x48AF,   0));
	}
	return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound2  (asound.ph2)                                                  *
 *                                                                        *
 * Dispatch table layout:                                                 *
 *   asound_commands1: commands  0– 8  (max=8,    base=0)                *
 *   asound_commands2: command     16  (max=0x10, base=0x10, 1 entry)    *
 *   asound_commands3: commands 24–27  (max=0x1B, base=0x18, 4 entries)  *
 *   asound_commands4: commands 32–35  (max=0x23, base=0x20, 4 entries)  *
 *   asound_commands5: commands 64–72  (max=0x48, base=0x40, 9 entries)  *
 *                                                                        *
 * Channel-load helpers:                                                  *
 *   sub_103FF=ch0  sub_10404=ch1  sub_10409=ch2  sub_1040E=ch3          *
 *   sub_10413=ch4  sub_10418=ch5  loc_1041D=ch6  loc_10422=ch7          *
 *   (ch8 not used in any sound command)                                  *
 *                                                                        *
 * sub_105F3 = command1 (fade both banks)                                 *
 * sub_10481 = command2 (lower setPtr2, cx=0x1A26)                       *
 * sub_105FA = command3 (lower bank fade helper)                          *
 * sub_104A9 = command4 (upper setPtr2, cx=0x1A26)                       *
 * sub_104BF = command6                                                   *
 * loc_10548 = command7                                                   *
 * unk_106B2 = command8 (OR of all channel activeCount fields)            *
 * sub_106DF = isSoundActive guard                                        *
 * sub_1039C = playSoundData(pData, ADLIB_CHANNEL_MIDWAY)  (upper pool)  *
 * sub_10352 = playSoundData(pData, 0)                     (lower pool)  *
 *                                                                        *
 * commands5 entry 4 (command 68) = nullsub_3 = no-op                    *
 *-----------------------------------------------------------------------*/

const ASound2::CommandPtr ASound2::_commandList[73] = {
	// commands 0–8  (asound_commands1)
	&ASound2::command0,  &ASound2::command1,  &ASound2::command2,  &ASound2::command3,
	&ASound2::command4,  &ASound2::command5,  &ASound2::command6,  &ASound2::command7,
	&ASound2::command8,
	// 9–15 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// command 16  (asound_commands2)
	&ASound2::command16,
	// 17–23 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 24–27  (asound_commands3)
	&ASound2::command24, &ASound2::command25, &ASound2::command26, &ASound2::command27,
	// 28–31 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 32–35  (asound_commands4)
	&ASound2::command32, &ASound2::command33, &ASound2::command34, &ASound2::command35,
	// 36–63 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 64–72  (asound_commands5)
	&ASound2::command64, &ASound2::command65, &ASound2::command66, &ASound2::command67,
	&ASound2::command68, &ASound2::command69, &ASound2::command70, &ASound2::command71,
	&ASound2::command72
};

ASound2::ASound2(Audio::Mixer *mixer, OPL::OPL *opl)
		: PhantomASound(mixer, opl, "asound.ph2", 0x2040) {
}

int ASound2::command(int commandId, int param) {
	if (commandId > 72 || !_commandList[commandId])
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

// commands 0–8: delegate to base ASound
int ASound2::command0() { return ASound::command0(); }
int ASound2::command1() { return ASound::command1(); }
int ASound2::command2() { return ASound::command2(); }
int ASound2::command3() { return ASound::command3(); }
int ASound2::command4() { return ASound::command4(); }
int ASound2::command5() { return ASound::command5(); }
int ASound2::command6() { return ASound::command6(); }
int ASound2::command7() { return ASound::command7(); }
int ASound2::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// command16 (sub_11CBC) – isSoundActive guard, command1, load ch0–5
// ---------------------------------------------------------------------------
int ASound2::command16() {
	byte *pData = loadData(0x0C36, 88);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x0C8E, 102));
		_channels[2].load(loadData(0x0CF4,  90));
		_channels[3].load(loadData(0x0D4E,  85));
		_channels[4].load(loadData(0x0DA3,  14));
		_channels[5].load(loadData(0x0DB1,  15));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 24–27 (asound_commands3) – upper channel pool via sub_1039C
// ---------------------------------------------------------------------------

// sub_11D78
int ASound2::command24() {
	playSound(0x1A4A, 51);
	playSound(0x1A7D, 46);
	return 0;
}

// sub_11D85
int ASound2::command25() {
	playSound(0x1AAB, 44);
	playSound(0x1AD7, 46);
	return 0;
}

// sub_11D92
int ASound2::command26() {
	playSound(0x1B05, 12);
	return 0;
}

// sub_11D99
int ASound2::command27() {
	playSound(0x1B11, 81);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 32–35 (asound_commands4)
// ---------------------------------------------------------------------------

// command32 (sub_11DDC) – command1, six loadAny calls from channel 0
// (sub_10352 = playSoundData(pData, 0))
int ASound2::command32() {
	ASound::command1();
	playSoundData(loadData(0x1BE4, 211), 0);
	playSoundData(loadData(0x1CB7, 359), 0);
	playSoundData(loadData(0x1E1E, 170), 0);
	playSoundData(loadData(0x1EC8,  16), 0);
	playSoundData(loadData(0x1ED8,  23), 0);
	playSoundData(loadData(0x1EEF,  19), 0);
	return 0;
}

// command33 (sub_11DA0) – isSoundActive guard, command1, load ch0–7
int ASound2::command33() {
	byte *pData = loadData(0x1B62, 53);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x1B97, 14));
		_channels[2].load(loadData(0x1BA5, 14));
		_channels[3].load(loadData(0x1BB3, 14));
		_channels[4].load(loadData(0x1BC1,  4));
		_channels[5].load(loadData(0x1BC5,  4));
		_channels[6].load(loadData(0x1BC9, 12));
		_channels[7].load(loadData(0x1BD5, 15));
	}
	return 0;
}

// command34 (sub_11CEC) – isSoundActive guard, command1, load ch0–6
int ASound2::command34() {
	byte *pData = loadData(0x0DC0, 495);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x0FAF, 599));
		_channels[2].load(loadData(0x1206, 404));
		_channels[3].load(loadData(0x139A, 459));
		_channels[4].load(loadData(0x1565, 718));
		_channels[5].load(loadData(0x1833, 154));
		_channels[6].load(loadData(0x18CD,  91));
	}
	return 0;
}

// command35 (sub_11E04) – isSoundActive guard, command1, load ch0–6
int ASound2::command35() {
	byte *pData = loadData(0x1F02, 100);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x1F66,  10));
		_channels[2].load(loadData(0x1F70,  29));
		_channels[3].load(loadData(0x1F8D,  65));
		_channels[4].load(loadData(0x1FCE,  41));
		_channels[5].load(loadData(0x1FF7,  55));
		_channels[6].load(loadData(0x202E,  34));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 64–72 (asound_commands5) – upper channel pool via sub_1039C
// ---------------------------------------------------------------------------

// sub_11D22
int ASound2::command64() {
	playSound(0x1928, 20);
	return 0;
}

// sub_11D29
int ASound2::command65() {
	playSound(0x193C, 10);
	return 0;
}

// sub_11D30
int ASound2::command66() {
	playSound(0x1946, 22);
	playSound(0x195C, 17);
	return 0;
}

// sub_11D3D
int ASound2::command67() {
	playSound(0x196D, 18);
	return 0;
}

// nullsub_3 – no-op
int ASound2::command68() {
	return 0;
}

// sub_11D45
int ASound2::command69() {
	playSound(0x197F, 38);
	playSound(0x19A5, 38);
	playSound(0x19CB, 26);
	return 0;
}

// sub_11D58
int ASound2::command70() {
	playSound(0x19E5, 12);
	playSound(0x19F1, 14);
	return 0;
}

// sub_11D65
int ASound2::command71() {
	playSound(0x19FF, 14);
	return 0;
}

// sub_11D6C
int ASound2::command72() {
	playSound(0x1A0D,  3);
	playSound(0x1A10, 22);
	return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound3  (asound.ph3)                                                  *
 *                                                                        *
 * Dispatch table layout:                                                 *
 *   asound_commands1: commands  0–8   (max=8,    base=0)                *
 *   asound_commands2: command   16    (max=0x10, base=0x10, 1 entry)    *
 *   asound_commands3: commands 24–27  (max=0x1B, base=0x18, 4 entries)  *
 *   asound_commands4: commands 32–37  (max=0x25, base=0x20, 6 entries)  *
 *   asound_commands5: commands 64–76  (max=0x4B, base=0x40, 12 entries) *
 *     (entry 12, command 76 = nullsub_8, is a no-op)                    *
 *                                                                        *
 * Channel-load helpers:                                                  *
 *   sub_103FF=ch0  sub_10404=ch1  sub_10409=ch2  sub_1040E=ch3          *
 *   sub_10413=ch4  sub_10418=ch5  sub_1041D=ch6  sub_10422=ch7          *
 *   loc_10427 =ch8                                                       *
 *                                                                        *
 * sub_106DF = isSoundActive guard                                        *
 * sub_1039C = playSoundData(pData, ADLIB_CHANNEL_MIDWAY) (upper pool)   *
 *                                                                        *
 * sub_11CC6 (helper) – isSoundActive guard on 0xC36, command1,          *
 *   loads ch0–7; called by command34 which then adds ch8 at 0x298E.     *
 *-----------------------------------------------------------------------*/

const ASound3::CommandPtr ASound3::_commandList[77] = {
	// commands 0–8  (asound_commands1)
	&ASound3::command0,  &ASound3::command1,  &ASound3::command2,  &ASound3::command3,
	&ASound3::command4,  &ASound3::command5,  &ASound3::command6,  &ASound3::command7,
	&ASound3::command8,
	// 9–15 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// command 16  (asound_commands2)
	&ASound3::command16,
	// 17–23 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 24–27  (asound_commands3)
	&ASound3::command24, &ASound3::command25, &ASound3::command26, &ASound3::command27,
	// 28–31 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 32–37  (asound_commands4)
	&ASound3::command32, &ASound3::command33, &ASound3::command34, &ASound3::command35,
	&ASound3::command36, &ASound3::command37,
	// 38–63 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,
	// commands 64–75  (asound_commands5)
	&ASound3::command64, &ASound3::command65, &ASound3::command66, &ASound3::command67,
	&ASound3::command68, &ASound3::command69, &ASound3::command70, &ASound3::command71,
	&ASound3::command72, &ASound3::command73, &ASound3::command74, &ASound3::command75,
	// command 76 = nullsub_8
	nullptr
};

ASound3::ASound3(Audio::Mixer *mixer, OPL::OPL *opl)
		: PhantomASound(mixer, opl, "asound.ph3", 0x20c0) {
}

int ASound3::command(int commandId, int param) {
	if (commandId > 76 || !_commandList[commandId])
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

// commands 0–8: delegate to base ASound
int ASound3::command0() { return ASound::command0(); }
int ASound3::command1() { return ASound::command1(); }
int ASound3::command2() { return ASound::command2(); }
int ASound3::command3() { return ASound::command3(); }
int ASound3::command4() { return ASound::command4(); }
int ASound3::command5() { return ASound::command5(); }
int ASound3::command6() { return ASound::command6(); }
int ASound3::command7() { return ASound::command7(); }
int ASound3::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// sub_11CC6 – shared helper used by command34.
// isSoundActive guard on 0xC36; if not active: command1, load ch0–7.
// (The tiny ch6 and ch7 blocks — 4 bytes each — are very short sub-blocks
// within the same composite sound.)
// ---------------------------------------------------------------------------
void ASound3::sub11CC6() {
	byte *pData = loadData(0x0C36, 53);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x0C6B, 14));
		_channels[2].load(loadData(0x0C79, 14));
		_channels[3].load(loadData(0x0C87, 14));
		_channels[4].load(loadData(0x0C95,  4));
		_channels[5].load(loadData(0x0C99,  4));
		_channels[6].load(loadData(0x0C9D, 12));
		_channels[7].load(loadData(0x0CA9, 15));
	}
}

// ---------------------------------------------------------------------------
// command16 (sub_11D98) – isSoundActive guard, command1, load ch0–5
// ---------------------------------------------------------------------------
int ASound3::command16() {
	byte *pData = loadData(0x24F2, 172);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x259E, 137));
		_channels[2].load(loadData(0x2627, 135));
		_channels[3].load(loadData(0x26AE, 179));
		_channels[4].load(loadData(0x2761, 175));
		_channels[5].load(loadData(0x2810, 186));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 24–27 (asound_commands3) – upper channel pool (sub_1039C)
// ---------------------------------------------------------------------------

// sub_11E54
int ASound3::command24() {
	playSound(0x2A7C, 51);
	playSound(0x2AAF, 46);
	return 0;
}

// sub_11E61
int ASound3::command25() {
	playSound(0x2ADD, 44);
	playSound(0x2B09, 46);
	return 0;
}

// sub_11E6E
int ASound3::command26() {
	playSound(0x2B37, 12);
	return 0;
}

// sub_11E75
int ASound3::command27() {
	playSound(0x2B43, 12);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 32–37 (asound_commands4)
// ---------------------------------------------------------------------------

// command32 (sub_11E7C) – isSoundActive guard, command1, load ch0–7
int ASound3::command32() {
	byte *pData = loadData(0x2B94, 108);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x2C00,  73));
		_channels[2].load(loadData(0x2C49,  67));
		_channels[3].load(loadData(0x2C8C, 151));
		_channels[4].load(loadData(0x2D23, 171));
		_channels[5].load(loadData(0x2DCE,  87));
		_channels[6].load(loadData(0x2E25,  95));
		_channels[7].load(loadData(0x2E84, 110));
	}
	return 0;
}

// command33 (sub_11D32) – isSoundActive guard, command1, load ch0–6
int ASound3::command33() {
	byte *pData = loadData(0x149E, 525);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x16AB, 648));
		_channels[2].load(loadData(0x1933, 252));
		_channels[3].load(loadData(0x1A2F, 502));
		_channels[4].load(loadData(0x1C25, 680));
		_channels[5].load(loadData(0x1ECD, 418));
		_channels[6].load(loadData(0x206F,   3));
	}
	return 0;
}

// command34 (sub_11E2F) – calls sub11CC6 (loads ch0–7 if not active),
// then unconditionally loads ch8 at 0x298E
int ASound3::command34() {
	sub11CC6();
	_channels[8].load(loadData(0x298E, 10));
	return 0;
}

// command35 (sub_11D02) – isSoundActive guard, command1, load ch0–5
int ASound3::command35() {
	byte *pData = loadData(0x0CB8, 413);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x0E55, 270));
		_channels[2].load(loadData(0x0F63, 238));
		_channels[3].load(loadData(0x1051, 264));
		_channels[4].load(loadData(0x1159, 434));
		_channels[5].load(loadData(0x130B, 403));
	}
	return 0;
}

// command36 (sub_11D68) – isSoundActive guard, command1, load ch0–5
int ASound3::command36() {
	byte *pData = loadData(0x2072, 196);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x2136, 692));
		_channels[2].load(loadData(0x23EA,  83));
		_channels[3].load(loadData(0x243D,  24));
		_channels[4].load(loadData(0x2455,  78));
		_channels[5].load(loadData(0x24A3,  79));
	}
	return 0;
}

// command37 (sub_11DF8) – single upper-pool voice
int ASound3::command37() {
	playSound(0x298E, 10);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 64–75 (asound_commands5)
//
// sub_11DC8/sub_11DD4: load ch6 and ch8 directly (skipping ch7).
// sub_11DE0: loads ch6, ch7, ch8.
// sub_11E22/sub_11E47: load ch7 and ch8 directly.
// Remaining entries use the upper pool (sub_1039C).
// ---------------------------------------------------------------------------

// sub_11DC8 – ch6 + ch8
int ASound3::command64() {
	_channels[6].load(loadData(0x28CA, 50));
	_channels[8].load(loadData(0x28FC, 29));
	return 0;
}

// sub_11DD4 – ch6 + ch8
int ASound3::command65() {
	_channels[6].load(loadData(0x2919, 17));
	_channels[8].load(loadData(0x292A, 13));
	return 0;
}

// sub_11DE0 – ch6 + ch7 + ch8
int ASound3::command66() {
	_channels[6].load(loadData(0x2937, 31));
	_channels[7].load(loadData(0x2956, 15));
	_channels[8].load(loadData(0x2965, 31));
	return 0;
}

// sub_11DF2 – upper pool x1
int ASound3::command67() {
	playSound(0x2984, 10);
	return 0;
}

// sub_11DFE – upper pool x3
int ASound3::command68() {
	playSound(0x2998, 22);
	playSound(0x29AE, 20);
	playSound(0x29C2, 22);
	return 0;
}

// sub_11E10 – upper pool x1
int ASound3::command69() {
	playSound(0x29D8, 18);
	return 0;
}

// sub_11E16 – upper pool x1
int ASound3::command70() {
	playSound(0x2B4F, 15);
	return 0;
}

// sub_11E1C – upper pool x1
int ASound3::command71() {
	playSound(0x2B5E, 54);
	return 0;
}

// sub_11E22 – ch7 + ch8
int ASound3::command72() {
	_channels[7].load(loadData(0x29EA, 17));
	_channels[8].load(loadData(0x2A18, 17));
	return 0;
}

// sub_11E39 – upper pool x1
int ASound3::command73() {
	playSound(0x2A44, 10);
	return 0;
}

// sub_11E40 – upper pool x1
int ASound3::command74() {
	playSound(0x2A4E, 46);
	return 0;
}

// sub_11E47 – ch7 + ch8
int ASound3::command75() {
	_channels[7].load(loadData(0x29FB, 29));
	_channels[8].load(loadData(0x2A29, 27));
	return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound4  (asound.ph4)                                                  *
 *                                                                        *
 * Dispatch table layout:                                                 *
 *   asound_commands1: commands  0–8   (max=8,    base=0)                *
 *   asound_commands2: command   16    (max=0x10, base=0x10, 1 entry)    *
 *   asound_commands3: commands 24–27  (max=0x1B, base=0x18, 4 entries)  *
 *   asound_commands4: commands 64–70  (max=0x46, base=0x40, 7 entries)  *
 *     (The 0x20-range table also points at asound_commands4 but has     *
 *      max=0, making commands 32–63 unreachable.)                       *
 *                                                                        *
 * Channel-load helpers:                                                  *
 *   sub_103FF=ch0  sub_10404=ch1  sub_10409=ch2  sub_1040E=ch3          *
 *   sub_10413=ch4  sub_10418=ch5  sub_1041D=ch6                         *
 *                                                                        *
 * sub_106DF = isSoundActive guard                                        *
 * sub_1039C = playSoundData(pData, ADLIB_CHANNEL_MIDWAY) (upper pool)   *
 *                                                                        *
 * commands 24 and 25 share the same handler (sub_11D0A).                *
 * Two unreferenced subs (sub_11D5E, sub_11D6B) contain dead sound data  *
 * that is never played; they are omitted.                                *
 * An unreferenced random-pitch stub exists between sub_11CAB and        *
 * sub_11CD3; it was never wired into any command.                        *
 *-----------------------------------------------------------------------*/

const ASound4::CommandPtr ASound4::_commandList[71] = {
	// commands 0–8  (asound_commands1)
	&ASound4::command0,  &ASound4::command1,  &ASound4::command2,  &ASound4::command3,
	&ASound4::command4,  &ASound4::command5,  &ASound4::command6,  &ASound4::command7,
	&ASound4::command8,
	// 9–15 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// command 16  (asound_commands2)
	&ASound4::command16,
	// 17–23 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 24–27  (asound_commands3)
	&ASound4::command24, &ASound4::command25, &ASound4::command26, &ASound4::command27,
	// 28–63 absent (the 0x20-range table has max=0, unreachable)
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 64–70  (asound_commands4)
	&ASound4::command64, &ASound4::command65, &ASound4::command66, &ASound4::command67,
	&ASound4::command68, &ASound4::command69, &ASound4::command70
};

ASound4::ASound4(Audio::Mixer *mixer, OPL::OPL *opl)
		: PhantomASound(mixer, opl, "asound.ph4", 0x1f90) {
}

int ASound4::command(int commandId, int param) {
	if (commandId > 70 || !_commandList[commandId])
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

// commands 0–8: delegate to base ASound
int ASound4::command0() { return ASound::command0(); }
int ASound4::command1() { return ASound::command1(); }
int ASound4::command2() { return ASound::command2(); }
int ASound4::command3() { return ASound::command3(); }
int ASound4::command4() { return ASound::command4(); }
int ASound4::command5() { return ASound::command5(); }
int ASound4::command6() { return ASound::command6(); }
int ASound4::command7() { return ASound::command7(); }
int ASound4::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// command16 (sub_11CD3) – isSoundActive guard, command1, load ch0–6
// ---------------------------------------------------------------------------
int ASound4::command16() {
	byte *pData = loadData(0x0C36, 63);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x0C75, 636));
		_channels[2].load(loadData(0x0EF1,  40));
		_channels[3].load(loadData(0x0F19,  40));
		_channels[4].load(loadData(0x0F41,  38));
		_channels[5].load(loadData(0x0F67,  41));
		_channels[6].load(loadData(0x0F90, 106));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 24–27 (asound_commands3) – upper pool (sub_1039C)
//
// commands 24 and 25 are both wired to the same handler (sub_11D0A),
// which loads two upper-pool sounds.
// ---------------------------------------------------------------------------

// sub_11D0A  (shared by both command24 and command25)
int ASound4::command24() {
	playSound(0x0FFA, 18);
	playSound(0x100C, 11);
	return 0;
}

int ASound4::command25() {
	return command24();
}

// sub_11D78
int ASound4::command26() {
	playSound(0x119D, 12);
	return 0;
}

// sub_11D7F
int ASound4::command27() {
	playSound(0x11A9, 121);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 64–70 (asound_commands4, base 0x40)
// All entries use the upper pool (sub_1039C).
// ---------------------------------------------------------------------------

// sub_11D16
int ASound4::command64() {
	playSound(0x1017, 26);
	playSound(0x1031, 17);
	return 0;
}

// sub_11D22
int ASound4::command65() {
	playSound(0x1042,  9);
	playSound(0x104B, 20);
	return 0;
}

// sub_11D2E
int ASound4::command66() {
	playSound(0x105F,  9);
	playSound(0x1068, 16);
	return 0;
}

// sub_11D3A
int ASound4::command67() {
	playSound(0x1078,  9);
	playSound(0x1081, 14);
	return 0;
}

// sub_11D46
int ASound4::command68() {
	playSound(0x108F, 12);
	return 0;
}

// sub_11D4C
int ASound4::command69() {
	playSound(0x109B, 10);
	return 0;
}

// sub_11D52
int ASound4::command70() {
	playSound(0x10A5,  3);
	playSound(0x10A8, 58);
	return 0;
}

/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* ASound5  (asound.ph5)                                                  *
 *                                                                        *
 * Dispatch table layout:                                                 *
 *   asound_commands1: commands  0–8   (max=8,    base=0)                *
 *   asound_commands2: command   16    (max=0x10, base=0x10, 1 entry)    *
 *   asound_commands3: commands 24–27  (max=0x1B, base=0x18, 4 entries)  *
 *   asound_commands4: commands 32–39  (max=0x27, base=0x20, 8 entries)  *
 *   asound_commands5: commands 64–78  (max=0x4E, base=0x40, 15 entries) *
 *     (entry 15, command 79 = nullsub_8, silently ignored)              *
 *                                                                        *
 * Channel-load helpers:                                                  *
 *   sub_103FF=ch0  sub_10404=ch1  sub_10409=ch2  sub_1040E=ch3          *
 *   sub_10413=ch4  sub_10418=ch5  loc_1041D=ch6  loc_10422=ch7          *
 *   loc_10427=ch8                                                        *
 *                                                                        *
 * In this driver IDA named the lower-pool loadAny as AdlibChannel_loadAny*
 * (starts from ch0) and sub_1039C as the upper-pool loader (ch6-8).     *
 *   AdlibChannel_loadAny → playSoundData(pData, 0)                      *
 *   sub_1039C            → playSound() / playSoundData(pData, MIDWAY)   *
 *                                                                        *
 * loc_11D42 (cmd37) and loc_11D72 (cmd36) load channels in non-         *
 * sequential order — the sound data for each channel is not stored       *
 * contiguously. Block sizes are derived from the full sorted data map.   *
 *                                                                        *
 * commands 70/77/78 all reference the same data block at 0x40BA.        *
 * A dead sub (sub_11F09) and a dead random-pitch stub are ignored.       *
 *-----------------------------------------------------------------------*/

const ASound5::CommandPtr ASound5::_commandList[79] = {
	// commands 0–8  (asound_commands1)
	&ASound5::command0,  &ASound5::command1,  &ASound5::command2,  &ASound5::command3,
	&ASound5::command4,  &ASound5::command5,  &ASound5::command6,  &ASound5::command7,
	&ASound5::command8,
	// 9–15 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// command 16  (asound_commands2)
	&ASound5::command16,
	// 17–23 absent
	nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 24–27  (asound_commands3)
	&ASound5::command24, &ASound5::command25, &ASound5::command26, &ASound5::command27,
	// 28–31 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 32–39  (asound_commands4)
	&ASound5::command32, &ASound5::command33, &ASound5::command34, &ASound5::command35,
	&ASound5::command36, &ASound5::command37, &ASound5::command38, &ASound5::command39,
	// 40–63 absent
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	// commands 64–78  (asound_commands5)
	&ASound5::command64, &ASound5::command65, &ASound5::command66, &ASound5::command67,
	&ASound5::command68, &ASound5::command69, &ASound5::command70, &ASound5::command71,
	&ASound5::command72, &ASound5::command73, &ASound5::command74, &ASound5::command75,
	&ASound5::command76, &ASound5::command77, &ASound5::command78
};

ASound5::ASound5(Audio::Mixer *mixer, OPL::OPL *opl)
		: PhantomASound(mixer, opl, "asound.ph5", 0x2140) {
}

int ASound5::command(int commandId, int param) {
	if (commandId > 78 || !_commandList[commandId])
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

// commands 0–8: delegate to base ASound
int ASound5::command0() { return ASound::command0(); }
int ASound5::command1() { return ASound::command1(); }
int ASound5::command2() { return ASound::command2(); }
int ASound5::command3() { return ASound::command3(); }
int ASound5::command4() { return ASound::command4(); }
int ASound5::command5() { return ASound::command5(); }
int ASound5::command6() { return ASound::command6(); }
int ASound5::command7() { return ASound::command7(); }
int ASound5::command8() { return ASound::command8(); }

// ---------------------------------------------------------------------------
// command16 (sub_11E84) – isSoundActive guard, command1, load ch0–5
// ---------------------------------------------------------------------------
int ASound5::command16() {
	byte *pData = loadData(0x4142, 120);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x41BA, 146));
		_channels[2].load(loadData(0x424C, 133));
		_channels[3].load(loadData(0x42D1,  69));
		_channels[4].load(loadData(0x4316, 152));
		_channels[5].load(loadData(0x43AE,  14));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 24–27 (asound_commands3) – upper pool (sub_1039C)
// ---------------------------------------------------------------------------

// sub_11EE8
int ASound5::command24() {
	playSound(0x51FA, 51);
	playSound(0x522D, 46);
	return 0;
}

// sub_11EF5
int ASound5::command25() {
	playSound(0x525B, 44);
	playSound(0x5287, 46);
	return 0;
}

// sub_11F02
int ASound5::command26() {
	playSound(0x52B5, 12);
	return 0;
}

// sub_11E71 – three upper-pool voices
int ASound5::command27() {
	playSound(0x4040, 10);
	playSound(0x404A, 23);
	playSound(0x4061, 25);
	return 0;
}

// ---------------------------------------------------------------------------
// commands 32–39 (asound_commands4)
// ---------------------------------------------------------------------------

// sub_11EB4 – command1, eight loadAny (lower pool, AdlibChannel_loadAny)
int ASound5::command32() {
	ASound::command1();
	playSoundData(loadData(0x43BC, 689), 0);
	playSoundData(loadData(0x466D, 262), 0);
	playSoundData(loadData(0x4773, 480), 0);
	playSoundData(loadData(0x4953, 504), 0);
	playSoundData(loadData(0x4B4B, 584), 0);
	playSoundData(loadData(0x4D93, 308), 0);
	playSoundData(loadData(0x4EC7, 426), 0);
	playSoundData(loadData(0x5071, 357), 0);
	return 0;
}

// sub_11DA2 – isSoundActive guard, command1, load ch0–6
int ASound5::command33() {
	byte *pData = loadData(0x21C6, 609);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x2427, 652));
		_channels[2].load(loadData(0x26B3, 272));
		_channels[3].load(loadData(0x27C3, 558));
		_channels[4].load(loadData(0x29F1, 712));
		_channels[5].load(loadData(0x2CB9, 464));
		_channels[6].load(loadData(0x2E89,  21));
	}
	return 0;
}

// sub_11DD8 – isSoundActive guard, command1, load ch0–5 (non-sequential)
// ch1 and ch3 use data from the 0x4000 region; ch0/ch2/ch4 from 0x2E9E region
int ASound5::command34() {
	byte *pData = loadData(0x2E9E, 1521);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x4003,    7));
		_channels[2].load(loadData(0x348F, 1424));
		_channels[3].load(loadData(0x400A,    7));
		_channels[4].load(loadData(0x3A1F, 1508));
		_channels[5].load(loadData(0x4011,    9));
	}
	return 0;
}

// loc_11D72 – isSoundActive guard, command1, load ch0–5 (non-sequential)
// ch1/ch3/ch5 use data from the 0x2196 region interleaved with ch0/ch2/ch4
int ASound5::command35() {
	byte *pData = loadData(0x1D0A, 320);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x2196,  18));
		_channels[2].load(loadData(0x1E4A, 304));
		_channels[3].load(loadData(0x21A8,  18));
		_channels[4].load(loadData(0x1F7A, 540));
		_channels[5].load(loadData(0x21BA,  12));
	}
	return 0;
}

// loc_11D42 – isSoundActive guard, command1, load ch0–5 (non-sequential)
// ch4 and ch5 reuse blocks at 0x15EC and 0x18D9 that precede ch0's data
int ASound5::command36() {
	byte *pData = loadData(0x15F5, 740);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x18E2, 326));
		_channels[2].load(loadData(0x1A28, 561));
		_channels[3].load(loadData(0x1C59, 177));
		_channels[4].load(loadData(0x15EC,   9));
		_channels[5].load(loadData(0x18D9,   9));
	}
	return 0;
}

// loc_11D00 – isSoundActive guard, command1, load ch0–8
int ASound5::command37() {
	byte *pData = loadData(0x1190, 397);
	if (!isSoundActive(pData)) {
		ASound::command1();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x131D, 378));
		_channels[2].load(loadData(0x1497,  60));
		_channels[3].load(loadData(0x14D3,  64));
		_channels[4].load(loadData(0x1513,  44));
		_channels[5].load(loadData(0x153F,  44));
		_channels[6].load(loadData(0x156B,  50));
		_channels[7].load(loadData(0x159D,  52));
		_channels[8].load(loadData(0x15D1,  27));
	}
	return 0;
}

// loc_11CD0 – isSoundActive guard, command2 (lower-bank fade), load ch0–5
int ASound5::command38() {
	byte *pData = loadData(0x0C36, 329);
	if (!isSoundActive(pData)) {
		ASound::command2();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x0D7F, 201));
		_channels[2].load(loadData(0x0E48, 200));
		_channels[3].load(loadData(0x0F10, 162));
		_channels[4].load(loadData(0x0FB2, 228));
		_channels[5].load(loadData(0x1096, 250));
	}
	return 0;
}

// sub_11F10 – isSoundActive guard, command3 (lower-bank fade only), load ch0–5
int ASound5::command39() {
	byte *pData = loadData(0x5312, 599);
	if (!isSoundActive(pData)) {
		ASound::command3();
		_channels[0].load(pData);
		_channels[1].load(loadData(0x5569, 275));
		_channels[2].load(loadData(0x567C, 289));
		_channels[3].load(loadData(0x579D, 243));
		_channels[4].load(loadData(0x5890, 196));
		_channels[5].load(loadData(0x5954, 206));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// commands 64–78 (asound_commands5) – upper pool (sub_1039C) unless noted
// ---------------------------------------------------------------------------

// sub_11E08
int ASound5::command64() {
	playSound(0x4101, 10);
	return 0;
}

// sub_11E0E
int ASound5::command65() {
	playSound(0x401A, 18);
	return 0;
}

// sub_11E14
int ASound5::command66() {
	playSound(0x402C, 10);
	return 0;
}

// sub_11E1A
int ASound5::command67() {
	playSound(0x4036, 10);
	return 0;
}

// sub_11E21
int ASound5::command68() {
	playSound(0x407A, 18);
	return 0;
}

// sub_11E27
int ASound5::command69() {
	playSound(0x408C, 46);
	return 0;
}

// sub_11E2D – also shared by command77 and command78
int ASound5::command70() {
	playSound(0x40BA, 14);
	return 0;
}

// sub_11E33
int ASound5::command71() {
	playSound(0x40C8, 10);
	return 0;
}

// sub_11E39
int ASound5::command72() {
	playSound(0x40D2, 10);
	return 0;
}

// sub_11E3F
int ASound5::command73() {
	playSound(0x40DC, 11);
	playSound(0x40E7, 26);
	return 0;
}

// sub_11E4C
int ASound5::command74() {
	playSound(0x410B, 20);
	return 0;
}

// sub_11E52
int ASound5::command75() {
	playSound(0x4129, 11);
	playSound(0x4134, 14);
	return 0;
}

// sub_11E5F – same block as command70
int ASound5::command76() {
	return command70();
}

// sub_11E65 – same block as command70
int ASound5::command77() {
	return command70();
}

// sub_11E6B
int ASound5::command78() {
	playSound(0x411F, 10);
	return 0;
}

/*-----------------------------------------------------------------------*/

const ASound9::CommandPtr ASound9::_commandList[72] = {
	&ASound9::command0,  &ASound9::command1,  &ASound9::command2,  &ASound9::command3,
	&ASound9::command4,  &ASound9::command5,  &ASound9::command6,  &ASound9::command7,
	&ASound9::command8,  nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound9::command16, nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound9::command24, &ASound9::command25, &ASound9::command26, &ASound9::command27,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound9::command32, &ASound9::command33, &ASound9::command34, &ASound9::command35,
	&ASound9::command36, &ASound9::command37, &ASound9::command38, &ASound9::command39,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	nullptr,             nullptr,             nullptr,             nullptr,
	&ASound9::command64, &ASound9::command65, &ASound9::command66, &ASound9::command67,
	&ASound9::command68, &ASound9::command69, &ASound9::command70, &ASound9::command71
};

ASound9::ASound9(Audio::Mixer *mixer, OPL::OPL *opl) : PhantomASound(mixer, opl, "asound.ph9", 0x20c0) {
}

int ASound9::command(int commandId, int param) {
	if (commandId > 71 || !_commandList[commandId])
		return 0;

	_commandParam = param;
	_frameCounter = 0;
	return (this->*_commandList[commandId])();
}

int ASound9::command0() {
	return 0;
}

int ASound9::command1() {
	return 0;
}

int ASound9::command2() {
	return 0;
}

int ASound9::command3() {
	return 0;
}

int ASound9::command4() {
	return 0;
}

int ASound9::command5() {
	return 0;
}

int ASound9::command6() {
	return 0;
}

int ASound9::command7() {
	return 0;
}

int ASound9::command8() {
	return 0;
}

int ASound9::command24() {
	return 0;
}

int ASound9::command25() {
	return 0;
}

int ASound9::command26() {
	return 0;
}

int ASound9::command27() {
	return 0;
}

int ASound9::command32() {
	return 0;
}

int ASound9::command33() {
	return 0;
}

int ASound9::command34() {
	return 0;
}

int ASound9::command35() {
	return 0;
}

int ASound9::command36() {
	return 0;
}

int ASound9::command37() {
	return 0;
}

int ASound9::command38() {
	return 0;
}

int ASound9::command39() {
	return 0;
}

int ASound9::command64() {
	return 0;
}

int ASound9::command65() {
	return 0;
}

int ASound9::command66() {
	return 0;
}

int ASound9::command67() {
	return 0;
}

int ASound9::command68() {
	return 0;
}

int ASound9::command69() {
	return 0;
}

int ASound9::command70() {
	return 0;
}

int ASound9::command71() {
	return 0;
}

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
