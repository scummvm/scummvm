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

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/file.h"
#include "chewy/global.h"
#include "chewy/resource.h"

namespace Chewy {

#define SURIMY_START 0
#define SURIMY_END 7
#define SCHNULLER 8
#define SCHNULL_BAND 22
#define MUND_START 9
#define MUND_END 11
#define SCHNULL_OFF 23
#define TDISP_START 12
#define TDISP_END 19
#define TDISP_EIN 20
#define TDISP_AUS 21
#define MUSIC_OFF 24
#define MUSIC_ON1 25
#define MUSIC_ON2 26
#define EXIT 27

const int16 OPTION_ICONS[9 * 4] = {
	 18,   61,   40,   76,
	112,   61,  130,   76,
	 82,  104,  144,  139,
	164,   93,  194,  115,
	198,   80,  206,  115,
	210,   55,  302,  138,
	126,  146,  210,  198,
	 22,   92,   44,  136,
	 50,   92,   72,  136
};

static const int16 FILE_ICONS[8 * 4] = {
	14, 73, 32, 94,
	14, 96, 32, 118,
	36, 64, 310, 128,
	16, 143, 76, 193,
	78, 143, 130, 193,
	132, 143, 178, 193,
	180, 143, 228, 193,
	232, 143, 310, 193
};

static const int16 CREDITS_POS[65][2] = {
	{40,  160}, {80,  170}, {40,  190}, {80,  200}, {80,  210},
	{40,  230}, {80,  240}, {80,  250}, {80,  260}, {40,  280},
	{80,  290}, {80,  300}, {80,  310}, {80,  320}, {40,  340},
	{80,  350}, {80,  360}, {40,  380}, {80,  390}, {40,  410},
	{80,  420}, {40,  440}, {80,  450}, {40,  470}, {80,  480},
	{80,  490}, {80,  500}, {80,  510}, {80,  520}, {40,  540},
	{80,  550}, {80,  560}, {80,  570}, {80,  580}, {40,  600},
	{80,  610}, {80,  620}, {80,  630}, {80,  640}, {80,  650},
	{80,  660}, {80,  670}, {80,  680}, {80,  690}, {80,  700},
	{80,  710}, {80,  720}, {80,  730}, {80,  740}, {80,  750},
	{80,  760}, {40,  780}, {40,  790}, {40,  800}, {40,  810},
	{40,  820}, {40,  840}, {80,  850}, {40,  870}, {80,  880},
	{80,  890}, {80,  910}, {80,  920}, {80,  930}, {80,  940}
};

static const bool CREDITS_TYPE[65] = {
	true, false, true, false, false, true, false, false, false, true,
	false, false, false, false, true, false, false, true, false, true,
	false, true, false, true, false, false, false, false, false, true,
	false, false, false, false, true, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false,
	false, true, true, true, true, true, true, false, true, false,
	false, true, true, true, true
};

static const char *CREDITS_TEXT[65] = {
	"Idea & Story:",
	"Carsten Wieland",
	"Programming:",
	"Helmut Theuerkauf",
	"Alexander Diessner",
	"Graphics & Animation:",
	"Carsten Wieland",
	"Nihat Keesen",
	"Stefan Frank",
	"Text & Dialogues:",
	"Helmut Theuerkauf",
	"Alexander Diessner",
	"Carsten Wieland",
	"Wolfgang Walk",
	"Music:",
	"Carsten Wieland",
	"Stelter Studios",
	"Sound FX:",
	"Helmut Theuerkauf",
	"Producer & Lecturer:",
	"Wolfgang Walk",
	"Minister of financial affairs:",
	"Carsten (Dagobert) Korte",
	"Testers:",
	"Lutz Rafflenbeul",
	"Thomas Friedmann",
	"Bernhard Ewers",
	"Christian von der Hotline",
	"Carsten Korte",
	"The voices:",
	"Chewy......Renier Baaken",
	"Howard.....Wolfgang Walk",
	"Nichelle...Indhira Mohammed",
	"Clint......Alexander Schottky",
	"Also cast:",
	"Renier Baaken",
	"Guido B\x94sherz",
	"Gerhard Fehn",
	"Alice Krause",
	"Reinhard Lie\xE1",
	"Willi Meyer",
	"Nicole Meister",
	"Lutz Rafflenbeul",
	"Alexander Schottky",
	"Bernd Schulze",
	"Susanne Simenec",
	"Helmut Theuerkauf",
	"Andreas Vogelpoth",
	"Mark Wagener",
	"Wolfgang Walk",
	"Thomas Piet Wiesenm\x81ller",
	"Speech recorded by",
	"tmp Studio, Moers by Willi Meyer",
	"Cut by Hartmut Stelter",
	"Studios Hamburg and",
	"Carsten Wieland",
	"Soundsystem:",
	"AIL (c) Miles Design",
	"Adventure Engine:",
	"I.C.M. developed by",
	"New Generation Software",
	"Song Boo Boo Ba Baby composed",
	"by Haiko Ruttmann,",
	"Lyrics Wolfgang Walk,",
	"featuring Indhira Mohammed."
};

	Common::File *File::open(const char *name) {
	Common::File *f = new Common::File();
	if (f->open(name)) {
		return f;
	} else {
		delete f;
		return nullptr;
	}
}

bool File::readArray(Common::SeekableReadStream *src, uint16 *arr, size_t size) {
	Common::SeekableReadStream *rs = src->readStream(size * 2);

	bool result = (uint32)rs->size() == (size * 2);
	if (result) {
		for (; size > 0; --size, ++arr)
			*arr = rs->readUint16LE();
	}

	delete rs;
	return result;
}

int16 call_fileio(int16 palette, int16 mode) {
	short ret;
	ret = 0;
	ioptr.popx = 30;
	ioptr.popy = 50;
	switch (palette) {

	case PAL1:
		ioptr.m_col = io_pal1;
		break;
	}
	switch (mode) {

	case LOADIO:
		ioptr.f1 = 0;
		ioptr.f2 = 1;
		ioptr.f3 = 1;
		ioptr.f4 = 1;
		ioptr.key_nr = 2;
		break;

	case SAVEIO:
		ioptr.f1 = 1;
		ioptr.f2 = 0;
		ioptr.f3 = 1;
		ioptr.f4 = 1;
		ioptr.key_nr = 1;
		break;
	case ALLIO:
		ioptr.f1 = 1;
		ioptr.f2 = 1;
		ioptr.f3 = 1;
		ioptr.f4 = 1;
		ioptr.key_nr = 0;
		break;

	}
	curblk.no_back = false;
	cursor_wahl(CUR_SAVE);
	out->sprite_save(cur_back, minfo.x + curblk.page_off_x,
	                  minfo.y + curblk.page_off_y, 16,
	                  16, scr_width);
	cur->hide_cur();
	cur->show_cur();
	cur->hide_cur();
	ret = iog->io_menu(&ioptr);
	cur->show_cur();
	curblk.no_back = true;
	return ret;
}

int16 file_menue() {
	int16 u_index;
	int16 key = 0;
	int16 i, j;
	int16 x[8];
	int16 y[8];
	int16 mode[9];
	int16 ret;
	char *fnames;
	char *tmp;
	int16 text_off, active_slot;
	int16 rect, line;
	taf_info *ti = mem->taf_adr(OPTION_TAF);
	//SpriteResource *options = new SpriteResource(OPTION_TAF);
	ret = 0;
	room->load_tgp(1, &room_blk, GBOOK_TGP, 0, "back/gbook.tgp");
	out->setze_zeiger(workptr);
	out->map_spr2screen(ablage[room_blk.AkAblage], 0, 0);
	out->setze_zeiger(screen0);
	room->set_ak_pal(&room_blk);
	fx->blende1(workptr, screen0, pal, 150, 0, 0);
	out->setze_zeiger(workptr);

	fnames = iog->io_init(&ioptr);
	fnames += 1;
	if (!modul) {
		x[0] = 1;
		x[1] = 1;
		y[0] = 0;
		y[1] = 0;
		for (i = 2; i < 8; i++) {
			x[i] = 5;
			y[i] = 5;
		}
		for (i = 0; i < 8; i++)
			mode[i] = 0;
		mode[8] = 1;
		text_off = 0;
		active_slot = 0;
		while (key != ESC) {
			out->map_spr2screen(ablage[room_blk.AkAblage], 0, 0);
			for (i = 28, j = 0; i < 35; i++, j++) {
#if 0
				TAFChunk *sprite = options->getSprite(i);
				// TODO: korrektur?
				if (!mode[j])
					out->sprite_set(sprite->data, 16 + /*ti->korrektur[i << 1] + */x[j],
									76 + /*ti->korrektur[(i << 1) + 1] + */y[j], 0);
				else
					out->sprite_set(sprite->data, 16 /* + ti->korrektur[i << 1]*/,
									76 /*+ ti->korrektur[(i << 1) + 1]*/, 0);
#endif
				if (!mode[j])
					out->sprite_set(ti->image[i], 16 + ti->korrektur[i << 1] + x[j],
									76 + ti->korrektur[(i << 1) + 1] + y[j], 0);
				else
					out->sprite_set(ti->image[i], 16 + ti->korrektur[i << 1],
									76 + ti->korrektur[(i << 1) + 1], 0);
			}
			key = in->get_switch_code();
			if (mode[2] || mode[3] || mode[8]) {
				tmp = fnames + (text_off * 40);
				for (i = 0; i < 6; i++) {
					if (tmp != 0) {
						if (i != active_slot) {
							out->printxy(40, 68 + (i * 10), 14, 300, 0, "%2d.", text_off + i);
							out->printxy(70, 68 + (i * 10), 14, 300, 0, tmp);
						} else {
							if (mode[8]) {
								out->printxy(40, 68 + (i * 10), 14, 300, 0, "%2d.", text_off + i);
								out->printxy(70, 68 + (i * 10), 14, 300, 0, tmp);
							} else {
								out->box_fill(40, 68 + (i * 10), 308, 68 + 8 + (i * 10), 42);
								out->printxy(40, 68 + (i * 10), 255, 300, 0, "%2d.", text_off + i);
								out->printxy(70, 68 + (i * 10), 255, 300, 0, tmp);
							}
						}
					}
					tmp += 40;
				}
			}
			if (mode[0])
				--mode[0];
			if (mode[1])
				--mode[1];
			if (mode[4])
				--mode[4];
			if (mode[4] == 1)
				key = ESC;
			if (mode[5])
				--mode[5];
			if (mode[5] == 1) {
#ifdef ENGLISCH
				out->printxy(120, 138, 255, 300, 0, "QUIT ?? Y/N ");
#else
				out->printxy(120, 138, 255, 300, 0, "BEENDEN ?? J/N ");
#endif
				out->back2screen(workpage);
				in->alter_kb_handler();

				key = getch();
				if (key == 'j' || key == 'J' || key == 'y' || key == 'Y' || key == 'z' || key == 'Z') {
					ret = 1;
 					key = ESC;
				} else
					key = 0;
				in->neuer_kb_handler(&kbinfo);
			}
			if (mode[6])
				--mode[6];
			if (mode[6] == 1) {
				option_menue(ti);
			}
			if ((minfo.button == 1) || (key == ENTER)) {
				WAIT_TASTE_LOS
				rect = in->maus_vector(minfo.x, minfo.y, FILE_ICONS, 8);
				key = 0;
				switch (rect) {
				case 0:
					key = CURSOR_UP + ALT;
					break;
				case 1:
					key = CURSOR_DOWN + ALT;
					break;
				case 2:
					line = (minfo.y - 68) / 10;
					if (line == active_slot)
						key = ENTER;
					else
						active_slot = line;
					if (active_slot > 5)
						active_slot = 5;
					break;
				case 3:
					key = F1_KEY;
					break;
				case 4:
					key = F2_KEY;
					break;
				case 5:
					key = F3_KEY;
					break;
				case 6:
					key = F4_KEY;
					break;
				case 7:
					key = F5_KEY;
					break;
				}
				minfo.button = 0;
			}

			switch (key) {
			case F1_KEY:
				mode[2] = 1;
				mode[3] = 0;
				mode[8] = 0;

				break;

			case F2_KEY:
				mode[2] = 0;
				mode[3] = 1;
				mode[8] = 0;

				break;

			case F3_KEY:
				mode[4] = 10;
				break;

			case F4_KEY:
				mode[5] = 10;
				break;

			case F5_KEY:
				mode[6] = 10;
				break;

			case CURSOR_UP+ALT:
				WAIT_TASTE_LOS
				if (mode[2] || mode[3]) {
					mode[0] = 10;
					if (active_slot > 0)
						--active_slot;
					else if (text_off > 0)
						-- text_off;
				}
				break;

			case CURSOR_DOWN+ALT:
				if (mode[2] || mode[3]) {
					mode[1] = 10;
					if (active_slot < 5)
						++active_slot;
					else if (text_off < (20 - 6))
						++ text_off;
				}
				WAIT_TASTE_LOS
				break;

			case ENTER:
			case ENTER+ALT:
				WAIT_TASTE_LOS
				if (mode[3]) {
					tmp = fnames + ((text_off + active_slot) * 40);
					if (tmp[0]) {
						CurrentSong = -1;
						iog->load(text_off + active_slot,
						          ioptr.save_path);
						key = ESC;
					}
				} else if (mode[2]) {
					out->back2screen(workpage);
					out->setze_zeiger(screen0);
					in->alter_kb_handler();
					tmp = fnames + ((text_off + active_slot) * 40);
					key = out->scanxy(70, 68 + (active_slot * 10),
					                  255, 42, 14, 0, "%36s36", tmp);
					in->neuer_kb_handler(&kbinfo);
					out->setze_zeiger(workptr);
					if (key != 27) {
						iog->save_entry(text_off + active_slot,
						                ioptr.save_path);
					}
					key = 0;
				}
				break;

			case CURSOR_UP:
				cur->move(minfo.x, --minfo.y);
				break;

			case CURSOR_DOWN:
				cur->move(minfo.x, ++minfo.y);
				break;

			case CURSOR_LEFT:
				cur->move(--minfo.x, minfo.y);
				break;

			case CURSOR_RIGHT:
				cur->move(++minfo.x, minfo.y);
				break;

			}
			cur->plot_cur();
			out->back2screen(workpage);

			EVENTS_UPDATE;
			SHOULD_QUIT_RETURN0;
		}
	}
	free(ti);

	room->load_tgp(_G(spieler).PersonRoomNr[P_CHEWY], &room_blk, EPISODE1_TGP, GED_LOAD, EPISODE1);
	fx_blend = BLEND1;
	room->set_ak_pal(&room_blk);
	u_index = ged->ged_idx(spieler_vector[P_CHEWY].Xypos[0] + spieler_mi[P_CHEWY].HotX,
	                        spieler_vector[P_CHEWY].Xypos[1] + spieler_mi[P_CHEWY].HotY,
	                        room->GedXAnz[room_blk.AkAblage],
	                        ged_mem[room_blk.AkAblage]);
	check_shad(u_index, 1);

	return ret;
}

void option_menue(taf_info *ti) {
	long akt_clock = 0, stop_clock = 0;
	room->load_tgp(0, &room_blk, GBOOK_TGP, 0, "back/gbook.tgp");
	out->setze_zeiger(workptr);
	out->map_spr2screen(ablage[room_blk.AkAblage], 0, 0);
	out->setze_zeiger(screen0);

	room->set_ak_pal(&room_blk);
	fx->blende1(workptr, screen0, pal, 150, 0, 0);
	out->setze_zeiger(workptr);
	int16 key = 0;
	int16 surimy_ani = SURIMY_START;
	int16 mund_ani = MUND_START;
	int16 mund_delay = 3;
	int16 mund_count = mund_delay;
	int16 tdisp_ani = TDISP_START;
	int16 tdisp_delay = 3;
	int16 tdisp_count = tdisp_delay;
	FrameSpeed = 0;
	int16 delay_count = _G(spieler).DelaySpeed;
	warning("stop_clock = (clock() / CLK_TCK) + 1;");
	while (key != ESC) {
		out->map_spr2screen(ablage[room_blk.AkAblage], 0, 0);
		++FrameSpeed;
		warning("akt_clock = clock() / CLK_TCK;");
		if (akt_clock >= stop_clock) {
			//TmpFrame = FrameSpeed;
			_G(spieler).DelaySpeed = (FrameSpeed >> 1) / _G(spieler).FramesPerSecond;

			FrameSpeed = 0;
			warning("stop_clock = (clock() / CLK_TCK) + 1;");
		}

		out->sprite_set(ti->image[surimy_ani], 18 + ti->korrektur[surimy_ani << 1],
		                8 + ti->korrektur[(surimy_ani << 1) + 1], 0);
		short bar_off = (_G(spieler).FramesPerSecond - 6) * 16;
		out->box_fill(33 + bar_off, 65, 33 + 17 + bar_off, 65 + 8, 0);
		out->printxy(36 + bar_off, 65, 255, 300, 0, "%d", _G(spieler).FramesPerSecond << 1);

		if (_G(spieler).SoundSwitch) {
			out->sprite_set(ti->image[mund_ani],
				            18 + ti->korrektur[mund_ani << 1],
				            8 + ti->korrektur[(mund_ani << 1) + 1], 0);
			out->sprite_set(ti->image[SCHNULL_OFF],
				            18 + ti->korrektur[SCHNULL_OFF << 1],
				            8 + ti->korrektur[(SCHNULL_OFF << 1) + 1], 0);
		} else {
			out->sprite_set(ti->image[SCHNULLER],
				            18 + ti->korrektur[SCHNULLER << 1],
				            8 + ti->korrektur[(SCHNULLER << 1) + 1], 0);
			out->sprite_set(ti->image[SCHNULL_BAND],
				            18 + ti->korrektur[SCHNULL_BAND << 1],
				            8 + ti->korrektur[(SCHNULL_BAND << 1) + 1], 0);
		}
		out->pop_box(32 - 2, 104 - 12, 42 + 4, 136 + 2, 192, 183, 182);
		out->printxy(32 + 3, 104 - 10, 15, 300, 0, "S");
		out->box_fill(33, 136 - (_G(spieler).SoundVol >> 1), 42, 136, 15);

		out->pop_box(52 - 2, 104 - 12, 62 + 4, 136 + 2, 192, 183, 182);
		out->printxy(52 + 3, 104 - 10, 31, 300, 0, "M");
		out->box_fill(53, 136 - (_G(spieler).MusicVol >> 1), 62, 136, 31);
		if (_G(spieler).MusicSwitch) {
			out->sprite_set(ti->image[MUSIC_ON1],
				            18 + ti->korrektur[MUSIC_ON1 << 1],
				            8 + ti->korrektur[(MUSIC_ON1 << 1) + 1], 0);
			out->sprite_set(ti->image[MUSIC_ON2],
				            18 + ti->korrektur[MUSIC_ON2 << 1],
				            8 + ti->korrektur[(MUSIC_ON2 << 1) + 1], 0);
		} else
			out->sprite_set(ti->image[MUSIC_OFF],
				            18 + ti->korrektur[MUSIC_OFF << 1],
				            8 + ti->korrektur[(MUSIC_OFF << 1) + 1], 0);

		if (_G(spieler).DisplayText) {
			out->sprite_set(ti->image[tdisp_ani],
				            18 + ti->korrektur[tdisp_ani << 1],
				            8 + ti->korrektur[(tdisp_ani << 1) + 1], 0);
			out->sprite_set(ti->image[TDISP_EIN],
				            18 + ti->korrektur[TDISP_EIN << 1],
				            8 + ti->korrektur[(TDISP_EIN << 1) + 1], 0);
		} else
			out->sprite_set(ti->image[TDISP_AUS],
				            18 + ti->korrektur[TDISP_AUS << 1],
				            8 + ti->korrektur[(TDISP_AUS << 1) + 1], 0);

		out->sprite_set(ti->image[EXIT],
		                18 + ti->korrektur[EXIT << 1],
		                8 + ti->korrektur[(EXIT << 1) + 1], 0);

		key = in->get_switch_code();
		if ((minfo.button == 1) || (key == ENTER)) {
			WAIT_TASTE_LOS
			int16 rect = in->maus_vector(minfo.x, minfo.y, OPTION_ICONS, 9);
			switch (rect) {
			case 0:
				if (_G(spieler).FramesPerSecond > 6)
					--_G(spieler).FramesPerSecond;
				break;
			case 1:
				if (_G(spieler).FramesPerSecond < 10)
					++_G(spieler).FramesPerSecond;
				break;
			case 2:
				if (_G(spieler).SoundSwitch) {
					_G(spieler).SoundSwitch = false;
					det->disable_room_sound();
				} else {
					_G(spieler).SoundSwitch = true;
					det->enable_room_sound();
				}
				break;
			case 3:
			case 4:
				if (_G(spieler).DisplayText) {
					_G(spieler).DisplayText = false;
					atds->setHasSpeech(true);
					_G(spieler).SpeechSwitch = true;
				} else {
					_G(spieler).DisplayText = true;
					atds->setHasSpeech(false);
					_G(spieler).SpeechSwitch = false;
				}
				break;
			case 5:
				if (_G(spieler).MusicSwitch) {
					_G(spieler).MusicSwitch = false;
					ailsnd->stopMod();
				} else {
					_G(spieler).MusicSwitch = true;
					CurrentSong = -1;
					load_room_music(_G(spieler).PersonRoomNr[P_CHEWY]);
				}
				break;
			case 6:
				key = ESC;
				break;
			case 7:
				_G(spieler).SoundVol = (136 - minfo.y) << 1;
				ailsnd->setSoundMasterVol(_G(spieler).SoundVol);
				break;
			case 8:
				_G(spieler).MusicVol = (136 - minfo.y) << 1;
				ailsnd->setMusicMasterVol(_G(spieler).MusicVol);
				break;
			}
			minfo.button = 0;
		}
		switch (key) {

		case CURSOR_UP:
			cur->move(minfo.x, --minfo.y);
			break;

		case CURSOR_DOWN:
			cur->move(minfo.x, ++minfo.y);
			break;

		case CURSOR_LEFT:
			cur->move(--minfo.x, minfo.y);
			break;

		case CURSOR_RIGHT:
			cur->move(++minfo.x, minfo.y);
			break;
		}

		cur->plot_cur();
		out->back2screen(workpage);
		EVENTS_UPDATE;
		SHOULD_QUIT_RETURN;

		if (!delay_count) {
			if (surimy_ani < SURIMY_END)
				++surimy_ani;
			else
				surimy_ani = SURIMY_START;
			if (mund_count > 0)
				--mund_count;
			else {
				if (mund_ani < MUND_END)
					++mund_ani;
				else
					mund_ani = MUND_START;
				mund_count = mund_delay;
			}
			if (tdisp_count > 0)
				--tdisp_count;
			else {
				if (tdisp_ani < TDISP_END)
					++tdisp_ani;
				else
					tdisp_ani = TDISP_START;
				tdisp_count = tdisp_delay;
			}
			delay_count = _G(spieler).DelaySpeed;
		} else
			--delay_count;
	}

	room->load_tgp(1, &room_blk, GBOOK_TGP, 0, "back/gbook.tgp");
	out->setze_zeiger(workptr);
	out->map_spr2screen(ablage[room_blk.AkAblage], 0, 0);
	out->setze_zeiger(screen0);
	room->set_ak_pal(&room_blk);
	fx->blende1(workptr, screen0, pal, 150, 0, 0);
	out->setze_zeiger(workptr);
}

void gbook() {
	int lineScrolled = 0;
	int fontCol;

	room->open_handle("BACK/GBOOK.TGP", "rb", 0);
	room->load_tgp(5, &room_blk, 1, 0, "BACK/GBOOK.TGP");
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	out->setze_zeiger(screen0);
	room->set_ak_pal(&room_blk);
	fx->blende1(workptr, screen0, pal, 150, 0, 0);

	for (int i = 0; i < 6; ++i) {
		int color = 63 - (6 * i);
		
		out->raster_col(6 - i, color, 0, 0);
		out->raster_col(7 + i, color, 0, 0);

		color = 63 - (4 * i);
		out->raster_col(37 - i, color, color, color);
		out->raster_col(38 + i, color, color, color);		
	}

	_G(spieler).DelaySpeed = 2;

	for (;;) {
		if (in->get_switch_code() == ESC || SHOULD_QUIT)
			break;

		// Display the starfield background
		out->setze_zeiger(workptr);
		out->map_spr2screen(ablage[room_blk.AkAblage],
			_G(spieler).scrollx, _G(spieler).scrolly);

		// Animate moving the background
		if (++_G(spieler).scrollx >= 320)
			_G(spieler).scrollx = 0;

		if (in->get_switch_code() == ESC)
			break;

		++lineScrolled;
		g_events->delay(50);
		bool stillScrolling = false;
		
		for (int i = 0; i < 65; ++i) {
			int destY = CREDITS_POS[i][1] - lineScrolled;
			if (destY >= 160 || destY <= 40)
				continue;

			if (CREDITS_TYPE[i]) {
				fontCol = 32;
				out->set_fontadr(font6x8);
				out->set_vorschub(fvorx6x8, fvory6x8);
			} else {
				fontCol = 1;
				out->set_fontadr(font8x8);
				out->set_vorschub(fvorx8x8, fvory8x8);
			}
			stillScrolling = true;

			int fgCol = fontCol + (160 - destY) / 10;
			out->printxy(CREDITS_POS[i][0], destY, fgCol, 300, scr_width, CREDITS_TEXT[i]);
		}

		if (!stillScrolling)
			break;

		out->setze_zeiger(nullptr);
		out->back2screen(workpage);
	}

	out->set_fontadr(font8x8);
	out->set_vorschub(fvorx8x8, fvorx8x8);
	room->open_handle("back/episode1.tgp", "rb", 0);

	room->set_ak_pal(&room_blk);
	hide_cur();
	uhr->reset_timer(0, 5);
}

} // namespace Chewy
