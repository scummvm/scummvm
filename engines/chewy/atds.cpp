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
#include "chewy/globals.h"
#include "chewy/main.h"
#include "chewy/sound.h"

namespace Chewy {

#define MAX_DIALOG_DATA_SIZE 6 * 3 * 6

bool AtsTxtHeader::load(Common::SeekableReadStream *src) {
	_txtNr = src->readUint16LE();
	_aMov = src->readSint16LE();
	_curNr = src->readSint16LE();
	src->skip(2);

	return true;
}

bool InvUse::load(Common::SeekableReadStream *src) {
	_objId = src->readSint16LE();
	_objNr = src->readSint16LE();
	_txtNr = src->readSint16LE();
	return true;
}

void AadInfo::load(Common::SeekableReadStream *src) {
	_x = src->readSint16LE();
	_y = src->readSint16LE();
	_color = src->readSint16LE();
}

void AadInfoArray::load(const void *data, size_t count) {
	resize(count);
	Common::MemoryReadStream src((const byte *)data, count * AadInfo::SIZE());

	for (uint i = 0; i < count; ++i)
		(*this)[i].load(&src);
}

bool AadTxtHeader::load(const void *src) {
	Common::MemoryReadStream rs((const byte *)src, 8);

	_diaNr = rs.readSint16LE();
	_perNr = rs.readSint16LE();
	_aMov = rs.readSint16LE();
	_curNr = rs.readSint16LE();
	return true;
}

bool AdsTxtHeader::load(const void *src) {
	Common::MemoryReadStream rs((const byte *)src, 8);

	_diaNr = rs.readSint16LE();
	_perNr = rs.readSint16LE();
	_aMov = rs.readSint16LE();
	_curNr = rs.readSint16LE();
	return true;
}

bool AtsStrHeader::load(Common::SeekableReadStream *src) {
	_vocNr = src->readUint16LE();
	return true;
}


Atdsys::Atdsys() {
	SplitStringInit init_ssi = { nullptr, 0, 0, 220, 4, SPLIT_MITTE, 8, 8,};
	_aadv._dialog = false;
	_aadv._strNr = -1;
	_aadv._silentCount = false;
	_adsv._dialog = -1;
	_adsv._autoDia = false;
	_adsv._strNr = -1;
	_adsv._silentCount = false;
	_atsv._display = DISPLAY_NONE;
	_atsv._silentCount = false;
	_atdsv._delay = &_tmpDelay;
	_tmpDelay = 1;
	_atdsv._silent = false;
	_atdsv.Display = DISPLAY_TXT;
	_atdsv._diaNr = -1;
	_atdsv.aad_str = 0;
	_atdsv._vocNr = -1;
	_atdsv._eventsEnabled = true;
	_ssret._next = false;
	_ssr = &_ssret;
	for (int16 i = 0; i < AAD_MAX_PERSON; i++)
		_ssi[i] = init_ssi;
	_invBlockNr = -1;
	_invUseMem = nullptr;

	_dialogResource = new DialogResource(ADS_TXT_STEUER);
}

Atdsys::~Atdsys() {
	for (int16 i = 0; i < MAX_HANDLE; i++) {
		if (i == ADH_HANDLE)
			continue;

		close_handle(i);
	}

	if (_invUseMem)
		free(_invUseMem);

	delete _dialogResource;
}

void Atdsys::set_delay(int16 *delay, int16 silent) {
	_atdsv._delay = delay;
	_atdsv._silent = silent;
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
	int16 z_len = (_ssi->_width / _ssi->Fvorx) + 1;
	int16 max_len = z_len * _ssi->_lines;
	if (txt_len > max_len)
		txt_len = max_len;

	int16 ret = *_atdsv._delay * (txt_len + z_len);
	return ret;
}

SplitStringRet *Atdsys::split_string(SplitStringInit *ssi_) {
	_ssret._nr = 0;
	_ssret._next = false;
	_ssret._strPtr = _splitPtr;
	_ssret._x = _splitX;
	int16 zeichen_anz = (ssi_->_width / ssi_->Fvorx) + 1;
	memset(_splitPtr, 0, sizeof(char *) * MAX_STR_SPLIT);
	calc_txt_win(ssi_);
	char *str_adr = ssi_->_str;
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
				_splitPtr[_ssret._nr] = start_adr;
				start_adr[tmp_count] = 0;
				if (ssi_->_mode == SPLIT_MITTE)
					_splitX[_ssret._nr] = ssi_->_x + ((ssi_->_width - (strlen(start_adr) * ssi_->Fvorx)) >> 1);
				else
					_splitX[_ssret._nr] = ssi_->_x;
				++_ssret._nr;
				if (_ssret._nr == ssi_->_lines) {
					ende = true;
					bool ende1 = false;
					while (!ende1) {
						if (*str_adr == ATDS_END_TEXT)
							ende1 = true;
						else if (*str_adr != ' ' && *str_adr != 0) {
							ende1 = true;
							_ssret._next = true;
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
				if ((_ssret._nr + test_zeilen) >= ssi_->_lines) {
					if (count < zeichen_anz) {
						tmp_count = count;
						ende = true;
					}
					_splitPtr[_ssret._nr] = start_adr;
					start_adr[tmp_count] = 0;
					if (ssi_->_mode == SPLIT_MITTE)
						_splitX[_ssret._nr] = ssi_->_x + ((ssi_->_width - (strlen(start_adr) * ssi_->Fvorx)) >> 1);
					else
						_splitX[_ssret._nr] = ssi_->_x;
					++_ssret._nr;
					bool ende1 = false;
					while (!ende1) {
						if (*str_adr == ATDS_END_TEXT)
							ende1 = true;
						else if (*str_adr != ' ' && *str_adr != 0) {
							ende1 = true;
							_ssret._next = true;
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
	if (_ssret._nr <= ssi_->_lines)
		_ssret._y = ssi_->_y + (ssi_->_lines - _ssret._nr) * ssi_->FHoehe;
	else
		_ssret._y = ssi_->_y;

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
	if (ssi_->_x - (ssi_->_width >> 1) < 2)
		ssi_->_x = 2;
	else if (ssi_->_x + (ssi_->_width >> 1) > (SCREEN_WIDTH - 2))
		ssi_->_x = ((SCREEN_WIDTH - 2) - ssi_->_width);
	else
		ssi_->_x -= (ssi_->_width >> 1);

	if (ssi_->_y - (ssi_->_lines * ssi_->FHoehe) < 2) {
		ssi_->_y = 2;
	} else if (ssi_->_y + (ssi_->_lines * ssi_->FHoehe) > (SCREEN_HEIGHT - 2))
		ssi_->_y = (SCREEN_HEIGHT - 2) - (ssi_->_lines * ssi_->FHoehe);
	else {
		ssi_->_y -= (ssi_->_lines * ssi_->FHoehe);
	}
}

void Atdsys::set_split_win(int16 nr, int16 x, int16 y) {
	_ssi[nr]._x = x;
	_ssi[nr]._y = y;
}

Common::Stream *Atdsys::pool_handle(const char *fname) {
	Common::File *f = new Common::File();
	f->open(fname);
	if (f->isOpen()) {
		_atdshandle[ATDS_HANDLE] = f;
	} else {
		error("Error reading from %s", fname);
	}
	return f;
}

void Atdsys::set_handle(const char *fname, int16 mode, Common::Stream *handle, int16 chunk_start, int16 chunk_anz) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(handle);
	ChunkHead Ch;
	char *tmp_adr = atds_adr(fname, chunk_start, chunk_anz);
	if (rs) {
		_atdshandle[mode] = rs;
		_atdsmem[mode] = tmp_adr;
		_atdspooloff[mode] = chunk_start;
		switch (mode) {
		case INV_USE_DATA:
			_G(mem)->file->selectPoolItem(rs, _atdspooloff[mode]);
			rs->seek(-ChunkHead::SIZE(), SEEK_CUR);

			if (!Ch.load(rs)) {
				error("Error reading from %s", fname);
			} else {
				free(_invUseMem);
				_invUseMem = (char *)MALLOC(Ch.size + 3l);

				if (Ch.size) {
					if (!rs->read(_invUseMem, Ch.size)) {
						error("Error reading from %s", fname);
					} else {
						crypt(_invUseMem, Ch.size);
					}
				}
				_invUseMem[Ch.size] = (char)BLOCKENDE;
				_invUseMem[Ch.size + 1] = (char)BLOCKENDE;
				_invUseMem[Ch.size + 2] = (char)BLOCKENDE;
			}
			break;
		}
	}
}

void Atdsys::open_handle(const char *fname, int16 mode) {
	char *tmp_adr = nullptr;

	if (mode == ADH_DATA) {
		if (_atdsmem[mode])
			free(_atdsmem[mode]);

		// Set to nullptr on purpose, this shouldn't be used
		_atdshandle[mode] = nullptr;
		// +3 bytes to signify the end of the stream (BLOCKENDE)
		_atdsmem[mode] = (char *)MALLOC(MAX_DIALOG_DATA_SIZE + 3);
		_adsBlock = (AdsBlock *)_atdsmem[mode];
		return;
	}

	if (mode != INV_IDX_DATA)
		tmp_adr = atds_adr(fname, 0, 20000);

	Common::File *f = new Common::File();
	f->open(fname);
	if (f->isOpen()) {
		close_handle(mode);
		_atdshandle[mode] = f;
		_atdsmem[mode] = tmp_adr;

		if (mode == INV_IDX_DATA)
			_atdsmem[INV_IDX_HANDLE] = (char *)MALLOC(INV_STRC_NR * sizeof(InvUse));
	} else {
		error("Error reading from %s", fname);
	}
}

void Atdsys::close_handle(int16 mode) {
	Common::Stream *stream = _atdshandle[mode];
	if (stream) {
		delete _atdshandle[mode];
		_atdshandle[mode] = nullptr;

		for (int i = 0; i < MAX_HANDLE; ++i) {
			if (i == ADH_HANDLE)
				continue;

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
	uint32 size = _G(mem)->file->getPoolSize(fname, chunk_start, chunk_anz);
	if (size) {
		tmp_adr = (char *)MALLOC(size + 3);
	}

	return tmp_adr;
}

void Atdsys::load_atds(int16 chunk_nr, int16 mode) {
	char *txt_adr = _atdsmem[mode];

	if (mode == ADH_DATA) {
		Chunk *chunk = _dialogResource->getChunk(chunk_nr);
		uint8 *chunkData = _dialogResource->getChunkData(chunk_nr);

		assert(chunk->size <= MAX_DIALOG_DATA_SIZE);
		memcpy(_atdsmem[ADH_HANDLE], chunkData, chunk->size);
		delete[] chunkData;

		txt_adr[chunk->size] = (char)BLOCKENDE;
		txt_adr[chunk->size + 1] = (char)BLOCKENDE;
		txt_adr[chunk->size + 2] = (char)BLOCKENDE;
		return;
	}

	ChunkHead Ch;
	Common::SeekableReadStream *stream = dynamic_cast<Common::SeekableReadStream *>(_atdshandle[mode]);

	if (stream && txt_adr) {
		_G(mem)->file->selectPoolItem(stream, chunk_nr + _atdspooloff[mode]);
		stream->seek(-ChunkHead::SIZE(), SEEK_CUR);
		if (!Ch.load(stream)) {
			error("load_atds error");
		} else {
			if (Ch.size) {
				if (stream->read(txt_adr, Ch.size) != Ch.size) {
					error("load_atds error");
				} else if (mode != ADH_DATA) {
					crypt(txt_adr, Ch.size);
				}
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
	_dialogResource->updateChunk(dia_nr, (byte *)_atdsmem[ADH_HANDLE]);
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
	case ATS_DATA:
		_ats_st_header[0] = atsheader;
		break;

	case INV_USE_DATA:
		_ats_st_header[1] = atsheader;
		break;

	case INV_USE_DEF:
		_ats_st_header[2] = atsheader;
		break;

	case INV_ATS_DATA:
		_ats_st_header[3] = atsheader;
		break;

	default:
		break;
	}
}

void Atdsys::set_ats_mem(int16 mode) {
	switch (mode) {
	case ATS_DATA:
		_ats_sheader = _ats_st_header[0];
		_atsmem = _atdsmem[mode];
		break;

	case INV_USE_DATA:
		_ats_sheader = _ats_st_header[1];
		_atsmem = _atdsmem[mode];
		break;

	case INV_USE_DEF:
		_ats_sheader = _ats_st_header[2];
		_atsmem = _invUseMem;
		break;

	case INV_ATS_DATA:
		_ats_sheader = _ats_st_header[3];
		_atsmem = _atdsmem[mode];
		break;

	default:
		break;
	}
}

DisplayMode Atdsys::start_ats(int16 txt_nr, int16 txt_mode, int16 color, int16 mode, int16 *voc_nr) {
	*voc_nr = -1;
	set_ats_mem(mode);

	_atsv._display = DISPLAY_NONE;

	if (_atsmem) {
		if (_atsv._display != DISPLAY_NONE)
			stop_ats();

		int16 txt_anz;
		_atsv._ptr = ats_get_txt(txt_nr, txt_mode, &txt_anz, mode);

		if (_atsv._ptr) {
			_atsv._display = _atdsv.Display;
			char *ptr = _atsv._ptr;
			_atsv._txtLen = 0;

			while (*ptr++ != ATDS_END_TEXT)
				++_atsv._txtLen;

			*voc_nr = _atsv._strHeader._vocNr - ATDS_VOC_OFFSET;

			if ((byte)*_atsv._ptr == 248) {
				// Special code for no message to display
				_atsv._display = (_atdsv.Display == DISPLAY_TXT || *voc_nr == -1) ?
					DISPLAY_NONE : DISPLAY_VOC;

			} else {
				_atsv._delayCount = get_delay(_atsv._txtLen);
				_printDelayCount1 = _atsv._delayCount / 10;
				_atsv._color = color;
				_mousePush = true;

				if (*voc_nr == -1) {
					_atsv._display = (_atdsv.Display == DISPLAY_VOC) ?
						DISPLAY_NONE : DISPLAY_TXT;
				}
			}
		}
	}

	return _atsv._display;
}

void Atdsys::stop_ats() {
	_atsv._display = DISPLAY_NONE;
}

DisplayMode &Atdsys::ats_get_status() {
	return _atsv._display;
}

void Atdsys::print_ats(int16 x, int16 y, int16 scrx, int16 scry) {
	if (_atsv._display == DISPLAY_TXT || _atsv._display == DISPLAY_ALL) {
		if (_atdsv._eventsEnabled) {
			switch (_G(in)->getSwitchCode()) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_RETURN:
			case MOUSE_LEFT:
				if (_mousePush == false) {
					if (_atsv._silentCount <= 0 && _atsv._delayCount > _printDelayCount1) {
						_mousePush = true;
						_atsv._delayCount = 0;
						g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
						g_events->_kbInfo._keyCode = '\0';
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

		if (_atsv._silentCount <= 0) {
			char *tmp_ptr = _atsv._ptr;
			_atsSsi = _ssi[0];
			_atsSsi._str = tmp_ptr;
			_atsSsi.Fvorx = _G(fontMgr)->getFont()->getDataWidth();
			_atsSsi.FHoehe = _G(fontMgr)->getFont()->getDataHeight();
			_atsSsi._x = x - scrx;
			_atsSsi._y = y - scry;
			char *start_ptr = tmp_ptr;
			str_null2leer(start_ptr, start_ptr + _atsv._txtLen - 1);
			SplitStringInit tmp_ssi = _atsSsi;
			_ssr = split_string(&tmp_ssi);

			for (int16 i = 0; i < _ssr->_nr; i++) {
				_G(out)->printxy(_ssr->_x[i],
				              _ssr->_y + (i * _atsSsi.FHoehe) + 1,
				              0, 300, 0, _ssr->_strPtr[i]);
				_G(out)->printxy(_ssr->_x[i],
				              _ssr->_y + (i * _atsSsi.FHoehe) - 1,
				              0, 300, 0, _ssr->_strPtr[i]);
				_G(out)->printxy(_ssr->_x[i] + 1,
				              _ssr->_y + (i * _atsSsi.FHoehe),
				              0, 300, 0, _ssr->_strPtr[i]);
				_G(out)->printxy(_ssr->_x[i] - 1,
				              _ssr->_y + (i * _atsSsi.FHoehe),
				              0, 300, 0, _ssr->_strPtr[i]);
				_G(out)->printxy(_ssr->_x[i],
				              _ssr->_y + (i * _atsSsi.FHoehe),
				              _atsv._color,
				              300, 0, _ssr->_strPtr[i]);
				tmp_ptr += strlen(_ssr->_strPtr[i]) + 1;
			}

			str_null2leer(start_ptr, start_ptr + _atsv._txtLen - 1);
			if (_atsv._delayCount <= 0) {
				if (_ssr->_next == false) {
					_atsv._display = (_atsv._display == DISPLAY_ALL) ?
						DISPLAY_VOC : DISPLAY_NONE;
				} else {
					_atsv._ptr = tmp_ptr;
					_atsv._txtLen = 0;
					while (*tmp_ptr++ != ATDS_END_TEXT)
						++_atsv._txtLen;
					_atsv._delayCount = get_delay(_atsv._txtLen);
					_printDelayCount1 = _atsv._delayCount / 10;
					_atsv._silentCount = _atdsv._silent;
				}
			} else {
				--_atsv._delayCount;
			}
		} else {
			--_atsv._silentCount;
		}
	}
}

char *Atdsys::ats_get_txt(int16 txt_nr, int16 txt_mode, int16 *txt_anz, int16 mode) {
	char *str_ = nullptr;
	set_ats_mem(mode);

	_atsv._txtMode = txt_mode;

	if (!get_steuer_bit(txt_nr, ATS_AKTIV_BIT, mode)) {
		uint8 status = _ats_sheader[(txt_nr * MAX_ATS_STATUS) + (_atsv._txtMode + 1) / 2];
		int16 ak_nybble = (_atsv._txtMode + 1) % 2;

		uint8 lo_hi[2];
		lo_hi[1] = status >> 4;
		lo_hi[0] = status &= 15;
		str_ = ats_search_block(_atsv._txtMode, _atsmem);
		if (str_ != nullptr) {
			ats_search_nr(txt_nr, &str_);
			if (str_ != nullptr) {
				ats_search_str(txt_anz, &lo_hi[ak_nybble], (uint8)_ats_sheader[txt_nr * MAX_ATS_STATUS], &str_);

				if (str_ != nullptr) {
					status = 0;
					lo_hi[1] <<= 4;
					status |= lo_hi[0];
					status |= lo_hi[1];
					_ats_sheader[(txt_nr * MAX_ATS_STATUS) + (_atsv._txtMode + 1) / 2] = status;
				}
			}
		}
	}

	// WORKAROUND: Proper word wrapping some inventory items' look desc
	if (txt_mode == TXT_MARK_LOOK && str_ != nullptr) {
		switch (txt_nr) {
		case CYB_KRONE_INV:
			if (str_[37] == ' ' && str_[56] == '\0') {
				str_[37] = '\0';
				str_[56] = ' ';
			}
			break;
		case YEL_CARD_INV:
			if (str_[39] == ' ' && str_[46] == '\0') {
				str_[39] = '\0';
				str_[46] = ' ';
			}
			break;
		case PAPIER_INV:
			if (str_[32] == ' ') {
				str_[32] = '\0';
				*txt_anz = 2;
			}
			break;
		default:
			break;
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
		_atsv._txtHeader.load(&rs1);

		if (READ_LE_UINT16(start_str) == 0xFEF0 &&
				_atsv._txtHeader._txtNr == txt_nr) {
			// Found match
			*str = start_str + 2 + AtsTxtHeader::SIZE();

			if (_atsv._txtMode) {
				Common::MemoryReadStream rs2((const byte *)*str,
					AtsStrHeader::SIZE());
				_atsv._strHeader.load(&rs2);
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

				if (_atsv._txtMode != TXT_MARK_NAME) {
					Common::MemoryReadStream rs((const byte *)start_str,
						AtsStrHeader::SIZE());
					_atsv._strHeader.load(&rs);
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

					if (_atsv._txtMode != TXT_MARK_NAME) {
						Common::MemoryReadStream rs((const byte *)start_str,
							AtsStrHeader::SIZE());
						_atsv._strHeader.load(&rs);
					}
				} else {
					++count;
					tmp_str += AtsStrHeader::SIZE() + 2;
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
	if (_aadv._dialog)
		stopAad();

	if (_atdsmem[AAD_HANDLE]) {
		_aadv._ptr = _atdsmem[AAD_HANDLE];
		aad_search_dia(dia_nr, &_aadv._ptr);
		if (_aadv._ptr) {
			_aadv._person.load(_aadv._ptr, _aadv._txtHeader->_perNr);
			_aadv._ptr += _aadv._txtHeader->_perNr * sizeof(AadInfo);

			_aadv._dialog = true;
			_aadv._strNr = 0;
			_aadv._strHeader = (AadStrHeader *)_aadv._ptr;
			_aadv._ptr += sizeof(AadStrHeader);
			int16 txt_len;
			aad_get_zeilen(_aadv._ptr, &txt_len);
			_aadv._delayCount = get_delay(txt_len);
			_printDelayCount1 = _aadv._delayCount / 10;

			_atdsv._diaNr = dia_nr;
			if (_atdsv.aad_str != nullptr)
				_atdsv.aad_str(_atdsv._diaNr, 0, _aadv._strHeader->_akPerson, AAD_STR_START);
			_mousePush = true;
			stop_ats();
			_atdsv._vocNr = -1;
		}
	}

	return _aadv._dialog;
}

void Atdsys::stopAad() {
	_aadv._dialog = false;
	_aadv._strNr = -1;
}

void Atdsys::print_aad(int16 scrx, int16 scry) {
	if (_aadv._dialog) {
		if (_atdsv._eventsEnabled) {
			switch (_G(in)->getSwitchCode()) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_RETURN:
			case MOUSE_LEFT:
				EVENTS_CLEAR;

				if (_mousePush == false) {
					if (_aadv._silentCount <= 0 && _aadv._delayCount > _printDelayCount1) {
						_mousePush = true;
						_aadv._delayCount = 0;
						g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
						g_events->_kbInfo._keyCode = '\0';
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

		if (_aadv._silentCount <= 0) {
			char *tmp_ptr = _aadv._ptr;
			_ssi[_aadv._strHeader->_akPerson]._str = tmp_ptr;
			if (_aadv._person[_aadv._strHeader->_akPerson]._x != -1) {
				_ssi[_aadv._strHeader->_akPerson]._x = _aadv._person[_aadv._strHeader->_akPerson]._x - scrx;
			}
			if (_aadv._person[_aadv._strHeader->_akPerson]._y != -1) {
				_ssi[_aadv._strHeader->_akPerson]._y = _aadv._person[_aadv._strHeader->_akPerson]._y - scry;
			}
			_ssi[_aadv._strHeader->_akPerson].Fvorx = _G(fontMgr)->getFont()->getDataWidth();
			_ssi[_aadv._strHeader->_akPerson].FHoehe = _G(fontMgr)->getFont()->getDataHeight();
			char *start_ptr = tmp_ptr;
			int16 txt_len;
			aad_get_zeilen(start_ptr, &txt_len);
			str_null2leer(start_ptr, start_ptr + txt_len - 1);
			SplitStringInit tmp_ssi = _ssi[_aadv._strHeader->_akPerson];
			_ssr = split_string(&tmp_ssi);

			if (_atdsv.Display != DISPLAY_VOC ||
			        (_aadv._strHeader->_vocNr - ATDS_VOC_OFFSET) == -1) {
				for (int16 i = 0; i < _ssr->_nr; i++) {
					_G(out)->printxy(_ssr->_x[i] + 1,
					              _ssr->_y + (i * _ssi[_aadv._strHeader->_akPerson].FHoehe),
					              0, 300, 0, _ssr->_strPtr[i]);
					_G(out)->printxy(_ssr->_x[i] - 1,
					              _ssr->_y + (i * _ssi[_aadv._strHeader->_akPerson].FHoehe),
					              0, 300, 0, _ssr->_strPtr[i]);
					_G(out)->printxy(_ssr->_x[i],
					              _ssr->_y + (i * _ssi[_aadv._strHeader->_akPerson].FHoehe) + 1,
					              0, 300, 0, _ssr->_strPtr[i]);
					_G(out)->printxy(_ssr->_x[i],
					              _ssr->_y + (i * _ssi[_aadv._strHeader->_akPerson].FHoehe) - 1,
					              0, 300, 0, _ssr->_strPtr[i]);
					_G(out)->printxy(_ssr->_x[i],
					              _ssr->_y + (i * _ssi[_aadv._strHeader->_akPerson].FHoehe),
					              _aadv._person[_aadv._strHeader->_akPerson]._color,
					              300, 0, _ssr->_strPtr[i]);
					tmp_ptr += strlen(_ssr->_strPtr[i]) + 1;
				}
				str_null2leer(start_ptr, start_ptr + txt_len - 1);

			}

			if (_atdsv.Display != DISPLAY_TXT &&
					(_aadv._strHeader->_vocNr - ATDS_VOC_OFFSET) != -1) {
				if (_atdsv._vocNr != _aadv._strHeader->_vocNr - ATDS_VOC_OFFSET) {
					_atdsv._vocNr = _aadv._strHeader->_vocNr - ATDS_VOC_OFFSET;
					g_engine->_sound->playSpeech(_atdsv._vocNr,
						_atdsv.Display == DISPLAY_VOC);
					int16 vocx = _G(spieler_vector)[_aadv._strHeader->_akPerson].Xypos[0] -
								 _G(spieler).scrollx + _G(spieler_mi)[_aadv._strHeader->_akPerson].HotX;
					g_engine->_sound->setSoundChannelBalance(0, getStereoPos(vocx));

					if (_atdsv.Display == DISPLAY_VOC) {
						_aadv._strNr = -1;
						_aadv._delayCount = 1;
					}
				}

				if (_atdsv.Display != DISPLAY_ALL) {
					for (int16 i = 0; i < _ssr->_nr; i++) {
						tmp_ptr += strlen(_ssr->_strPtr[i]) + 1;
					}
					str_null2leer(start_ptr, start_ptr + txt_len - 1);
				}
			}

			if (_aadv._delayCount <= 0) {
				_aadv._ptr = tmp_ptr;
				while (*tmp_ptr == ' ' || *tmp_ptr == 0)
					++tmp_ptr;
				if (tmp_ptr[1] == ATDS_END ||
				        tmp_ptr[1] == ATDS_END_EINTRAG) {
					if (_atdsv.aad_str != 0)
						_atdsv.aad_str(_atdsv._diaNr, _aadv._strNr, _aadv._strHeader->_akPerson, AAD_STR_END);
					_aadv._dialog = false;
					_adsv._autoDia = false;
					_aadv._strNr = -1;
					_ssret._next = false;
				} else {
					if (_ssr->_next == false) {
						++_aadv._strNr;
						while (*_aadv._ptr++ != ATDS_END_TEXT);

						int16 tmp_person = _aadv._strHeader->_akPerson;
						int16 tmp_str_nr = _aadv._strNr;
						_aadv._strHeader = (AadStrHeader *)_aadv._ptr;
						_aadv._ptr += sizeof(AadStrHeader);
						if (_atdsv.aad_str != nullptr) {
							if (tmp_person != _aadv._strHeader->_akPerson) {
								_atdsv.aad_str(_atdsv._diaNr, tmp_str_nr, tmp_person, AAD_STR_END);
								_atdsv.aad_str(_atdsv._diaNr, _aadv._strNr, _aadv._strHeader->_akPerson, AAD_STR_START);
							}
						}
					}
					aad_get_zeilen(_aadv._ptr, &txt_len);
					_aadv._delayCount = get_delay(txt_len);
					_printDelayCount1 = _aadv._delayCount / 10;
					_aadv._silentCount = _atdsv._silent;
				}
			} else {
				if (_atdsv.Display != DISPLAY_VOC ||
				        (_aadv._strHeader->_vocNr - ATDS_VOC_OFFSET) == -1)
					--_aadv._delayCount;

				else if (_atdsv.Display == DISPLAY_VOC) {
					warning("FIXME - unknown constant SMP_PLAYING");

					_aadv._delayCount = 0;
				}
			}
		} else {
			--_aadv._silentCount;
		}
	}
}

int16 Atdsys::aadGetStatus() {
	return _aadv._strNr;
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
				_aadv._txtHeader = (AadTxtHeader *)start_ptr;
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

	load_atds(dia_nr, ADS_DATA);
	bool ende = false;

	if (_atdsmem[ADS_HANDLE][0] == (char)BLOCKENDE &&
		    _atdsmem[ADS_HANDLE][1] == (char)BLOCKENDE &&
		    _atdsmem[ADS_HANDLE][2] == (char)BLOCKENDE)
		ende = true;

	if (!ende) {
		_adsv._ptr = _atdsmem[ADS_HANDLE];
		_adsv._txtHeader.load(_adsv._ptr);

		if (_adsv._txtHeader._diaNr == dia_nr) {
			ret = true;
			_adsv._ptr += AdsTxtHeader::SIZE();
			_adsv._person.load(_adsv._ptr, _adsv._txtHeader._perNr);
			_adsv._ptr += _adsv._txtHeader._perNr * AadInfo::SIZE();
			_adsv._dialog = dia_nr;
			_adsv._strNr = 0;
			_adsStack[0] = 0;
			_adsStackPtr = 1;
		}
	}
	return ret;
}

void Atdsys::stop_ads() {
	_adsv._dialog = -1;
	_adsv._autoDia = false;

}

int16 Atdsys::ads_get_status() {
	return _adsv._dialog;
}

int16 Atdsys::check_item(int16 block_nr, int16 item_nr) {
	int16 ret = true;
	char *tmp_adr = _adsv._ptr;
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
	memset(_ePtr, 0, sizeof(char *) * ADS_MAX_BL_EIN);
	if (_adsv._dialog != -1) {
		_adsv._blkPtr = _adsv._ptr;
		ads_search_block(block_nr, &_adsv._blkPtr);
		if (_adsv._blkPtr) {
			for (int16 i = 0; i < ADS_MAX_BL_EIN; i++) {
				char *tmp_adr = _adsv._blkPtr;
				ads_search_item(i, &tmp_adr);
				if (tmp_adr) {
					char nr = tmp_adr[-1];
					tmp_adr += sizeof(AadStrHeader);
					if (_adsBlock[block_nr]._show[(int16)nr] == true) {
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
	_adsnb._blkNr = blk_nr;
	if (!_aadv._dialog) {
		if (!_adsv._autoDia) {
			ads_search_item(_eNr[item_nr], &_adsv._blkPtr);
			if (_adsv._blkPtr) {
				if (start_ads_auto_dia(_adsv._blkPtr))
					_adsv._autoDia = true;
				if (_G(bit)->is_bit((uint8)_adsBlock[blk_nr].Steuer[_eNr[item_nr]], ADS_EXIT_BIT) == true) {
					stop_ads();
					_adsnb._endNr = _eNr[item_nr];
					_adsnb._blkNr = -1;
				}
			}
		}
	}

	return &_adsnb;
}

AdsNextBlk *Atdsys::calc_next_block(int16 blk_nr, int16 item_nr) {
	if (_G(bit)->is_bit((uint8)_adsBlock[blk_nr].Steuer[_eNr[item_nr]], ADS_SHOW_BIT) == false)
		_adsBlock[blk_nr]._show[_eNr[item_nr]] = false;
	_adsnb._endNr = _eNr[item_nr];
	if (_G(bit)->is_bit((uint8)_adsBlock[blk_nr].Steuer[_eNr[item_nr]], ADS_RESTART_BIT) == true) {
		_adsnb._blkNr = 0;

		_adsStackPtr = 0;
	} else {
		if (_adsBlock[blk_nr]._next[_eNr[item_nr]]) {
			_adsnb._blkNr = _adsBlock[blk_nr]._next[_eNr[item_nr]];

			int16 anzahl = 0;
			while (!anzahl && _adsnb._blkNr != -1) {

				anzahl = 0;
				ads_item_ptr(_adsnb._blkNr, &anzahl);
				if (!anzahl) {
					_adsnb._blkNr = return_block(_adsBlock);
				}
			}
		} else {
			_adsnb._blkNr = return_block(_adsBlock);
		}
	}
	_adsStack[_adsStackPtr] = _adsnb._blkNr;
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
		} else {
			--_adsStackPtr;
		}
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
	_aadv._dialog = false;
	if (item_adr) {
		_aadv._person = _adsv._person;
		_aadv._ptr = item_adr;
		_aadv._dialog = true;
		_aadv._strNr = 0;
		_aadv._strHeader = (AadStrHeader *)_aadv._ptr;
		_aadv._ptr += sizeof(AadStrHeader);
		int16 txt_len;
		aad_get_zeilen(_aadv._ptr, &txt_len);
		_aadv._delayCount = get_delay(txt_len);
		_atdsv._diaNr = _adsv._txtHeader._diaNr + 10000;

		if (_atdsv.aad_str != nullptr)
			_atdsv.aad_str(_atdsv._diaNr, 0, _aadv._strHeader->_akPerson, AAD_STR_START);
		_mousePush = true;
		stop_ats();
	} else {
		_aadv._dialog = false;
	}

	return _aadv._dialog;
}

void Atdsys::hide_item(int16 dia_nr, int16 blk_nr, int16 item_nr) {
	if (_adsv._dialog == dia_nr) {
		if (check_item(blk_nr, item_nr))
			_adsBlock[blk_nr]._show[item_nr] = false;
	} else {
		load_atds(dia_nr, ADH_DATA);
		if (check_item(blk_nr, item_nr))
			_adsBlock[blk_nr]._show[item_nr] = false;
		save_ads_header(dia_nr);
	}
}

void Atdsys::show_item(int16 dia_nr, int16 blk_nr, int16 item_nr) {
	if (_adsv._dialog == dia_nr) {
		if (check_item(blk_nr, item_nr))
			_adsBlock[blk_nr]._show[item_nr] = true;
	} else {
		load_atds(dia_nr, ADH_DATA);
		if (check_item(blk_nr, item_nr))
			_adsBlock[blk_nr]._show[item_nr] = true;
		save_ads_header(dia_nr);
	}
}

int16 Atdsys::calc_inv_no_use(int16 cur_inv, int16 test_nr, int16 mode) {
	int16 txt_nr = -1;
	if (cur_inv != -1) {
		if (_invBlockNr != cur_inv) {
			_invBlockNr = cur_inv + 1;
			load_atds(_invBlockNr + _atdspooloff[mode], INV_USE_DATA);

			Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(
				_atdshandle[INV_IDX_HANDLE]);
			if (rs) {
				rs->seek(InvUse::SIZE() * _invBlockNr
				      * INV_STRC_NR, SEEK_SET);

				InvUse *iu = (InvUse *)_atdsmem[INV_IDX_HANDLE];
				for (int16 i = 0; i < INV_STRC_NR; ++i, ++iu) {
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

		for (int16 i = 0; i < INV_STRC_NR && !ok; i++) {
			if (iu[i]._objId == mode) {
				if (iu[i]._objNr == test_nr) {
					txt_nr = iu[i]._txtNr;
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

void Atdsys::saveAtdsStream(Common::WriteStream *stream) {
	_dialogResource->saveStream(stream);
}

void Atdsys::loadAtdsStream(Common::SeekableReadStream* stream) {
	_dialogResource->loadStream(stream);
}

uint32 Atdsys::getAtdsStreamSize() const {
	return _dialogResource->getStreamSize();
}
} // namespace Chewy
