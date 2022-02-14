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

#include "common/config-manager.h"
#include "common/memstream.h"
#include "chewy/chewy.h"
#include "chewy/defines.h"
#include "chewy/file.h"
#include "chewy/global.h"

namespace Chewy {

static void font_load();

void standard_init() {
	_G(mem) = new Memory();
	_G(out) = new McgaGraphics();
	_G(in) = new InputMgr();
	fx = new Effect();
	txt = new Text();
	bit = new BitClass();
	ged = new GedClass(&ged_user_func);
	room = new Room();
	obj = new Object(&_G(spieler));
	uhr = new Timer(MAX_TIMER_OBJ, ani_timer);
	det = new Detail();
	atds = new Atdsys();
	_G(sndPlayer) = new SoundPlayer();
	flc = new Flic();
	mov = new MovClass();

	_G(out)->init();
	_G(out)->cls();
	_G(out)->palette_save();
	_G(out)->set_clip(0, 0, 320, 200);
	_G(out)->set_writemode(0);
	_G(scr_width) = 0;
	_G(screen0) = (byte *)g_screen->getPixels();
	_G(in)->neuer_kb_handler(&kbinfo);

	_G(in)->rectangle(0, 0, 320, 210);
	_G(in)->neuer_maushandler(&minfo);
	_G(out)->init_mausmode(&minfo);
	curblk.page_off_x = 0;
	curblk.page_off_y = 0;
	curblk.xsize = 16;
	curblk.ysize = 16;

	// WORKAROUND: Moved from init_load because the original
	// uses curtaf->image below before curtaf was initialized
	curtaf = _G(mem)->taf_adr(CURSOR);

	curblk.sprite = curtaf->image;
	curblk.cur_back = _G(cur_back);
	curblk.no_back = true;
	curani.ani_anf = 0;
	curani.ani_end = 0;
	curani.delay = 0;
	
	_G(cur) = new Cursor(_G(out), _G(in), &curblk);
	_G(cur)->set_cur_ani(&curani);

	_G(iog) = new IOGame(_G(out), _G(in), _G(cur));
	strcpy(ioptr.id, "CHE");
	strcpy(ioptr.save_path, SAVEDIR);
	ioptr.delay = 8;
	alloc_buffers();
	_G(pal)[765] = 63;
	_G(pal)[766] = 63;
	_G(pal)[767] = 63;
	_G(out)->einblenden(_G(pal), 0);
	room->set_timer_start(1);
	font_load();

	_G(out)->cls();
	_G(in)->neuer_kb_handler(&kbinfo);

	var_init();
	_G(ablage) = room->get_ablage();
	_G(ged_mem) = room->get_ged_mem();

	_G(zoom_horizont) = 140;
	_G(pal)[765] = 63;
	_G(pal)[766] = 63;
	_G(pal)[767] = 63;
	_G(out)->einblenden(_G(pal), 0);
	_G(out)->cls();
	uhr->set_new_timer(0, 5, SEC_10_MODE);

	curblk.cur_back = _G(cur_back);
	sound_init();
	init_load();
}

void var_init() {
	Rdi = det->get_room_detail_info();
	Sdi = &Rdi->Sinfo[0];
	Adi = &Rdi->Ainfo[0];

	_G(auto_p_nr) = 0;
	_G(menu_item) = CUR_WALK;
	_G(inventar_nr) = 0;
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

	for (int16 i = 0; i < MAX_PERSON; i++) {
		PersonAni[i] = -1;
		PersonTaf[i] = 0;
		spieler_mi[i].Mode = false;

		ani_stand_flag[i] = false;
		spz_delay[i] = 0;
		_G(spieler).PersonRoomNr[i] = -1;
		_G(spieler).PersonDia[i] = -1;
	}
	_G(spieler).PersonRoomNr[P_CHEWY] = _G(room_start_nr);

	gpkt.Vorschub = spieler_mi[P_CHEWY].Vorschub;
	init_room();
	init_atds();
	_G(spieler).FramesPerSecond = 7;
	_G(spieler).DisplayText = true;
	_G(currentSong) = -1;
	_G(SetUpScreenFunc) = nullptr;
	_G(pfeil_delay) = 0;
	_G(pfeil_ani) = 0;
	_G(timer_action_ctr) = 0;
	flags.CursorStatus = true;
	_G(savegameFlag) = false;
}

void init_room() {
	room_blk.AkAblage = 0;
	room_blk.LowPalMem = _G(pal);
	room_blk.InvFile = INVENTAR;
	room_blk.DetFile = DETAILTEST;
	room_blk.InvSprAdr = &_G(inv_spr)[0];
	room_blk.Rmo = _G(spieler).room_m_obj;
	room_blk.Rsi = _G(spieler).room_s_obj;
	room_blk.AadLoad = true;
	room_blk.AtsLoad = true;
	strcpy(room_blk.RoomDir, "room/");

	room->open_handle(EPISODE1_GEP, "rb", R_GEPDATEI);
}

void init_atds() {
	// Close any prior handles
	atds->close_handle(AAD_DATEI);
	atds->close_handle(ATS_DATEI);
	atds->close_handle(ADS_DATEI);
	atds->close_handle(INV_USE_DATEI);
	atds->close_handle(INV_ATS_DATEI);
	atds->close_handle(ATDS_HANDLE);

	// New set up
	Stream *handle = atds->pool_handle(ATDS_TXT, "rb");
	atds->set_handle(ATDS_TXT, ATS_DATEI, handle, ATS_TAP_OFF, ATS_TAP_MAX);
	atds->init_ats_mode(ATS_DATEI, _G(spieler).Ats);
	atds->set_handle(ATDS_TXT, INV_ATS_DATEI, handle, INV_TAP_OFF, INV_TAP_MAX);
	atds->init_ats_mode(INV_ATS_DATEI, _G(spieler).InvAts);
	atds->set_handle(ATDS_TXT, AAD_DATEI, handle, AAD_TAP_OFF, AAD_TAP_MAX);
	atds->set_handle(ATDS_TXT, ADS_DATEI, handle, ADS_TAP_OFF, ADS_TAP_MAX);
	atds->set_handle(ATDS_TXT, INV_USE_DATEI, handle, USE_TAP_OFF, USE_TAP_MAX);
	atds->init_ats_mode(INV_USE_DATEI, _G(spieler).InvUse);
	atds->init_ats_mode(INV_USE_DEF, _G(spieler).InvUseDef);
	atds->open_handle(INV_USE_IDX, "rb", INV_IDX_DATEI);
	_G(mem)->file->fcopy(ADSH_TMP, "txt/diah.adh");
	atds->open_handle(ADSH_TMP, "rb", 3);
	_G(spieler).AadSilent = 10;
	_G(spieler).DelaySpeed = 5;
	spieler_vector[P_CHEWY].Delay = _G(spieler).DelaySpeed;
	atds->set_delay(&_G(spieler).DelaySpeed, _G(spieler).AadSilent);
	for (int16 i = 0; i < AAD_MAX_PERSON; i++)
		atds->set_split_win(i, &_G(ssi)[i]);
	atds->set_string_end_func(&atds_string_start);
}

void new_game() {
	_G(spieler).clear();

	for (int16 i = 0; i < MAX_MOV_OBJ; i++) {
		_G(spieler).room_m_obj[i].RoomNr = -1;
		_G(spieler).InventSlot[i] = -1;
	}
	for (int16 i = 0; i < MAX_FEST_OBJ; i++)
		_G(spieler).room_s_obj[i].RoomNr = -1;
	for (int16 i = 0; i < MAX_EXIT; i++)
		_G(spieler).room_e_obj[i].RoomNr = -1;

	obj->load(INVENTAR_IIB, &_G(spieler).room_m_obj[0]);
	obj->load(INVENTAR_SIB, &_G(spieler).room_s_obj[0]);
	obj->load(EXIT_EIB, &_G(spieler).room_e_obj[0]);

	byte *tmp = (byte *)MALLOC(ROOM_ATS_MAX);
	Common::File f;
	if (f.open(ROOM_ATS_STEUER)) {
		if (!f.read(tmp, ROOM_ATS_MAX)) {
			error("new_game error");
		}

		f.close();
	} else {
		error("new_game error");
	}

	for (int16 i = 0; i < ROOM_ATS_MAX; i++)
		_G(spieler).Ats[i * MAX_ATS_STATUS] = (uint8)tmp[i];
	free(tmp);

	tmp = (byte *)MALLOC(MAX_MOV_OBJ);

	if (f.open(INV_ATS_STEUER)) {
		if (!f.read(tmp, MAX_MOV_OBJ)) {
			error("new_game error");
		}

		f.close();
	} else {
		error("new_game error");
	}

	for (int16 i = 0; i < MAX_MOV_OBJ; i++)
		_G(spieler).InvAts[i * MAX_ATS_STATUS] = (uint8)tmp[i];
	free(tmp);

	obj->sort();
	for (int16 i = 0; i < obj->spieler_invnr[0]; i++)
		_G(spieler).InventSlot[i] = obj->spieler_invnr[i + 1];

	AkChewyTaf = 0;
	load_chewy_taf(CHEWY_NORMAL);
}

static void font_load() {

	// Load the 8x8 font
	_G(mem)->tff_adr(FONT8x8, &_G(font8x8));
	_G(out)->set_fontadr(_G(font8x8));

	int16 vorx;
	int16 vory;
	int16 fntbr;
	int16 fnth;
	_G(out)->get_fontinfo(&vorx, &vory, &fntbr, &fnth);
	_G(fvorx8x8) = vorx;
	_G(fvory8x8) = vory;

	// Load the 6x8 font
	_G(mem)->tff_adr(FONT6x8, &_G(font6x8));
	_G(out)->set_fontadr(_G(font6x8));
	_G(out)->get_fontinfo(&vorx, &vory, &fntbr, &fnth);
	_G(fvorx6x8) = vorx - 2;
	_G(fvory6x8) = vory;
	_G(out)->set_vorschub(_G(fvorx6x8), vory);
	atds->set_font(_G(font8x8), _G(fvorx8x8), 10);
}

void init_load() {
	AkChewyTaf = 0;
	load_chewy_taf(CHEWY_NORMAL);

	spz_akt_id = -1;
	spz_tinfo = nullptr;
	set_spz_delay(3);

	menutaf = _G(mem)->taf_adr(MENUTAF);
	_G(spblende) = _G(mem)->void_adr("cut/blende.rnd");
	room->load_room(&room_blk, _G(room_start_nr), &_G(spieler));
	_G(out)->set_palette(_G(pal));
}

void tidy() {
	sound_exit();
	_G(in)->alter_kb_handler();
	_G(in)->init();
	free_buffers();
	obj->free_inv_spr(&_G(inv_spr)[0]);

	delete _G(iog);
	delete _G(cur);
	delete mov;
	delete flc;
	delete _G(sndPlayer);
	delete atds;
	delete det;
	delete uhr;
	delete obj;
	delete room;
	delete ged;
	delete bit;
	delete txt;
	delete fx;
	delete _G(in);
	delete _G(out);
	delete _G(mem);

	_G(iog) = nullptr;
	_G(cur) = nullptr;
	mov = nullptr;
	flc = nullptr;
	_G(sndPlayer) = nullptr;
	atds = nullptr;
	det = nullptr;
	uhr = nullptr;
	obj = nullptr;
	room = nullptr;
	ged = nullptr;
	bit = nullptr;
	txt = nullptr;
	fx = nullptr;
	_G(in) = nullptr;
	_G(out) = nullptr;
	_G(mem) = nullptr;
}

void set_speed() {
	_G(in)->speed(_G(spieler).MausSpeed, _G(spieler).MausSpeed * 2);
}

#define GRAVIS 8
#define RAP10 9

void sound_init() {
	_G(spieler).SoundSwitch = false;
	_G(spieler).MusicSwitch = false;
	_G(frequenz) = 22050;

	_G(sndPlayer)->initMixMode();
	_G(spieler).MusicVol = 63;
	_G(spieler).SoundVol = 63;
	_G(sndPlayer)->setMusicMasterVol(_G(spieler).MusicVol);
	_G(sndPlayer)->setSoundMasterVol(_G(spieler).SoundVol);

	_G(music_handle) = room->open_handle(DETAIL_TVP, "rb", R_VOCDATEI);

	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(_G(music_handle));
	assert(rs);

	rs->seek(0);
	_G(EndOfPool) = 0;
	NewPhead Nph;
	if (!Nph.load(rs)) {
		error("sound_init error");
	} else {
		_G(EndOfPool) = Nph.PoolAnz - 1;
	}

	atds->setHasSpeech(true);
	_G(spieler).DisplayText = false;
	_G(spieler).SoundSwitch = true;
	_G(spieler).MusicSwitch = true;
	_G(spieler).SpeechSwitch = true;
}

void sound_exit() {
	_G(sndPlayer)->exitMixMode();
}

void show_intro() {
	if (!ConfMan.getBool("shown_intro")) {
		ConfMan.setBool("shown_intro", true);
		flags.NoPalAfterFlc = true;
		flic_cut(135, CFO_MODE);
	}
}

static const char *CUTSCENES = "cutscenes";
static const int MAX_CUTSCENES = 35;

void register_cutscene(int cutsceneNum) {
	assert(cutsceneNum >= 1 && cutsceneNum <= MAX_CUTSCENES);
	Common::String creditsStr;
	if (ConfMan.hasKey(CUTSCENES)) {
		creditsStr = ConfMan.get(CUTSCENES);
	} else {
		for (int i = 0; i < MAX_CUTSCENES; ++i)
			creditsStr += '0';
	}

	creditsStr.setChar('1', cutsceneNum - 1);
	ConfMan.set(CUTSCENES, creditsStr);
	ConfMan.flushToDisk();
}

void getCutscenes(Common::Array<int> &cutscenes) {
	cutscenes.clear();
	if (!ConfMan.hasKey(CUTSCENES))
		return;

	Common::String cutStr = ConfMan.get(CUTSCENES);
	for (int i = 0; i < MAX_CUTSCENES; ++i) {
		if (cutStr[i] == '1')
			cutscenes.push_back(i + 1);
	}
}

} // namespace Chewy
