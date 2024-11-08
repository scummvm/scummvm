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
#include "chewy/atds.h"
#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/font.h"
#include "chewy/globals.h"
#include "chewy/mcga_graphics.h"
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

bool DialogCloseupTxtHeader::load(const void *src) {
	Common::MemoryReadStream rs((const byte *)src, 8);

	_diaNr = rs.readSint16LE();
	_perNr = rs.readSint16LE();
	_aMov = rs.readSint16LE();
	_curNr = rs.readSint16LE();
	return true;
}

Atdsys::Atdsys() {
	SplitStringInit init_ssi = { nullptr, 0, 0 };
	_aadv._dialog = false;
	_aadv._strNr = -1;
	_aadv._silentCount = false;
	_dialogCloseup._dialog = -1;
	_dialogCloseup._autoDia = false;
	_dialogCloseup._strNr = -1;
	_dialogCloseup._silentCount = false;
	_atdsv._delay = 1;
	_atdsv._silent = false;
	_atdsv._diaNr = -1;
	_atdsv.aad_str = nullptr;
	_atdsv._vocNr = -1;
	_atdsv._eventsEnabled = true;
	for (int16 i = 0; i < AAD_MAX_PERSON; i++)
		_ssi[i] = init_ssi;
	_invBlockNr = -1;

	_dialogResource = new DialogResource(ADS_TXT_STEUER);
	_text = _G(txt);

	_dialogCloseupNextBlock._blkNr = 0;
	_dialogCloseupNextBlock._endNr = 0;
	_dialogCloseupStackPtr = 0;

	init();
	initItemUseWith();
}

Atdsys::~Atdsys() {
	for (int16 i = 0; i < MAX_HANDLE; i++) {
		if (_atdsMem[i])
			free(_atdsMem[i]);
		_atdsMem[i] = nullptr;
	}

	delete _dialogResource;
}

void Atdsys::init() {
	set_handle(AAD_DATA, AAD_TAP_OFF, AAD_TAP_MAX);
	set_handle(DIALOG_CLOSEUP_DATA, ADS_TAP_OFF, ADS_TAP_MAX);
	_G(gameState).AadSilent = 10;
	_G(gameState).DelaySpeed = 5;
	_G(moveState)[P_CHEWY].Delay = _G(gameState).DelaySpeed;
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

		assert(objA <= 255);

		const uint32 key = (objA & 0xff) << 16 | objB;
		_itemUseWithDesc[key] = txtNum;
	}

	f.close();
}

void Atdsys::set_delay(int16 *delay, int16 silent) {
	_atdsv._delay = *delay;
	_atdsv._silent = silent;
}

void Atdsys::set_string_end_func
(void (*strFunc)(int16 diaNr, int16 strNr, int16 personNr, int16 mode)) {
	_atdsv.aad_str = strFunc;
}

int16 Atdsys::get_delay(int16 txt_len) {
	const int16 width = 220;
	const int16 lines = 4;
	const int16 w = _G(fontMgr)->getFont()->getDataWidth();
	int16 z_len = (width / w) + 1;
	int16 maxLen = z_len * lines;
	if (txt_len > maxLen)
		txt_len = maxLen;

	int16 ret = _atdsv._delay * (txt_len + z_len);
	return ret;
}

void Atdsys::split_string(SplitStringInit *ssi_, SplitStringRet *ret) {
	const int16 w = _G(fontMgr)->getFont()->getDataWidth();
	const int16 h = _G(fontMgr)->getFont()->getDataHeight();
	const int16 width = 220;
	const int16 lines = 4;

	ret->_nr = 0;
	ret->_next = false;
	ret->_strPtr = _splitPtr;
	ret->_x = _splitX;
	int16 zeichen_anz = (width / w) + 1;
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
				_splitPtr[ret->_nr] = start_adr;
				start_adr[tmp_count] = 0;
				_splitX[ret->_nr] = ssi_->_x + ((width - (strlen(start_adr) * w)) >> 1);
				++ret->_nr;
				if (ret->_nr == lines) {
					endLoop = true;
					bool endInnerLoop = false;
					while (!endInnerLoop) {
						if (*str_adr == ATDS_END_TEXT)
							endInnerLoop = true;
						else if (*str_adr != ' ' && *str_adr != 0) {
							endInnerLoop = true;
							ret->_next = true;
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
				if (ret->_nr + test_zeilen >= lines) {
					if (count < zeichen_anz) {
						tmp_count = count;
						endLoop = true;
					}
					_splitPtr[ret->_nr] = start_adr;
					start_adr[tmp_count] = 0;
					_splitX[ret->_nr] = ssi_->_x + ((width - (strlen(start_adr) * w)) >> 1);
					++ret->_nr;
					bool ende1 = false;
					while (!ende1) {
						if (*str_adr == ATDS_END_TEXT)
							ende1 = true;
						else if (*str_adr != ' ' && *str_adr != 0) {
							ende1 = true;
							ret->_next = true;
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
	if (ret->_nr <= lines)
		ret->_y = ssi_->_y + (lines - ret->_nr) * h;
	else
		ret->_y = ssi_->_y;
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
	const int16 width = 220;
	const int16 lines = 4;

	if (ssi_->_x - (width >> 1) < 2)
		ssi_->_x = 2;
	else if (ssi_->_x + (width >> 1) > (SCREEN_WIDTH - 2))
		ssi_->_x = ((SCREEN_WIDTH - 2) - width);
	else
		ssi_->_x -= (width >> 1);

	if (ssi_->_y - (lines * h) < 2) {
		ssi_->_y = 2;
	} else if (ssi_->_y + (lines * h) > (SCREEN_HEIGHT - 2))
		ssi_->_y = (SCREEN_HEIGHT - 2) - (lines * h);
	else {
		ssi_->_y -= (lines * h);
	}
}

void Atdsys::set_split_win(int16 nr, int16 x, int16 y) {
	_ssi[nr]._x = x;
	_ssi[nr]._y = y;
}

void Atdsys::set_handle(int16 mode, int16 chunkStart, int16 chunkNr) {
	assert(mode == AAD_DATA || mode == DIALOG_CLOSEUP_DATA);

	uint32 size = _text->findLargestChunk(chunkStart, chunkStart + chunkNr);
	char *tmp_adr = size ? (char *)MALLOC(size + 3) : nullptr;

	if (_atdsMem[mode])
		free(_atdsMem[mode]);
	_atdsMem[mode] = tmp_adr;
	_atdsPoolOff[mode] = chunkStart;
}

void Atdsys::load_atds(int16 chunkNr, int16 mode) {
	assert(mode == AAD_DATA || mode == DIALOG_CLOSEUP_DATA);

	char *txt_adr = _atdsMem[mode];

	if (txt_adr) {
		const uint32 chunkSize = _text->getChunk(chunkNr + _atdsPoolOff[mode])->size;
		const uint8 *chunkData = _text->getChunkData(chunkNr + _atdsPoolOff[mode]);
		memcpy(txt_adr, chunkData, chunkSize);
		delete[] chunkData;
		txt_adr[chunkSize] = (char)BLOCKENDE;
		txt_adr[chunkSize + 1] = (char)BLOCKENDE;
		txt_adr[chunkSize + 2] = (char)BLOCKENDE;
	}
}

bool Atdsys::start_ats(int16 txtNr, int16 txtMode, int16 color, int16 mode, int16 *vocNr) {
	assert(mode == ATS_DATA || mode == INV_USE_DATA || mode == INV_USE_DEF);

	EVENTS_CLEAR;
	g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
	g_events->_kbInfo._keyCode = '\0';
	_G(minfo).button = 0;

	*vocNr = -1;

	_atsv.shown = false;

	Common::StringArray textArray;

	if (mode != INV_USE_DEF) {
		const uint8 roomNum = _G(room)->_roomInfo->_roomNr;
		textArray = getTextArray(roomNum, txtNr, mode, txtMode);
	} else {
		textArray = getTextArray(0, txtNr, mode, -1);
	}

	_atsv.text.clear();
	for (uint i = 0; i < textArray.size(); i++)
		_atsv.text += textArray[i] + " ";
	_atsv.text.deleteLastChar();

	if (_atsv.text.size() > 0) {
		*vocNr = txtMode != TXT_MARK_NAME ? _text->getLastSpeechId() : -1;
		_atsv.shown = true;
		_atsv._txtMode = txtMode;
		_atsv._delayCount = get_delay(_atsv.text.size());
		_atsv._color = color;
		_printDelayCount1 = _atsv._delayCount / 10;
		_mousePush = true;
	}

	return _atsv.shown;
}

void Atdsys::stop_ats() {
	_atsv.shown = false;
}

void Atdsys::print_ats(int16 x, int16 y, int16 scrX, int16 scrY) {
	if (_atsv.shown) {
		if (_atdsv._eventsEnabled) {
			switch (g_events->getSwitchCode()) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_RETURN:
			case Common::MOUSE_BUTTON_LEFT:
				if (!_mousePush) {
					EVENTS_CLEAR;
					g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
					g_events->_kbInfo._keyCode = '\0';
					_G(minfo).button = 0;

					if (_atsv._silentCount <= 0 && _atsv._delayCount > _printDelayCount1) {
						_mousePush = true;
						_atsv._delayCount = 0;
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
			// TODO: Rewrite this
			SplitStringInit *atsSsi = &_ssi[0];
			char *txt = new char[_atsv.text.size() + 2];
			const int16 h = _G(fontMgr)->getFont()->getDataHeight();
			uint shownLen = 0;
			SplitStringRet splitString;

			Common::strlcpy(txt, _atsv.text.c_str(), _atsv.text.size() + 1);
			txt[_atsv.text.size() + 1] = ATDS_END_TEXT;
			atsSsi->_str = txt;
			atsSsi->_x = x - scrX;
			atsSsi->_y = y - scrY;
			split_string(atsSsi, &splitString);

			for (int16 i = 0; i < splitString._nr; i++) {
				if (g_engine->_sound->subtitlesEnabled()) {
					_G(out)->printxy(splitString._x[i],
									 splitString._y + (i * h) + 1,
									 0, 300, 0, splitString._strPtr[i]);
					_G(out)->printxy(splitString._x[i],
									 splitString._y + (i * h) - 1,
									 0, 300, 0, splitString._strPtr[i]);
					_G(out)->printxy(splitString._x[i] + 1,
									 splitString._y + (i * h),
									 0, 300, 0, splitString._strPtr[i]);
					_G(out)->printxy(splitString._x[i] - 1,
									 splitString._y + (i * h),
									 0, 300, 0, splitString._strPtr[i]);
					_G(out)->printxy(splitString._x[i],
									 splitString._y + (i * h),
									 _atsv._color,
									 300, 0, splitString._strPtr[i]);
				}

				shownLen += strlen(splitString._strPtr[i]) + 1;
			}

			delete[] txt;

			if (_atsv._delayCount <= 0) {
				if (!splitString._next) {
					_atsv.shown = false;
				} else {
					_atsv._delayCount = get_delay(_atsv.text.size() - shownLen);
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

void Atdsys::set_ats_str(int16 txtNr, int16 txtMode, int16 strNr, int16 mode) {
	_text->setSubtextNum(txtNr, txtMode, strNr, mode);
}

void Atdsys::set_all_ats_str(int16 txtNr, int16 strNr, int16 mode) {
	for (int16 i = 0; i < 5; i++)
		set_ats_str(txtNr, i, strNr, mode);
}

int16 Atdsys::getControlBit(int16 txtNr, int16 bitIdx) {
	return _text->getControlBit(txtNr, bitIdx);
}

void Atdsys::setControlBit(int16 txtNr, int16 bitIdx) {
	_text->setControlBit(txtNr, bitIdx);
}

void Atdsys::delControlBit(int16 txtNr, int16 bitIdx) {
	_text->delControlBit(txtNr, bitIdx);
}

int16 Atdsys::start_aad(int16 diaNr, bool continueWhenSpeechEnds) {
	if (_aadv._dialog)
		stopAad();

	_continueWhenSpeechEnds = continueWhenSpeechEnds;

	EVENTS_CLEAR;
	g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
	g_events->_kbInfo._keyCode = '\0';
	_G(minfo).button = 0;

	if (_atdsMem[AAD_HANDLE]) {
		_aadv._ptr = _atdsMem[AAD_HANDLE];
		aad_search_dia(diaNr, &_aadv._ptr);

		if (_aadv._ptr) {
			_aadv._person.load(_aadv._ptr, _aadv._txtHeader->_perNr);
			_aadv._ptr += _aadv._txtHeader->_perNr * sizeof(AadInfo);

			_aadv._dialog = true;
			_aadv._strNr = 0;
			_aadv._strHeader = (AadStrHeader *)_aadv._ptr;
			_aadv._ptr += sizeof(AadStrHeader);
			int16 txtLen = aadGetTxtLen(_aadv._ptr);
			_aadv._delayCount = get_delay(txtLen);
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
			switch (g_events->getSwitchCode()) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_RETURN:
			case Common::MOUSE_BUTTON_LEFT:
				if (!_mousePush) {
					EVENTS_CLEAR;
					g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
					g_events->_kbInfo._keyCode = '\0';
					_G(minfo).button = 0;

					if (_aadv._silentCount <= 0 && _aadv._delayCount > _printDelayCount1) {
						_mousePush = true;
						_aadv._delayCount = 0;
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
			int16 txtLen = aadGetTxtLen(start_ptr);
			str_null2leer(start_ptr, start_ptr + txtLen - 1);
			SplitStringInit tmp_ssi = _ssi[personId];
			SplitStringRet splitString;
			split_string(&tmp_ssi, &splitString);

			const int16 h = _G(fontMgr)->getFont()->getDataHeight();
			for (int16 i = 0; i < splitString._nr; i++) {
				if (g_engine->_sound->subtitlesEnabled() ||
					_aadv._strHeader->_vocNr - ATDS_VOC_OFFSET == -1) {
					_G(out)->printxy(splitString._x[i] + 1,
									 splitString._y + (i * h),
									 0, 300, 0, splitString._strPtr[i]);
					_G(out)->printxy(splitString._x[i] - 1,
									 splitString._y + (i * h),
									 0, 300, 0, splitString._strPtr[i]);
					_G(out)->printxy(splitString._x[i],
									 splitString._y + (i * h) + 1,
									 0, 300, 0, splitString._strPtr[i]);
					_G(out)->printxy(splitString._x[i],
									 splitString._y + (i * h) - 1,
									 0, 300, 0, splitString._strPtr[i]);
					_G(out)->printxy(splitString._x[i],
									 splitString._y + (i * h),
									 _aadv._person[personId]._color,
									 300, 0, splitString._strPtr[i]);
				}
				tmp_ptr += strlen(splitString._strPtr[i]) + 1;
			}
			str_null2leer(start_ptr, start_ptr + txtLen - 1);

			if (g_engine->_sound->speechEnabled() &&
					_aadv._strHeader->_vocNr - ATDS_VOC_OFFSET != -1) {
				if (_atdsv._vocNr != _aadv._strHeader->_vocNr - ATDS_VOC_OFFSET) {
					_atdsv._vocNr = _aadv._strHeader->_vocNr - ATDS_VOC_OFFSET;
					g_engine->_sound->playSpeech(_atdsv._vocNr, false);
				}

				if (_continueWhenSpeechEnds && _atdsv._vocNr >= 0 && !g_engine->_sound->isSpeechActive())
					stopAad();
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
					_dialogCloseup._autoDia = false;
					_aadv._strNr = -1;
					splitString._next = false;
				} else {
					if (!splitString._next) {
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
					txtLen = aadGetTxtLen(_aadv._ptr);
					_aadv._delayCount = get_delay(txtLen);
					_printDelayCount1 = _aadv._delayCount / 10;
					_aadv._silentCount = _atdsv._silent;
				}
			} else {
				if (_aadv._strHeader->_vocNr - ATDS_VOC_OFFSET == -1)
					--_aadv._delayCount;
			}
		} else {
			--_aadv._silentCount;
		}
	}
}

int16 Atdsys::aadGetStatus() {
	return _aadv._strNr;
}

int16 Atdsys::aadGetTxtLen(char *str) {
	char *ptr = str;
	while (*str != ATDS_END_TEXT)
		++str;

	int16 txtLen = (str - ptr) - 1;
	return txtLen;
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
				_aadv._txtHeader = (DialogCloseupTxtHeader *)start_ptr;
				*ptr = start_ptr + sizeof(DialogCloseupTxtHeader);
			} else {
				start_ptr += sizeof(DialogCloseupTxtHeader) + pos[1] * sizeof(AadInfo);
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

bool  Atdsys::startDialogCloseup(int16 diaNr) {
	bool ret = false;
	bool end = false;

	load_atds(diaNr, DIALOG_CLOSEUP_DATA);

	if (_atdsMem[ADS_HANDLE][0] == (char)BLOCKENDE &&
		    _atdsMem[ADS_HANDLE][1] == (char)BLOCKENDE &&
		    _atdsMem[ADS_HANDLE][2] == (char)BLOCKENDE)
		end = true;

	if (!end) {
		_dialogCloseup._ptr = _atdsMem[ADS_HANDLE];
		_dialogCloseup._txtHeader.load(_dialogCloseup._ptr);

		if (_dialogCloseup._txtHeader._diaNr == diaNr) {
			ret = true;
			_dialogCloseup._ptr += DialogCloseupTxtHeader::SIZE();
			_dialogCloseup._person.load(_dialogCloseup._ptr, _dialogCloseup._txtHeader._perNr);
			_dialogCloseup._ptr += _dialogCloseup._txtHeader._perNr * AadInfo::SIZE();
			_dialogCloseup._dialog = diaNr;
			_dialogCloseup._strNr = 0;
			_dialogCloseupStack[0] = 0;
			_dialogCloseupStackPtr = 1;
		}
	}
	return ret;
}

void Atdsys::stopDialogCloseup() {
	_dialogCloseup._dialog = -1;
	_dialogCloseup._autoDia = false;
}

int16 Atdsys::getDialogCloseupStatus() {
	return _dialogCloseup._dialog;
}

char **Atdsys::dialogCloseupItemPtr(uint16 dialogNum, int16 blockNr, int16 *retNr) {
	*retNr = 0;
	memset(_ePtr, 0, sizeof(char *) * DIALOG_CLOSEUP_MAX);
	if (_dialogCloseup._dialog != -1) {
		_dialogCloseup._blockPtr = _dialogCloseup._ptr;
		dialogCloseupSearchBlock(blockNr, &_dialogCloseup._blockPtr);
		if (_dialogCloseup._blockPtr) {
			for (int16 i = 0; i < DIALOG_CLOSEUP_MAX; i++) {
				char *itemPtr = _dialogCloseup._blockPtr;
				dialogCloseupSearchItem(i, &itemPtr);
				if (itemPtr) {
					char nr = itemPtr[-1];
					itemPtr += sizeof(AadStrHeader);
					if (_dialogResource->isItemShown(dialogNum, blockNr, (int16)nr)) {
						_ePtr[*retNr] = itemPtr;
						_eNr[*retNr] = (int16)nr;
						++(*retNr);
					}
				}
			}
		}
	}

	return _ePtr;
}

DialogCloseupNextBlock *Atdsys::dialogCloseupItemChoice(uint16 dialogNum, int16 blockNr, int16 itemNr) {
	_dialogCloseupNextBlock._blkNr = blockNr;
	if (!_aadv._dialog) {
		if (!_dialogCloseup._autoDia) {
			dialogCloseupSearchItem(_eNr[itemNr], &_dialogCloseup._blockPtr);
			if (_dialogCloseup._blockPtr) {
				if (startAutoDialogCloseup(_dialogCloseup._blockPtr))
					_dialogCloseup._autoDia = true;
				if (_dialogResource->hasExitBit(dialogNum, blockNr, _eNr[itemNr])) {
					stopDialogCloseup();
					_dialogCloseupNextBlock._endNr = _eNr[itemNr];
					_dialogCloseupNextBlock._blkNr = -1;
				}
			}
		}
	}

	return &_dialogCloseupNextBlock;
}

DialogCloseupNextBlock *Atdsys::calcNextDialogCloseupBlock(uint16 dialogNum, int16 blockNr, int16 itemNr) {
	if (!_dialogResource->hasShowBit(dialogNum, blockNr, _eNr[itemNr]))
		_dialogResource->setItemShown(dialogNum, blockNr, _eNr[itemNr], false);
	_dialogCloseupNextBlock._endNr = _eNr[itemNr];

	if (_dialogResource->hasRestartBit(dialogNum, blockNr, _eNr[itemNr])) {
		_dialogCloseupNextBlock._blkNr = 0;

		_dialogCloseupStackPtr = 0;
	} else {
		const uint8 nextBlock = _dialogResource->getNextBlock(dialogNum, blockNr, _eNr[itemNr]);
		if (nextBlock) {
			_dialogCloseupNextBlock._blkNr = nextBlock;

			int16 option = 0;
			while (!option && _dialogCloseupNextBlock._blkNr != -1) {

				option = 0;
				dialogCloseupItemPtr(dialogNum, _dialogCloseupNextBlock._blkNr, &option);
				if (!option) {
					_dialogCloseupNextBlock._blkNr = getDialogCloseupBlock(dialogNum);
				}
			}
		} else {
			_dialogCloseupNextBlock._blkNr = getDialogCloseupBlock(dialogNum);
		}
	}
	_dialogCloseupStack[_dialogCloseupStackPtr] = _dialogCloseupNextBlock._blkNr;
	++_dialogCloseupStackPtr;

	return &_dialogCloseupNextBlock;
}

int16 Atdsys::getDialogCloseupBlock(uint16 dialogNum) {
	_dialogCloseupStackPtr -= 1;
	int16 ret = -1;
	bool end = false;
	while (_dialogCloseupStackPtr >= 0 && !end) {
		short blk_nr = _dialogCloseupStack[_dialogCloseupStackPtr];
		int16 option;
		dialogCloseupItemPtr(dialogNum, blk_nr, &option);
		if (option) {
			ret = blk_nr;
			end = true;
		} else {
			--_dialogCloseupStackPtr;
		}
	}

	++_dialogCloseupStackPtr;
	return ret;
}

void Atdsys::dialogCloseupSearchBlock(int16 blockNr, char **ptr) {
	char *start_ptr = *ptr;
	bool end = false;
	while (!end) {
		if (*start_ptr == (char)blockNr) {
			end = true;
			*ptr = start_ptr;
		} else {
			start_ptr += 2 + sizeof(AadStrHeader);
			while (*start_ptr++ != ATDS_END_BLOCK) {}
			if (start_ptr[0] == ATDS_END &&
			        start_ptr[1] == ATDS_END) {
				end = true;
				*ptr = nullptr;
			}
		}
	}
}

void Atdsys::dialogCloseupSearchItem(int16 itemNr, char **blkAdr) {
	char *start_ptr = *blkAdr + 1;
	bool end = false;
	while (!end) {
		if (*start_ptr == itemNr) {
			end = true;
			*blkAdr = start_ptr + 1;
		} else {
			start_ptr += 1 + sizeof(AadStrHeader);
			while (*start_ptr++ != ATDS_END_ENTRY) {}
			if (*start_ptr == ATDS_END_BLOCK) {
				end = true;
				*blkAdr = nullptr;
			}
		}
	}
}

int16 Atdsys::startAutoDialogCloseup(char *itemAdr) {
	_aadv._dialog = false;
	if (itemAdr) {
		_aadv._person = _dialogCloseup._person;
		_aadv._ptr = itemAdr;
		_aadv._dialog = true;
		_aadv._strNr = 0;
		_aadv._strHeader = (AadStrHeader *)_aadv._ptr;
		_aadv._ptr += sizeof(AadStrHeader);
		int16 txtLen = aadGetTxtLen(_aadv._ptr);
		_aadv._delayCount = get_delay(txtLen);
		_atdsv._diaNr = _dialogCloseup._txtHeader._diaNr + 10000;

		if (_atdsv.aad_str != nullptr)
			_atdsv.aad_str(_atdsv._diaNr, 0, _aadv._strHeader->_akPerson, AAD_STR_START);
		_mousePush = true;
		stop_ats();
	} else {
		_aadv._dialog = false;
	}

	return _aadv._dialog;
}

void Atdsys::hideDialogCloseupItem(int16 diaNr, int16 blockNr, int16 itemNr) {
	_dialogResource->setItemShown(diaNr, blockNr, itemNr, false);
}

void Atdsys::showDialogCloseupItem(int16 diaNr, int16 blockNr, int16 itemNr) {
	_dialogResource->setItemShown(diaNr, blockNr, itemNr, true);
}

int16 Atdsys::calc_inv_no_use(int16 curInv, int16 testNr) {
	if (curInv != -1)
		_invBlockNr = curInv + 1;

	assert(curInv <= 255);

	const uint32 key = (curInv & 0xff) << 16 | testNr;
	return (_itemUseWithDesc.contains(key)) ? _itemUseWithDesc[key] : -1;
}

int8 Atdsys::getStereoPos(int16 x) {
	return floor(x / 2.5);
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

Common::StringArray Atdsys::getTextArray(uint dialogNum, uint entryNum, int type, int subEntry) {
	if (dialogNum == 45 && entryNum == 295 && type == 1 && subEntry == -1 &&
			g_engine->getLanguage() == Common::EN_ANY) {
		// WORKAROUND: Taxi hotspot in room 45 (Big City)
		Common::StringArray results;
		results.push_back("Taxi");
		return results;
	} else if (!getControlBit(entryNum, ATS_ACTIVE_BIT))
		return _text->getTextArray(dialogNum, entryNum, type, subEntry);
	else
		return Common::StringArray();
}

Common::String Atdsys::getTextEntry(uint dialogNum, uint entryNum, int type, int subEntry) {
	if (!getControlBit(entryNum, ATS_ACTIVE_BIT))
		return _text->getTextEntry(dialogNum, entryNum, type, subEntry);
	else
		return Common::String();
}

int16 Atdsys::getLastSpeechId() {
	return _text->getLastSpeechId();
}

} // namespace Chewy
