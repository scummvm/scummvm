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

#include "chewy/defines.h"
#include "chewy/file.h"
#include "chewy/global.h"

namespace Chewy {

extern int16 room_start_nr;

void standard_init() {
	mem = new memory;
	out = new mcga_grafik;
	err = new fehler;
	in = new maus;
	fx = new effect;
	txt = new text;
	bit = new bitclass;
	ged = new gedclass(&ged_user_func);
	room = new Room;
	obj = new objekt(&spieler);
	uhr = new timer(MAX_TIMER_OBJ, ani_timer);
	det = new detail;
	atds = new atdsys;
	ailsnd = new ailclass;
	flc = new flic;
	mov = new movclass;

	init_load();

	out->vsync_start();
	out->init();
	out->cls();
	out->palette_save(0);
	out->set_clip(0, 0, 320, 200);
	out->set_writemode(0);
	scr_width = 0;
	screen0 = (byte *) 0xa0000;
	in->neuer_kb_handler(&kbinfo);

	in->rectangle(0, 0, 320, 210);
	in->neuer_maushandler(&minfo);
	out->init_mausmode(&minfo);
	curblk.page_off_x = 0;
	curblk.page_off_y = 0;
	curblk.xsize = 16;
	curblk.ysize = 16;

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
	ioptr.save_funktion = &save;
	ioptr.load_funktion = &load;
	ioptr.delay = 8;
	alloc_buffers();
	pal [765] = 63;
	pal [766] = 63;
	pal [767] = 63;
	out->einblenden(pal, 0);
	room->set_timer_start(1);

	out->cls();
	in->neuer_kb_handler(&kbinfo);

	var_init();
	ablage = room->get_ablage();
	ged_mem = room->get_ged_mem();

	zoom_horizont = 140;
	pal [765] = 63;
	pal [766] = 63;
	pal [767] = 63;
	out->einblenden(pal, 0);
	out->cls();
	uhr->set_new_timer(0, 5, SEC_10_MODE);

	curblk.cur_back = cur_back;
	sound_init();
}

void var_init() {
	int16 i;
	Rdi = det->get_room_detail_info();
	Sdi = &Rdi->Sinfo[0];
	Adi = &Rdi->Ainfo[0];

	life_handler = false;
	auto_p_nr = 0;
	menu_item = 0;
	inventar_nr = 0;
	ged_mov_ebene = 1;
	new_game();
	spieler.MainMenuY = MENU_Y;
	spieler.DispFlag = true;
	spieler.AkInvent = -1;
	spieler.MausSpeed = 6;
	spieler.ScrollxStep = 1;
	spieler.ScrollyStep = 1;

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
		spieler.PersonRoomNr[i] = -1;
		spieler.PersonDia[i] = -1;
	}
	spieler.PersonRoomNr[P_CHEWY] = room_start_nr;

	gpkt.Vorschub = spieler_mi[P_CHEWY].Vorschub;
	init_room();
	init_atds();
	spieler.FramesPerSecond = 7;
	spieler.DisplayText = true;
	CurrentSong = -1;
	SetUpScreenFunc = nullptr;
	pfeil_delay = 0;
	pfeil_ani = 0;
	flags.CursorStatus = true;
}

void init_room() {

	room_blk.AkAblage = 0;
	room_blk.LowPalMem = pal;
	room_blk.InvFile = INVENTAR;
	room_blk.DetFile = DETAILTEST;
	room_blk.InvSprAdr = &inv_spr[0];
	room_blk.Rmo = spieler.room_m_obj;
	room_blk.Rsi = spieler.room_s_obj;
	room_blk.AadLoad = true;
	room_blk.AtsLoad = true;
	strcpy(room_blk.RoomDir, "room/\0");

	room->open_handle(&background[0], "rb", R_TGPDATEI);
	ERROR

	room->open_handle(&backged[0], "rb", R_GEPDATEI);
	ERROR
}

void init_atds() {
	char tmp_path[MAXPFAD];
	int16 i;

	Stream *handle = atds->pool_handle(ATDS_TXT, "rb");
	ERROR
	atds->set_handle(ATDS_TXT, ATS_DATEI, handle, ATS_TAP_OFF, ATS_TAP_MAX);
	ERROR
	atds->init_ats_mode(ATS_DATEI, spieler.Ats);

	atds->set_handle(ATDS_TXT, INV_ATS_DATEI, handle, INV_TAP_OFF, INV_TAP_MAX);
	ERROR
	atds->init_ats_mode(INV_ATS_DATEI, spieler.InvAts);

	atds->set_handle(ATDS_TXT, AAD_DATEI, handle, AAD_TAP_OFF, AAD_TAP_MAX);
	ERROR

	atds->set_handle(ATDS_TXT, ADS_DATEI, handle, ADS_TAP_OFF, ADS_TAP_MAX);
	ERROR

	strcpy(tmp_path + 2, ADSH_TMP);

	mem->file->fcopy(tmp_path, ADS_TXT_STEUER);
	atds->open_handle(tmp_path, "rb+", ADH_DATEI);
	ERROR

	atds->set_handle(ATDS_TXT, INV_USE_DATEI, handle, USE_TAP_OFF, USE_TAP_MAX);
	ERROR
	atds->init_ats_mode(INV_USE_DATEI, spieler.InvUse);
	atds->init_ats_mode(INV_USE_DEF, spieler.InvUseDef);

	atds->open_handle(INV_USE_IDX, "rb", INV_IDX_DATEI);
	ERROR
	spieler.AadSilent = 10;
	spieler.DelaySpeed = 5;
	spieler_vector[P_CHEWY].Delay = spieler.DelaySpeed;
	atds->set_delay(&spieler.DelaySpeed, spieler.AadSilent);
	for (i = 0; i < AAD_MAX_PERSON; i++)
		atds->set_split_win(i, &ssi[i]);
	atds->set_string_end_func(&atds_string_start);
	ERROR
}

void new_game() {
	int16 i;
	char *tmp;

	memset(&spieler, 0, sizeof(Spieler));

	for (i = 0; i < MAX_MOV_OBJ; i++) {
		spieler.room_m_obj[i].RoomNr = -1;
		spieler.InventSlot[i] = -1;
	}
	for (i = 0; i < MAX_FEST_OBJ; i++)
		spieler.room_s_obj[i].RoomNr = -1;
	for (i = 0; i < MAX_EXIT; i++)
		spieler.room_e_obj[i].RoomNr = -1;

	obj->load(INVENTAR_IIB, &spieler.room_m_obj[0]);
	ERROR
	obj->load(INVENTAR_SIB, &spieler.room_s_obj[0]);
	ERROR
	obj->load(EXIT_EIB, &spieler.room_e_obj[0]);
	ERROR

	tmp = (char *)calloc(ROOM_ATS_MAX, 1);
	ERROR

	Stream *handle = chewy_fopen(ROOM_ATS_STEUER, "rb");
	if (handle) {
		if (!chewy_fread(tmp, ROOM_ATS_MAX, 1, handle)) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		chewy_fclose(handle);
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
	ERROR
	for (i = 0; i < ROOM_ATS_MAX; i++)
		spieler.Ats[i * MAX_ATS_STATUS] = (uint8)tmp[i];
	free(tmp);

	tmp = (char *)calloc(MAX_MOV_OBJ, 1);
	ERROR
	handle = chewy_fopen(INV_ATS_STEUER, "rb");
	if (handle) {
		if (!chewy_fread(tmp, MAX_MOV_OBJ, 1, handle)) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		chewy_fclose(handle);
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
	ERROR
	for (i = 0; i < MAX_MOV_OBJ; i++)
		spieler.InvAts[i * MAX_ATS_STATUS] = (uint8)tmp[i];
	free(tmp);

	obj->sort();
	for (i = 0; i < obj->spieler_invnr[0]; i++)
		spieler.InventSlot[i] = obj->spieler_invnr[i + 1];
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

	curtaf = mem->taf_adr(CURSOR);
	ERROR

	menutaf = mem->taf_adr(MENUTAF);
	ERROR
	mem->file->get_tafinfo(MENUTAF, &tafheader);
	ERROR

	spblende = mem->void_adr("cut/blende.rnd");
	ERROR

	room->load_room(&room_blk, room_start_nr, &spieler);
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
	in->speed(spieler.MausSpeed, spieler.MausSpeed * 2);
}

void init_life_handler() {
	life_anz = false;
	life_flag = false;
	life_handler = true;
	warning("STUB - init_life_handler");
#if 0
	_disable();
	old1chandler = _dos_getvect(0x1c);
	_dos_setvect(0x1c, life_line);
	_enable();
#endif
}

void remove_life_handler() {
	warning("STUB - remove_life_handler");
	if (life_handler) {
		life_handler = false;
#if 0
		_disable();
		_dos_setvect(0x1c, old1chandler);
		_enable();
#endif
	}
}

void life_line() {
	warning("STUB - life_line");
}
#if 0
#pragma aux asm_set_es_ds =\
"push ax"\
"push es"\
"mov ax,ds"\
"mov es,ax"
#pragma aux asm_pop_es_ax =\
"pop es"\
"pop ax"
void interrupt __far life_line(void) {
	int16 i, x;
	asm_set_es_ds();
	old1chandler();
	if (!life_flag) {
		life_flag = TRUE;
		if (life_y > 190) {
			life_y = 0;
			out->cls();
		}
		x = strlen(life_str);
		out->printxy(life_x, life_y, 255, 0, scr_width, life_str);
		for (i = 0; i < life_anz; i++)
			out->printchar('.', 255, 0, scr_width);
		++life_anz;
		if (life_anz > 50 - x) {
			life_anz = 0;

			out->box_fill(x * fvorx6x8, life_y, 319, life_y + 8, 0);

		}
		life_flag = FALSE;
	}
	asm_pop_es_ax();
}
#endif

#define GRAVIS 8
#define RAP10 9
void sound_init() {
	flags.InitSound = false;
	spieler.SoundSwitch = false;
	spieler.MusicSwitch = false;
	frequenz = 22050;

	ERROR
	warning("STUB - Installing sound files");
#if 0
	system("cd sound");
	Stream *test_handle = chewy_fopen("SET.INT", "rb");
	if (!test_handle) {
		test_handle = chewy_fopen("SET.INT", "wb");
		if (test_handle) {
			chewy_fputc('N', test_handle);
			chewy_fputc('G', test_handle);
			chewy_fputc('S', test_handle);
			chewy_fclose(test_handle);
		} else {
			modul = DATEI;
			fcode = OPENFEHLER;
			err->set_user_msg("Set.Int schreiben.");
		}
		switch_2_cd();
		ERROR
		out->printxy(0, 0, 255, 0, scr_width, "Copying Sound-Files...");
		char tmp_path[MAXPFAD] = {0};
		strcpy(tmp_path, "copy sound\\*.dig \0");
		strcat(tmp_path, HD_Lw);
		system(tmp_path);
		strcpy(tmp_path, "copy sound\\aildrvr.lst \0");
		strcat(tmp_path, HD_Lw);
		system(tmp_path);
		strcpy(tmp_path, "copy sound\\setsound.exe \0");
		strcat(tmp_path, HD_Lw);
		system(tmp_path);
		strcpy(tmp_path, "copy sound\\dos4gw.exe \0");
		strcat(tmp_path, HD_Lw);
		system(tmp_path);
		switch_2_hd();
		in->init();
		in->alter_kb_handler();
		out->restore_mode();
		system("setsound.exe");
		out->vsync_start();
		out->init();
		out->cls();
		out->palette_save(0);
		out->set_clip(0, 0, 320, 200);
		out->set_writemode(0);
		detect.Nager = in->init();
		in->neuer_kb_handler(&kbinfo);
		if (detect.Nager) {
			in->rectangle(0, 0, 320, 210);
			in->neuer_maushandler(&minfo);
			out->init_mausmode(&minfo);
			curblk.page_off_x = 0;
			curblk.page_off_y = 0;
			curblk.xsize = 16;
			curblk.ysize = 16;

			curblk.sprite = curtaf->image;
			curblk.cur_back = cur_back;
			curblk.no_back = TRUE;
			curani.ani_anf = 0;
			curani.ani_end = 0;
			curani.delay = 0;
			mouse_hot_x = 0;
			mouse_hot_y = 0;
		}
	} else
		chewy_fclose(test_handle);
	detect.SoundSource = ailsnd->init(frequenz);
	ERROR
	system("cd ..");

	switch_2_cd();
	ERROR
	if (detect.SoundSource) {
		ailsnd->init_mix_mode();
		spieler.MusicVol = 63;
		spieler.SoundVol = 63;
		ailsnd->set_music_mastervol(spieler.MusicVol);
		ailsnd->set_sound_mastervol(spieler.SoundVol);
		ailsnd->switch_music(1);
		ailsnd->switch_sound(1);
		flags.InitSound = TRUE;
		room->open_handle(DETAIL_TVP, "rb", R_VOCDATEI);
		ERROR
		det->set_sound_area(Ci.SoundSlot, SOUND_SLOT_SIZE);

		music_handle = room->get_sound_handle();
		chewy_fseek(music_handle, 0, SEEK_SET);
		EndOfPool = 0;
		NewPhead Nph;
		if (!chewy_fread(&Nph, sizeof(NewPhead), 1, music_handle)) {
			modul = DATEI;
			fcode = READFEHLER;
		} else
			EndOfPool = Nph.PoolAnz - 1;
		ERROR
		speech_handle = chewy_fopen(SPEECH_TVP, "rb");
		if (!speech_handle) {
			modul = DATEI;
			fcode = OPENFEHLER;
			err->set_user_msg("SPEECH.TVP\0");
		} else {
			ailsnd->init_double_buffer(SpeechBuf[0], SpeechBuf[1], SPEECH_HALF_BUF, 0);
			atds->set_speech_handle(speech_handle);

		}
#endif
		if (!modul) {
			spieler.SoundSwitch = true;
			spieler.MusicSwitch = true;
			spieler.SpeechSwitch = true;
		}
#if 0
	}
#endif
}

void sound_exit() {
	warning("STUB - sound_exit");
#if 0
	if (detect.SoundSource && flags.InitSound) {
		ailsnd->exit_mix_mode();
		ailsnd->exit1();
		if (speech_handle)
			chewy_fclose(speech_handle);
	}
#endif
}

} // namespace Chewy
