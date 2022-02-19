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

#include "common/system.h"
#include "common/memstream.h"
#include "chewy/chewy.h"
#include "chewy/events.h"
#include "chewy/file.h"
#include "chewy/flic.h"
#include "chewy/globals.h"
#include "chewy/sound.h"

namespace Chewy {

#define SOUND_SLOT_SIZE 500000

void decode_flc(byte *vscr, const byte *dbuf) {
	Common::MemoryReadStream src(dbuf, 0xffffffff);
	int h = src.readUint16LE();
	int val;

	for (; h > 0; --h, vscr += SCREEN_WIDTH) {
		// Get value, and handle any line skips if needed
		for (val = src.readUint16LE(); val & 0x4000; val = src.readUint16LE()) {
			vscr += (-val * SCREEN_WIDTH) & 0xffff;
		}

		// Iteration for pixels within the line
		byte *dest = vscr;
		for (; val > 0; --val) {
			dest += src.readByte();
			int8 b = src.readSByte();

			if (b < 0) {
				b = -b;
				int pair = src.readUint16LE();
				for (; b > 0; --b, dest += 2)
					WRITE_LE_UINT16(dest, pair);

			} else {
				for (; b > 0; --b, dest += 2)
					WRITE_LE_UINT16(dest, src.readUint16LE());
			}
		}
	}
}

void decode_rle(byte *vscr, const byte *dbuf, int br, int h) {
	for (; h > 0; --h, vscr += SCREEN_WIDTH) {
		byte *dest = vscr;
		++dbuf;		// Skip number of entries in line

		int x;
		for (x = 0; x < br; ) {
			int8 len = (int8)*dbuf++;
			if (len < 0) {
				// Copy a number of bytes specified in lower 7 bits
				len = -len;
				if (len) {
					Common::copy(dbuf, dbuf + len, dest);
					dbuf += len;
					dest += len;
					x += len;
				}
			} else {
				// Run length in the lower 7 bits of the next byte
				byte v = *dbuf++;
				if (len) {
					Common::fill(dest, dest + len, v);
					x += len;
					dest += len;
				}
			}
		}

		assert(x == br);
	}
}


Flic::Flic() {
	Common::fill(&_sounds[0], &_sounds[50], (byte *)nullptr);
	_soundBuffer = new byte[SOUND_SLOT_SIZE];
}

Flic::~Flic() {
	delete[] _soundBuffer;
}

int16 Flic::decode_frame() {
	ChunkHead chunk_header;
	int16 action_ret = 0;

	byte *tmp_buf = _loadBuffer;
	bool update_flag = false;
	if (_frameHeader.chunks != 0) {
		_fadeFlag = false;
		for (uint16 i = 0; i < _frameHeader.chunks; i++) {
			Common::MemoryReadStream rs(tmp_buf, ChunkHead::SIZE());
			chunk_header.load(&rs);

			tmp_buf += ChunkHead::SIZE();
			chunk_header.size -= ChunkHead::SIZE();

			switch (chunk_header.type) {
			case COLOR_256:
				col256_chunk(tmp_buf);
				break;

			case COLOR_64:
				col64_chunk(tmp_buf);
				break;

			case BYTE_RUN:
				decode_rle(_virtScreen, tmp_buf,
				           (int)_flicHeader.width,
				           (int)_flicHeader.height);
				update_flag = true;
				break;

			case DELTA_FLC:
				decode_flc(_virtScreen, tmp_buf);
				update_flag = true;
				break;

			case DELTA_FLI:
				delta_chunk_byte(tmp_buf);

				update_flag = true;
				break;

			case CLS:
				_G(out)->set_pointer(_virtScreen);
				_G(out)->cls();
				_G(out)->set_pointer(nullptr);

				update_flag = true;
				break;

			case UNPRESSED:
				_G(out)->back2back(_loadBuffer, _virtScreen);

				update_flag = true;
				break;

			case PSTAMP:
				break;

			default:
				update_flag = true;
				break;
			}

			tmp_buf += chunk_header.size;
		}
		if (update_flag != false) {
			if (_flicUser) {
				_G(out)->set_pointer(_virtScreen);
				action_ret = _flicUser(_currentFrame);
				_G(out)->set_pointer(nullptr);
			}
			_G(out)->back2screen(_virtScreen - 4);
			if (_fadeFlag != false) {
				_G(out)->einblenden(_fadePal, _fadeDelay);
				_fadeFlag = false;
			}
		}
	}

	return action_ret;
}

void Flic::col256_chunk(byte *tmp) {
	int packets = *(int16 *)tmp;
	tmp += 2;

	if (_clsFlag == true)
		_G(out)->cls();
	else
		_clsFlag = true;

	if (tmp[1] == 0) {
		tmp += 2;
		for (int i = 0; i < PALETTE_SIZE; i++)
			tmp[i] >>= 2;
		if (_fadeFlag == false)
			_G(out)->set_palette(tmp);
		else {
			memset(_fadePal, 0, PALETTE_SIZE);
			_G(out)->set_palette(_fadePal);
			memcpy(_fadePal, tmp, PALETTE_SIZE);
		}
	} else {
		byte col = 0;
		for (int count = 0; count < packets; count++) {
			col += *tmp++;
			byte anz = *tmp++;
			for (int i = 0; i < anz; i++) {
				byte r = *tmp++ >> 2;
				byte g = *tmp++ >> 2;
				byte b = *tmp++ >> 2;
				_G(out)->raster_col(col, r, g, b);
				++col;
			}
		}
	}
}

void Flic::col64_chunk(byte *tmp) {
	int packets = *((int16 *)tmp);
	tmp += 2;

	if (_clsFlag == true)
		_G(out)->cls();
	else
		_clsFlag = true;

	if (!tmp[1]) {
		if (_fadeFlag == false)
			_G(out)->set_palette(tmp + 2);
		else {
			memset(_fadePal, 0, PALETTE_SIZE);
			_G(out)->set_palette(_fadePal);
			memcpy(_fadePal, tmp + 2, PALETTE_SIZE);
		}
	} else {
		byte col = 0;
		for (int count = 0; count < packets; count++) {
			col += *tmp++;
			byte anz = *tmp++;
			for (int i = 0; i < anz; i++) {
				byte r = *tmp++ >> 2;
				byte g = *tmp++ >> 2;
				byte b = *tmp++ >> 2;
				_G(out)->raster_col(col, r, g, b);
				++col;
			}
		}
	}
}

void Flic::delta_chunk_byte(byte *tmp) {
	byte last_byte = 0;
	bool last_flag;
	byte *abl = _virtScreen;
	short int *ipo = (short int *)tmp;
	short int rest_height = *ipo++;
	tmp += 2;
	for (short int i = 0; (i < rest_height) && (i < 200); i++) {
		byte *tabl = abl;
		short int mode_word = *ipo++;
		if (mode_word & 0x4000) {
			mode_word = -mode_word;
			abl += (int16)(mode_word * _flicHeader.width);
			tabl = abl;
			mode_word = *ipo++;
		}
		if (mode_word & 0x8000) {
			last_byte = (byte)(mode_word & 0xff);
			last_flag = true;
			mode_word = *ipo++;
		} else
			last_flag = false;
		tmp = (byte *)ipo;
		if (mode_word) {
			short int pcount = 0;
			for (short int j = 0; (pcount < mode_word) && (j <= _flicHeader.width); ++pcount) {
				byte skip = *tmp++;
				abl += skip;
				signed char tmp_count = (signed char)*tmp++;
				short signed int count = (short signed int)tmp_count;
				if (count > 0) {
					count <<= 1;
					while ((count) && (j < _flicHeader.width)) {
						*abl++ = *tmp++;
						++j;
						--count;
					}
				} else {
					count = -count;
					short int data = *(short int *)tmp;
					tmp += 2;
					while ((count > 0) && (j < _flicHeader.width)) {
						*((short int *)abl) = data;
						abl += 2;
						j += 2;
						--count;
					}
				}
			}
			if (last_flag)
				*abl++ = last_byte;
		}
		abl = tabl + _flicHeader.width;
		ipo = (short int *)tmp;
	}
}

int16 Flic::custom_play(CustomInfo *ci) {
	int16 ret = 0;

	_cInfo = ci;
	_loadBuffer = ci->TempArea;
	_virtScreen = ci->VirtScreen + 4;
	_music = ci->MusicSlot;
	_sound = _soundBuffer;

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(ci->Handle);
	if (rs) {
		if (_customHeader.load(rs)) {
			if (!scumm_strnicmp(_customHeader.id, "CFO", 4)) {
				_clsFlag = false;
				_fadeFlag = false;
				_fadeDelay = 0;
				_currentFrame = 0;

				for (uint16 i = 0; (i < _customHeader.frames) && (!_G(modul)) && (ret >= 0); i++) {
					if (!_customFrame.load(rs)) {
						error("flic error");
					} else {
						if ((_customFrame.type != PREFIX) && (_customFrame.type != CUSTOM)) {
							uint32 start = g_system->getMillis() + _customHeader.speed;
							if (_customFrame.size) {
								if (rs->read(_loadBuffer, _customFrame.size) != _customFrame.size) {
									error("flic error");
								} else {
									ret = decode_cframe();
								}
							}

							// Show the next frame
							g_screen->update();

							// Loop until the frame time expires
							uint32 ende;
							do {
								ende = g_system->getMillis();
								g_events->update();
								SHOULD_QUIT_RETURN0;
							} while (ende <= start);
							++_currentFrame;

						} else if (_customFrame.type == CUSTOM) {
							decode_custom_frame(
								dynamic_cast<Common::SeekableReadStream *>(ci->Handle));

						} else {
							_G(out)->raster_col(255, 63, 63, 63);
							_G(out)->printxy(0, 0, 255, 0, 0, "Unknown Frame Type");
							taste;
						}
					}
				}
			}
		}
	} else {
		error("flic error");
	}

	return ret;
}

void Flic::decode_custom_frame(Common::SeekableReadStream *handle) {
	uint16 para[10];
	TmfHeader *th = (TmfHeader *)_music;

	for (uint16 i = 0; (i < _customFrame.chunks) && (!_G(modul)); i++) {
		ChunkHead chead;
		if (!chead.load(handle)) {
			error("flic error");
		}

		switch (chead.type) {
		case FADE_IN:
			error("decode_custom_frame: Unused frame type FADE_IN found");
			break;

		case FADE_OUT:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				error("flic error");
			} else
				_G(out)->ausblenden(para[0]);
			break;

		case LOAD_MUSIC:
			if (handle->read(_music, chead.size) != chead.size) {
				error("flic error");
			} else {
				byte *tmp = _music;
				tmp += sizeof(TmfHeader);
				tmp += ((uint32)th->pattern_anz) * 1024l;
				for (uint16 j = 0; j < 31; j++) {
					if (th->instrument[j].laenge) {
						th->ipos[j] = tmp;
						tmp += th->instrument[j].laenge;
					}
				}
			}

			break;

		case LOAD_RAW:
			error("decode_custom_frame: Unused frame type LOAD_RAW found");
			break;

		case LOAD_VOC:
			if (!File::readArray(handle, &para[0], 1) ||
				handle->read(_sound, chead.size - 2) != (chead.size - 2)) {
				error("flic error");
			} else {
				_sounds[para[0]] = _sound;
				_sSize[para[0]] = chead.size - 2;
				_sound += chead.size;
			}
			break;

		case PLAY_MUSIC:
			if (!strncmp(th->id, "TMF", 4))
#ifndef AIL
				snd->playMod(th);
#else
				_G(sndPlayer)->playMod(th);
#endif

			break;

		case PLAY_SEQ:
			error("decode_custom_frame: Unused frame type PLAY_SEQ found");
			break;

		case PLAY_PATTERN:
			error("decode_custom_frame: Unused frame type PLAY_PATTERN found");
			break;

		case STOP_MUSIC:
#ifndef AIL
			snd->stopMod();
#else
			_G(sndPlayer)->stopMod();
#endif
			break;

		case WAIT_MSTOP: {
			musik_info mi;
			do {
#ifndef AIL
				snd->getMusicInfo(&mi);
#else
				_G(sndPlayer)->getMusicInfo(&mi);
#endif
			} while (mi.musik_playing != 0);
			}
			break;

		case SET_MVOL:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				error("flic error");
			} else
				g_engine->_sound->setMusicVolume(para[0] * Audio::Mixer::kMaxChannelVolume / 120);
			break;

		case SET_LOOPMODE:
			error("decode_custom_frame: Unused frame type SET_LOOPMODE found");
			break;

		case PLAY_RAW:
			break;

		case PLAY_VOC:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				error("flic error");
			} else {
				uint16 number = para[0];
				uint16 channel = para[1];
				uint16 volume = para[2] * Audio::Mixer::kMaxChannelVolume / 63;
				uint16 repeat = para[3];
				assert(number < MAX_SOUND_EFFECTS);

				Chewy::Sound *sound = g_engine->_sound;
				sound->setSoundVolume(volume);
				sound->playSound(_sounds[number], _sSize[number], channel, repeat, DisposeAfterUse::NO);
			}
			break;

		case SET_SVOL:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				error("flic error");
			} else
				g_engine->_sound->setSoundVolume(para[0]);
			break;

		case SET_CVOL:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				error("flic error");
			} else
				g_engine->_sound->setSoundChannelVolume(para[0], para[1]);
			break;

		case FREE_EFFECT:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				error("flic error");
			} else
				free_sound(para[0]);
			break;

		case MFADE_IN:
			error("decode_custom_frame: Unused frame type MFADE_IN found");
			break;

		case MFADE_OUT:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				error("flic error");
			} else
#ifndef AIL
				snd->fadeOut(para[0]);
#else
				_G(sndPlayer)->fadeOut(para[0]);
#endif
			break;

		case SET_STEREO:
			if (!File::readArray(handle, &para[0], chead.size / 2)) {
				error("flic error");
			} else
				g_engine->_sound->setSoundChannelBalance(para[0], para[1]);
			break;

		case SET_SPEED:
			error("decode_custom_frame: Unused frame type SET_SPEED found");
			break;

		case CLEAR_SCREEN:
			_G(out)->set_pointer(_virtScreen);
			_G(out)->cls();
			_G(out)->set_pointer(nullptr);
			_G(out)->cls();
			break;

		default:
			_G(out)->printxy(0, 10, 255, 0, 0, "Unknown Chunk %d ", chead.type);
			break;
		}
	}
}

int16 Flic::decode_cframe() {
	byte *tmp_buf = _loadBuffer;
	int16 update_flag = false;
	int16 action_ret = 0;

	if (_customFrame.chunks != 0) {
		for (uint16 i = 0; i < _customFrame.chunks; i++) {
			Common::MemoryReadStream rs(tmp_buf, ChunkHead::SIZE());
			ChunkHead chunk_header;
			chunk_header.load(&rs);

			tmp_buf += ChunkHead::SIZE();
			chunk_header.size -= ChunkHead::SIZE();

			switch (chunk_header.type) {
			case COLOR_256:
				col256_chunk(tmp_buf);
				break;

			case COLOR_64:
				break;

			case BYTE_RUN:
				decode_rle(_virtScreen, tmp_buf,
				    _customHeader.width, _customHeader.height);
				update_flag = true;
				break;

			case DELTA_FLC:
				decode_flc(_virtScreen, tmp_buf);
				update_flag = true;

				break;
			case DELTA_FLI:
				break;

			case CLS:
				_G(out)->set_pointer(_virtScreen);
				_G(out)->cls();
				_G(out)->set_pointer(nullptr);

				update_flag = true;
				break;

			case UNPRESSED:
				_G(out)->back2back(_loadBuffer, _virtScreen);

				update_flag = true;
				break;

			case PSTAMP:
				break;

			default:
				_G(out)->raster_col(255, 63, 63, 63);
				_G(out)->printxy(0, 0, 255, 0, 0, "Unknown CHUNK");

				update_flag = true;
				break;
			}

			tmp_buf += chunk_header.size;
		}

		if (update_flag != false) {
			if (_customUser) {
				_G(out)->back2back(_virtScreen, _loadBuffer);
				_G(out)->set_pointer(_virtScreen);
				action_ret = _customUser(_currentFrame);
				_G(out)->set_pointer(nullptr);
				_G(out)->back2screen(_virtScreen - 4);
				_G(out)->back2back(_loadBuffer, _virtScreen);
			} else
				_G(out)->back2screen(_virtScreen - 4);
			if (_fadeFlag != false) {
				_G(out)->einblenden(_fadePal, _fadeDelay);
				_fadeFlag = false;
			}
		}
	}

	return action_ret;
}

void Flic::free_sound(int16 nr) {
	byte *fsound = _sounds[nr];
	long fsize = _sSize[nr];
	if ((fsound != 0) && (fsize != 0)) {
		long copysize = SOUND_SLOT_SIZE;
		copysize -= (long)(fsound - _soundBuffer);
		memmove(fsound, fsound + fsize, copysize);
		for (int16 i = 0; i < 50; i++) {
			if (_sounds[i] == fsound) {
				_sounds[i] = 0;
				_sSize[i] = 0;
			} else if (_sounds[i] > fsound)
				_sounds[i] -= fsize;
		}
		_sound -= fsize;
	}
}

void Flic::set_custom_user_function(int16(*user_funktion)(int16 frame)) {
	_customUser = user_funktion;
}

void Flic::remove_custom_user_function() {
	_customUser = nullptr;
}

void Flic::set_flic_user_function(int16(*user_funktion)(int16 frame)) {
	_flicUser = user_funktion;
}

void Flic::remove_flic_user_function() {
	_flicUser = nullptr;
}

} // namespace Chewy
