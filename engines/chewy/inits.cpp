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

#include "common/config-manager.h"
#include "common/memstream.h"
#include "chewy/chewy.h"
#include "chewy/defines.h"
#include "chewy/file.h"
#include "chewy/global.h"

namespace Chewy {

extern int16 room_start_nr;
static void font_load();

void standard_init() {
	mem = new memory();
	out = new mcga_grafik();
	err = new fehler();
	in = new maus();
	fx = new effect();
	txt = new text();
	bit = new bitclass();
	ged = new gedclass(&ged_user_func);
	room = new Room();
	obj = new objekt(&_G(spieler));
	uhr = new timer(MAX_TIMER_OBJ, ani_timer);
	det = new detail();
	atds = new atdsys();
	ailsnd = new ailclass();
	flc = new flic();
	mov = new movclass();

	out->vsync_start();
	out->init();
	out->cls();
	out->palette_save();
	out->set_clip(0, 0, 320, 200);
	out->set_writemode(0);
	scr_width = 0;
	screen0 = (byte *)g_screen->getPixels();
	in->neuer_kb_handler(&kbinfo);

	in->rectangle(0, 0, 320, 210);
	in->neuer_maushandler(&minfo);
	out->init_mausmode(&minfo);
	curblk.page_off_x = 0;
	curblk.page_off_y = 0;
	curblk.xsize = 16;
	curblk.ysize = 16;

	// WORKAROUND: Moved from init_load because the original
	// uses curtaf->image below before curtaf was initialized
	curtaf = mem->taf_adr(CURSOR);

	curblk.sprite = curtaf->image;
	curblk.cur_back = cur_back;
	curblk.no_back = true;
	curani.ani_anf = 0;
	curani.ani_end = 0;
	curani.delay = 0;
	mouse_hot_x = 0;
	mouse_hot_y = 0;

	cur = new cursor(out, in, &curblk);
	cur->set_cur_ani(&curani);

	iog = new io_game(out, in, cur);
	strcpy(ioptr.id, "CHE\0");
	strcpy(ioptr.save_path, SAVEDIR);
	ioptr.delay = 8;
	alloc_buffers();
	pal [765] = 63;
	pal [766] = 63;
	pal [767] = 63;
	out->einblenden(pal, 0);
	room->set_timer_start(1);
	font_load();

	out->cls();
	in->neuer_kb_handler(&kbinfo);

	var_init();
	ablage = room->get_ablage();
	ged_mem = room->get_ged_mem();

	_G(zoom_horizont) = 140;
	pal [765] = 63;
	pal [766] = 63;
	pal [767] = 63;
	out->einblenden(pal, 0);
	out->cls();
	uhr->set_new_timer(0, 5, SEC_10_MODE);

	curblk.cur_back = cur_back;
	sound_init();
	init_load();
}

void var_init() {
	int16 i;
	Rdi = det->get_room_detail_info();
	Sdi = &Rdi->Sinfo[0];
	Adi = &Rdi->Ainfo[0];

	life_handler = false;
	_G(auto_p_nr) = 0;
	menu_item = 0;
	inventar_nr = 0;
	_G(ged_mov_ebene) = 1;
	new_game();
	_G(spieler).MainMenuY = MENU_Y;
	_G(spieler).DispFlag = true;
	_G(spieler).AkInvent = -1;
	_G(spieler).MausSpeed = 6;
	_G(spieler).ScrollxStep = 1;
	_G(spieler).ScrollyStep = 1;

	spieler_mi[P_CHEWY].HotX = CH_HOT_X;
	spieler_mi[P_CHEWY].HotY = CH_HOT_Y;
	spieler_mi[P_CHEWY].HotMovX = CH_HOT_MOV_X;
	spieler_mi[P_CHEWY].HotMovY = CH_HOT_MOV_Y;
	spieler_mi[P_CHEWY].Vorschub = CH_X_PIX;
	spieler_mi[P_CHEWY].Id = CHEWY_OBJ;

	spieler_mi[P_HOWARD].HotX = HO_HOT_X;
	spieler_mi[P_HOWARD].HotY = HO_HOT_Y ;
	spieler_mi[P_HOWARD].HotMovX = HO_HOT_MOV_X;
	spieler_mi[P_HOWARD].HotMovY = HO_HOT_MOV_Y;
	spieler_mi[P_HOWARD].Vorschub = HO_X_PIX;
	spieler_mi[P_HOWARD].Id = HOWARD_OBJ;

	spieler_mi[P_NICHELLE].HotX = NI_HOT_X;
	spieler_mi[P_NICHELLE].HotY = NI_HOT_Y ;
	spieler_mi[P_NICHELLE].HotMovX = NI_HOT_MOV_X;
	spieler_mi[P_NICHELLE].HotMovY = 62;
	spieler_mi[P_NICHELLE].Vorschub = NI_X_PIX;
	spieler_mi[P_NICHELLE].Id = NICHELLE_OBJ;

	spieler_mi[P_NICHELLE].Mode = true;

	for (i = 0; i < MAX_PERSON; i++) {
		PersonAni[i] = -1;
		PersonTaf[i] = 0;
		spieler_mi[i].Mode = 0;

		ani_stand_flag[i] = false;
		spz_delay[i] = 0;
		_G(spieler).PersonRoomNr[i] = -1;
		_G(spieler).PersonDia[i] = -1;
	}
	_G(spieler).PersonRoomNr[P_CHEWY] = room_start_nr;

	gpkt.Vorschub = spieler_mi[P_CHEWY].Vorschub;
	init_room();
	init_atds();
	_G(spieler).FramesPerSecond = 7;
	_G(spieler).DisplayText = true;
	CurrentSong = -1;
	SetUpScreenFunc = nullptr;
	_G(pfeil_delay) = 0;
	_G(pfeil_ani) = 0;
	timer_action_ctr = 0;
	flags.CursorStatus = true;
	savegameFlag = false;
}

void init_room() {
	room_blk.AkAblage = 0;
	room_blk.LowPalMem = pal;
	room_blk.InvFile = INVENTAR;
	room_blk.DetFile = DETAILTEST;
	room_blk.InvSprAdr = &inv_spr[0];
	room_blk.Rmo = _G(spieler).room_m_obj;
	room_blk.Rsi = _G(spieler).room_s_obj;
	room_blk.AadLoad = true;
	room_blk.AtsLoad = true;
	strcpy(room_blk.RoomDir, "room/");

	room->open_handle(&background[0], "rb", R_TGPDATEI);
	ERROR

	room->open_handle(&backged[0], "rb", R_GEPDATEI);
	ERROR
}

void init_atds() {
	int16 i;

	Stream *handle = atds->pool_handle(ATDS_TXT, "rb");
	ERROR
	atds->set_handle(ATDS_TXT, ATS_DATEI, handle, ATS_TAP_OFF, ATS_TAP_MAX);
	ERROR
	atds->init_ats_mode(ATS_DATEI, _G(spieler).Ats);

	atds->set_handle(ATDS_TXT, INV_ATS_DATEI, handle, INV_TAP_OFF, INV_TAP_MAX);
	ERROR
	atds->init_ats_mode(INV_ATS_DATEI, _G(spieler).InvAts);

	atds->set_handle(ATDS_TXT, AAD_DATEI, handle, AAD_TAP_OFF, AAD_TAP_MAX);
	ERROR

	atds->set_handle(ATDS_TXT, ADS_DATEI, handle, ADS_TAP_OFF, ADS_TAP_MAX);
	ERROR

	atds->set_handle(ATDS_TXT, INV_USE_DATEI, handle, USE_TAP_OFF, USE_TAP_MAX);
	ERROR
	atds->init_ats_mode(INV_USE_DATEI, _G(spieler).InvUse);
	atds->init_ats_mode(INV_USE_DEF, _G(spieler).InvUseDef);

	atds->open_handle(INV_USE_IDX, "rb", INV_IDX_DATEI);
	ERROR
	_G(spieler).AadSilent = 10;
	_G(spieler).DelaySpeed = 5;
	spieler_vector[P_CHEWY].Delay = _G(spieler).DelaySpeed;
	atds->set_delay(&_G(spieler).DelaySpeed, _G(spieler).AadSilent);
	for (i = 0; i < AAD_MAX_PERSON; i++)
		atds->set_split_win(i, &ssi[i]);
	atds->set_string_end_func(&atds_string_start);
	ERROR
}

void new_game() {
	int16 i;
	byte *tmp;

	_G(spieler).clear();

	for (i = 0; i < MAX_MOV_OBJ; i++) {
		_G(spieler).room_m_obj[i].RoomNr = -1;
		_G(spieler).InventSlot[i] = -1;
	}
	for (i = 0; i < MAX_FEST_OBJ; i++)
		_G(spieler).room_s_obj[i].RoomNr = -1;
	for (i = 0; i < MAX_EXIT; i++)
		_G(spieler).room_e_obj[i].RoomNr = -1;

	obj->load(INVENTAR_IIB, &_G(spieler).room_m_obj[0]);
	ERROR
	obj->load(INVENTAR_SIB, &_G(spieler).room_s_obj[0]);
	ERROR
	obj->load(EXIT_EIB, &_G(spieler).room_e_obj[0]);
	ERROR

	tmp = (byte *)MALLOC(ROOM_ATS_MAX);
	ERROR

	Common::File f;
	if (f.open(ROOM_ATS_STEUER)) {
		if (!f.read(tmp, ROOM_ATS_MAX)) {
			modul = DATEI;
			fcode = READFEHLER;
		}

		f.close();
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
	ERROR

	for (i = 0; i < ROOM_ATS_MAX; i++)
		_G(spieler).Ats[i * MAX_ATS_STATUS] = (uint8)tmp[i];
	free(tmp);

	tmp = (byte *)MALLOC(MAX_MOV_OBJ);
	ERROR

	if (f.open(INV_ATS_STEUER)) {
		if (!f.read(tmp, MAX_MOV_OBJ)) {
			modul = DATEI;
			fcode = READFEHLER;
		}

		f.close();
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
	ERROR

	for (i = 0; i < MAX_MOV_OBJ; i++)
		_G(spieler).InvAts[i * MAX_ATS_STATUS] = (uint8)tmp[i];
	free(tmp);

	obj->sort();
	for (i = 0; i < obj->spieler_invnr[0]; i++)
		_G(spieler).InventSlot[i] = obj->spieler_invnr[i + 1];

	AkChewyTaf = 0;
	load_chewy_taf(1);
}

static void font_load() {
	int16 vorx;
	int16 vory;
	int16 fntbr;
	int16 fnth;

	// Load the 8x8 font
	mem->tff_adr(FONT8x8, &font8x8);
	ERROR
	out->set_fontadr(font8x8);

	out->get_fontinfo(&vorx, &vory, &fntbr, &fnth);
	fvorx8x8 = vorx;
	fvory8x8 = vory;

	// Load the 6x8 font
	mem->tff_adr(FONT6x8, &font6x8);
	ERROR
	out->set_fontadr(font6x8);
	out->get_fontinfo(&vorx, &vory, &fntbr, &fnth);
	fvorx6x8 = vorx - 2;
	fvory6x8 = vory;
	out->set_vorschub(fvorx6x8, vory);
	atds->set_font(font8x8, fvorx8x8, 10);
}

void init_load() {
	taf_dateiheader *tafheader;
	AkChewyTaf = 0;
	load_chewy_taf(CHEWY_NORMAL);

	spz_taf_handle = room->open_handle(CH_SPZ_FILE, "rb", R_SPEZTAF);
	err->set_user_msg("Chewy Spezial Taf");
	ERROR

	spz_akt_id = -1;
	spz_tinfo = 0;
	set_spz_delay(3);

	menutaf = mem->taf_adr(MENUTAF);
	ERROR
	mem->file->get_tafinfo(MENUTAF, &tafheader);
	ERROR

	spblende = mem->void_adr("cut/blende.rnd");
	ERROR

	room->load_room(&room_blk, room_start_nr, &_G(spieler));
	ERROR
	out->set_palette(pal);
}

void get_detect(char *fname_) {
	Stream *handle;
	modul = 0;
	fcode = 0;
	handle = chewy_fopen(fname_, "rb");
	if (handle) {
		if (!chewy_fread(&detect, sizeof(DetectInfo), 1, handle)) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		chewy_fclose(handle);
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
}

void error() {
	in->alter_kb_handler();
	out->rest_palette();
	out->restore_mode();

	Common::String msg = err->get_user_msg();
	err->msg();
	tidy();

	::error("%s", msg.c_str());
}

void tidy() {
	sound_exit();
	in->alter_kb_handler();
	in->init();
	free_buffers();
	obj->free_inv_spr(&inv_spr[0]);

	delete iog;
	delete cur;
	delete mov;
	delete flc;
	delete ailsnd;
	delete atds;
	delete det;
	delete uhr;
	delete obj;
	delete room;
	delete ged;
	delete bit;
	delete txt;
	delete fx;
	delete in;
	delete err;
	delete out;
	delete mem;

	iog = nullptr;
	cur = nullptr;
	mov = nullptr;
	flc = nullptr;
	ailsnd = nullptr;
	atds = nullptr;
	det = nullptr;
	uhr = nullptr;
	obj = nullptr;
	room = nullptr;
	ged = nullptr;
	bit = nullptr;
	txt = nullptr;
	fx = nullptr;
	in = nullptr;
	err = nullptr;
	out = nullptr;
	mem = nullptr;
}

void set_speed() {
	in->speed(_G(spieler).MausSpeed, _G(spieler).MausSpeed * 2);
}

#define GRAVIS 8
#define RAP10 9

void sound_init() {
	flags.InitSound = false;
	_G(spieler).SoundSwitch = false;
	_G(spieler).MusicSwitch = false;
	frequenz = 22050;

	detect.SoundSource = ailsnd->init(frequenz);

	if (detect.SoundSource) {
		ailsnd->init_mix_mode();
		_G(spieler).MusicVol = 63;
		_G(spieler).SoundVol = 63;
		ailsnd->set_music_mastervol(_G(spieler).MusicVol);
		ailsnd->set_sound_mastervol(_G(spieler).SoundVol);
		ailsnd->switch_music(1);
		ailsnd->switch_sound(1);
		flags.InitSound = true;

		room->open_handle(DETAIL_TVP, "rb", R_VOCDATEI);
		ERROR
		det->set_sound_area(Ci.SoundSlot, SOUND_SLOT_SIZE);

		music_handle = room->get_sound_handle();
		Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(music_handle);
		assert(rs);

		rs->seek(0);
		EndOfPool = 0;
		NewPhead Nph;
		if (!Nph.load(rs)) {
			modul = DATEI;
			fcode = READFEHLER;
		} else {
			EndOfPool = Nph.PoolAnz - 1;
		}
		ERROR

		speech_handle = chewy_fopen(SPEECH_TVP, "rb");
		if (!speech_handle) {
			modul = DATEI;
			fcode = OPENFEHLER;
			err->set_user_msg("speech.tvp");
		} else {
			ailsnd->init_double_buffer(SpeechBuf[0], SpeechBuf[1], SPEECH_HALF_BUF, 0);
			atds->set_speech_handle(speech_handle);

			atds->setHasSpeech(true);
			_G(spieler).DisplayText = false;
		}

		if (!modul) {
			_G(spieler).SoundSwitch = true;
			_G(spieler).MusicSwitch = true;
			_G(spieler).SpeechSwitch = true;
		}
	}
}

void sound_exit() {
	if (detect.SoundSource && flags.InitSound) {
		ailsnd->exit_mix_mode();
		ailsnd->exit1();
		if (speech_handle)
			chewy_fclose(speech_handle);
	}
}

#define CSP_INT "csp.int"

void show_intro() {
	if (!ConfMan.getBool("shown_intro")) {
		ConfMan.setBool("shown_intro", true);
		flags.NoPalAfterFlc = true;
		flic_cut(135, 0);
	}
}

void test_intro(int testVal) {
	// No implementation
}


} // namespace Chewy
