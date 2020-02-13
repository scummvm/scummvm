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

/* OPL implementation for hardware OPL using ALSA Direct FM API.
 *
 * Caveats and limitations:
 * - Pretends to be a softsynth (emitting silence).
 * - Dual OPL2 mode requires OPL3 hardware.
 * - Every register write leads to a series of register writes on the hardware,
 *   due to the lack of direct register access in the ALSA Direct FM API.
 * - No timers
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/scummsys.h"

#include "common/debug.h"
#include "common/str.h"
#include "audio/fmopl.h"

#include <sys/ioctl.h>
#include <alsa/asoundlib.h>
#include <sound/asound_fm.h>

namespace OPL {
namespace ALSA {

class OPL : public ::OPL::RealOPL {
private:
	enum {
		kOpl2Voices = 9,
		kVoices = 18,
		kOpl2Operators = 18,
		kOperators = 36
	};

	Config::OplType _type;
	int _iface;
	snd_hwdep_t *_opl;
	snd_dm_fm_voice _oper[kOperators];
	snd_dm_fm_note _voice[kVoices];
	snd_dm_fm_params _params;
	int index[2];
	static const int voiceToOper0[kVoices];
	static const int regOffsetToOper[0x20];

	void writeOplReg(int c, int r, int v);
	void clear();

public:
	OPL(Config::OplType type);
	~OPL();

	bool init();
	void reset();

	void write(int a, int v);
	byte read(int a);

	void writeReg(int r, int v);
};

const int OPL::voiceToOper0[OPL::kVoices] =
	{ 0, 1, 2, 6, 7, 8, 12, 13, 14, 18, 19, 20, 24, 25, 26, 30, 31, 32 };

const int OPL::regOffsetToOper[0x20] =
	{ 0,  1,  2,  3,  4,  5, -1, -1, 6, 7, 8, 9, 10, 11, -1, -1,
	 12, 13, 14, 15, 16, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

OPL::OPL(Config::OplType type) : _type(type), _opl(nullptr), _iface(0) {
}

OPL::~OPL() {
	stop();

	if (_opl) {
		snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_RESET, nullptr);
		snd_hwdep_close(_opl);
	}
}

void OPL::clear() {
	index[0] = index[1] = 0;

	memset(_oper, 0, sizeof(_oper));
	memset(_voice, 0, sizeof(_voice));
	memset(&_params, 0, sizeof(_params));

	for (int i = 0; i < kOperators; ++i) {
		_oper[i].op = (i / 3) % 2;
		_oper[i].voice = (i / 6) * 3 + (i % 3);
	}

	for (int i = 0; i < kVoices; ++i)
		_voice[i].voice = i;

	// For OPL3 hardware we need to set up the panning in OPL2 modes
	if (_iface == SND_HWDEP_IFACE_OPL3) {
		if (_type == Config::kDualOpl2) {
			for (int i = 0; i < kOpl2Operators; ++i)
				_oper[i].left = 1; // FIXME below
			for (int i = kOpl2Operators; i < kOperators; ++i)
				_oper[i].right = 1;
		} else if (_type == Config::kOpl2) {
			for (int i = 0; i < kOpl2Operators; ++i) {
				_oper[i].left = 1;
				_oper[i].right = 1;
			}
		}
	}
}

bool OPL::init() {
	clear();

	int card = -1;
	snd_ctl_t *ctl;
	snd_hwdep_info_t *info;
	snd_hwdep_info_alloca(&info);

	int iface = SND_HWDEP_IFACE_OPL3;
	if (_type == Config::kOpl2)
		iface = SND_HWDEP_IFACE_OPL2;

	// Look for OPL hwdep interface
	while (!snd_card_next(&card) && card >= 0) {
		int dev = -1;
		Common::String name = Common::String::format("hw:%d", card);

		if (snd_ctl_open(&ctl, name.c_str(), 0) < 0)
			continue;

		while (!snd_ctl_hwdep_next_device(ctl, &dev) && dev >= 0) {
			name = Common::String::format("hw:%d,%d", card, dev);

			if (snd_hwdep_open(&_opl, name.c_str(), SND_HWDEP_OPEN_WRITE) < 0)
				continue;

			if (!snd_hwdep_info(_opl, info)) {
				int found = snd_hwdep_info_get_iface(info);
				// OPL3 can be used for (Dual) OPL2 mode
				if (found == iface || found == SND_HWDEP_IFACE_OPL3) {
					snd_ctl_close(ctl);
					_iface = found;
					reset();
					return true;
				}
			}

			// Wrong interface, try next device
			snd_hwdep_close(_opl);
			_opl = nullptr;
		}

		snd_ctl_close(ctl);
	}

	return false;
}

void OPL::reset() {
	snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_RESET, nullptr);
	if (_iface == SND_HWDEP_IFACE_OPL3)
		snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_SET_MODE, (void *)SNDRV_DM_FM_MODE_OPL3);

	clear();

	// Sync up with the hardware
	snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_SET_PARAMS, (void *)&_params);
	for (uint i = 0; i < (_iface == SND_HWDEP_IFACE_OPL3 ? kVoices : kOpl2Voices); ++i)
		snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_PLAY_NOTE, (void *)&_voice[i]);
	for (uint i = 0; i < (_iface == SND_HWDEP_IFACE_OPL3 ? kOperators : kOpl2Operators); ++i)
		snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_SET_VOICE, (void *)&_oper[i]);
}

void OPL::write(int port, int val) {
	val &= 0xff;
	int chip = (port & 2) >> 1;

	if (port & 1) {
		switch(_type) {
		case Config::kOpl2:
			writeOplReg(0, index[0], val);
			break;
		case Config::kDualOpl2:
			if (port & 8) {
				writeOplReg(0, index[0], val);
				writeOplReg(1, index[1], val);
			} else
				writeOplReg(chip, index[chip], val);
			break;
		case Config::kOpl3:
			writeOplReg(chip, index[chip], val);
			break;
		default:
			break;
		}
	} else {
		switch(_type) {
		case Config::kOpl2:
			index[0] = val;
			break;
		case Config::kDualOpl2:
			if (port & 8) {
				index[0] = val;
				index[1] = val;
			} else
				index[chip] = val;
			break;
		case Config::kOpl3:
			index[chip] = val;
			break;
		default:
			break;
		}
	}
}

byte OPL::read(int port) {
	return 0;
}

void OPL::writeReg(int r, int v) {
	switch (_type) {
	case Config::kOpl2:
		writeOplReg(0, r, v);
		break;
	case Config::kDualOpl2:
		writeOplReg(0, r, v);
		writeOplReg(1, r, v);
		break;
	case Config::kOpl3:
		writeOplReg(r >= 0x100, r & 0xff, v);
		break;
	default:
		break;
	}
}

void OPL::writeOplReg(int c, int r, int v) {
	if (r == 0x04 && c == 1 && _type == Config::kOpl3) {
		snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_SET_CONNECTION, reinterpret_cast<void *>(v & 0x3f));
	} else if (r == 0x08 && c == 0) {
		_params.kbd_split = (v >> 6) & 0x1;
		snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_SET_PARAMS, (void *)&_params);
	} else if (r == 0xbd && c == 0) {
		_params.hihat = v & 0x1;
		_params.cymbal = (v >> 1) & 0x1;
		_params.tomtom = (v >> 2) & 0x1;
		_params.snare = (v >> 3) & 0x1;
		_params.bass = (v >> 4) & 0x1;
		_params.rhythm = (v >> 5) & 0x1;
		_params.vib_depth = (v >> 6) & 0x1;
		_params.am_depth = (v >> 7) & 0x1;
		snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_SET_PARAMS, (void *)&_params);
	} else if (r < 0xa0 || r >= 0xe0) {
		// Operator
		int idx = regOffsetToOper[r & 0x1f];

		if (idx == -1)
			return;

		if (c == 1)
			idx += kOpl2Operators;

		switch (r & 0xf0) {
		case 0x20:
		case 0x30:
			_oper[idx].harmonic = v & 0xf;
			_oper[idx].kbd_scale = (v >> 4) & 0x1;
			_oper[idx].do_sustain = (v >> 5) & 0x1;
			_oper[idx].vibrato = (v >> 6) & 0x1;
			_oper[idx].am = (v >> 7) & 0x1;
			snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_SET_VOICE, (void *)&_oper[idx]);
			break;
		case 0x40:
		case 0x50:
			_oper[idx].volume = ~v & 0x3f;
			_oper[idx].scale_level = (v >> 6) & 0x3;
			snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_SET_VOICE, (void *)&_oper[idx]);
			break;
		case 0x60:
		case 0x70:
			_oper[idx].decay = v & 0xf;
			_oper[idx].attack = (v >> 4) & 0xf;
			snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_SET_VOICE, (void *)&_oper[idx]);
			break;
		case 0x80:
		case 0x90:
			_oper[idx].release = v & 0xf;
			_oper[idx].sustain = (v >> 4) & 0xf;
			snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_SET_VOICE, (void *)&_oper[idx]);
			break;
		case 0xe0:
		case 0xf0:
			_oper[idx].waveform = v & (_type == Config::kOpl3 ? 0x7 : 0x3);
			snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_SET_VOICE, (void *)&_oper[idx]);
			break;
		default:
			break;
		}
	} else {
		// Voice
		int idx = r & 0xf;

		if (idx >= kOpl2Voices)
			return;

		if (c == 1)
			idx += kOpl2Voices;

		int opIdx = voiceToOper0[idx];

		switch (r & 0xf0) {
		case 0xa0:
			_voice[idx].fnum = (_voice[idx].fnum & 0x300) | (v & 0xff);
			snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_PLAY_NOTE, (void *)&_voice[idx]);
			break;
		case 0xb0:
			_voice[idx].fnum = ((v << 8) & 0x300) | (_voice[idx].fnum & 0xff);
			_voice[idx].octave = (v >> 2) & 0x7;
			_voice[idx].key_on = (v >> 5) & 0x1;
			snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_PLAY_NOTE, (void *)&_voice[idx]);
			break;
		case 0xc0:
			_oper[opIdx].connection = _oper[opIdx + 3].connection = v & 0x1;
			_oper[opIdx].feedback = _oper[opIdx + 3].feedback = (v >> 1) & 0x7;
			if (_type == Config::kOpl3) {
				_oper[opIdx].left = _oper[opIdx + 3].left = (v >> 4) & 0x1;
				_oper[opIdx].right = _oper[opIdx + 3].right = (v >> 5) & 0x1;
			}
			snd_hwdep_ioctl(_opl, SNDRV_DM_FM_IOCTL_SET_VOICE, (void *)&_oper[opIdx]);
			break;
		default:
			break;
		}
	}
}

OPL *create(Config::OplType type) {
	return new OPL(type);
}

} // End of namespace ALSA
} // End of namespace OPL
