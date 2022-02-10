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
#include "chewy/file.h"
#include "chewy/global.h"
#include "chewy/sound.h"

namespace Chewy {

bool AtsTxtHeader::load(Common::SeekableReadStream *src) {
	TxtNr = src->readUint16LE();
	AMov = src->readUint16LE();
	CurNr = src->readUint16LE();
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


atdsys::atdsys() {
	SplitStringInit init_ssi = { nullptr, 0, 0, 220, 4, SPLIT_MITTE, 8, 8,};
	aadv.Dialog = false;
	aadv.StrNr = -1;
	aadv.SilentCount = false;
	adsv.Dialog = -1;
	adsv.AutoDia = false;
	adsv.StrNr = -1;
	adsv.SilentCount = false;
	atsv.Display = false;
	atsv.SilentCount = false;
	atdsv.Delay = &tmp_delay;
	tmp_delay = 1;
	atdsv.Silent = false;
	atdsv.Display = DISPLAY_TXT;
	atdsv.DiaNr = -1;
	atdsv.aad_str = 0;
	atdsv.VocNr = -1;
	atdsv._eventsEnabled = true;
	ssret.Next = false;
	ssr = &ssret;
	for (int16 i = 0; i < AAD_MAX_PERSON; i++)
		ssi[i] = init_ssi;
	inv_block_nr = -1;
	inv_use_mem = nullptr;
	inzeig = in->get_in_zeiger();
}

atdsys::~atdsys() {
	for (int16 i = 0; i < MAX_HANDLE; i++)
		close_handle(i);

	if (inv_use_mem)
		free(inv_use_mem);
}

void atdsys::set_font(byte *font_adr, int16 fvorx, int16 fhoehe) {
	atdsv.Font = font_adr;
	atdsv.Fvorx = fvorx;
	atdsv.Fhoehe = fhoehe;
}

void atdsys::set_delay(int16 *delay, int16 silent) {
	atdsv.Delay = delay;
	atdsv.Silent = silent;
}

void atdsys::set_string_end_func
(void (*str_func)(int16 dia_nr, int16 str_nr, int16 person_nr, int16 mode)) {
	atdsv.aad_str = str_func;
}

void atdsys::setHasSpeech(bool hasSpeech) {
	_hasSpeech = hasSpeech;
	updateSoundSettings();
}

void atdsys::updateSoundSettings() {
	atdsv.Display = DISPLAY_TXT;

	if (_hasSpeech) {
		// TODO: In the future, properly implement DISPLAY_ALL
		if (!g_engine->_sound->isSpeechMuted())
			atdsv.Display = DISPLAY_VOC;
	}
}

int16 atdsys::get_delay(int16 txt_len) {
	int16 z_len = (ssi->Width / ssi->Fvorx) + 1;
	int16 max_len = z_len * ssi->Zeilen;
	if (txt_len > max_len)
		txt_len = max_len;

	int16 ret = *atdsv.Delay * (txt_len + z_len);
	return ret;
}

SplitStringRet *atdsys::split_string(SplitStringInit *ssi_) {
	ssret.Anz = 0;
	ssret.Next = false;
	ssret.StrPtr = split_ptr;
	ssret.X = split_x;
	int16 zeichen_anz = (ssi_->Width / ssi_->Fvorx) + 1;
	memset(split_ptr, 0, sizeof(char *)*MAX_STR_SPLIT);
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
				split_ptr[ssret.Anz] = start_adr;
				start_adr[tmp_count] = 0;
				if (ssi_->Mode == SPLIT_MITTE)
					split_x[ssret.Anz] = ssi_->X + ((ssi_->Width - (strlen(start_adr) * ssi_->Fvorx)) >> 1);
				else
					split_x[ssret.Anz] = ssi_->X;
				++ssret.Anz;
				if (ssret.Anz == ssi_->Zeilen) {
					ende = true;
					bool ende1 = false;
					while (!ende1) {
						if (*str_adr == ATDS_END_TEXT)
							ende1 = true;
						else if (*str_adr != ' ' && *str_adr != 0) {
							ende1 = true;
							ssret.Next = true;
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
				if ((ssret.Anz + test_zeilen) >= ssi_->Zeilen) {
					if (count < zeichen_anz) {
						tmp_count = count;
						ende = true;
					}
					split_ptr[ssret.Anz] = start_adr;
					start_adr[tmp_count] = 0;
					if (ssi_->Mode == SPLIT_MITTE)
						split_x[ssret.Anz] = ssi_->X + ((ssi_->Width - (strlen(start_adr) * ssi_->Fvorx)) >> 1);
					else
						split_x[ssret.Anz] = ssi_->X;
					++ssret.Anz;
					bool ende1 = false;
					while (!ende1) {
						if (*str_adr == ATDS_END_TEXT)
							ende1 = true;
						else if (*str_adr != ' ' && *str_adr != 0) {
							ende1 = true;
							ssret.Next = true;
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
	if (ssret.Anz <= ssi_->Zeilen)
		ssret.Y = ssi_->Y + (ssi_->Zeilen - ssret.Anz) * ssi_->FHoehe;
	else
		ssret.Y = ssi_->Y;

	return &ssret;
}

void atdsys::str_null2leer(char *str_start, char *str_end) {
	while (str_start < str_end) {
		if (*str_start == 0)
			*str_start = 32;
		++str_start;
	}
}

void atdsys::calc_txt_win(SplitStringInit *ssi_) {
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

void atdsys::set_split_win(int16 nr, SplitStringInit *ssinit) {
	ssi[nr] = ssinit[0];
}

Stream *atdsys::pool_handle(const char *fname_, const char *fmode) {
	Stream *handle = chewy_fopen(fname_, fmode);
	if (handle) {
		atdshandle[ATDS_HANDLE] = handle;
	} else {
		error("Error reading from %s", fname_);
	}

	return handle;
}

void atdsys::set_handle(const char *fname_, int16 mode, Stream *handle, int16 chunk_start, int16 chunk_anz) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(handle);
	ChunkHead Ch;
	char *tmp_adr = atds_adr(fname_, chunk_start, chunk_anz);
	if (!modul) {
		if (rs) {
			atdshandle[mode] = rs;
			atdsmem[mode] = tmp_adr;
			atdspooloff[mode] = chunk_start;
			switch (mode) {
			case INV_USE_DATEI:
				mem->file->select_pool_item(rs, atdspooloff[mode]);
				rs->seek(-ChunkHead::SIZE(), SEEK_CUR);

				if (!Ch.load(rs)) {
					error("Error reading from %s", fname_);
				} else {
					free(inv_use_mem);
					inv_use_mem = (char *)MALLOC(Ch.size + 3l);

					if (!modul) {
						if (Ch.size) {
							if (!rs->read(inv_use_mem, Ch.size)) {
								error("Error reading from %s", fname_);
							} else
								crypt(inv_use_mem, Ch.size);
						}
						inv_use_mem[Ch.size] = (char)BLOCKENDE;
						inv_use_mem[Ch.size + 1] = (char)BLOCKENDE;
						inv_use_mem[Ch.size + 2] = (char)BLOCKENDE;
					}
				}
				break;

			}
		}
	} else
		error("Error reading from %s", fname_);
}

void atdsys::open_handle(const char *fname_, const char *fmode, int16 mode) {
	char *tmp_adr = nullptr;

	if (mode != INV_IDX_DATEI)
		tmp_adr = atds_adr(fname_, 0, 20000);
	if (!modul) {
		Stream *stream = chewy_fopen(fname_, fmode);
		if (stream) {
			close_handle(mode);
			atdshandle[mode] = stream;
			atdsmem[mode] = tmp_adr;

			switch (mode) {
			case ADH_DATEI:
				ads_block = (AdsBlock *)atdsmem[ADH_HANDLE];
				break;

			case INV_IDX_DATEI:
				atdsmem[INV_IDX_HANDLE] = (char *)MALLOC(INV_STRC_ANZ * sizeof(InvUse));
				break;

			default:
				break;
			}
		} else {
			error("Error reading from %s", fname_);
		}
	}
}

void atdsys::close_handle(int16 mode) {
	Stream *stream = atdshandle[mode];
	if (stream) {
		chewy_fclose(atdshandle[mode]);

		for (int i = 0; i < MAX_HANDLE; ++i) {
			if (atdshandle[i] == stream)
				atdshandle[i] = nullptr;
		}
	}

	if (atdsmem[mode])
		free(atdsmem[mode]);
	atdsmem[mode] = nullptr;
}

char *atdsys::atds_adr(const char *fname_, int16 chunk_start, int16 chunk_anz) {
	char *tmp_adr = nullptr;
	uint32 size = mem->file->get_poolsize(fname_, chunk_start, chunk_anz);
	if (size) {
		tmp_adr = (char *)MALLOC(size + 3l);
	}

	return tmp_adr;
}

void atdsys::load_atds(int16 chunk_nr, int16 mode) {
	ChunkHead Ch;
	char *txt_adr = atdsmem[mode];
	Common::SeekableReadStream *stream = dynamic_cast<Common::SeekableReadStream *>(atdshandle[mode]);

	if (stream && txt_adr) {
		mem->file->select_pool_item(stream, chunk_nr + atdspooloff[mode]);
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

void atdsys::save_ads_header(int16 dia_nr) {
	ChunkHead Ch;
	if (atdshandle[ADH_HANDLE]) {
		mem->file->select_pool_item(atdshandle[ADH_HANDLE], dia_nr);
		Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(
			atdshandle[ADH_HANDLE]);

		rs->seek(-ChunkHead::SIZE(), SEEK_CUR);

		if (!Ch.load(rs)) {
			error("save_ads_header error");
		} else {
			if (Ch.size) {
				Common::SeekableWriteStream *ws = g_engine->_tempFiles.createWriteStreamForMember(ADSH_TMP);
				ws->seek(rs->pos());
				if (ws->write(atdsmem[ADH_HANDLE], Ch.size) != Ch.size) {
					error("save_ads_header error");
				}

				delete ws;
			}
		}
	} else {
		error("save_ads_header error");
	}
}

void atdsys::crypt(char *txt_, uint32 size) {
	uint8 *sp = (uint8 *)txt_;
	for (uint32 i = 0; i < size; i++) {
		*sp = -(*sp);
		++sp;
	}
}

void atdsys::init_ats_mode(int16 mode, uint8 *atsheader) {
	switch (mode) {
	case ATS_DATEI:
		ats_st_header[0] = atsheader;
		break;

	case INV_USE_DATEI:
		ats_st_header[1] = atsheader;
		break;

	case INV_USE_DEF:
		ats_st_header[2] = atsheader;
		break;

	case INV_ATS_DATEI:
		ats_st_header[3] = atsheader;
		break;

	default:
		break;
	}
}

void atdsys::set_ats_mem(int16 mode) {
	switch (mode) {
	case ATS_DATEI:
		ats_sheader = ats_st_header[0];
		atsmem = atdsmem[mode];
		break;

	case INV_USE_DATEI:
		ats_sheader = ats_st_header[1];
		atsmem = atdsmem[mode];
		break;

	case INV_USE_DEF:
		ats_sheader = ats_st_header[2];
		atsmem = inv_use_mem;
		break;

	case INV_ATS_DATEI:
		ats_sheader = ats_st_header[3];
		atsmem = atdsmem[mode];
		break;

	default:
		break;
	}
}

bool atdsys::start_ats(int16 txt_nr, int16 txt_mode, int16 color, int16 mode, int16 *voc_nr) {
	*voc_nr = -1;
	set_ats_mem(mode);
	if (atsmem) {
		if (atsv.Display)
			stop_ats();

		int16 txt_anz;
		atsv.Ptr = ats_get_txt(txt_nr, txt_mode, &txt_anz, mode);
		if (atsv.Ptr) {
			atsv.Display = true;
			char *ptr = atsv.Ptr;
			atsv.TxtLen = 0;
			while (*ptr++ != ATDS_END_TEXT)
				++atsv.TxtLen;
			if ((byte)*atsv.Ptr == 248) {
				// Special code for no message to display
				atsv.Display = false;
			} else {
				atsv.DelayCount = get_delay(atsv.TxtLen);
				_printDelayCount1 = atsv.DelayCount / 10;
				atsv.Color = color;
				_mousePush = true;
			}
			*voc_nr = atsv.StrHeader.VocNr - ATDS_VOC_OFFSET;
			if ((atdsv.Display == DISPLAY_VOC) && (*voc_nr != -1)) {
				atsv.Display = false;
			}
		} else
			atsv.Display = false;
	} else
		atsv.Display = false;

	return atsv.Display;
}

void atdsys::stop_ats() {
	atsv.Display = false;
}

int16 atdsys::ats_get_status() {
	return atsv.Display;
}

void atdsys::print_ats(int16 x, int16 y, int16 scrx, int16 scry) {
	if (atsv.Display) {
		if (atdsv._eventsEnabled) {
			switch (in->get_switch_code()) {
			case ESC:
			case ENTER:
			case MAUS_LINKS:
				if (_mousePush == false) {
					if (atsv.SilentCount <= 0 && atsv.DelayCount > _printDelayCount1) {
						_mousePush = true;
						atsv.DelayCount = 0;
						inzeig->kbinfo->scan_code = Common::KEYCODE_INVALID;
						inzeig->kbinfo->key_code = '\0';
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

		if (atsv.SilentCount <= 0) {
			char *tmp_ptr = atsv.Ptr;
			out->set_fontadr(atdsv.Font);
			out->set_vorschub(atdsv.Fvorx, 0);
			ats_ssi = ssi[0];
			ats_ssi.Str = tmp_ptr;
			ats_ssi.Fvorx = atdsv.Fvorx;
			ats_ssi.FHoehe = atdsv.Fhoehe;
			ats_ssi.X = x - scrx;
			ats_ssi.Y = y - scry;
			char *start_ptr = tmp_ptr;
			str_null2leer(start_ptr, start_ptr + atsv.TxtLen - 1);
			SplitStringInit tmp_ssi = ats_ssi;
			ssr = split_string(&tmp_ssi);

			for (int16 i = 0; i < ssr->Anz; i++) {
				out->printxy(ssr->X[i],
				              ssr->Y + (i * ats_ssi.FHoehe) + 1,
				              0, 300, 0, ssr->StrPtr[i]);
				out->printxy(ssr->X[i],
				              ssr->Y + (i * ats_ssi.FHoehe) - 1,
				              0, 300, 0, ssr->StrPtr[i]);
				out->printxy(ssr->X[i] + 1,
				              ssr->Y + (i * ats_ssi.FHoehe),
				              0, 300, 0, ssr->StrPtr[i]);
				out->printxy(ssr->X[i] - 1,
				              ssr->Y + (i * ats_ssi.FHoehe),
				              0, 300, 0, ssr->StrPtr[i]);
				out->printxy(ssr->X[i],
				              ssr->Y + (i * ats_ssi.FHoehe),
				              atsv.Color,
				              300, 0, ssr->StrPtr[i]);
				tmp_ptr += strlen(ssr->StrPtr[i]) + 1;
			}

			str_null2leer(start_ptr, start_ptr + atsv.TxtLen - 1);
			if (atsv.DelayCount <= 0) {
				if (ssr->Next == false) {
					atsv.Display = false;
				} else {
					atsv.Ptr = tmp_ptr;
					atsv.TxtLen = 0;
					while (*tmp_ptr++ != ATDS_END_TEXT)
						++atsv.TxtLen;
					atsv.DelayCount = get_delay(atsv.TxtLen);
					_printDelayCount1 = atsv.DelayCount / 10;
					atsv.SilentCount = atdsv.Silent;
				}
			} else
				--atsv.DelayCount;
		} else
			--atsv.SilentCount;
	}
}

char *atdsys::ats_get_txt(int16 txt_nr, int16 txt_mode, int16 *txt_anz, int16 mode) {
	char *str_ = nullptr;
	set_ats_mem(mode);

	atsv.TxtMode = txt_mode;

	if (!get_steuer_bit(txt_nr, ATS_AKTIV_BIT, mode)) {
		uint8 status = ats_sheader[(txt_nr * MAX_ATS_STATUS) + (atsv.TxtMode + 1) / 2];
		int16 ak_nybble = (atsv.TxtMode + 1) % 2;

		uint8 lo_hi[2];
		lo_hi[1] = status >> 4;
		lo_hi[0] = status &= 15;
		str_ = ats_search_block(atsv.TxtMode, atsmem);
		if (str_ != nullptr) {
			ats_search_nr(txt_nr, &str_);
			if (str_ != nullptr) {
				ats_search_str(txt_anz, &lo_hi[ak_nybble], (uint8)ats_sheader[txt_nr * MAX_ATS_STATUS], &str_);

				if (str_ != nullptr) {
					status = 0;
					lo_hi[1] <<= 4;
					status |= lo_hi[0];
					status |= lo_hi[1];
					ats_sheader[(txt_nr * MAX_ATS_STATUS) + (atsv.TxtMode + 1) / 2] = status;
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

void atdsys::set_ats_str(int16 txt_nr, int16 txt_mode, int16 str_nr, int16 mode) {
	set_ats_mem(mode);
	uint8 status = ats_sheader[(txt_nr * MAX_ATS_STATUS) + (txt_mode + 1) / 2];
	int16 ak_nybble = (txt_mode + 1) % 2;

	uint8 lo_hi[2];
	lo_hi[1] = status >> 4;
	lo_hi[0] = status &= 15;
	lo_hi[ak_nybble] = str_nr;
	status = 0;
	lo_hi[1] <<= 4;
	status |= lo_hi[0];
	status |= lo_hi[1];
	ats_sheader[(txt_nr * MAX_ATS_STATUS) + (txt_mode + 1) / 2] = status;
}

void atdsys::set_ats_str(int16 txt_nr, int16 str_nr, int16 mode) {
	for (int16 i = 0; i < 5; i++)
		set_ats_str(txt_nr, i, str_nr, mode);
}

int16 atdsys::get_ats_str(int16 txt_nr, int16 txt_mode, int16 mode) {
	set_ats_mem(mode);
	uint8 status = ats_sheader[(txt_nr * MAX_ATS_STATUS) + (txt_mode + 1) / 2];
	int16 ak_nybble = (txt_mode + 1) % 2;

	uint8 lo_hi[2];
	lo_hi[1] = status >> 4;
	lo_hi[0] = status &= 15;

	return (int16)lo_hi[ak_nybble];
}

int16 atdsys::get_steuer_bit(int16 txt_nr, int16 bit_idx, int16 mode) {
	set_ats_mem(mode);
	int16 ret = bit->is_bit(ats_sheader[txt_nr * MAX_ATS_STATUS], bit_idx);
	return ret;
}

void atdsys::set_steuer_bit(int16 txt_nr, int16 bit_idx, int16 mode) {
	set_ats_mem(mode);
	bit->set_bit(&ats_sheader[txt_nr * MAX_ATS_STATUS], bit_idx);
}

void atdsys::del_steuer_bit(int16 txt_nr, int16 bit_idx, int16 mode) {
	set_ats_mem(mode);
	bit->del_bit(&ats_sheader[txt_nr * MAX_ATS_STATUS], bit_idx);
}

char *atdsys::ats_search_block(int16 txt_mode, char *txt_adr) {
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

void atdsys::ats_search_nr(int16 txt_nr, char **str_) {
	char *start_str = *str_;

	bool done1 = false;
	while (!done1) {
		Common::MemoryReadStream rs1((const byte *)start_str, AtsTxtHeader::SIZE());
		atsv.TxtHeader.load(&rs1);

		if (atsv.TxtHeader.TxtNr == 0xFEF0 &&
				atsv.TxtHeader.AMov == txt_nr) {
			// Found match
			*str_ = start_str + AtsTxtHeader::SIZE();

			if (atsv.TxtMode) {
				Common::MemoryReadStream rs2((const byte *)*str_, AtsStrHeader::SIZE());
				atsv.StrHeader.load(&rs2);
			}

			*str_ += AtsStrHeader::SIZE();
			break;
		}

		start_str += AtsTxtHeader::SIZE() + AtsStrHeader::SIZE();

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
						*str_ = nullptr;
					} else {
						done2 = true;
					}
				}
			}
		}
	}
}

void atdsys::ats_search_str(int16 *anz, uint8 *status, uint8 steuer, char **str_) {
	char *tmp_str = *str_;
	char *start_str = *str_;
	*anz = 0;
	bool ende = false;
	int16 count = 0;

	while (!ende) {
		if (count == *status) {
			if (!*tmp_str) {
				++*anz;
			} else if (*tmp_str == ATDS_END_TEXT) {
				ende = true;
				*str_ = start_str;
				start_str -= AtsStrHeader::SIZE();

				if (atsv.TxtMode != TXT_MARK_NAME) {
					Common::MemoryReadStream rs((const byte *)start_str,
						AtsStrHeader::SIZE());
					atsv.StrHeader.load(&rs);
				}

				if (tmp_str[1] != ATDS_END) {
					if (!bit->is_bit(steuer, ATS_COUNT_BIT))
						++*status;
				} else {

					if (bit->is_bit(steuer, ATS_RESET_BIT))
						*status = 0;
				}
			}
		} else {
			if (*tmp_str == ATDS_END_TEXT) {
				if (tmp_str[1] == ATDS_END) {
					ende = false;
					*anz = 0;
					*status = count;
					*str_ = start_str;
					start_str -= AtsStrHeader::SIZE();
					if (atsv.TxtMode != TXT_MARK_NAME) {
						Common::MemoryReadStream rs((const byte *)start_str,
							AtsStrHeader::SIZE());
						atsv.StrHeader.load(&rs);
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
				*str_ = nullptr;
			}
		}

		++tmp_str;
	}
}

int16 atdsys::start_aad(int16 dia_nr) {
	if (aadv.Dialog)
		stop_aad();

	if (atdsmem[AAD_HANDLE]) {
		aadv.Ptr = atdsmem[AAD_HANDLE];
		aad_search_dia(dia_nr, &aadv.Ptr);
		if (aadv.Ptr) {
			aadv.Person = (AadInfo *)aadv.Ptr;
			aadv.Ptr += aadv.TxtHeader->PerAnz * sizeof(AadInfo);
			aadv.Dialog = true;
			aadv.StrNr = 0;
			aadv.StrHeader = (AadStrHeader *)aadv.Ptr;
			aadv.Ptr += sizeof(AadStrHeader);
			int16 txt_len;
			aad_get_zeilen(aadv.Ptr, &txt_len);
			aadv.DelayCount = get_delay(txt_len);
			_printDelayCount1 = aadv.DelayCount / 10;

			atdsv.DiaNr = dia_nr;
			if (atdsv.aad_str != nullptr)
				atdsv.aad_str(atdsv.DiaNr, 0, aadv.StrHeader->AkPerson, AAD_STR_START);
			_mousePush = true;
			stop_ats();
			atdsv.VocNr = -1;
		}
	}

	return aadv.Dialog;
}

void atdsys::stop_aad() {
	aadv.Dialog = false;
	aadv.StrNr = -1;
}

void atdsys::print_aad(int16 scrx, int16 scry) {
	if (aadv.Dialog) {
		if (atdsv._eventsEnabled) {
			switch (in->get_switch_code()) {
			case ESC:
			case ENTER:
			case MAUS_LINKS:
				if (_mousePush == false) {
					if (aadv.SilentCount <= 0 && aadv.DelayCount > _printDelayCount1) {
						_mousePush = true;
						aadv.DelayCount = 0;
						inzeig->kbinfo->scan_code = Common::KEYCODE_INVALID;
						inzeig->kbinfo->key_code = '\0';
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

		if (aadv.SilentCount <= 0) {
			char *tmp_ptr = aadv.Ptr;
			out->set_fontadr(atdsv.Font);
			out->set_vorschub(atdsv.Fvorx, 0);
			ssi[aadv.StrHeader->AkPerson].Str = tmp_ptr;
			if (aadv.Person[aadv.StrHeader->AkPerson].X != -1) {
				ssi[aadv.StrHeader->AkPerson].X = aadv.Person[aadv.StrHeader->AkPerson].X - scrx;
			}
			if (aadv.Person[aadv.StrHeader->AkPerson].Y != -1) {
				ssi[aadv.StrHeader->AkPerson].Y = aadv.Person[aadv.StrHeader->AkPerson].Y - scry;
			}
			ssi[aadv.StrHeader->AkPerson].Fvorx = atdsv.Fvorx;
			ssi[aadv.StrHeader->AkPerson].FHoehe = atdsv.Fhoehe;
			char *start_ptr = tmp_ptr;
			int16 txt_len;
			aad_get_zeilen(start_ptr, &txt_len);
			str_null2leer(start_ptr, start_ptr + txt_len - 1);
			SplitStringInit tmp_ssi = ssi[aadv.StrHeader->AkPerson];
			ssr = split_string(&tmp_ssi);

			if (atdsv.Display == DISPLAY_TXT ||
			        (aadv.StrHeader->VocNr - ATDS_VOC_OFFSET) == -1) {
				for (int16 i = 0; i < ssr->Anz; i++) {
					out->printxy(ssr->X[i] + 1,
					              ssr->Y + (i * ssi[aadv.StrHeader->AkPerson].FHoehe),
					              0, 300, 0, ssr->StrPtr[i]);
					out->printxy(ssr->X[i] - 1,
					              ssr->Y + (i * ssi[aadv.StrHeader->AkPerson].FHoehe),
					              0, 300, 0, ssr->StrPtr[i]);
					out->printxy(ssr->X[i],
					              ssr->Y + (i * ssi[aadv.StrHeader->AkPerson].FHoehe) + 1,
					              0, 300, 0, ssr->StrPtr[i]);
					out->printxy(ssr->X[i],
					              ssr->Y + (i * ssi[aadv.StrHeader->AkPerson].FHoehe) - 1,
					              0, 300, 0, ssr->StrPtr[i]);
					out->printxy(ssr->X[i],
					              ssr->Y + (i * ssi[aadv.StrHeader->AkPerson].FHoehe),
					              aadv.Person[aadv.StrHeader->AkPerson].Color,
					              300, 0, ssr->StrPtr[i]);
					tmp_ptr += strlen(ssr->StrPtr[i]) + 1;
				}
				str_null2leer(start_ptr, start_ptr + txt_len - 1);

			} else if (atdsv.Display == DISPLAY_VOC) {
				if (atdsv.VocNr != aadv.StrHeader->VocNr - ATDS_VOC_OFFSET) {
					atdsv.VocNr = aadv.StrHeader->VocNr - ATDS_VOC_OFFSET;
					g_engine->_sound->playSpeech(atdsv.VocNr);
					int16 vocx = spieler_vector[aadv.StrHeader->AkPerson].Xypos[0] -
								 _G(spieler).scrollx + spieler_mi[aadv.StrHeader->AkPerson].HotX;
					g_engine->_sound->setSoundChannelBalance(0, getStereoPos(vocx));
					aadv.DelayCount = 1;
				}

				for (int16 i = 0; i < ssr->Anz; i++) {
					tmp_ptr += strlen(ssr->StrPtr[i]) + 1;
				}
				str_null2leer(start_ptr, start_ptr + txt_len - 1);
			}

			if (aadv.DelayCount <= 0) {
				aadv.Ptr = tmp_ptr;
				while (*tmp_ptr == ' ' || *tmp_ptr == 0)
					++tmp_ptr;
				if (tmp_ptr[1] == ATDS_END ||
				        tmp_ptr[1] == ATDS_END_EINTRAG) {
					if (atdsv.aad_str != 0)
						atdsv.aad_str(atdsv.DiaNr, aadv.StrNr, aadv.StrHeader->AkPerson, AAD_STR_END);
					aadv.Dialog = false;
					adsv.AutoDia = false;
					aadv.StrNr = -1;
					ssret.Next = false;
				} else {
					if (ssr->Next == false) {
						++aadv.StrNr;
						while (*aadv.Ptr++ != ATDS_END_TEXT);

						int16 tmp_person = aadv.StrHeader->AkPerson;
						int16 tmp_str_nr = aadv.StrNr;
						aadv.StrHeader = (AadStrHeader *)aadv.Ptr;
						aadv.Ptr += sizeof(AadStrHeader);
						if (atdsv.aad_str != nullptr) {
							if (tmp_person != aadv.StrHeader->AkPerson) {
								atdsv.aad_str(atdsv.DiaNr, tmp_str_nr, tmp_person, AAD_STR_END);
								atdsv.aad_str(atdsv.DiaNr, aadv.StrNr, aadv.StrHeader->AkPerson, AAD_STR_START);
							}
						}
					}
					aad_get_zeilen(aadv.Ptr, &txt_len);
					aadv.DelayCount = get_delay(txt_len);
					_printDelayCount1 = aadv.DelayCount / 10;
					aadv.SilentCount = atdsv.Silent;
				}
			} else {
				if (atdsv.Display == DISPLAY_TXT ||
				        (aadv.StrHeader->VocNr - ATDS_VOC_OFFSET) == -1)
					--aadv.DelayCount;

				else if (atdsv.Display == DISPLAY_VOC) {
					warning("FIXME - unknown constant SMP_PLAYING");

					aadv.DelayCount = 0;
				}
			}
		} else {
			--aadv.SilentCount;
		}
	}
}

int16 atdsys::aad_get_status() {
	return aadv.StrNr;
}

int16 atdsys::aad_get_zeilen(char *str_, int16 *txt_len) {
	*txt_len = 0;
	char *ptr = str_;
	int16 zeilen = 0;
	while (*str_ != ATDS_END_TEXT) {
		if (*str_++ == 0)
			++zeilen;
	}
	*txt_len = (str_ - ptr) - 1;

	return zeilen;
}

void atdsys::aad_search_dia(int16 dia_nr, char **ptr) {
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
				aadv.TxtHeader = (AadTxtHeader *)start_ptr;
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

bool  atdsys::ads_start(int16 dia_nr) {
	bool ret = false;

	load_atds(dia_nr, ADS_DATEI);
	if (!modul) {
		bool ende = false;
		if (atdsmem[ADS_HANDLE][0] == (char)BLOCKENDE &&
		        atdsmem[ADS_HANDLE][1] == (char)BLOCKENDE &&
		        atdsmem[ADS_HANDLE][2] == (char)BLOCKENDE)
			ende = true;
		if (!ende) {
			adsv.Ptr = atdsmem[ADS_HANDLE];
			adsv.TxtHeader = (AdsTxtHeader *)adsv.Ptr;
			if (adsv.TxtHeader->DiaNr == dia_nr) {
				ret = true;
				adsv.Ptr += sizeof(AdsTxtHeader);
				adsv.Person = (AadInfo *) adsv.Ptr;
				adsv.Ptr += adsv.TxtHeader->PerAnz * sizeof(AadInfo);
				adsv.Dialog = dia_nr;
				adsv.StrNr = 0;
				ads_stack[0] = 0;
				ads_stack_ptr = 1;
			}
		}
	}
	return ret;
}

void atdsys::stop_ads() {
	adsv.Dialog = -1;
	adsv.AutoDia = false;

}

int16 atdsys::ads_get_status() {
	return adsv.Dialog;
}

int16 atdsys::check_item(int16 block_nr, int16 item_nr) {
	int16 ret = true;
	char *tmp_adr = adsv.Ptr;
	ads_search_block(block_nr, &tmp_adr);
	if (tmp_adr) {
		ads_search_item(item_nr, &tmp_adr);
		if (tmp_adr) {
			ret = true;
		}
	}
	return ret;
}

char **atdsys::ads_item_ptr(int16 block_nr, int16 *anzahl) {
	*anzahl = 0;
	memset(e_ptr, 0, sizeof(char *)*ADS_MAX_BL_EIN);
	if (adsv.Dialog != -1) {
		adsv.BlkPtr = adsv.Ptr;
		ads_search_block(block_nr, &adsv.BlkPtr);
		if (adsv.BlkPtr) {
			for (int16 i = 0; i < ADS_MAX_BL_EIN; i++) {
				char *tmp_adr = adsv.BlkPtr;
				ads_search_item(i, &tmp_adr);
				if (tmp_adr) {
					char nr = tmp_adr[-1];
					tmp_adr += sizeof(AadStrHeader);
					if (ads_block[block_nr].Show[(int16)nr] == true) {
						e_ptr[*anzahl] = tmp_adr;
						e_nr[*anzahl] = (int16)nr;
						++(*anzahl);
					}
				}
			}
		}
	}

	return e_ptr;
}

AdsNextBlk *atdsys::ads_item_choice(int16 blk_nr, int16 item_nr) {
	adsnb.BlkNr = blk_nr;
	if (!aadv.Dialog) {
		if (!adsv.AutoDia) {
			ads_search_item(e_nr[item_nr], &adsv.BlkPtr);
			if (adsv.BlkPtr) {
				if (start_ads_auto_dia(adsv.BlkPtr))
					adsv.AutoDia = true;
				if (bit->is_bit((uint8)ads_block[blk_nr].Steuer[e_nr[item_nr]], ADS_EXIT_BIT) == true) {
					stop_ads();
					adsnb.EndNr = e_nr[item_nr];
					adsnb.BlkNr = -1;
				}
			}
		}
	}

	return &adsnb;
}

AdsNextBlk *atdsys::calc_next_block(int16 blk_nr, int16 item_nr) {
	if (bit->is_bit((uint8)ads_block[blk_nr].Steuer[e_nr[item_nr]], ADS_SHOW_BIT) == false)
		ads_block[blk_nr].Show[e_nr[item_nr]] = false;
	adsnb.EndNr = e_nr[item_nr];
	if (bit->is_bit((uint8)ads_block[blk_nr].Steuer[e_nr[item_nr]], ADS_RESTART_BIT) == true) {
		adsnb.BlkNr = 0;

		ads_stack_ptr = 0;
	} else {
		if (ads_block[blk_nr].Next[e_nr[item_nr]]) {
			adsnb.BlkNr = ads_block[blk_nr].Next[e_nr[item_nr]];

			int16 anzahl = 0;
			while (!anzahl && adsnb.BlkNr != -1) {

				anzahl = 0;
				ads_item_ptr(adsnb.BlkNr, &anzahl);
				if (!anzahl) {
					adsnb.BlkNr = return_block(ads_block);
				}
			}
		} else {
			adsnb.BlkNr = return_block(ads_block);
		}
	}
	ads_stack[ads_stack_ptr] = adsnb.BlkNr;
	++ads_stack_ptr;

	return &adsnb;
}

int16 atdsys::return_block(AdsBlock *ab) {
	ads_stack_ptr -= 1;
	int16 ret = -1;
	bool ende = false;
	while (ads_stack_ptr >= 0 && !ende) {
		short blk_nr = ads_stack[ads_stack_ptr];
		int16 anz;
		ads_item_ptr(blk_nr, &anz);
		if (anz) {
			ret = blk_nr;
			ende = true;
		} else
			--ads_stack_ptr;
	}
	++ads_stack_ptr;
	return ret;
}

void atdsys::ads_search_block(int16 blk_nr, char **ptr) {
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

void atdsys::ads_search_item(int16 item_nr, char **blk_adr) {
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

int16 atdsys::start_ads_auto_dia(char *item_adr) {
	aadv.Dialog = false;
	if (item_adr) {
		aadv.Person = adsv.Person;
		aadv.Ptr = item_adr;
		aadv.Dialog = true;
		aadv.StrNr = 0;
		aadv.StrHeader = (AadStrHeader *)aadv.Ptr;
		aadv.Ptr += sizeof(AadStrHeader);
		int16 txt_len;
		aad_get_zeilen(aadv.Ptr, &txt_len);
		aadv.DelayCount = get_delay(txt_len);
		atdsv.DiaNr = adsv.TxtHeader->DiaNr + 10000;

		if (atdsv.aad_str != nullptr)
			atdsv.aad_str(atdsv.DiaNr, 0, aadv.StrHeader->AkPerson, AAD_STR_START);
		_mousePush = true;
		stop_ats();
	} else {
		aadv.Dialog = false;
	}

	return aadv.Dialog;
}

void atdsys::hide_item(int16 dia_nr, int16 blk_nr, int16 item_nr) {
	if (adsv.Dialog == dia_nr) {
		if (check_item(blk_nr, item_nr))
			ads_block[blk_nr].Show[item_nr] = false;
	} else {
		load_atds(dia_nr, ADH_DATEI);
		if (check_item(blk_nr, item_nr))
			ads_block[blk_nr].Show[item_nr] = false;
		save_ads_header(dia_nr);
	}
}

void atdsys::show_item(int16 dia_nr, int16 blk_nr, int16 item_nr) {
	if (adsv.Dialog == dia_nr) {
		if (check_item(blk_nr, item_nr))
			ads_block[blk_nr].Show[item_nr] = true;
	} else {
		load_atds(dia_nr, ADH_DATEI);
		if (check_item(blk_nr, item_nr))
			ads_block[blk_nr].Show[item_nr] = true;
		save_ads_header(dia_nr);
	}
}

int16 atdsys::calc_inv_no_use(int16 cur_inv, int16 test_nr, int16 mode) {
	int16 txt_nr = -1;
	if (cur_inv != -1) {
		if (inv_block_nr != cur_inv) {
			inv_block_nr = cur_inv + 1;
			load_atds(inv_block_nr + atdspooloff[mode], INV_USE_DATEI);

			Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(
				atdshandle[INV_IDX_HANDLE]);
			if (rs) {
				rs->seek(InvUse::SIZE() * inv_block_nr
				      * INV_STRC_ANZ, SEEK_SET);

				InvUse *iu = (InvUse *)atdsmem[INV_IDX_HANDLE];
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

		InvUse *iu = (InvUse *)atdsmem[INV_IDX_HANDLE];
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

int16 atdsys::getStereoPos(int16 x) {
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
