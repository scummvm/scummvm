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
#include "chewy/globals.h"
#include "chewy/main.h"
#include "chewy/sound.h"

namespace Chewy {

void standard_init() {
	_G(mem) = new Memory();
	_G(out) = new McgaGraphics();
	_G(in) = new InputMgr();
	_G(fx) = new Effect();
	_G(txt) = new Text();
	_G(bit) = new BitClass();
	_G(ged) = new GedClass(&ged_user_func);
	_G(room) = new Room();
	_G(obj) = new Object(&_G(spieler));
	_G(uhr) = new Timer(MAX_TIMER_OBJ, _G(ani_timer));
	_G(det) = new Detail();
	_G(atds) = new Atdsys();
	_G(sndPlayer) = new SoundPlayer();
	_G(flc) = new Flic();
	_G(mov) = new MovClass();

	_G(out)->init();
	_G(out)->cls();
	_G(out)->savePalette();
	_G(out)->setClip(0, 0, 320, 200);
	_G(out)->setWriteMode(0);
	_G(scr_width) = 0;
	_G(screen0) = (byte *)g_screen->getPixels();
	_G(in)->neuer_kb_handler(&_G(kbinfo));

	_G(out)->initMouseMode(&_G(minfo));
	_G(curblk).page_off_x = 0;
	_G(curblk).page_off_y = 0;
	_G(curblk).xsize = 16;
	_G(curblk).ysize = 16;

	// WORKAROUND: Moved from init_load because the original
	// uses _G(curtaf)->_image below before _G(curtaf) was initialized
	_G(curtaf) = _G(mem)->taf_adr(CURSOR);

	_G(curblk).sprite = _G(curtaf)->_image;
	_G(curblk).cur_back = _G(cur_back);
	_G(curblk).no_back = true;
	_G(curani)._start = 0;
	_G(curani)._end = 0;
	_G(curani)._delay = 0;
	
	_G(cur) = new Cursor(_G(out), _G(in), &_G(curblk));
	_G(cur)->set_cur_ani(&_G(curani));

	_G(iog) = new IOGame(_G(out), _G(in), _G(cur));
	strcpy(_G(ioptr).id, "CHE");
	strcpy(_G(ioptr).save_path, SAVEDIR);
	_G(ioptr).delay = 8;
	alloc_buffers();
	_G(pal)[765] = 63;
	_G(pal)[766] = 63;
	_G(pal)[767] = 63;
	_G(out)->einblenden(_G(pal), 0);
	_G(room)->set_timer_start(1);

	_G(out)->cls();
	_G(in)->neuer_kb_handler(&_G(kbinfo));

	var_init();
	_G(ablage) = _G(room)->get_ablage();
	_G(ged_mem) = _G(room)->get_ged_mem();

	_G(zoom_horizont) = 140;
	_G(pal)[765] = 63;
	_G(pal)[766] = 63;
	_G(pal)[767] = 63;
	_G(out)->einblenden(_G(pal), 0);
	_G(out)->cls();
	_G(uhr)->setNewTimer(0, 5, SEC_10_MODE);

	_G(curblk).cur_back = _G(cur_back);
	sound_init();
	init_load();
}

void var_init() {
	_G(Rdi) = _G(det)->getRoomDetailInfo();
	_G(Sdi) = &_G(Rdi)->Sinfo[0];
	_G(Adi) = &_G(Rdi)->Ainfo[0];

	_G(auto_p_nr) = 0;
	_G(menu_item) = CUR_WALK;
	_G(inventar_nr) = 0;
	_G(ged_mov_ebene) = 1;
	new_game();
	_G(spieler).MainMenuY = MENU_Y;
	_G(spieler).DispFlag = true;
	_G(spieler).AkInvent = -1;
	_G(spieler).ScrollxStep = 1;
	_G(spieler).ScrollyStep = 1;

	_G(spieler_mi)[P_CHEWY].HotX = CH_HOT_X;
	_G(spieler_mi)[P_CHEWY].HotY = CH_HOT_Y;
	_G(spieler_mi)[P_CHEWY].HotMovX = CH_HOT_MOV_X;
	_G(spieler_mi)[P_CHEWY].HotMovY = CH_HOT_MOV_Y;
	_G(spieler_mi)[P_CHEWY].Vorschub = CH_X_PIX;
	_G(spieler_mi)[P_CHEWY].Id = CHEWY_OBJ;

	_G(spieler_mi)[P_HOWARD].HotX = HO_HOT_X;
	_G(spieler_mi)[P_HOWARD].HotY = HO_HOT_Y ;
	_G(spieler_mi)[P_HOWARD].HotMovX = HO_HOT_MOV_X;
	_G(spieler_mi)[P_HOWARD].HotMovY = HO_HOT_MOV_Y;
	_G(spieler_mi)[P_HOWARD].Vorschub = HO_X_PIX;
	_G(spieler_mi)[P_HOWARD].Id = HOWARD_OBJ;

	_G(spieler_mi)[P_NICHELLE].HotX = NI_HOT_X;
	_G(spieler_mi)[P_NICHELLE].HotY = NI_HOT_Y ;
	_G(spieler_mi)[P_NICHELLE].HotMovX = NI_HOT_MOV_X;
	_G(spieler_mi)[P_NICHELLE].HotMovY = 62;
	_G(spieler_mi)[P_NICHELLE].Vorschub = NI_X_PIX;
	_G(spieler_mi)[P_NICHELLE].Id = NICHELLE_OBJ;

	_G(spieler_mi)[P_NICHELLE].Mode = true;

	for (int16 i = 0; i < MAX_PERSON; i++) {
		_G(PersonAni)[i] = -1;
		_G(PersonTaf)[i] = 0;
		_G(spieler_mi)[i].Mode = false;

		_G(ani_stand_flag)[i] = false;
		_G(spz_delay)[i] = 0;
		_G(spieler)._personRoomNr[i] = -1;
		_G(spieler).PersonDia[i] = -1;
	}
	_G(spieler)._personRoomNr[P_CHEWY] = _G(room_start_nr);

	_G(gpkt).Vorschub = _G(spieler_mi)[P_CHEWY].Vorschub;
	init_room();
	init_atds();
	_G(spieler).FramesPerSecond = 7;
	_G(spieler).DisplayText = true;
	_G(currentSong) = -1;
	_G(SetUpScreenFunc) = nullptr;
	_G(pfeil_delay) = 0;
	_G(pfeil_ani) = 0;
	_G(timer_action_ctr) = 0;
	_G(flags).CursorStatus = true;
	_G(savegameFlag) = false;
}

void init_room() {
	_G(room_blk).AkAblage = 0;
	_G(room_blk).LowPalMem = _G(pal);
	_G(room_blk).InvFile = INVENTAR;
	_G(room_blk).DetFile = DETAILTEST;
	_G(room_blk).InvSprAdr = &_G(inv_spr)[0];
	_G(room_blk).Rmo = _G(spieler).room_m_obj;
	_G(room_blk).Rsi = _G(spieler).room_s_obj;
	_G(room_blk).AadLoad = true;
	_G(room_blk).AtsLoad = true;
	strcpy(_G(room_blk).RoomDir, "room/");

	_G(room)->open_handle(EPISODE1_GEP, R_GEP_DATA);
}

void init_atds() {
	// Close any prior handles
	_G(atds)->close_handle(AAD_DATA);
	_G(atds)->close_handle(ATS_DATA);
	_G(atds)->close_handle(ADS_DATA);
	_G(atds)->close_handle(INV_USE_DATA);
	_G(atds)->close_handle(INV_ATS_DATA);
	_G(atds)->close_handle(ATDS_HANDLE);

	// New set up
	Stream *handle = _G(atds)->pool_handle(ATDS_TXT);
	_G(atds)->set_handle(ATDS_TXT, ATS_DATA, handle, ATS_TAP_OFF, ATS_TAP_MAX);
	_G(atds)->init_ats_mode(ATS_DATA, _G(spieler).Ats);
	_G(atds)->set_handle(ATDS_TXT, INV_ATS_DATA, handle, INV_TAP_OFF, INV_TAP_MAX);
	_G(atds)->init_ats_mode(INV_ATS_DATA, _G(spieler).InvAts);
	_G(atds)->set_handle(ATDS_TXT, AAD_DATA, handle, AAD_TAP_OFF, AAD_TAP_MAX);
	_G(atds)->set_handle(ATDS_TXT, ADS_DATA, handle, ADS_TAP_OFF, ADS_TAP_MAX);
	_G(atds)->set_handle(ATDS_TXT, INV_USE_DATA, handle, USE_TAP_OFF, USE_TAP_MAX);
	_G(atds)->init_ats_mode(INV_USE_DATA, _G(spieler).InvUse);
	_G(atds)->init_ats_mode(INV_USE_DEF, _G(spieler).InvUseDef);
	_G(atds)->open_handle(INV_USE_IDX, INV_IDX_DATA);
	_G(mem)->file->fcopy(ADSH_TMP, "txt/diah.adh");
	_G(atds)->open_handle(ADSH_TMP, 3);
	_G(spieler).AadSilent = 10;
	_G(spieler).DelaySpeed = 5;
	_G(spieler_vector)[P_CHEWY].Delay = _G(spieler).DelaySpeed;
	_G(atds)->set_delay(&_G(spieler).DelaySpeed, _G(spieler).AadSilent);
	for (int16 i = 0; i < AAD_MAX_PERSON; i++)
		_G(atds)->set_split_win(i, &_G(ssi)[i]);
	_G(atds)->set_string_end_func(&atdsStringStart);
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

	_G(obj)->load(INVENTAR_IIB, &_G(spieler).room_m_obj[0]);
	_G(obj)->load(INVENTAR_SIB, &_G(spieler).room_s_obj[0]);
	_G(obj)->load(EXIT_EIB, &_G(spieler).room_e_obj[0]);

	Common::File f;

	if (!f.open(ROOM_ATS_STEUER))
		error("Error reading file: %s", ROOM_ATS_STEUER);
	for (int16 i = 0; i < ROOM_ATS_MAX; i++)
		_G(spieler).Ats[i * MAX_ATS_STATUS] = f.readByte();
	f.close();

	if (!f.open(INV_ATS_STEUER))
		error("Error reading file: %s", INV_ATS_STEUER);
	for (int16 i = 0; i < MAX_MOV_OBJ; i++)
		_G(spieler).InvAts[i * MAX_ATS_STATUS] = f.readByte();
	f.close();

	_G(obj)->sort();
	for (int16 i = 0; i < _G(obj)->spieler_invnr[0]; i++)
		_G(spieler).InventSlot[i] = _G(obj)->spieler_invnr[i + 1];

	_G(AkChewyTaf) = 0;
	load_chewy_taf(CHEWY_NORMAL);
}

void init_load() {
	_G(AkChewyTaf) = 0;
	load_chewy_taf(CHEWY_NORMAL);

	_G(spz_akt_id) = -1;
	_G(spz_tinfo) = nullptr;
	set_spz_delay(3);

	_G(menutaf) = _G(mem)->taf_adr(MENUTAF);

	Common::File f;
	if (!f.open("cut/blende.rnd"))
		error("Error reading file: cut/blende.rnd");
	_G(spblende) = (byte *)MALLOC(f.size() + sizeof(uint32));
	WRITE_LE_INT32(_G(spblende), f.size());
	f.read(_G(spblende) + sizeof(uint32), f.size());
	f.close();

	_G(room)->loadRoom(&_G(room_blk), _G(room_start_nr), &_G(spieler));
	_G(out)->setPalette(_G(pal));
}

void tidy() {
	sound_exit();
	_G(in)->alter_kb_handler();
	free_buffers();
	_G(obj)->free_inv_spr(&_G(inv_spr)[0]);

	delete _G(iog);
	delete _G(cur);
	delete _G(mov);
	delete _G(flc);
	delete _G(sndPlayer);
	delete _G(atds);
	delete _G(det);
	delete _G(uhr);
	delete _G(obj);
	delete _G(room);
	delete _G(ged);
	delete _G(bit);
	delete _G(txt);
	delete _G(fx);
	delete _G(in);
	delete _G(out);
	delete _G(mem);

	_G(iog) = nullptr;
	_G(cur) = nullptr;
	_G(mov) = nullptr;
	_G(flc) = nullptr;
	_G(sndPlayer) = nullptr;
	_G(atds) = nullptr;
	_G(det) = nullptr;
	_G(uhr) = nullptr;
	_G(obj) = nullptr;
	_G(room) = nullptr;
	_G(ged) = nullptr;
	_G(bit) = nullptr;
	_G(txt) = nullptr;
	_G(fx) = nullptr;
	_G(in) = nullptr;
	_G(out) = nullptr;
	_G(mem) = nullptr;
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
	g_engine->_sound->setMusicVolume(_G(spieler).MusicVol * Audio::Mixer::kMaxChannelVolume / 120);
	g_engine->_sound->setSoundVolume(_G(spieler).SoundVol * Audio::Mixer::kMaxChannelVolume / 120);

	_G(music_handle) = _G(room)->open_handle(DETAIL_TVP, R_VOC_DATA);

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

	_G(atds)->setHasSpeech(true);
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
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_135);
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
