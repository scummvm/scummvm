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

#include "common/memstream.h"
#include "chewy/ngshext.h"
#include "chewy/atds.h"
#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/file.h"
#include "chewy/globals.h"
#include "chewy/sound.h"

namespace Chewy {

bool AtsTxtHeader::load(Common::SeekableReadStream *src) {
	TxtNr = src->readUint16LE();
	AMov = src->readSint16LE();
	CurNr = src->readSint16LE();
	src->skip(2);

	return true;
}

bool InvUse::load(Common::SeekableReadStream *src) {
	ObjId = src->readSint16LE();
	ObjNr = src->readSint16LE();
	TxtNr = src->readSint16LE();
	return true;
}

bool AadInfo::load(const void *src) {
	Common::MemoryReadStream rs((const byte *)src, 6);

	X = rs.readSint16LE();
	Y = rs.readSint16LE();
	Color = rs.readSint16LE();
	return true;
}

bool AadTxtHeader::load(const void *src) {
	Common::MemoryReadStream rs((const byte *)src, 8);

	DiaNr = rs.readSint16LE();
	PerAnz = rs.readSint16LE();
	AMov = rs.readSint16LE();
	CurNr = rs.readSint16LE();
	return true;
}

bool AtsStrHeader::load(Common::SeekableReadStream *src) {
	VocNr = src->readUint16LE();
	return true;
}


Atdsys::Atdsys() {
	SplitStringInit init_ssi = { nullptr, 0, 0, 220, 4, SPLIT_MITTE, 8, 8,};
	_aadv.Dialog = false;
	_aadv.StrNr = -1;
	_aadv.SilentCount = false;
	_adsv.Dialog = -1;
	_adsv.AutoDia = false;
	_adsv.StrNr = -1;
	_adsv.SilentCount = false;
	_atsv.Display = false;
	_atsv.SilentCount = false;
	_atdsv.Delay = &_tmpDelay;
	_tmpDelay = 1;
	_atdsv.Silent = false;
	_atdsv.Display = DISPLAY_TXT;
	_atdsv.DiaNr = -1;
	_atdsv.aad_str = 0;
	_atdsv.VocNr = -1;
	_atdsv._eventsEnabled = true;
	_ssret.Next = false;
	_ssr = &_ssret;
	for (int16 i = 0; i < AAD_MAX_PERSON; i++)
		_ssi[i] = init_ssi;
	_invBlockNr = -1;
	_invUseMem = nullptr;
	_inzeig = _G(in)->get_in_zeiger();
}

Atdsys::~Atdsys() {
	for (int16 i = 0; i < MAX_HANDLE; i++)
		close_handle(i);

	if (_invUseMem)
		free(_invUseMem);
}

void Atdsys::set_font(byte *font_adr, int16 fvorx, int16 fhoehe) {
	_atdsv.Font = font_adr;
	_atdsv.Fvorx = fvorx;
	_atdsv.Fhoehe = fhoehe;
}

void Atdsys::set_delay(int16 *delay, int16 silent) {
	_atdsv.Delay = delay;
	_atdsv.Silent = silent;
}

void Atdsys::set_string_end_func
(void (*str_func)(int16 dia_nr, int16 str_nr, int16 person_nr, int16 mode)) {
	_atdsv.aad_str = str_func;
}

void Atdsys::setHasSpeech(bool hasSpeech) {
	_hasSpeech = hasSpeech;
	updateSoundSettings();
}

void Atdsys::updateSoundSettings() {
	if (!_hasSpeech)
		_atdsv.Display = DISPLAY_TXT;
	else
		_atdsv.Display = g_engine->_sound->getSpeechSubtitlesMode();
}

int16 Atdsys::get_delay(int16 txt_len) {
	int16 z_len = (_ssi->Width / _ssi->Fvorx) + 1;
	int16 max_len = z_len * _ssi->Zeilen;
	if (txt_len > max_len)
		txt_len = max_len;

	int16 ret = *_atdsv.Delay * (txt_len + z_len);
	return ret;
}

SplitStringRet *Atdsys::split_string(SplitStringInit *ssi_) {
	_ssret.Anz = 0;
	_ssret.Next = false;
	_ssret.StrPtr = _splitPtr;
	_ssret.X = _splitX;
	int16 zeichen_anz = (ssi_->Width / ssi_->Fvorx) + 1;
	memset(_splitPtr, 0, sizeof(char *)*MAX_STR_SPLIT);
	calc_txt_win(ssi_);
	char *str_adr = ssi_->Str;
	int16 count = 0;
	int16 tmp_count = 0;
	bool ende = false;
	char *start_adr = str_adr;

	while (!ende) {
		switch (*str_adr) {
		case 0:
			if (str_adr[1] != ATDS_END_TEXT) {
				str_adr[0] = ' ';
			}
			// Fall through
		case 0x20:
			if (count < zeichen_anz && *str_adr == 0) {

				tmp_count = count;
			}
			if (count < zeichen_anz && *str_adr != 0) {

				tmp_count = count;
				++str_adr;
				++count;
			} else {
				_splitPtr[_ssret.Anz] = start_adr;
				start_adr[tmp_count] = 0;
				if (ssi_->Mode == SPLIT_MITTE)
					_splitX[_ssret.Anz] = ssi_->X + ((ssi_->Width - (strlen(start_adr) * ssi_->Fvorx)) >> 1);
				else
					_splitX[_ssret.Anz] = ssi_->X;
				++_ssret.Anz;
				if (_ssret.Anz == ssi_->Zeilen) {
					ende = true;
					bool ende1 = false;
					while (!ende1) {
						if (*str_adr == ATDS_END_TEXT)
							ende1 = true;
						else if (*str_adr != ' ' && *str_adr != 0) {
							ende1 = true;
							_ssret.Next = true;
						}
						++str_adr;
					}
				} else if (*str_adr == 0 && count < zeichen_anz) {
					ende = true;
				} else {
					str_adr = start_adr + tmp_count + 1;
					start_adr = str_adr;
					count = 0;
					tmp_count = 0;
				}
			}
			break;

		case '!':
		case '?':
		case '.':
		case ',':
			if (str_adr[1] == 0 || str_adr[1] == ' ') {
				int16 test_zeilen;
				if (*str_adr == ',')
					test_zeilen = 1;
				else
					test_zeilen = 2;
				++count;
				++str_adr;
				if ((_ssret.Anz + test_zeilen) >= ssi_->Zeilen) {
					if (count < zeichen_anz) {
						tmp_count = count;
						ende = true;
					}
					_splitPtr[_ssret.Anz] = start_adr;
					start_adr[tmp_count] = 0;
					if (ssi_->Mode == SPLIT_MITTE)
						_splitX[_ssret.Anz] = ssi_->X + ((ssi_->Width - (strlen(start_adr) * ssi_->Fvorx)) >> 1);
					else
						_splitX[_ssret.Anz] = ssi_->X;
					++_ssret.Anz;
					bool ende1 = false;
					while (!ende1) {
						if (*str_adr == ATDS_END_TEXT)
							ende1 = true;
						else if (*str_adr != ' ' && *str_adr != 0) {
							ende1 = true;
							_ssret.Next = true;
						}
						++str_adr;
					}
					if (!ende) {
						str_adr = start_adr + tmp_count + 1;
						start_adr = str_adr;
						count = 0;
						tmp_count = 0;
					}
				}
			} else {
				++count;
				++str_adr;
			}
			break;

		default:
			++count;
			++str_adr;
			break;

		}
	}
	if (_ssret.Anz <= ssi_->Zeilen)
		_ssret.Y = ssi_->Y + (ssi_->Zeilen - _ssret.Anz) * ssi_->FHoehe;
	else
		_ssret.Y = ssi_->Y;

	return &_ssret;
}

void Atdsys::str_null2leer(char *str_start, char *str_end) {
	while (str_start < str_end) {
		if (*str_start == 0)
			*str_start = 32;
		++str_start;
	}
}

void Atdsys::calc_txt_win(SplitStringInit *ssi_) {
	if (ssi_->X - (ssi_->Width >> 1) < 2)
		ssi_->X = 2;
	else if (ssi_->X + (ssi_->Width >> 1) > (SCREEN_WIDTH - 2))
		ssi_->X = ((SCREEN_WIDTH - 2) - ssi_->Width);
	else
		ssi_->X -= (ssi_->Width >> 1);

	if (ssi_->Y - (ssi_->Zeilen * ssi_->FHoehe) < 2) {
		ssi_->Y = 2;
	} else if (ssi_->Y + (ssi_->Zeilen * ssi_->FHoehe) > (SCREEN_HEIGHT - 2))
		ssi_->Y = (SCREEN_HEIGHT - 2) - (ssi_->Zeilen * ssi_->FHoehe);
	else {
		ssi_->Y -= (ssi_->Zeilen * ssi_->FHoehe);
	}
}

void Atdsys::set_split_win(int16 nr, SplitStringInit *ssinit) {
	_ssi[nr] = ssinit[0];
}

Stream *Atdsys::pool_handle(const char *fname) {
	Stream *handle = chewy_fopen(fname);
	if (handle) {
		_atdshandle[ATDS_HANDLE] = handle;
	} else {
		error("Error reading from %s", fname);
	}

	return handle;
}

void Atdsys::set_handle(const char *fname, int16 mode, Stream *handle, int16 chunk_start, int16 chunk_anz) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(handle);
	ChunkHead Ch;
	char *tmp_adr = atds_adr(fname, chunk_start, chunk_anz);
	if (!_G(modul)) {
		if (rs) {
			_atdshandle[mode] = rs;
			_atdsmem[mode] = tmp_adr;
			_atdspooloff[mode] = chunk_start;
			switch (mode) {
			case INV_USE_DATEI:
				_G(mem)->file->select_pool_item(rs, _atdspooloff[mode]);
				rs->seek(-ChunkHead::SIZE(), SEEK_CUR);

				if (!Ch.load(rs)) {
					error("Error reading from %s", fname);
				} else {
					free(_invUseMem);
					_invUseMem = (char *)MALLOC(Ch.size + 3l);

					if (!_G(modul)) {
						if (Ch.size) {
							if (!rs->read(_invUseMem, Ch.size)) {
								error("Error reading from %s", fname);
							} else
								crypt(_invUseMem, Ch.size);
						}
						_invUseMem[Ch.size] = (char)BLOCKENDE;
						_invUseMem[Ch.size + 1] = (char)BLOCKENDE;
						_invUseMem[Ch.size + 2] = (char)BLOCKENDE;
					}
				}
				break;
			}
		}
	} else {
		error("Error reading from %s", fname);
	}
}

void Atdsys::open_handle(const char *fname, int16 mode) {
	char *tmp_adr = nullptr;

	if (mode != INV_IDX_DATEI)
		tmp_adr = atds_adr(fname, 0, 20000);
	if (!_G(modul)) {
		Stream *stream = chewy_fopen(fname);
		if (stream) {
			close_handle(mode);
			_atdshandle[mode] = stream;
			_atdsmem[mode] = tmp_adr;

			switch (mode) {
			case ADH_DATEI:
				_adsBlock = (AdsBlock *)_atdsmem[ADH_HANDLE];
				break;

			case INV_IDX_DATEI:
				_atdsmem[INV_IDX_HANDLE] = (char *)MALLOC(INV_STRC_ANZ * sizeof(InvUse));
				break;

			default:
				break;
			}
		} else {
			error("Error reading from %s", fname);
		}
	}
}

void Atdsys::close_handle(int16 mode) {
	Stream *stream = _atdshandle[mode];
	if (stream) {
		chewy_fclose(_atdshandle[mode]);

		for (int i = 0; i < MAX_HANDLE; ++i) {
			if (_atdshandle[i] == stream)
				_atdshandle[i] = nullptr;
		}
	}

	if (_atdsmem[mode])
		free(_atdsmem[mode]);
	_atdsmem[mode] = nullptr;
}

char *Atdsys::atds_adr(const char *fname, int16 chunk_start, int16 chunk_anz) {
	char *tmp_adr = nullptr;
	uint32 size = _G(mem)->file->get_poolsize(fname, chunk_start, chunk_anz);
	if (size) {
		tmp_adr = (char *)MALLOC(size + 3l);
	}

	return tmp_adr;
}

void Atdsys::load_atds(int16 chunk_nr, int16 mode) {
	ChunkHead Ch;
	char *txt_adr = _atdsmem[mode];
	Common::SeekableReadStream *stream = dynamic_cast<Common::SeekableReadStream *>(_atdshandle[mode]);

	if (stream && txt_adr) {
		_G(mem)->file->select_pool_item(stream, chunk_nr + _atdspooloff[mode]);
		stream->seek(-ChunkHead::SIZE(), SEEK_CUR);
		if (!Ch.load(stream)) {
			error("load_atds error");
		} else {
			if (Ch.size) {
				if (stream->read(txt_adr, Ch.size) != Ch.size) {
					error("load_atds error");
				} else if (mode != ADH_DATEI)
					crypt(txt_adr, Ch.size);
			}
			txt_adr[Ch.size] = (char)BLOCKENDE;
			txt_adr[Ch.size + 1] = (char)BLOCKENDE;
			txt_adr[Ch.size + 2] = (char)BLOCKENDE;
		}
	} else {
		error("load_atds error");
	}
}

void Atdsys::save_ads_header(int16 dia_nr) {
	ChunkHead Ch;
	if (_atdshandle[ADH_HANDLE]) {
		_G(mem)->file->select_pool_item(_atdshandle[ADH_HANDLE], dia_nr);
		Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(
			_atdshandle[ADH_HANDLE]);

		rs->seek(-ChunkHead::SIZE(), SEEK_CUR);

		if (!Ch.load(rs)) {
			error("save_ads_header error");
		} else {
			if (Ch.size) {
				Common::SeekableWriteStream *ws = g_engine->_tempFiles.createWriteStreamForMember(ADSH_TMP);
				ws->seek(rs->pos());
				if (ws->write(_atdsmem[ADH_HANDLE], Ch.size) != Ch.size) {
					error("save_ads_header error");
				}

				delete ws;
			}
		}
	} else {
		error("save_ads_header error");
	}
}

void Atdsys::crypt(char *txt, uint32 size) {
	uint8 *sp = (uint8 *)txt;
	for (uint32 i = 0; i < size; i++) {
		*sp = -(*sp);
		++sp;
	}
}

void Atdsys::init_ats_mode(int16 mode, uint8 *atsheader) {
	switch (mode) {
	case ATS_DATEI:
		_ats_st_header[0] = atsheader;
		break;

	case INV_USE_DATEI:
		_ats_st_header[1] = atsheader;
		break;

	case INV_USE_DEF:
		_ats_st_header[2] = atsheader;
		break;

	case INV_ATS_DATEI:
		_ats_st_header[3] = atsheader;
		break;

	default:
		break;
	}
}

void Atdsys::set_ats_mem(int16 mode) {
	switch (mode) {
	case ATS_DATEI:
		_ats_sheader = _ats_st_header[0];
		_atsmem = _atdsmem[mode];
		break;

	case INV_USE_DATEI:
		_ats_sheader = _ats_st_header[1];
		_atsmem = _atdsmem[mode];
		break;

	case INV_USE_DEF:
		_ats_sheader = _ats_st_header[2];
		_atsmem = _invUseMem;
		break;

	case INV_ATS_DATEI:
		_ats_sheader = _ats_st_header[3];
		_atsmem = _atdsmem[mode];
		break;

	default:
		break;
	}
}

bool Atdsys::start_ats(int16 txt_nr, int16 txt_mode, int16 color, int16 mode, int16 *voc_nr) {
	*voc_nr = -1;
	set_ats_mem(mode);

	if (_atsmem) {
		if (_atsv.Display)
			stop_ats();

		int16 txt_anz;
		_atsv.Ptr = ats_get_txt(txt_nr, txt_mode, &txt_anz, mode);

		if (_atsv.Ptr) {
			_atsv.Display = true;
			char *ptr = _atsv.Ptr;
			_atsv.TxtLen = 0;

			while (*ptr++ != ATDS_END_TEXT)
				++_atsv.TxtLen;

			if ((byte)*_atsv.Ptr == 248) {
				// Special code for no message to display
				_atsv.Display = false;
			} else {
				_atsv.DelayCount = get_delay(_atsv.TxtLen);
				_printDelayCount1 = _atsv.DelayCount / 10;
				_atsv.Color = color;
				_mousePush = true;
			}

			*voc_nr = _atsv.StrHeader.VocNr - ATDS_VOC_OFFSET;

			if ((_atdsv.Display == DISPLAY_VOC) && (*voc_nr != -1)) {
				_atsv.Display = false;
			}
		} else {
			_atsv.Display = false;
		}
	} else {
		_atsv.Display = false;
	}

	return _atsv.Display;
}

void Atdsys::stop_ats() {
	_atsv.Display = false;
}

int16 Atdsys::ats_get_status() {
	return _atsv.Display;
}

void Atdsys::print_ats(int16 x, int16 y, int16 scrx, int16 scry) {
	if (_atsv.Display) {
		if (_atdsv._eventsEnabled) {
			switch (_G(in)->get_switch_code()) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_RETURN:
			case MAUS_LINKS:
				if (_mousePush == false) {
					if (_atsv.SilentCount <= 0 && _atsv.DelayCount > _printDelayCount1) {
						_mousePush = true;
						_atsv.DelayCount = 0;
						_inzeig->kbinfo->scan_code = Common::KEYCODE_INVALID;
						_inzeig->kbinfo->key_code = '\0';
					}
				}
				break;

			default:
				_mousePush = false;
				break;
			}
		} else {
			_mousePush = false;
		}

		if (_atsv.SilentCount <= 0) {
			char *tmp_ptr = _atsv.Ptr;
			_G(out)->set_fontadr(_atdsv.Font);
			_G(out)->set_vorschub(_atdsv.Fvorx, 0);
			_atsSsi = _ssi[0];
			_atsSsi.Str = tmp_ptr;
			_atsSsi.Fvorx = _atdsv.Fvorx;
			_atsSsi.FHoehe = _atdsv.Fhoehe;
			_atsSsi.X = x - scrx;
			_atsSsi.Y = y - scry;
			char *start_ptr = tmp_ptr;
			str_null2leer(start_ptr, start_ptr + _atsv.TxtLen - 1);
			SplitStringInit tmp_ssi = _atsSsi;
			_ssr = split_string(&tmp_ssi);

			for (int16 i = 0; i < _ssr->Anz; i++) {
				_G(out)->printxy(_ssr->X[i],
				              _ssr->Y + (i * _atsSsi.FHoehe) + 1,
				              0, 300, 0, _ssr->StrPtr[i]);
				_G(out)->printxy(_ssr->X[i],
				              _ssr->Y + (i * _atsSsi.FHoehe) - 1,
				              0, 300, 0, _ssr->StrPtr[i]);
				_G(out)->printxy(_ssr->X[i] + 1,
				              _ssr->Y + (i * _atsSsi.FHoehe),
				              0, 300, 0, _ssr->StrPtr[i]);
				_G(out)->printxy(_ssr->X[i] - 1,
				              _ssr->Y + (i * _atsSsi.FHoehe),
				              0, 300, 0, _ssr->StrPtr[i]);
				_G(out)->printxy(_ssr->X[i],
				              _ssr->Y + (i * _atsSsi.FHoehe),
				              _atsv.Color,
				              300, 0, _ssr->StrPtr[i]);
				tmp_ptr += strlen(_ssr->StrPtr[i]) + 1;
			}

			str_null2leer(start_ptr, start_ptr + _atsv.TxtLen - 1);
			if (_atsv.DelayCount <= 0) {
				if (_ssr->Next == false) {
					_atsv.Display = false;
				} else {
					_atsv.Ptr = tmp_ptr;
					_atsv.TxtLen = 0;
					while (*tmp_ptr++ != ATDS_END_TEXT)
						++_atsv.TxtLen;
					_atsv.DelayCount = get_delay(_atsv.TxtLen);
					_printDelayCount1 = _atsv.DelayCount / 10;
					_atsv.SilentCount = _atdsv.Silent;
				}
			} else
				--_atsv.DelayCount;
		} else
			--_atsv.SilentCount;
	}
}

char *Atdsys::ats_get_txt(int16 txt_nr, int16 txt_mode, int16 *txt_anz, int16 mode) {
	char *str_ = nullptr;
	set_ats_mem(mode);

	_atsv.TxtMode = txt_mode;

	if (!get_steuer_bit(txt_nr, ATS_AKTIV_BIT, mode)) {
		uint8 status = _ats_sheader[(txt_nr * MAX_ATS_STATUS) + (_atsv.TxtMode + 1) / 2];
		int16 ak_nybble = (_atsv.TxtMode + 1) % 2;

		uint8 lo_hi[2];
		lo_hi[1] = status >> 4;
		lo_hi[0] = status &= 15;
		str_ = ats_search_block(_atsv.TxtMode, _atsmem);
		if (str_ != nullptr) {
			ats_search_nr(txt_nr, &str_);
			if (str_ != nullptr) {
				ats_search_str(txt_anz, &lo_hi[ak_nybble], (uint8)_ats_sheader[txt_nr * MAX_ATS_STATUS], &str_);

				if (str_ != nullptr) {
					status = 0;
					lo_hi[1] <<= 4;
					status |= lo_hi[0];
					status |= lo_hi[1];
					_ats_sheader[(txt_nr * MAX_ATS_STATUS) + (_atsv.TxtMode + 1) / 2] = status;
				}
			}
		}
	}

	// WORKAROUND: Proper word wrapping some inventory items' look desc
	if (txt_mode == TXT_MARK_LOOK) {
		if (txt_nr == CYB_KRONE_INV && str_[37] == ' ' && str_[56] == '\0') {
			str_[37] = '\0';
			str_[56] = ' ';
		}
		if (txt_nr == YEL_CARD_INV && str_[39] == ' ' && str_[46] == '\0') {
			str_[39] = '\0';
			str_[46] = ' ';
		}
	}

	return str_;
}

void Atdsys::set_ats_str(int16 txt_nr, int16 txt_mode, int16 str_nr, int16 mode) {
	set_ats_mem(mode);
	uint8 status = _ats_sheader[(txt_nr * MAX_ATS_STATUS) + (txt_mode + 1) / 2];
	int16 ak_nybble = (txt_mode + 1) % 2;

	uint8 lo_hi[2];
	lo_hi[1] = status >> 4;
	lo_hi[0] = status &= 15;
	lo_hi[ak_nybble] = str_nr;
	status = 0;
	lo_hi[1] <<= 4;
	status |= lo_hi[0];
	status |= lo_hi[1];
	_ats_sheader[(txt_nr * MAX_ATS_STATUS) + (txt_mode + 1) / 2] = status;
}

void Atdsys::set_ats_str(int16 txt_nr, int16 str_nr, int16 mode) {
	for (int16 i = 0; i < 5; i++)
		set_ats_str(txt_nr, i, str_nr, mode);
}

int16 Atdsys::get_ats_str(int16 txt_nr, int16 txt_mode, int16 mode) {
	set_ats_mem(mode);
	uint8 status = _ats_sheader[(txt_nr * MAX_ATS_STATUS) + (txt_mode + 1) / 2];
	int16 ak_nybble = (txt_mode + 1) % 2;

	uint8 lo_hi[2];
	lo_hi[1] = status >> 4;
	lo_hi[0] = status &= 15;

	return (int16)lo_hi[ak_nybble];
}

int16 Atdsys::get_steuer_bit(int16 txt_nr, int16 bit_idx, int16 mode) {
	set_ats_mem(mode);
	int16 ret = _G(bit)->is_bit(_ats_sheader[txt_nr * MAX_ATS_STATUS], bit_idx);
	return ret;
}

void Atdsys::set_steuer_bit(int16 txt_nr, int16 bit_idx, int16 mode) {
	set_ats_mem(mode);
	_G(bit)->set_bit(&_ats_sheader[txt_nr * MAX_ATS_STATUS], bit_idx);
}

void Atdsys::del_steuer_bit(int16 txt_nr, int16 bit_idx, int16 mode) {
	set_ats_mem(mode);
	_G(bit)->del_bit(&_ats_sheader[txt_nr * MAX_ATS_STATUS], bit_idx);
}

char *Atdsys::ats_search_block(int16 txt_mode, char *txt_adr) {
	char *strP = txt_adr;
	int ende = 0;

	for (; !ende; ++strP) {
		if (strP[0] == (char)BLOCKENDE &&
		        strP[1] == (char)BLOCKENDE &&
		        strP[2] == (char)BLOCKENDE) {
			ende = 2;
		} else if (strP[0] == (char)0xf2 && strP[1] == (char)0xfe) {
			if (strP[2] == (char)txt_mode)
				ende = 1;
			strP += 2;
		}
	}

	if (ende == 2)
		strP = nullptr;

	return strP;
}

void Atdsys::ats_search_nr(int16 txt_nr, char **str) {
	char *start_str = *str;

	bool done1 = false;
	while (!done1) {
		Common::MemoryReadStream rs1((const byte *)start_str + 2, AtsTxtHeader::SIZE());
		_atsv.TxtHeader.load(&rs1);

		if (READ_LE_UINT16(start_str) == 0xFEF0 &&
				_atsv.TxtHeader.TxtNr == txt_nr) {
			// Found match
			*str = start_str + 2 + AtsTxtHeader::SIZE();

			if (_atsv.TxtMode) {
				Common::MemoryReadStream rs2((const byte *)*str, AtsStrHeader::SIZE());
				_atsv.StrHeader.load(&rs2);
			}

			*str += AtsStrHeader::SIZE();
			break;
		}

		start_str += 2 + AtsTxtHeader::SIZE() + AtsStrHeader::SIZE();

		// Need to iterate over the following string to next entry
		bool done2 = false;
		for (; !done2; start_str++) {
			if (*start_str == 13) {
				// Reached end of line
				if (READ_LE_UINT16(start_str + 1) == 0xFEF1)
					start_str += 4;
				else if (start_str[1] == 0xe) {
					++start_str;
					if (start_str[1] == 0xf && start_str[2] == 0xf && start_str[3] == 0xf) {
						done1 = done2 = true;
						*str = nullptr;
					} else {
						done2 = true;
					}
				}
			}
		}
	}
}

void Atdsys::ats_search_str(int16 *anz, uint8 *status, uint8 steuer, char **str) {
	char *tmp_str = *str;
	char *start_str = *str;
	*anz = 0;
	bool ende = false;
	int16 count = 0;

	while (!ende) {
		if (count == *status) {
			if (!*tmp_str) {
				++*anz;
			} else if (*tmp_str == ATDS_END_TEXT) {
				ende = true;
				*str = start_str;
				start_str -= AtsStrHeader::SIZE();

				if (_atsv.TxtMode != TXT_MARK_NAME) {
					Common::MemoryReadStream rs((const byte *)start_str,
						AtsStrHeader::SIZE());
					_atsv.StrHeader.load(&rs);
				}

				if (tmp_str[1] != ATDS_END) {
					if (!_G(bit)->is_bit(steuer, ATS_COUNT_BIT))
						++*status;
				} else {

					if (_G(bit)->is_bit(steuer, ATS_RESET_BIT))
						*status = 0;
				}
			}
		} else {
			if (*tmp_str == ATDS_END_TEXT) {
				if (tmp_str[1] == ATDS_END) {
					ende = false;
					*anz = 0;
					*status = count;
					*str = start_str;
					start_str -= AtsStrHeader::SIZE();
					if (_atsv.TxtMode != TXT_MARK_NAME) {
						Common::MemoryReadStream rs((const byte *)start_str,
							AtsStrHeader::SIZE());
						_atsv.StrHeader.load(&rs);
					}
				} else {
					++count;

					tmp_str += AtsStrHeader::SIZE();
					start_str = tmp_str + 1;
				}
			} else if (*tmp_str == ATDS_END ||
			           (tmp_str[0] == (char)BLOCKENDE &&
			            tmp_str[1] == (char)BLOCKENDE &&
			            tmp_str[2] == (char)BLOCKENDE)) {
				ende = false;
				*anz = 0;
				*str = nullptr;
			}
		}

		++tmp_str;
	}
}

int16 Atdsys::start_aad(int16 dia_nr) {
	if (_aadv.Dialog)
		stop_aad();

	if (_atdsmem[AAD_HANDLE]) {
		_aadv.Ptr = _atdsmem[AAD_HANDLE];
		aad_search_dia(dia_nr, &_aadv.Ptr);
		if (_aadv.Ptr) {
			_aadv.Person = (AadInfo *)_aadv.Ptr;
			_aadv.Ptr += _aadv.TxtHeader->PerAnz * sizeof(AadInfo);
			_aadv.Dialog = true;
			_aadv.StrNr = 0;
			_aadv.StrHeader = (AadStrHeader *)_aadv.Ptr;
			_aadv.Ptr += sizeof(AadStrHeader);
			int16 txt_len;
			aad_get_zeilen(_aadv.Ptr, &txt_len);
			_aadv._delayCount = get_delay(txt_len);
			_printDelayCount1 = _aadv._delayCount / 10;

			_atdsv.DiaNr = dia_nr;
			if (_atdsv.aad_str != nullptr)
				_atdsv.aad_str(_atdsv.DiaNr, 0, _aadv.StrHeader->AkPerson, AAD_STR_START);
			_mousePush = true;
			stop_ats();
			_atdsv.VocNr = -1;
		}
	}

	return _aadv.Dialog;
}

void Atdsys::stop_aad() {
	_aadv.Dialog = false;
	_aadv.StrNr = -1;
}

void Atdsys::print_aad(int16 scrx, int16 scry) {
	if (_aadv.Dialog) {
		if (_atdsv._eventsEnabled) {
			switch (_G(in)->get_switch_code()) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_RETURN:
			case MAUS_LINKS:
				EVENTS_CLEAR;

				if (_mousePush == false) {
					if (_aadv.SilentCount <= 0 && _aadv._delayCount > _printDelayCount1) {
						_mousePush = true;
						_aadv._delayCount = 0;
						_inzeig->kbinfo->scan_code = Common::KEYCODE_INVALID;
						_inzeig->kbinfo->key_code = '\0';
					}
				}
				break;

			default:
				_mousePush = false;
				break;
			}
		} else {
			_mousePush = false;
		}

		if (_aadv.SilentCount <= 0) {
			char *tmp_ptr = _aadv.Ptr;
			_G(out)->set_fontadr(_atdsv.Font);
			_G(out)->set_vorschub(_atdsv.Fvorx, 0);
			_ssi[_aadv.StrHeader->AkPerson].Str = tmp_ptr;
			if (_aadv.Person[_aadv.StrHeader->AkPerson].X != -1) {
				_ssi[_aadv.StrHeader->AkPerson].X = _aadv.Person[_aadv.StrHeader->AkPerson].X - scrx;
			}
			if (_aadv.Person[_aadv.StrHeader->AkPerson].Y != -1) {
				_ssi[_aadv.StrHeader->AkPerson].Y = _aadv.Person[_aadv.StrHeader->AkPerson].Y - scry;
			}
			_ssi[_aadv.StrHeader->AkPerson].Fvorx = _atdsv.Fvorx;
			_ssi[_aadv.StrHeader->AkPerson].FHoehe = _atdsv.Fhoehe;
			char *start_ptr = tmp_ptr;
			int16 txt_len;
			aad_get_zeilen(start_ptr, &txt_len);
			str_null2leer(start_ptr, start_ptr + txt_len - 1);
			SplitStringInit tmp_ssi = _ssi[_aadv.StrHeader->AkPerson];
			_ssr = split_string(&tmp_ssi);

			if (_atdsv.Display != DISPLAY_VOC ||
			        (_aadv.StrHeader->VocNr - ATDS_VOC_OFFSET) == -1) {
				for (int16 i = 0; i < _ssr->Anz; i++) {
					_G(out)->printxy(_ssr->X[i] + 1,
					              _ssr->Y + (i * _ssi[_aadv.StrHeader->AkPerson].FHoehe),
					              0, 300, 0, _ssr->StrPtr[i]);
					_G(out)->printxy(_ssr->X[i] - 1,
					              _ssr->Y + (i * _ssi[_aadv.StrHeader->AkPerson].FHoehe),
					              0, 300, 0, _ssr->StrPtr[i]);
					_G(out)->printxy(_ssr->X[i],
					              _ssr->Y + (i * _ssi[_aadv.StrHeader->AkPerson].FHoehe) + 1,
					              0, 300, 0, _ssr->StrPtr[i]);
					_G(out)->printxy(_ssr->X[i],
					              _ssr->Y + (i * _ssi[_aadv.StrHeader->AkPerson].FHoehe) - 1,
					              0, 300, 0, _ssr->StrPtr[i]);
					_G(out)->printxy(_ssr->X[i],
					              _ssr->Y + (i * _ssi[_aadv.StrHeader->AkPerson].FHoehe),
					              _aadv.Person[_aadv.StrHeader->AkPerson].Color,
					              300, 0, _ssr->StrPtr[i]);
					tmp_ptr += strlen(_ssr->StrPtr[i]) + 1;
				}
				str_null2leer(start_ptr, start_ptr + txt_len - 1);

			}

			if (_atdsv.Display != DISPLAY_TXT &&
					(_aadv.StrHeader->VocNr - ATDS_VOC_OFFSET) != -1) {
				if (_atdsv.VocNr != _aadv.StrHeader->VocNr - ATDS_VOC_OFFSET) {
					_atdsv.VocNr = _aadv.StrHeader->VocNr - ATDS_VOC_OFFSET;
					g_engine->_sound->playSpeech(_atdsv.VocNr);
					int16 vocx = _G(spieler_vector)[_aadv.StrHeader->AkPerson].Xypos[0] -
								 _G(spieler).scrollx + _G(spieler_mi)[_aadv.StrHeader->AkPerson].HotX;
					g_engine->_sound->setSoundChannelBalance(0, getStereoPos(vocx));

					if (_atdsv.Display == DISPLAY_VOC) {
						_aadv.StrNr = -1;
						_aadv._delayCount = 1;
					}
				}

				if (_atdsv.Display != DISPLAY_ALL) {
					for (int16 i = 0; i < _ssr->Anz; i++) {
						tmp_ptr += strlen(_ssr->StrPtr[i]) + 1;
					}
					str_null2leer(start_ptr, start_ptr + txt_len - 1);
				}
			}

			if (_aadv._delayCount <= 0) {
				_aadv.Ptr = tmp_ptr;
				while (*tmp_ptr == ' ' || *tmp_ptr == 0)
					++tmp_ptr;
				if (tmp_ptr[1] == ATDS_END ||
				        tmp_ptr[1] == ATDS_END_EINTRAG) {
					if (_atdsv.aad_str != 0)
						_atdsv.aad_str(_atdsv.DiaNr, _aadv.StrNr, _aadv.StrHeader->AkPerson, AAD_STR_END);
					_aadv.Dialog = false;
					_adsv.AutoDia = false;
					_aadv.StrNr = -1;
					_ssret.Next = false;
				} else {
					if (_ssr->Next == false) {
						++_aadv.StrNr;
						while (*_aadv.Ptr++ != ATDS_END_TEXT);

						int16 tmp_person = _aadv.StrHeader->AkPerson;
						int16 tmp_str_nr = _aadv.StrNr;
						_aadv.StrHeader = (AadStrHeader *)_aadv.Ptr;
						_aadv.Ptr += sizeof(AadStrHeader);
						if (_atdsv.aad_str != nullptr) {
							if (tmp_person != _aadv.StrHeader->AkPerson) {
								_atdsv.aad_str(_atdsv.DiaNr, tmp_str_nr, tmp_person, AAD_STR_END);
								_atdsv.aad_str(_atdsv.DiaNr, _aadv.StrNr, _aadv.StrHeader->AkPerson, AAD_STR_START);
							}
						}
					}
					aad_get_zeilen(_aadv.Ptr, &txt_len);
					_aadv._delayCount = get_delay(txt_len);
					_printDelayCount1 = _aadv._delayCount / 10;
					_aadv.SilentCount = _atdsv.Silent;
				}
			} else {
				if (_atdsv.Display != DISPLAY_VOC ||
				        (_aadv.StrHeader->VocNr - ATDS_VOC_OFFSET) == -1)
					--_aadv._delayCount;

				else if (_atdsv.Display == DISPLAY_VOC) {
					warning("FIXME - unknown constant SMP_PLAYING");

					_aadv._delayCount = 0;
				}
			}
		} else {
			--_aadv.SilentCount;
		}
	}
}

int16 Atdsys::aad_get_status() {
	return _aadv.StrNr;
}

int16 Atdsys::aad_get_zeilen(char *str, int16 *txt_len) {
	*txt_len = 0;
	char *ptr = str;
	int16 zeilen = 0;
	while (*str != ATDS_END_TEXT) {
		if (*str++ == 0)
			++zeilen;
	}
	*txt_len = (str - ptr) - 1;

	return zeilen;
}

void Atdsys::aad_search_dia(int16 dia_nr, char **ptr) {
	char *start_ptr = *ptr;

	if (start_ptr[0] == (char)BLOCKENDE &&
	        start_ptr[1] == (char)BLOCKENDE &&
	        start_ptr[2] == (char)BLOCKENDE) {
		*ptr = nullptr;
	} else {
		bool ende = false;
		while (!ende) {
			uint16 *pos = (uint16 *)start_ptr;
			if (pos[0] == dia_nr) {
				ende = true;
				_aadv.TxtHeader = (AadTxtHeader *)start_ptr;
				*ptr = start_ptr + sizeof(AadTxtHeader);
			} else {
				start_ptr += sizeof(AadTxtHeader) + pos[1] * sizeof(AadInfo);
				bool ende1 = false;
				for (; !ende1; ++start_ptr) {
					if (*start_ptr != ATDS_END_TEXT)
						continue;
					if (start_ptr[1] == ATDS_END) {
						++start_ptr;

						if (start_ptr[1] == (char)BLOCKENDE &&
						        start_ptr[2] == (char)BLOCKENDE &&
						        start_ptr[3] == (char)BLOCKENDE) {
							ende = true;
							ende1 = true;
							*ptr = nullptr;
						} else {
							ende1 = true;
						}
					}
				}
			}
		}
	}
}

bool  Atdsys::ads_start(int16 dia_nr) {
	bool ret = false;

	load_atds(dia_nr, ADS_DATEI);
	if (!_G(modul)) {
		bool ende = false;
		if (_atdsmem[ADS_HANDLE][0] == (char)BLOCKENDE &&
		        _atdsmem[ADS_HANDLE][1] == (char)BLOCKENDE &&
		        _atdsmem[ADS_HANDLE][2] == (char)BLOCKENDE)
			ende = true;
		if (!ende) {
			_adsv.Ptr = _atdsmem[ADS_HANDLE];
			_adsv.TxtHeader = (AdsTxtHeader *)_adsv.Ptr;
			if (_adsv.TxtHeader->DiaNr == dia_nr) {
				ret = true;
				_adsv.Ptr += sizeof(AdsTxtHeader);
				_adsv.Person = (AadInfo *) _adsv.Ptr;
				_adsv.Ptr += _adsv.TxtHeader->PerAnz * sizeof(AadInfo);
				_adsv.Dialog = dia_nr;
				_adsv.StrNr = 0;
				_adsStack[0] = 0;
				_adsStackPtr = 1;
			}
		}
	}
	return ret;
}

void Atdsys::stop_ads() {
	_adsv.Dialog = -1;
	_adsv.AutoDia = false;

}

int16 Atdsys::ads_get_status() {
	return _adsv.Dialog;
}

int16 Atdsys::check_item(int16 block_nr, int16 item_nr) {
	int16 ret = true;
	char *tmp_adr = _adsv.Ptr;
	ads_search_block(block_nr, &tmp_adr);
	if (tmp_adr) {
		ads_search_item(item_nr, &tmp_adr);
		if (tmp_adr) {
			ret = true;
		}
	}
	return ret;
}

char **Atdsys::ads_item_ptr(int16 block_nr, int16 *anzahl) {
	*anzahl = 0;
	memset(_ePtr, 0, sizeof(char *)*ADS_MAX_BL_EIN);
	if (_adsv.Dialog != -1) {
		_adsv.BlkPtr = _adsv.Ptr;
		ads_search_block(block_nr, &_adsv.BlkPtr);
		if (_adsv.BlkPtr) {
			for (int16 i = 0; i < ADS_MAX_BL_EIN; i++) {
				char *tmp_adr = _adsv.BlkPtr;
				ads_search_item(i, &tmp_adr);
				if (tmp_adr) {
					char nr = tmp_adr[-1];
					tmp_adr += sizeof(AadStrHeader);
					if (_adsBlock[block_nr].Show[(int16)nr] == true) {
						_ePtr[*anzahl] = tmp_adr;
						_eNr[*anzahl] = (int16)nr;
						++(*anzahl);
					}
				}
			}
		}
	}

	return _ePtr;
}

AdsNextBlk *Atdsys::ads_item_choice(int16 blk_nr, int16 item_nr) {
	_adsnb.BlkNr = blk_nr;
	if (!_aadv.Dialog) {
		if (!_adsv.AutoDia) {
			ads_search_item(_eNr[item_nr], &_adsv.BlkPtr);
			if (_adsv.BlkPtr) {
				if (start_ads_auto_dia(_adsv.BlkPtr))
					_adsv.AutoDia = true;
				if (_G(bit)->is_bit((uint8)_adsBlock[blk_nr].Steuer[_eNr[item_nr]], ADS_EXIT_BIT) == true) {
					stop_ads();
					_adsnb.EndNr = _eNr[item_nr];
					_adsnb.BlkNr = -1;
				}
			}
		}
	}

	return &_adsnb;
}

AdsNextBlk *Atdsys::calc_next_block(int16 blk_nr, int16 item_nr) {
	if (_G(bit)->is_bit((uint8)_adsBlock[blk_nr].Steuer[_eNr[item_nr]], ADS_SHOW_BIT) == false)
		_adsBlock[blk_nr].Show[_eNr[item_nr]] = false;
	_adsnb.EndNr = _eNr[item_nr];
	if (_G(bit)->is_bit((uint8)_adsBlock[blk_nr].Steuer[_eNr[item_nr]], ADS_RESTART_BIT) == true) {
		_adsnb.BlkNr = 0;

		_adsStackPtr = 0;
	} else {
		if (_adsBlock[blk_nr].Next[_eNr[item_nr]]) {
			_adsnb.BlkNr = _adsBlock[blk_nr].Next[_eNr[item_nr]];

			int16 anzahl = 0;
			while (!anzahl && _adsnb.BlkNr != -1) {

				anzahl = 0;
				ads_item_ptr(_adsnb.BlkNr, &anzahl);
				if (!anzahl) {
					_adsnb.BlkNr = return_block(_adsBlock);
				}
			}
		} else {
			_adsnb.BlkNr = return_block(_adsBlock);
		}
	}
	_adsStack[_adsStackPtr] = _adsnb.BlkNr;
	++_adsStackPtr;

	return &_adsnb;
}

int16 Atdsys::return_block(AdsBlock *ab) {
	_adsStackPtr -= 1;
	int16 ret = -1;
	bool ende = false;
	while (_adsStackPtr >= 0 && !ende) {
		short blk_nr = _adsStack[_adsStackPtr];
		int16 anz;
		ads_item_ptr(blk_nr, &anz);
		if (anz) {
			ret = blk_nr;
			ende = true;
		} else
			--_adsStackPtr;
	}
	++_adsStackPtr;
	return ret;
}

void Atdsys::ads_search_block(int16 blk_nr, char **ptr) {
	char *start_ptr = *ptr;
	bool ende = false;
	while (!ende) {
		if (*start_ptr == (char)blk_nr) {
			ende = true;
			*ptr = start_ptr;
		} else {
			start_ptr += 2 + sizeof(AadStrHeader);
			while (*start_ptr++ != ATDS_END_BLOCK);
			if (start_ptr[0] == ATDS_END &&
			        start_ptr[1] == ATDS_END) {
				ende = true;
				*ptr = nullptr;
			}
		}
	}
}

void Atdsys::ads_search_item(int16 item_nr, char **blk_adr) {
	char *start_ptr = *blk_adr + 1;
	bool ende = false;
	while (!ende) {
		if (*start_ptr == item_nr) {
			ende = true;
			*blk_adr = start_ptr + 1;
		} else {
			start_ptr += 1 + sizeof(AadStrHeader);
			while (*start_ptr++ != ATDS_END_EINTRAG);
			if (*start_ptr == ATDS_END_BLOCK) {
				ende = true;
				*blk_adr = nullptr;
			}
		}
	}
}

int16 Atdsys::start_ads_auto_dia(char *item_adr) {
	_aadv.Dialog = false;
	if (item_adr) {
		_aadv.Person = _adsv.Person;
		_aadv.Ptr = item_adr;
		_aadv.Dialog = true;
		_aadv.StrNr = 0;
		_aadv.StrHeader = (AadStrHeader *)_aadv.Ptr;
		_aadv.Ptr += sizeof(AadStrHeader);
		int16 txt_len;
		aad_get_zeilen(_aadv.Ptr, &txt_len);
		_aadv._delayCount = get_delay(txt_len);
		_atdsv.DiaNr = _adsv.TxtHeader->DiaNr + 10000;

		if (_atdsv.aad_str != nullptr)
			_atdsv.aad_str(_atdsv.DiaNr, 0, _aadv.StrHeader->AkPerson, AAD_STR_START);
		_mousePush = true;
		stop_ats();
	} else {
		_aadv.Dialog = false;
	}

	return _aadv.Dialog;
}

void Atdsys::hide_item(int16 dia_nr, int16 blk_nr, int16 item_nr) {
	if (_adsv.Dialog == dia_nr) {
		if (check_item(blk_nr, item_nr))
			_adsBlock[blk_nr].Show[item_nr] = false;
	} else {
		load_atds(dia_nr, ADH_DATEI);
		if (check_item(blk_nr, item_nr))
			_adsBlock[blk_nr].Show[item_nr] = false;
		save_ads_header(dia_nr);
	}
}

void Atdsys::show_item(int16 dia_nr, int16 blk_nr, int16 item_nr) {
	if (_adsv.Dialog == dia_nr) {
		if (check_item(blk_nr, item_nr))
			_adsBlock[blk_nr].Show[item_nr] = true;
	} else {
		load_atds(dia_nr, ADH_DATEI);
		if (check_item(blk_nr, item_nr))
			_adsBlock[blk_nr].Show[item_nr] = true;
		save_ads_header(dia_nr);
	}
}

int16 Atdsys::calc_inv_no_use(int16 cur_inv, int16 test_nr, int16 mode) {
	int16 txt_nr = -1;
	if (cur_inv != -1) {
		if (_invBlockNr != cur_inv) {
			_invBlockNr = cur_inv + 1;
			load_atds(_invBlockNr + _atdspooloff[mode], INV_USE_DATEI);

			Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(
				_atdshandle[INV_IDX_HANDLE]);
			if (rs) {
				rs->seek(InvUse::SIZE() * _invBlockNr
				      * INV_STRC_ANZ, SEEK_SET);

				InvUse *iu = (InvUse *)_atdsmem[INV_IDX_HANDLE];
				for (int16 i = 0; i < INV_STRC_ANZ; ++i, ++iu) {
					if (!iu->load(rs)) {
						error("calc_inv_no_use error");
						break;
					}
				}
			} else {
				error("calc_inv_no_use error");
			}
		}

		InvUse *iu = (InvUse *)_atdsmem[INV_IDX_HANDLE];
		bool ok = false;

		for (int16 i = 0; i < INV_STRC_ANZ && !ok; i++) {
			if (iu[i].ObjId == mode) {
				if (iu[i].ObjNr == test_nr) {
					txt_nr = iu[i].TxtNr;
					ok = true;
				}
			}
		}
	}

	return txt_nr;
}

int16 Atdsys::getStereoPos(int16 x) {
	// TODO: Convert to ScummVM's balance (-127 ... 0 ... 127)
	return 0;
#if 0
	float fx_ = (float)x;

	fx_ /= 2.5;
	if ((fx_ - ((float)((int16)fx_))) > 0.5)
		fx_ += 1.0;

	return (int16)fx_;
#endif
}

} // namespace Chewy
