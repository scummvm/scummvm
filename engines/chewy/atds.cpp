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

#define FORBIDDEN_SYMBOL_EXCEPTION_fopen
#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
#define FORBIDDEN_SYMBOL_EXCEPTION_fseek
#define FORBIDDEN_SYMBOL_EXCEPTION_fread
#define FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE


#include "chewy/ngshext.h"
#include "chewy/atds.h"
#include "chewy/defines.h"
#include "chewy/global.h"
extern char *err_str;
int16 mouse_push;

atdsys::atdsys() {
	int16 i;
	SplitStringInit init_ssi = { 0, 0, 0, 220, 4, SPLIT_MITTE, 8, 8,};
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
	atdsv.SpeechHandle = 0;
	atdsv.VocNr = -1;
	ssret.Next = false;
	ssr = &ssret;
	for (i = 0; i < AAD_MAX_PERSON; i++)
		ssi[i] = init_ssi;
	inv_block_nr = -1;
	inv_use_mem = false;
	inzeig = in->get_in_zeiger();
	for (i = 0; i < MAX_HANDLE; i++) {
		atdshandle[i] = 0;
		atdsmem[i] = 0;
		atdspooloff[i] = 0;
	}
}

atdsys::~atdsys() {
	int16 i;
	for (i = 0; i < MAX_HANDLE; i++) {
		if (atdshandle[i])
			warning("FIXME : close handler");

		free(atdsmem[i]);
	}
	if (inv_use_mem)
		free(inv_use_mem);
}

char atds_id[4][4] = { "ATS",
                       "AAD",
                       "ADS",
                       "ADH",
                     };

void atdsys::set_font(char *font_adr, int16 fvorx, int16 fhoehe) {
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

void atdsys::set_display(int16 mode) {
	atdsv.Display = mode;
}

int16 atdsys::get_delay(int16 txt_len) {
	int16 max_len;
	int16 z_len;
	int16 ret;
	z_len = (ssi->Width / ssi->Fvorx) + 1;
	max_len = z_len * ssi->Zeilen;
	if (txt_len > max_len)
		txt_len = max_len;

	ret = *atdsv.Delay * (txt_len + z_len);
	return (ret);
}

SplitStringRet *atdsys::split_string(SplitStringInit *ssi_) {
	char *str_adr;
	char *start_adr;
	int16 zeichen_anz;
	int16 count;
	int16 tmp_count;
	int16 ende;
	int16 test_zeilen;
	int16 ende1;
	ssret.Anz = 0;
	ssret.Next = false;
	ssret.StrPtr = split_ptr;
	ssret.X = split_x;
	zeichen_anz = (ssi_->Width / ssi_->Fvorx) + 1;
	memset(split_ptr, 0, sizeof(char *)*MAX_STR_SPLIT);
	calc_txt_win(ssi_);
	str_adr = ssi_->Str;
	count = 0;
	tmp_count = 0;
	ende = 0;
	start_adr = str_adr;
	while (!ende) {
		switch (*str_adr) {
		case 0:
			if (str_adr[1] != ATDS_END_TEXT) {
				str_adr[0] = ' ';
			}
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
					ende = 1;
					ende1 = 0;
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
					ende = 1;
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
				if (*str_adr == ',')
					test_zeilen = 1;
				else
					test_zeilen = 2;
				++count;
				++str_adr;
				if ((ssret.Anz + test_zeilen) >= ssi_->Zeilen) {
					if (count < zeichen_anz) {
						tmp_count = count;
						ende = 1;
					}
					split_ptr[ssret.Anz] = start_adr;
					start_adr[tmp_count] = 0;
					if (ssi_->Mode == SPLIT_MITTE)
						split_x[ssret.Anz] = ssi_->X + ((ssi_->Width - (strlen(start_adr) * ssi_->Fvorx)) >> 1);
					else
						split_x[ssret.Anz] = ssi_->X;
					++ssret.Anz;
					ende1 = 0;
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
	return (&ssret);
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

void *atdsys::pool_handle(const char *fname_, const char *fmode) {
	FILE *handle;
	handle = fopen(fname_, fmode);
	if (handle) {
		close_handle(ATDS_HANDLE);
		atdshandle[ATDS_HANDLE] = handle;
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
		err->set_user_msg(fname_);
	}
	return (handle);
}

void atdsys::set_speech_handle(void *speech_handle_) {
	atdsv.SpeechHandle = speech_handle_;
}

void atdsys::set_handle(const char *fname_, int16 mode, void *handle, int16 chunk_start, int16 chunk_anz) {
	char *tmp_adr;
	ChunkHead Ch;
	tmp_adr = atds_adr(fname_, chunk_start, chunk_anz);
	if (!modul) {
		if (handle) {
			close_handle(mode);
			atdshandle[mode] = handle;
			atdsmem[mode] = tmp_adr;
			atdspooloff[mode] = chunk_start;
			switch (mode) {
			case INV_USE_DATEI:
				mem->file->select_pool_item(atdshandle[mode], atdspooloff[mode]);
				fseek((FILE *)atdshandle[mode], -(sizeof(ChunkHead)), SEEK_CUR);

				if (!fread(&Ch, sizeof(ChunkHead), 1, (FILE *)atdshandle[mode])) {
					modul = DATEI;
					fcode = READFEHLER;
				} else {
					inv_use_mem = (char *)calloc(Ch.size + 3l, 1);
					if (!modul) {
						if (Ch.size) {
							if (!fread(inv_use_mem, Ch.size, 1, (FILE *)atdshandle[mode])) {
								fcode = READFEHLER;
								modul = DATEI;
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
		err->set_user_msg(fname_);
}

void atdsys::open_handle(const char *fname_, const char *fmode, int16 mode) {
	FILE *handle;
	char *tmp_adr;

	if (mode != INV_IDX_DATEI)
		tmp_adr = atds_adr(fname_, 0, 20000);
	if (!modul) {
		handle = fopen(fname_, fmode);
		if (handle) {
			close_handle(mode);
			atdshandle[mode] = handle;
			atdsmem[mode] = tmp_adr;
			switch (mode) {
			case ADH_DATEI:
				ads_block = (AdsBlock *)atdsmem[ADH_HANDLE];
				break;

			case INV_IDX_DATEI:
				atdsmem[INV_IDX_HANDLE] = (char *)calloc(INV_STRC_ANZ * sizeof(InvUse), 1);
				break;

			}
		} else {
			modul = DATEI;
			fcode = OPENFEHLER;
			err->set_user_msg(fname_);
		}
	}
}

void atdsys::close_handle(int16 mode) {
	if (atdshandle[mode])
		fclose((FILE *)atdshandle[mode]);
	atdshandle[mode] = 0;
	if (atdsmem[mode])
		free(atdsmem[mode]);
	atdsmem[mode] = 0;
}

char *atdsys::atds_adr(const char *fname_, int16 chunk_start, int16 chunk_anz) {
	char *tmp_adr;
	uint32 size;
	tmp_adr = NULL;
	size = mem->file->get_poolsize(fname_, chunk_start, chunk_anz);
	if (size) {
		tmp_adr = (char *)calloc(size + 3l, 1);
	}
	return (tmp_adr);
}

void atdsys::load_atds(int16 chunk_nr, int16 mode) {
	FILE *handle;
	ChunkHead Ch;
	char *txt_adr;
	handle = (FILE *)atdshandle[mode];
	txt_adr = atdsmem[mode];
	if (handle && txt_adr) {
		mem->file->select_pool_item(handle, chunk_nr + atdspooloff[mode]);
		fseek(handle, -sizeof(ChunkHead), SEEK_CUR);
		if (!fread(&Ch, sizeof(ChunkHead), 1, handle)) {
			modul = DATEI;
			fcode = READFEHLER;
		} else {
			if (Ch.size) {
				if (!fread(txt_adr, Ch.size, 1, handle)) {
					fcode = READFEHLER;
					modul = DATEI;
				} else if (mode != ADH_DATEI)
					crypt(txt_adr, Ch.size);
			}
			txt_adr[Ch.size] = (char)BLOCKENDE;
			txt_adr[Ch.size + 1] = (char)BLOCKENDE;
			txt_adr[Ch.size + 2] = (char)BLOCKENDE;
		}
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
}

void atdsys::save_ads_header(int16 dia_nr) {
	ChunkHead Ch;
	if (atdshandle[ADH_HANDLE]) {
		mem->file->select_pool_item(atdshandle[ADH_HANDLE], dia_nr);

		fseek((FILE *)atdshandle[ADH_HANDLE], -sizeof(ChunkHead), SEEK_CUR);
		if (!fread(&Ch, sizeof(ChunkHead), 1, (FILE *)atdshandle[ADH_HANDLE])) {
			modul = DATEI;
			fcode = READFEHLER;
		} else {
			fseek((FILE *)atdshandle[ADH_HANDLE], 0, SEEK_CUR);
			if (Ch.size) {
				if (!fwrite(atdsmem[ADH_HANDLE], Ch.size, 1, (FILE *)atdshandle[ADH_HANDLE])) {
					fcode = WRITEFEHLER;
					modul = DATEI;
				}
			}
		}
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
}

void atdsys::crypt(char *txt_, uint32 size) {
	uint8 *sp;
	uint32 i;
	sp = (uint8 *)txt_;
	for (i = 0; i < size; i++) {
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

bool atdsys::start_ats(int16 txt_nr, int16 txt_mode, int16 color, int16 mode,
                        int16 *voc_nr) {
	int16 txt_anz;
	char *ptr;
	*voc_nr = -1;
	set_ats_mem(mode);
	if (atsmem) {
		atsv.Ptr = ats_get_txt(txt_nr, txt_mode, &txt_anz, mode);
		if (atsv.Ptr) {
			atsv.Display = true;
			ptr = atsv.Ptr;
			atsv.TxtLen = 0;
			while (*ptr++ != ATDS_END_TEXT)
				++atsv.TxtLen;
			if (*atsv.Ptr == '\xb0') { // FIXME. CHECK THE ACTUAL CODE

				atsv.Display = false;
			} else {
				atsv.DelayCount = get_delay(atsv.TxtLen);
				atsv.Color = color;
				mouse_push = true;
			}
			*voc_nr = atsv.StrHeader->VocNr - ATDS_VOC_OFFSET;
			if ((atdsv.Display == DISPLAY_VOC) && (*voc_nr != -1)) {
				atsv.Display = false;
			}
		} else
			atsv.Display = false;
	} else
		atsv.Display = false;
	return (atsv.Display);
}

void atdsys::stop_ats() {
	atsv.Display = false;
}

int16 atdsys::ats_get_status() {
	return (atsv.Display);
}

void atdsys::print_ats(int16 x, int16 y, int16 scrx, int16 scry) {
	int16 i;
	char *tmp_ptr;
	char *start_ptr;
	SplitStringInit tmp_ssi;
	if (atsv.Display) {
		switch (in->get_switch_code()) {
		case ESC:
		case ENTER:
		case MAUS_LINKS:
			if (mouse_push == false) {
				if (atsv.SilentCount <= 0) {
					mouse_push = true;
					atsv.DelayCount = 0;
					inzeig->kbinfo->scan_code = 0;
					inzeig->kbinfo->key_code = 0;
				}
			}
			break;

		default:
			mouse_push = false;
			break;

		}
		if (atsv.SilentCount <= 0) {
			tmp_ptr = atsv.Ptr;
			out->set_fontadr(atdsv.Font);
			out->set_vorschub(atdsv.Fvorx, 0);
			ats_ssi = ssi[0];
			ats_ssi.Str = tmp_ptr;
			ats_ssi.Fvorx = atdsv.Fvorx;
			ats_ssi.FHoehe = atdsv.Fhoehe;
			ats_ssi.X = x - scrx;
			ats_ssi.Y = y - scry;
			start_ptr = tmp_ptr;
			str_null2leer(start_ptr, start_ptr + atsv.TxtLen - 1);
			tmp_ssi = ats_ssi;
			ssr = split_string(&tmp_ssi);
			for (i = 0; i < ssr->Anz; i++) {

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
					atsv.SilentCount = atdsv.Silent;
				}
			} else
				--atsv.DelayCount;
		} else
			--atsv.SilentCount;
	}
}

char *atdsys::ats_get_txt(int16 txt_nr, int16 txt_mode, int16 *txt_anz, int16 mode) {
	char *str_;
	uint8 status;
	uint8 lo_hi[2];
	int16 ak_nybble;
	str_ = 0;
	set_ats_mem(mode);

	atsv.TxtMode = txt_mode;
	if (!get_steuer_bit(txt_nr, ATS_AKTIV_BIT, mode)) {

		status = ats_sheader[(txt_nr * MAX_ATS_STATUS) + (atsv.TxtMode + 1) / 2];
		ak_nybble = (atsv.TxtMode + 1) % 2;

		lo_hi[1] = status >> 4;
		lo_hi[0] = status &= 15;
		str_ = ats_search_block(atsv.TxtMode, atsmem);
		if (str_ != 0) {
			ats_search_nr(txt_nr, &str_);
			if (str_ != 0) {

				ats_search_str(txt_anz, &lo_hi[ak_nybble],
				               (uint8)ats_sheader[txt_nr * MAX_ATS_STATUS], &str_);
				if (str_ != 0) {
					status = 0;
					lo_hi[1] <<= 4;
					status |= lo_hi[0];
					status |= lo_hi[1];
					ats_sheader[(txt_nr * MAX_ATS_STATUS) + (atsv.TxtMode + 1) / 2] = status;
				}
			}
		}
	}
	return (str_);
}

void atdsys::set_ats_str(int16 txt_nr, int16 txt_mode, int16 str_nr, int16 mode) {
	uint8 status;
	uint8 lo_hi[2];
	int16 ak_nybble;
	set_ats_mem(mode);
	status = ats_sheader[(txt_nr * MAX_ATS_STATUS) + (txt_mode + 1) / 2];
	ak_nybble = (txt_mode + 1) % 2;

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
	int16 i;
	for (i = 0; i < 5; i++)
		set_ats_str(txt_nr, i, str_nr, mode);
}

int16 atdsys::get_ats_str(int16 txt_nr, int16 txt_mode, int16 mode) {
	uint8 status;
	uint8 lo_hi[2];
	int16 ak_nybble;
	set_ats_mem(mode);
	status = ats_sheader[(txt_nr * MAX_ATS_STATUS) + (txt_mode + 1) / 2];
	ak_nybble = (txt_mode + 1) % 2;

	lo_hi[1] = status >> 4;
	lo_hi[0] = status &= 15;
	return ((int16)lo_hi[ak_nybble]);
}

int16 atdsys::get_steuer_bit(int16 txt_nr, int16 bit_idx, int16 mode) {
	int16 ret;
	set_ats_mem(mode);
	ret = bit->is_bit(ats_sheader[txt_nr * MAX_ATS_STATUS], bit_idx);
	return (ret);
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
	char *str_;
	int16 ende;
	str_ = txt_adr;
	ende = 0;
	while (!ende) {
		if (str_[0] == (char)BLOCKENDE &&
		        str_[1] == (char)BLOCKENDE &&
		        str_[2] == (char)BLOCKENDE) {
			ende = 2;
		} else if (*str_ == (char)STEUERBYTE) {
			if (str_[1] == (char)txt_mode) {
				ende = 1;
				++str_;
			}
		}
		++str_;
	}
	if (ende == 2)
		str_ = 0;
	return (str_);
}

void atdsys::ats_search_nr(int16 txt_nr, char **str_) {
	char *start_str;
	int16 *pos;
	int16 ende;
	int16 ende1;
	start_str = *str_;
	ende = 0;
	while (!ende) {
		pos = (int16 *)start_str;
		if (*pos == txt_nr) {
			ende = 1;
			atsv.TxtHeader = (AtsTxtHeader *)pos;
			*str_ = (char *)pos;
			*str_ += sizeof(AtsTxtHeader);
			if (atsv.TxtMode != TXT_MARK_NAME)
				atsv.StrHeader = (AtsStrHeader *)str_;
			*str_ += sizeof(AtsStrHeader);
		} else {
			start_str += sizeof(AtsTxtHeader);
			start_str += sizeof(AtsStrHeader);
			ende1 = 0;
			while (!ende1) {
				if (start_str[0] == ATDS_END) {
					if ((start_str[1] == (char)BLOCKENDE &&
					        start_str[2] == (char)BLOCKENDE &&
					        start_str[3] == (char)BLOCKENDE) ||
					        start_str[1] == STEUERBYTE) {
						ende = 1;
						ende1 = 1;
						*str_ = 0;
					} else {
						ende1 = 1;

					}
				}
				++start_str;
			}
		}
	}
}

void atdsys::ats_search_str(int16 *anz, uint8 *status, uint8 steuer, char **str_) {
	char *tmp_str;
	char *start_str;
	int16 ende;
	int16 count;
	tmp_str = *str_;
	start_str = *str_;
	tmp_str += sizeof(AtsStrHeader);
	*anz = 0;
	ende = 0;
	count = 0;
	while (!ende) {
		if (count == *status) {
			if (!*tmp_str) {
				++*anz;
			} else if (*tmp_str == ATDS_END_TEXT) {
				ende = 1;
				*str_ = start_str;
				start_str -= sizeof(AtsStrHeader);
				if (atsv.TxtMode != TXT_MARK_NAME)
					atsv.StrHeader = (AtsStrHeader *)start_str;
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
					ende = 1;
					*anz = 0;
					*status = count;
					*str_ = start_str;
					start_str -= sizeof(AtsStrHeader);
					if (atsv.TxtMode != TXT_MARK_NAME)
						atsv.StrHeader = (AtsStrHeader *)start_str;
				} else {
					++count;

					tmp_str += sizeof(AtsStrHeader);
					start_str = tmp_str + 1;
				}
			} else if (*tmp_str == ATDS_END ||
			           (tmp_str[0] == (char)BLOCKENDE &&
			            tmp_str[1] == (char)BLOCKENDE &&
			            tmp_str[2] == (char)BLOCKENDE)) {
				ende = 1;
				*anz = 0;
				*str_ = 0;
			}
		}
		++tmp_str;
	}
}

int16 atdsys::start_aad(int16 dia_nr) {
	int16 *pos;
	int16 txt_len;

	aadv.Dialog = false;
	if (atdsmem[AAD_HANDLE]) {
		aadv.Ptr = atdsmem[AAD_HANDLE];
		aad_search_dia(dia_nr, &aadv.Ptr);
		if (aadv.Ptr) {
			pos = (int16 *)aadv.Ptr;
			aadv.Person = (AadInfo *) aadv.Ptr;
			aadv.Ptr += aadv.TxtHeader->PerAnz * sizeof(AadInfo);
			aadv.Dialog = true;
			aadv.StrNr = 0;
			aadv.StrHeader = (AadStrHeader *)aadv.Ptr;
			aadv.Ptr += sizeof(AadStrHeader);
			aad_get_zeilen(aadv.Ptr, &txt_len);
			aadv.DelayCount = get_delay(txt_len);
			atdsv.DiaNr = dia_nr;
			if (atdsv.aad_str != 0)
				atdsv.aad_str(atdsv.DiaNr, 0, aadv.StrHeader->AkPerson, AAD_STR_START);
			mouse_push = true;
			stop_ats();
			atdsv.VocNr = -1;
		}
	}
	return (aadv.Dialog);
}

void atdsys::stop_aad() {
	aadv.Dialog = false;
	aadv.StrNr = -1;
}

void atdsys::print_aad(int16 scrx, int16 scry) {
	int16 *pos;
	int16 i;
	int16 txt_len;
	int16 tmp_person;
	int16 tmp_str_nr;
	char *tmp_ptr;
	char *start_ptr;
	int16 vocx;
	SplitStringInit tmp_ssi;
	if (aadv.Dialog) {
		switch (in->get_switch_code()) {
		case ESC:
		case ENTER:
		case MAUS_LINKS:
			if (mouse_push == false) {
				if (aadv.SilentCount <= 0) {
					mouse_push = true;
					aadv.DelayCount = 0;
					inzeig->kbinfo->scan_code = 0;
					inzeig->kbinfo->key_code = 0;
				}
			}
			break;

		default:
			mouse_push = false;
			break;

		}
		if (aadv.SilentCount <= 0) {
			tmp_ptr = aadv.Ptr;
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
			start_ptr = tmp_ptr;
			aad_get_zeilen(start_ptr, &txt_len);
			str_null2leer(start_ptr, start_ptr + txt_len - 1);
			tmp_ssi = ssi[aadv.StrHeader->AkPerson];
			ssr = split_string(&tmp_ssi);

			if (atdsv.Display == DISPLAY_TXT ||
			        (aadv.StrHeader->VocNr - ATDS_VOC_OFFSET) == -1) {
				for (i = 0; i < ssr->Anz; i++) {
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
			}

#ifndef SCRIPT
			else if (atdsv.Display == DISPLAY_VOC) {
				if (atdsv.VocNr != aadv.StrHeader->VocNr - ATDS_VOC_OFFSET) {
					if (atdsv.SpeechHandle) {
						atdsv.VocNr = aadv.StrHeader->VocNr - ATDS_VOC_OFFSET;
						if (atdsv.VocNr != -1) {
							mem->file->select_pool_item(atdsv.SpeechHandle, atdsv.VocNr);
							ERROR

							vocx = spieler_vector[aadv.StrHeader->AkPerson].Xypos[0] -
							       spieler.scrollx + spieler_mi[aadv.StrHeader->AkPerson].HotX;
							ailsnd->set_stereo_pos(0, get_stereo_pos(vocx));
							ailsnd->start_db_voc(atdsv.SpeechHandle, 0, 63);
							ailsnd->set_stereo_pos(0, get_stereo_pos(vocx));
							aadv.DelayCount = 1;
						}
					} else {
						modul = DATEI;
						fcode = OPENFEHLER;
						err->set_user_msg("SPRACHAUSGABE.TVP\0");
					}
				}
				for (i = 0; i < ssr->Anz; i++) {
					tmp_ptr += strlen(ssr->StrPtr[i]) + 1;
				}
				str_null2leer(start_ptr, start_ptr + txt_len - 1);
			}
#endif
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

						tmp_person = aadv.StrHeader->AkPerson;
						tmp_str_nr = aadv.StrNr;
						pos = (int16 *)aadv.Ptr;
						aadv.StrHeader = (AadStrHeader *)aadv.Ptr;
						aadv.Ptr += sizeof(AadStrHeader);
						if (atdsv.aad_str != 0) {
							if (tmp_person != aadv.StrHeader->AkPerson) {
								atdsv.aad_str(atdsv.DiaNr, tmp_str_nr, tmp_person, AAD_STR_END);
								atdsv.aad_str(atdsv.DiaNr, aadv.StrNr, aadv.StrHeader->AkPerson, AAD_STR_START);
							}
						}
					}
					aad_get_zeilen(aadv.Ptr, &txt_len);
					aadv.DelayCount = get_delay(txt_len);
					aadv.SilentCount = atdsv.Silent;
				}
			} else {
				if (atdsv.Display == DISPLAY_TXT ||
				        (aadv.StrHeader->VocNr - ATDS_VOC_OFFSET) == -1)
					--aadv.DelayCount;
#ifndef SCRIPT
				else if (atdsv.Display == DISPLAY_VOC) {
					warning("FIXME - unknown constant SMP_PLAYING");

					aadv.DelayCount = 0;
				}
#endif
			}
		} else
			--aadv.SilentCount;
	}
}

int16 atdsys::aad_get_status() {
	return (aadv.StrNr);
}

int16 atdsys::aad_get_zeilen(char *str_, int16 *txt_len) {
	int16 zeilen;
	char *ptr;
	*txt_len = 0;
	ptr = str_;
	zeilen = 0;
	while (*str_ != ATDS_END_TEXT) {
		if (*str_++ == 0)
			++zeilen;
	}
	*txt_len = (str_ - ptr) - 1;
	return (zeilen);
}

void atdsys::aad_search_dia(int16 dia_nr, char **ptr) {
	char *start_ptr;
	int16 *pos;
	int16 ende;
	int16 ende1;
	start_ptr = *ptr;
	ende = 0;
	if (start_ptr[0] == (char)BLOCKENDE &&
	        start_ptr[1] == (char)BLOCKENDE &&
	        start_ptr[2] == (char)BLOCKENDE) {
		*ptr = 0;
	} else {
		while (!ende) {
			pos = (int16 *)start_ptr;
			if (pos[0] == dia_nr) {
				ende = 1;
				aadv.TxtHeader = (AadTxtHeader *)start_ptr;
				*ptr = start_ptr + sizeof(AadTxtHeader);
			} else {
				start_ptr += sizeof(AadTxtHeader) + pos[1] * sizeof(AadInfo);
				ende1 = 0;
				while (!ende1) {
					if (*start_ptr == ATDS_END) {
						if (start_ptr[1] == (char)BLOCKENDE &&
						        start_ptr[2] == (char)BLOCKENDE &&
						        start_ptr[3] == (char)BLOCKENDE) {
							ende = 1;
							ende1 = 1;
							*ptr = 0;
						} else
							ende1 = 1;
					}
					++start_ptr;
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
	return (ret);
}

void atdsys::stop_ads() {
	adsv.Dialog = -1;
	adsv.AutoDia = false;

}

int16 atdsys::ads_get_status() {
	return (adsv.Dialog);
}

int16 atdsys::check_item(int16 block_nr, int16 item_nr) {
	int16 ret;
	char *tmp_adr;
	ret = true;
	tmp_adr = adsv.Ptr;
	ads_search_block(block_nr, &tmp_adr);
	if (tmp_adr) {
		ads_search_item(item_nr, &tmp_adr);
		if (tmp_adr) {
			ret = true;
		}
	}
	return (ret);
}

char **atdsys::ads_item_ptr(int16 block_nr, int16 *anzahl) {
	int16 i;
	char nr;
	char *tmp_adr;
	*anzahl = 0;
	memset(e_ptr, 0, sizeof(char *)*ADS_MAX_BL_EIN);
	if (adsv.Dialog != -1) {
		adsv.BlkPtr = adsv.Ptr;
		ads_search_block(block_nr, &adsv.BlkPtr);
		if (adsv.BlkPtr) {
			for (i = 0; i < ADS_MAX_BL_EIN; i++) {
				tmp_adr = adsv.BlkPtr;
				ads_search_item(i, &tmp_adr);
				if (tmp_adr) {
					nr = tmp_adr[-1];
					tmp_adr += sizeof(AadStrHeader);
					if (ads_block[block_nr].Show[(int16)nr] == true) {
						e_ptr[*anzahl] = tmp_adr;
						e_nr [*anzahl] = (int16)nr;
						++(*anzahl);
					}
				}
			}
		}
	}
	return (e_ptr);
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
	return (&adsnb);
}

AdsNextBlk *atdsys::calc_next_block(int16 blk_nr, int16 item_nr) {
	int16 anzahl;

	if (bit->is_bit((uint8)ads_block[blk_nr].Steuer[e_nr[item_nr]], ADS_SHOW_BIT) == false)
		ads_block[blk_nr].Show[e_nr[item_nr]] = false;
	adsnb.EndNr = e_nr[item_nr];
	if (bit->is_bit((uint8)ads_block[blk_nr].Steuer[e_nr[item_nr]], ADS_RESTART_BIT) == true) {
		adsnb.BlkNr = 0;

		ads_stack_ptr = 0;
	} else {
		if (ads_block[blk_nr].Next[e_nr[item_nr]]) {
			adsnb.BlkNr = ads_block[blk_nr].Next[e_nr[item_nr]];

			anzahl = 0;
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
	return (&adsnb);
}

int16 atdsys::return_block(AdsBlock *ab) {
	int16 ret;
	short blk_nr;
	int16 anz;
	int16 ende;
	ads_stack_ptr -= 1;
	ret = -1;
	ende = 0;
	while (ads_stack_ptr >= 0 && !ende) {
		blk_nr = ads_stack[ads_stack_ptr];
		ads_item_ptr(blk_nr, &anz);
		if (anz) {
			ret = blk_nr;
			ende = 1;
		} else
			--ads_stack_ptr;
	}
	++ads_stack_ptr;
	return (ret);
}

void atdsys::ads_search_block(int16 blk_nr, char **ptr) {
	char *start_ptr;
	int16 ende;
	start_ptr = *ptr;
	ende = 0;
	while (!ende) {
		if (*start_ptr == (char)blk_nr) {
			ende = 1;
			*ptr = start_ptr;
		} else {
			start_ptr += 2 + sizeof(AadStrHeader);
			while (*start_ptr++ != ATDS_END_BLOCK);
			if (start_ptr[0] == ATDS_END &&
			        start_ptr[1] == ATDS_END) {
				ende = 1;
				*ptr = 0;
			}
		}
	}
}

void atdsys::ads_search_item(int16 item_nr, char **blk_adr) {
	char *start_ptr;
	int16 ende;
	start_ptr = *blk_adr + 1;
	ende = 0;
	while (!ende) {
		if (*start_ptr == item_nr) {
			ende = 1;
			*blk_adr = start_ptr + 1;
		} else {
			start_ptr += 1 + sizeof(AadStrHeader);
			while (*start_ptr++ != ATDS_END_EINTRAG);
			if (*start_ptr == ATDS_END_BLOCK) {
				ende = 1;
				*blk_adr = 0;
			}
		}
	}
}

int16 atdsys::start_ads_auto_dia(char *item_adr) {
	int16 txt_len;
	aadv.Dialog = false;
	if (item_adr) {
		aadv.Person = adsv.Person;
		aadv.Ptr = item_adr;
		aadv.Dialog = true;
		aadv.StrNr = 0;
		aadv.StrHeader = (AadStrHeader *)aadv.Ptr;
		aadv.Ptr += sizeof(AadStrHeader);
		aad_get_zeilen(aadv.Ptr, &txt_len);
		aadv.DelayCount = get_delay(txt_len);
		atdsv.DiaNr = adsv.TxtHeader->DiaNr + 10000;

		if (atdsv.aad_str != 0)
			atdsv.aad_str(atdsv.DiaNr, 0, aadv.StrHeader->AkPerson, AAD_STR_START);
		mouse_push = true;
		stop_ats();
	} else {
		aadv.Dialog = false;
	}
	return (aadv.Dialog);
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
	int16 i;
	bool ok;
	int16 txt_nr;
	InvUse *iu;
	txt_nr = -1;
	if (cur_inv != -1) {
		if (inv_block_nr != cur_inv) {
			inv_block_nr = cur_inv + 1;
			load_atds(inv_block_nr + atdspooloff[mode], INV_USE_DATEI);
			if (atdshandle[INV_IDX_HANDLE]) {
				fseek((FILE *)atdshandle[INV_IDX_HANDLE], sizeof(InvUse)*inv_block_nr
				      *INV_STRC_ANZ, SEEK_SET);
				if
				(!fread(atdsmem[INV_IDX_HANDLE], sizeof(InvUse)*INV_STRC_ANZ, 1, (FILE *)atdshandle[INV_IDX_HANDLE])) {
					modul = DATEI;
					fcode = READFEHLER;
				}
			} else {
				modul = DATEI;
				fcode = OPENFEHLER;
			}
		}
		iu = (InvUse *) atdsmem[INV_IDX_HANDLE];
		ok = false;
		for (i = 0; i < INV_STRC_ANZ && !ok; i++) {
			if (iu[i].ObjId == mode) {
				if (iu[i].ObjNr == test_nr) {
					txt_nr = iu[i].TxtNr;
					ok = true;
				}
			}
		}
	}
	return (txt_nr);
}

int16 atdsys::get_stereo_pos(int16 x) {
	float fx_ = (float)x;

	fx_ /= 2.5;
	if ((fx_ - ((float)((int16)fx_))) > 0.5)
		fx_ += 1.0;

	return ((int16)fx_);
}
