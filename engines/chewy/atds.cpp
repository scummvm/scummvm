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
#include "chewy/ngsdefs.h"
#include "chewy/sound.h"
#include "chewy/text.h"

namespace Chewy {

bool AtsTxtHeader::load(Common::SeekableReadStream *src) {
	_txtNr = src->readUint16LE();
	_aMov = src->readSint16LE();
	_curNr = src->readSint16LE();
	src->skip(2);

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

Atdsys::Atdsys() {
	SplitStringInit init_ssi = { nullptr, 0, 0, 220, 4, SPLIT_CENTER };
	_aadv._dialog = false;
	_aadv._strNr = -1;
	_aadv._silentCount = false;
	_adsv._dialog = -1;
	_adsv._autoDia = false;
	_adsv._strNr = -1;
	_adsv._silentCount = false;
	_tmpDelay = 1;
	_atdsv._delay = &_tmpDelay;
	_atdsv._silent = false;
	_atdsv._diaNr = -1;
	_atdsv.aad_str = nullptr;
	_atdsv._vocNr = -1;
	_atdsv._eventsEnabled = true;
	_ssret._next = false;
	_ssr = &_ssret;
	for (int16 i = 0; i < AAD_MAX_PERSON; i++)
		_ssi[i] = init_ssi;
	_invBlockNr = -1;
	_invUseMem = nullptr;

	_dialogResource = new DialogResource(ADS_TXT_STEUER);
	_text = new Text();

	_adsnb._blkNr = 0;
	_adsnb._endNr = 0;
	_adsStackPtr = 0;

	init();
	initItemUseWith();
}

Atdsys::~Atdsys() {
	if (_atdsHandle) {
		delete _atdsHandle;
		_atdsHandle = nullptr;
	}

	for (int16 i = 0; i < MAX_HANDLE; i++) {
		if (_atdsMem[i])
			free(_atdsMem[i]);
		_atdsMem[i] = nullptr;	
	}

	if (_invUseMem)
		free(_invUseMem);

	delete _dialogResource;
}

void Atdsys::init() {
	_atdsHandle = new Common::File();
	_atdsHandle->open(ATDS_TXT);
	if (!_atdsHandle->isOpen()) {
		error("Error opening %s", ATDS_TXT);
	}

	set_handle(ATDS_TXT, ATS_DATA, ATS_TAP_OFF, ATS_TAP_MAX);
	set_handle(ATDS_TXT, INV_ATS_DATA, INV_TAP_OFF, INV_TAP_MAX);
	set_handle(ATDS_TXT, AAD_DATA, AAD_TAP_OFF, AAD_TAP_MAX);
	set_handle(ATDS_TXT, ADS_DATA, ADS_TAP_OFF, ADS_TAP_MAX);
	set_handle(ATDS_TXT, INV_USE_DATA, USE_TAP_OFF, USE_TAP_MAX);
	_G(gameState).AadSilent = 10;
	_G(gameState).DelaySpeed = 5;
	_G(spieler_vector)
	[P_CHEWY].Delay = _G(gameState).DelaySpeed;
	set_delay(&_G(gameState).DelaySpeed, _G(gameState).AadSilent);
	set_string_end_func(&atdsStringStart);
}

void Atdsys::initItemUseWith() {
	int16 objA, objB, txtNum;

	Common::File f;
	f.open(INV_USE_IDX);

	// The file is 25200 bytes, and contains 25200 / 50 / 6 = 84 blocks
	int totalEntries = f.size() / 6;

	for (int entry = 0; entry < totalEntries; entry++) {
		objA = f.readSint16LE();
		objB = f.readSint16LE();
		txtNum = f.readSint16LE();

		assert(objA <= 255 && objB <= 65535);

		const uint32 key = (objA & 0xff) << 16 | objB;
		_itemUseWithDesc[key] = txtNum;
	}

	f.close();
}

void Atdsys::set_delay(int16 *delay, int16 silent) {
	_atdsv._delay = delay;
	_atdsv._silent = silent;
}

void Atdsys::set_string_end_func
(void (*strFunc)(int16 diaNr, int16 strNr, int16 personNr, int16 mode)) {
	_atdsv.aad_str = strFunc;
}

int16 Atdsys::get_delay(int16 txt_len) {
	const int16 w = _G(fontMgr)->getFont()->getDataWidth();
	int16 z_len = (_ssi->_width / w) + 1;
	int16 maxLen = z_len * _ssi->_lines;
	if (txt_len > maxLen)
		txt_len = maxLen;

	int16 ret = *_atdsv._delay * (txt_len + z_len);
	return ret;
}

SplitStringRet *Atdsys::split_string(SplitStringInit *ssi_) {
	const int16 w = _G(fontMgr)->getFont()->getDataWidth();
	const int16 h = _G(fontMgr)->getFont()->getDataHeight();

	_ssret._nr = 0;
	_ssret._next = false;
	_ssret._strPtr = _splitPtr;
	_ssret._x = _splitX;
	int16 zeichen_anz = (ssi_->_width / w) + 1;
	memset(_splitPtr, 0, sizeof(char *) * MAX_STR_SPLIT);
	calc_txt_win(ssi_);
	char *str_adr = ssi_->_str;
	int16 count = 0;
	int16 tmp_count = 0;
	bool endLoop = false;
	char *start_adr = str_adr;

	while (!endLoop) {
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
				if (ssi_->_mode == SPLIT_CENTER)
					_splitX[_ssret._nr] = ssi_->_x + ((ssi_->_width - (strlen(start_adr) * w)) >> 1);
				else
					_splitX[_ssret._nr] = ssi_->_x;
				++_ssret._nr;
				if (_ssret._nr == ssi_->_lines) {
					endLoop = true;
					bool endInnerLoop = false;
					while (!endInnerLoop) {
						if (*str_adr == ATDS_END_TEXT)
							endInnerLoop = true;
						else if (*str_adr != ' ' && *str_adr != 0) {
							endInnerLoop = true;
							_ssret._next = true;
						}
						++str_adr;
					}
				} else if (*str_adr == 0 && count < zeichen_anz) {
					endLoop = true;
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
						endLoop = true;
					}
					_splitPtr[_ssret._nr] = start_adr;
					start_adr[tmp_count] = 0;
					if (ssi_->_mode == SPLIT_CENTER)
						_splitX[_ssret._nr] = ssi_->_x + ((ssi_->_width - (strlen(start_adr) * w)) >> 1);
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
					if (!endLoop) {
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
		_ssret._y = ssi_->_y + (ssi_->_lines - _ssret._nr) * h;
	else
		_ssret._y = ssi_->_y;

	return &_ssret;
}

void Atdsys::str_null2leer(char *strStart, char *strEnd) {
	while (strStart < strEnd) {
		if (*strStart == 0)
			*strStart = 32;
		++strStart;
	}
}

void Atdsys::calc_txt_win(SplitStringInit *ssi_) {
	const int16 h = _G(fontMgr)->getFont()->getDataHeight();

	if (ssi_->_x - (ssi_->_width >> 1) < 2)
		ssi_->_x = 2;
	else if (ssi_->_x + (ssi_->_width >> 1) > (SCREEN_WIDTH - 2))
		ssi_->_x = ((SCREEN_WIDTH - 2) - ssi_->_width);
	else
		ssi_->_x -= (ssi_->_width >> 1);

	if (ssi_->_y - (ssi_->_lines * h) < 2) {
		ssi_->_y = 2;
	} else if (ssi_->_y + (ssi_->_lines * h) > (SCREEN_HEIGHT - 2))
		ssi_->_y = (SCREEN_HEIGHT - 2) - (ssi_->_lines * h);
	else {
		ssi_->_y -= (ssi_->_lines * h);
	}
}

void Atdsys::set_split_win(int16 nr, int16 x, int16 y) {
	_ssi[nr]._x = x;
	_ssi[nr]._y = y;
}

void Atdsys::set_handle(const char *fname, int16 mode, int16 chunkStart, int16 chunkNr) {
	ChunkHead Ch;
	char *tmp_adr = atds_adr(fname, chunkStart, chunkNr);
	if (_atdsHandle) {
		if (_atdsMem[mode])
			free(_atdsMem[mode]);
		_atdsMem[mode] = tmp_adr;
		_atdsPoolOff[mode] = chunkStart;
		switch (mode) {
		case INV_USE_DATA:
			_G(mem)->file->selectPoolItem(_atdsHandle, _atdsPoolOff[mode]);
			_atdsHandle->seek(-ChunkHead::SIZE(), SEEK_CUR);

			if (!Ch.load(_atdsHandle)) {
				error("Error reading from %s", fname);
			} else {
				free(_invUseMem);
				_invUseMem = (char *)MALLOC(Ch.size + 3l);

				if (Ch.size) {
					if (!_atdsHandle->read(_invUseMem, Ch.size)) {
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

char *Atdsys::atds_adr(const char *fname, int16 chunkStart, int16 chunkNr) {
	char *tmp_adr = nullptr;
	uint32 size = _G(mem)->file->getPoolSize(fname, chunkStart, chunkNr);
	if (size) {
		tmp_adr = (char *)MALLOC(size + 3);
	}

	return tmp_adr;
}

void Atdsys::load_atds(int16 chunkNr, int16 mode) {
	char *txt_adr = _atdsMem[mode];

	ChunkHead Ch;
	if (_atdsHandle && txt_adr) {
		_G(mem)->file->selectPoolItem(_atdsHandle, chunkNr + _atdsPoolOff[mode]);
		_atdsHandle->seek(-ChunkHead::SIZE(), SEEK_CUR);
		if (!Ch.load(_atdsHandle)) {
			error("load_atds error");
		} else {
			if (Ch.size) {
				if (_atdsHandle->read(txt_adr, Ch.size) != Ch.size) {
					error("load_atds error");
				} else {
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

void Atdsys::crypt(char *txt, uint32 size) {
	uint8 *sp = (uint8 *)txt;
	for (uint32 i = 0; i < size; i++) {
		*sp = -(*sp);
		++sp;
	}
}

void Atdsys::set_ats_mem(int16 mode) {
	switch (mode) {
	case ATS_DATA:
		_ats_sheader = _G(gameState).Ats;
		_atsMem = _atdsMem[mode];
		break;

	case INV_USE_DATA:
		_ats_sheader = _G(gameState).InvUse;
		_atsMem = _atdsMem[mode];
		break;

	case INV_USE_DEF:
		_ats_sheader = _G(gameState).InvUseDef;
		_atsMem = _invUseMem;
		break;

	case INV_ATS_DATA:
		_ats_sheader = _G(gameState).InvAts;
		_atsMem = _atdsMem[mode];
		break;

	default:
		break;
	}
}

bool Atdsys::start_ats(int16 txtNr, int16 txtMode, int16 color, int16 mode, int16 *vocNr) {
	*vocNr = -1;
	set_ats_mem(mode);

	_atsv.shown = false;

	if (_atsMem) {
		//const uint8 roomNum = _G(room)->_roomInfo->_roomNr;
		int16 txt_anz;
		_atsv._ptr = ats_get_txt(txtNr, txtMode, &txt_anz, mode);
		//_atsv._ptr = (char *)getTextEntry(roomNum, txtNr, txtMode).c_str();

		if (_atsv._ptr) {
			_atsv.shown = g_engine->_sound->subtitlesEnabled();
			char *ptr = _atsv._ptr;
			_atsv._txtLen = 0;

			while (*ptr++ != ATDS_END_TEXT)
				++_atsv._txtLen;

			*vocNr = _atsv.vocNum - ATDS_VOC_OFFSET;

			if ((byte)*_atsv._ptr == 248) {
				// Special code for no message to display
				_atsv.shown = false;

			} else {
				_atsv._delayCount = get_delay(_atsv._txtLen);
				_printDelayCount1 = _atsv._delayCount / 10;
				_atsv._color = color;
				_mousePush = true;

				if (*vocNr == -1) {
					_atsv.shown = g_engine->_sound->subtitlesEnabled();
				}
			}
		}
	}

	return _atsv.shown;
}

void Atdsys::stop_ats() {
	_atsv.shown = false;
}

void Atdsys::print_ats(int16 x, int16 y, int16 scrX, int16 scrY) {
	if (_atsv.shown) {
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
			SplitStringInit *_atsSsi = &_ssi[0];
			_atsSsi->_str = tmp_ptr;
			_atsSsi->_x = x - scrX;
			_atsSsi->_y = y - scrY;
			char *start_ptr = tmp_ptr;
			str_null2leer(start_ptr, start_ptr + _atsv._txtLen - 1);
			_ssr = split_string(_atsSsi);
			const int16 h = _G(fontMgr)->getFont()->getDataHeight();

			for (int16 i = 0; i < _ssr->_nr; i++) {
				_G(out)->printxy(_ssr->_x[i],
				              _ssr->_y + (i * h) + 1,
				              0, 300, 0, _ssr->_strPtr[i]);
				_G(out)->printxy(_ssr->_x[i],
				              _ssr->_y + (i * h) - 1,
				              0, 300, 0, _ssr->_strPtr[i]);
				_G(out)->printxy(_ssr->_x[i] + 1,
				              _ssr->_y + (i * h),
				              0, 300, 0, _ssr->_strPtr[i]);
				_G(out)->printxy(_ssr->_x[i] - 1,
				              _ssr->_y + (i * h),
				              0, 300, 0, _ssr->_strPtr[i]);
				_G(out)->printxy(_ssr->_x[i],
				              _ssr->_y + (i * h),
				              _atsv._color,
				              300, 0, _ssr->_strPtr[i]);
				tmp_ptr += strlen(_ssr->_strPtr[i]) + 1;
			}

			str_null2leer(start_ptr, start_ptr + _atsv._txtLen - 1);
			if (_atsv._delayCount <= 0) {
				if (_ssr->_next == false) {
					_atsv.shown = false;
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

char *Atdsys::ats_get_txt(int16 txtNr, int16 txtMode, int16 *retNr, int16 mode) {
	char *str_ = nullptr;
	set_ats_mem(mode);

	_atsv._txtMode = txtMode;

	if (!getControlBit(txtNr, ATS_ACTIVE_BIT)) {
		uint8 status = _ats_sheader[(txtNr * MAX_ATS_STATUS) + (_atsv._txtMode + 1) / 2];
		int16 ak_nybble = (_atsv._txtMode + 1) % 2;

		uint8 lo_hi[2];
		lo_hi[1] = status >> 4;
		lo_hi[0] = status &= 15;
		str_ = ats_search_block(_atsv._txtMode, _atsMem);
		if (str_ != nullptr) {
			ats_search_nr(txtNr, &str_);
			if (str_ != nullptr) {
				ats_search_str(retNr, &lo_hi[ak_nybble], (uint8)_ats_sheader[txtNr * MAX_ATS_STATUS], &str_);

				if (str_ != nullptr) {
					status = 0;
					lo_hi[1] <<= 4;
					status |= lo_hi[0];
					status |= lo_hi[1];
					_ats_sheader[(txtNr * MAX_ATS_STATUS) + (_atsv._txtMode + 1) / 2] = status;
				}
			}
		}
	}

	// WORKAROUND: Proper word wrapping some inventory items' look desc
	if (txtMode == TXT_MARK_LOOK && str_ != nullptr) {
		switch (txtNr) {
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
				*retNr = 2;
			}
			break;
		default:
			break;
		}
	}

	return str_;
}

void Atdsys::set_ats_str(int16 txtNr, int16 txtMode, int16 strNr, int16 mode) {
	set_ats_mem(mode);
	uint8 status = _ats_sheader[(txtNr * MAX_ATS_STATUS) + (txtMode + 1) / 2];
	int16 ak_nybble = (txtMode + 1) % 2;

	uint8 lo_hi[2];
	lo_hi[1] = status >> 4;
	lo_hi[0] = status &= 15;
	lo_hi[ak_nybble] = strNr;
	status = 0;
	lo_hi[1] <<= 4;
	status |= lo_hi[0];
	status |= lo_hi[1];
	_ats_sheader[(txtNr * MAX_ATS_STATUS) + (txtMode + 1) / 2] = status;
}

void Atdsys::set_ats_str(int16 txtNr, int16 strNr, int16 mode) {
	for (int16 i = 0; i < 5; i++)
		set_ats_str(txtNr, i, strNr, mode);
}

int16 Atdsys::get_ats_str(int16 txtNr, int16 txtMode, int16 mode) {
	set_ats_mem(mode);
	uint8 status = _ats_sheader[(txtNr * MAX_ATS_STATUS) + (txtMode + 1) / 2];
	int16 ak_nybble = (txtMode + 1) % 2;

	uint8 lo_hi[2];
	lo_hi[1] = status >> 4;
	lo_hi[0] = status &= 15;

	return (int16)lo_hi[ak_nybble];
}

int16 Atdsys::getControlBit(int16 txtNr, int16 bitIdx) {
	set_ats_mem(ATS_DATA);
	return (_ats_sheader[txtNr * MAX_ATS_STATUS] & bitIdx) != 0;
}

void Atdsys::setControlBit(int16 txtNr, int16 bitIdx) {
	set_ats_mem(ATS_DATA);
	_ats_sheader[txtNr * MAX_ATS_STATUS] |= bitIdx;
}

void Atdsys::delControlBit(int16 txtNr, int16 bitIdx) {
	set_ats_mem(ATS_DATA);
	_ats_sheader[txtNr * MAX_ATS_STATUS] &= ~bitIdx;
}

char *Atdsys::ats_search_block(int16 txtMode, char *txtAdr) {
	char *strP = txtAdr;
	int ende = 0;

	for (; !ende; ++strP) {
		if (strP[0] == (char)BLOCKENDE &&
		        strP[1] == (char)BLOCKENDE &&
		        strP[2] == (char)BLOCKENDE) {
			ende = 2;
		} else if (strP[0] == (char)0xf2 && strP[1] == (char)0xfe) {
			if (strP[2] == (char)txtMode)
				ende = 1;
			strP += 2;
		}
	}

	if (ende == 2)
		strP = nullptr;

	return strP;
}

void Atdsys::ats_search_nr(int16 txtNr, char **str) {
	char *start_str = *str;

	bool done1 = false;
	while (!done1) {
		Common::MemoryReadStream rs1((const byte *)start_str + 2, AtsTxtHeader::SIZE());
		_atsv._txtHeader.load(&rs1);

		if (READ_LE_UINT16(start_str) == 0xFEF0 &&
				_atsv._txtHeader._txtNr == txtNr) {
			// Found match
			*str = start_str + 2 + AtsTxtHeader::SIZE();

			if (_atsv._txtMode) {
				_atsv.vocNum = READ_LE_UINT16(str);
			}

			*str += 2;	// voc number
			break;
		}

		start_str += 2 + AtsTxtHeader::SIZE() + 2;	// txt header + voc number

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

void Atdsys::ats_search_str(int16 *nr, uint8 *status, uint8 controlByte, char **str) {
	char *tmp_str = *str;
	char *start_str = *str;
	*nr = 0;
	bool endLoop = false;
	int16 count = 0;

	while (!endLoop) {
		if (count == *status) {
			if (!*tmp_str) {
				++*nr;
			} else if (*tmp_str == ATDS_END_TEXT) {
				endLoop = true;
				*str = start_str;
				start_str -= 2;	// voc number

				if (_atsv._txtMode != TXT_MARK_NAME) {
					_atsv.vocNum = READ_LE_UINT16(start_str);
				}

				if (tmp_str[1] != ATDS_END) {
					if (!(controlByte & ATS_COUNT_BIT))
						++*status;
				} else {
					if (controlByte & ATS_RESET_BIT)
						*status = 0;
				}
			}
		} else {
			if (*tmp_str == ATDS_END_TEXT) {
				if (tmp_str[1] == ATDS_END) {
					endLoop = false;
					*nr = 0;
					*status = count;
					*str = start_str;
					start_str -= 2; // voc number

					if (_atsv._txtMode != TXT_MARK_NAME) {
						_atsv.vocNum = READ_LE_UINT16(start_str);
					}
				} else {
					++count;
					tmp_str += 2 + 2;	// voc number + 2
					start_str = tmp_str + 1;
				}
			} else if (*tmp_str == ATDS_END ||
			           (tmp_str[0] == (char)BLOCKENDE &&
			            tmp_str[1] == (char)BLOCKENDE &&
			            tmp_str[2] == (char)BLOCKENDE)) {
				endLoop = false;
				*nr = 0;
				*str = nullptr;
			}
		}

		++tmp_str;
	}
}

int16 Atdsys::start_aad(int16 diaNr) {
	if (_aadv._dialog)
		stopAad();

	if (_atdsMem[AAD_HANDLE]) {
		_aadv._ptr = _atdsMem[AAD_HANDLE];
		aad_search_dia(diaNr, &_aadv._ptr);

		//const uint8 roomNum = _G(room)->_roomInfo->_roomNr;
		//Common::StringArray s = getTextArray(roomNum, diaNr, AAD_DATA);

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

			_atdsv._diaNr = diaNr;
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

void Atdsys::print_aad(int16 scrX, int16 scrY) {
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
			const int16 personId = _aadv._strHeader->_akPerson;
			_ssi[personId]._str = tmp_ptr;
			if (_aadv._person[personId]._x != -1) {
				_ssi[personId]._x = _aadv._person[personId]._x - scrX;
			}
			if (_aadv._person[personId]._y != -1) {
				_ssi[personId]._y = _aadv._person[personId]._y - scrY;
			}
			char *start_ptr = tmp_ptr;
			int16 txt_len;
			aad_get_zeilen(start_ptr, &txt_len);
			str_null2leer(start_ptr, start_ptr + txt_len - 1);
			SplitStringInit tmp_ssi = _ssi[personId];
			_ssr = split_string(&tmp_ssi);

			if (g_engine->_sound->subtitlesEnabled() ||
			        (_aadv._strHeader->_vocNr - ATDS_VOC_OFFSET) == -1) {
				const int16 h = _G(fontMgr)->getFont()->getDataHeight();
				for (int16 i = 0; i < _ssr->_nr; i++) {
					_G(out)->printxy(_ssr->_x[i] + 1,
					              _ssr->_y + (i * h),
					              0, 300, 0, _ssr->_strPtr[i]);
					_G(out)->printxy(_ssr->_x[i] - 1,
					              _ssr->_y + (i * h),
					              0, 300, 0, _ssr->_strPtr[i]);
					_G(out)->printxy(_ssr->_x[i],
					              _ssr->_y + (i * h) + 1,
					              0, 300, 0, _ssr->_strPtr[i]);
					_G(out)->printxy(_ssr->_x[i],
					              _ssr->_y + (i * h) - 1,
					              0, 300, 0, _ssr->_strPtr[i]);
					_G(out)->printxy(_ssr->_x[i],
					              _ssr->_y + (i * h),
					              _aadv._person[personId]._color,
					              300, 0, _ssr->_strPtr[i]);
					tmp_ptr += strlen(_ssr->_strPtr[i]) + 1;
				}
				str_null2leer(start_ptr, start_ptr + txt_len - 1);

			}

			if (g_engine->_sound->speechEnabled() &&
					(_aadv._strHeader->_vocNr - ATDS_VOC_OFFSET) != -1) {
				if (_atdsv._vocNr != _aadv._strHeader->_vocNr - ATDS_VOC_OFFSET) {
					_atdsv._vocNr = _aadv._strHeader->_vocNr - ATDS_VOC_OFFSET;
					g_engine->_sound->playSpeech(_atdsv._vocNr, !g_engine->_sound->subtitlesEnabled());
					int16 vocx = _G(spieler_vector)[personId].Xypos[0] -
								 _G(gameState).scrollx + _G(spieler_mi)[personId].HotX;
					g_engine->_sound->setSoundChannelBalance(0, getStereoPos(vocx));

					if (!g_engine->_sound->subtitlesEnabled()) {
						_aadv._strNr = -1;
						_aadv._delayCount = 1;
					}
				}

				// FIXME: This breaks subtitles, as it removes
				// all string terminators. This was previously
				// used when either speech or subtitles (but not
				// both) were selected, but its logic is broken.
				// Check if it should be removed altogether.
				/*for (int16 i = 0; i < _ssr->_nr; i++) {
					tmp_ptr += strlen(_ssr->_strPtr[i]) + 1;
				}
				str_null2leer(start_ptr, start_ptr + txt_len - 1);*/
			}

			if (_aadv._delayCount <= 0) {
				_aadv._ptr = tmp_ptr;
				while (*tmp_ptr == ' ' || *tmp_ptr == 0)
					++tmp_ptr;
				if (tmp_ptr[1] == ATDS_END ||
				        tmp_ptr[1] == ATDS_END_ENTRY) {
					if (_atdsv.aad_str != 0)
						_atdsv.aad_str(_atdsv._diaNr, _aadv._strNr, personId, AAD_STR_END);
					_aadv._dialog = false;
					_adsv._autoDia = false;
					_aadv._strNr = -1;
					_ssret._next = false;
				} else {
					if (_ssr->_next == false) {
						++_aadv._strNr;
						while (*_aadv._ptr++ != ATDS_END_TEXT) {}

						const int16 tmp_person = _aadv._strHeader->_akPerson;
						const int16 tmp_str_nr = _aadv._strNr;
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
				if (g_engine->_sound->subtitlesEnabled() ||
				        (_aadv._strHeader->_vocNr - ATDS_VOC_OFFSET) == -1)
					--_aadv._delayCount;

				else if (!g_engine->_sound->subtitlesEnabled()) {
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

int16 Atdsys::aad_get_zeilen(char *str, int16 *txtLen) {
	*txtLen = 0;
	char *ptr = str;
	int16 zeilen = 0;
	while (*str != ATDS_END_TEXT) {
		if (*str++ == 0)
			++zeilen;
	}
	*txtLen = (str - ptr) - 1;

	return zeilen;
}

void Atdsys::aad_search_dia(int16 diaNr, char **ptr) {
	char *start_ptr = *ptr;

	if (start_ptr[0] == (char)BLOCKENDE &&
	        start_ptr[1] == (char)BLOCKENDE &&
	        start_ptr[2] == (char)BLOCKENDE) {
		*ptr = nullptr;
	} else {
		bool ende = false;
		while (!ende) {
			uint16 *pos = (uint16 *)start_ptr;
			if (pos[0] == diaNr) {
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

bool  Atdsys::ads_start(int16 diaNr) {
	bool ret = false;

	load_atds(diaNr, ADS_DATA);
	bool ende = false;

	if (_atdsMem[ADS_HANDLE][0] == (char)BLOCKENDE &&
		    _atdsMem[ADS_HANDLE][1] == (char)BLOCKENDE &&
		    _atdsMem[ADS_HANDLE][2] == (char)BLOCKENDE)
		ende = true;

	if (!ende) {
		_adsv._ptr = _atdsMem[ADS_HANDLE];
		_adsv._txtHeader.load(_adsv._ptr);

		if (_adsv._txtHeader._diaNr == diaNr) {
			ret = true;
			_adsv._ptr += AdsTxtHeader::SIZE();
			_adsv._person.load(_adsv._ptr, _adsv._txtHeader._perNr);
			_adsv._ptr += _adsv._txtHeader._perNr * AadInfo::SIZE();
			_adsv._dialog = diaNr;
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

char **Atdsys::ads_item_ptr(uint16 dialogNum, int16 blockNr, int16 *retNr) {
	*retNr = 0;
	memset(_ePtr, 0, sizeof(char *) * ADS_MAX_BL_EIN);
	if (_adsv._dialog != -1) {
		_adsv._blkPtr = _adsv._ptr;
		ads_search_block(blockNr, &_adsv._blkPtr);
		if (_adsv._blkPtr) {
			for (int16 i = 0; i < ADS_MAX_BL_EIN; i++) {
				char *tmp_adr = _adsv._blkPtr;
				ads_search_item(i, &tmp_adr);
				if (tmp_adr) {
					char nr = tmp_adr[-1];
					tmp_adr += sizeof(AadStrHeader);
					if (_dialogResource->isItemShown(dialogNum, blockNr, (int16)nr)) {
						_ePtr[*retNr] = tmp_adr;
						_eNr[*retNr] = (int16)nr;
						++(*retNr);
					}
				}
			}
		}
	}

	return _ePtr;
}

AdsNextBlk *Atdsys::ads_item_choice(uint16 dialogNum, int16 blockNr, int16 itemNr) {
	_adsnb._blkNr = blockNr;
	if (!_aadv._dialog) {
		if (!_adsv._autoDia) {
			ads_search_item(_eNr[itemNr], &_adsv._blkPtr);
			if (_adsv._blkPtr) {
				if (start_ads_auto_dia(_adsv._blkPtr))
					_adsv._autoDia = true;
				if (_dialogResource->hasExitBit(dialogNum, blockNr, _eNr[itemNr])) {
					stop_ads();
					_adsnb._endNr = _eNr[itemNr];
					_adsnb._blkNr = -1;
				}
			}
		}
	}

	return &_adsnb;
}

AdsNextBlk *Atdsys::calc_next_block(uint16 dialogNum, int16 blockNr, int16 itemNr) {
	if (!_dialogResource->hasShowBit(dialogNum, blockNr, _eNr[itemNr]))
		_dialogResource->setItemShown(dialogNum, blockNr, _eNr[itemNr], false);
	_adsnb._endNr = _eNr[itemNr];

	if (_dialogResource->hasRestartBit(dialogNum, blockNr, _eNr[itemNr])) {
		_adsnb._blkNr = 0;

		_adsStackPtr = 0;
	} else {
		const uint8 nextBlock = _dialogResource->getNextBlock(dialogNum, blockNr, _eNr[itemNr]);
		if (nextBlock) {
			_adsnb._blkNr = nextBlock;

			int16 anzahl = 0;
			while (!anzahl && _adsnb._blkNr != -1) {

				anzahl = 0;
				ads_item_ptr(dialogNum, _adsnb._blkNr, &anzahl);
				if (!anzahl) {
					_adsnb._blkNr = return_block(dialogNum);
				}
			}
		} else {
			_adsnb._blkNr = return_block(dialogNum);
		}
	}
	_adsStack[_adsStackPtr] = _adsnb._blkNr;
	++_adsStackPtr;

	return &_adsnb;
}

int16 Atdsys::return_block(uint16 dialogNum) {
	_adsStackPtr -= 1;
	int16 ret = -1;
	bool ende = false;
	while (_adsStackPtr >= 0 && !ende) {
		short blk_nr = _adsStack[_adsStackPtr];
		int16 anz;
		ads_item_ptr(dialogNum, blk_nr, &anz);
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

void Atdsys::ads_search_block(int16 blockNr, char **ptr) {
	char *start_ptr = *ptr;
	bool ende = false;
	while (!ende) {
		if (*start_ptr == (char)blockNr) {
			ende = true;
			*ptr = start_ptr;
		} else {
			start_ptr += 2 + sizeof(AadStrHeader);
			while (*start_ptr++ != ATDS_END_BLOCK) {}
			if (start_ptr[0] == ATDS_END &&
			        start_ptr[1] == ATDS_END) {
				ende = true;
				*ptr = nullptr;
			}
		}
	}
}

void Atdsys::ads_search_item(int16 itemNr, char **blkAdr) {
	char *start_ptr = *blkAdr + 1;
	bool ende = false;
	while (!ende) {
		if (*start_ptr == itemNr) {
			ende = true;
			*blkAdr = start_ptr + 1;
		} else {
			start_ptr += 1 + sizeof(AadStrHeader);
			while (*start_ptr++ != ATDS_END_ENTRY) {}
			if (*start_ptr == ATDS_END_BLOCK) {
				ende = true;
				*blkAdr = nullptr;
			}
		}
	}
}

int16 Atdsys::start_ads_auto_dia(char *itemAdr) {
	_aadv._dialog = false;
	if (itemAdr) {
		_aadv._person = _adsv._person;
		_aadv._ptr = itemAdr;
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

void Atdsys::hide_item(int16 diaNr, int16 blockNr, int16 itemNr) {
	_dialogResource->setItemShown(diaNr, blockNr, itemNr, false);
}

void Atdsys::show_item(int16 diaNr, int16 blockNr, int16 itemNr) {
	_dialogResource->setItemShown(diaNr, blockNr, itemNr, true);
}

int16 Atdsys::calc_inv_no_use(int16 curInv, int16 testNr, int16 mode) {
	assert(mode <= 255 && testNr <= 65535);

	const uint32 key = (mode & 0xff) << 16 | testNr;
	return (_itemUseWithDesc.contains(key)) ? _itemUseWithDesc[key] : -1;
}

int8 Atdsys::getStereoPos(int16 x) {
	return floor(x / 2.5) * 2 - 127;
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

Common::StringArray Atdsys::getTextArray(uint dialogNum, uint entryNum, int type) {
	if (!getControlBit(entryNum, ATS_ACTIVE_BIT))
		return _text->getTextArray(dialogNum, entryNum, type);
	else
		return Common::StringArray();
}

Common::String Atdsys::getTextEntry(uint dialogNum, uint entryNum, int type) {
	if (!getControlBit(entryNum, ATS_ACTIVE_BIT))
		return _text->getTextEntry(dialogNum, entryNum, type);
	else
		return Common::String();
}

} // namespace Chewy
