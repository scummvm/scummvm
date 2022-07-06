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
#include "chewy/cursor.h"
#include "chewy/defines.h"
#include "chewy/globals.h"
#include "chewy/main.h"
#include "chewy/mcga_graphics.h"
#include "chewy/memory.h"
#include "chewy/sound.h"

namespace Chewy {

void standard_init() {
	_G(mem) = new Memory();
	_G(out) = new McgaGraphics();
	_G(in) = new InputMgr();
	_G(fx) = new Effect();
	_G(txt) = new Text();
	_G(barriers) = new Barriers();
	_G(room) = new Room();
	_G(obj) = new Object(&_G(gameState));
	_G(uhr) = new Timer(MAX_TIMER_OBJ, _G(ani_timer));
	_G(det) = new Detail();
	_G(atds) = new Atdsys();
	_G(mov) = new MovClass();

	_G(out)->init();
	_G(out)->cls();
	_G(scr_width) = 0;

	_G(cur) = new Cursor();
	_G(cur)->setAnimation(0, 0, 0);

	alloc_buffers();
	_G(pal)[765] = 63;
	_G(pal)[766] = 63;
	_G(pal)[767] = 63;
	_G(out)->fadeIn(_G(pal));
	_G(room)->set_timer_start(1);

	_G(out)->cls();

	var_init();
	_G(ablage) = _G(room)->get_ablage();

	_G(zoom_horizont) = 140;
	_G(pal)[765] = 63;
	_G(pal)[766] = 63;
	_G(pal)[767] = 63;
	_G(out)->fadeIn(_G(pal));
	_G(out)->cls();
	_G(uhr)->setNewTimer(0, 5, SEC_10_MODE);

	init_load();
}

void var_init() {
	_G(Rdi) = _G(det)->getRoomDetailInfo();
	_G(Sdi) = &_G(Rdi)->staticSprite[0];
	_G(Adi) = &_G(Rdi)->Ainfo[0];

	_G(auto_p_nr) = 0;
	_G(menu_item) = CUR_WALK;
	_G(inventoryNr) = 0;
	new_game();
	_G(gameState).MainMenuY = MENU_Y;
	_G(gameState).DispFlag = true;
	_G(cur)->setInventoryCursor(-1);
	_G(gameState).ScrollxStep = 1;
	_G(gameState).ScrollyStep = 1;

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
		_G(gameState)._personRoomNr[i] = -1;
		_G(gameState).PersonDia[i] = -1;
	}
	_G(gameState)._personRoomNr[P_CHEWY] = _G(room_start_nr);

	_G(gpkt).Vorschub = _G(spieler_mi)[P_CHEWY].Vorschub;
	init_room();
	_G(gameState).FramesPerSecond = 7;
	_G(SetUpScreenFunc) = nullptr;
	_G(pfeil_delay) = 0;
	_G(pfeil_ani) = 0;
	_G(timer_action_ctr) = 0;
	_G(savegameFlag) = false;
}

void init_room() {
	_G(room_blk).AkAblage = 0;
	_G(room_blk).LowPalMem = _G(pal);
	_G(room_blk).InvFile = INVENTORY_TAF;
	_G(room_blk).DetFile = DETAILTEST;
	_G(room_blk).InvSprAdr = &_G(inv_spr)[0];
	_G(room_blk).Rmo = _G(gameState).room_m_obj;
	_G(room_blk).Rsi = _G(gameState).room_s_obj;
	_G(room_blk).AadLoad = true;
	_G(room_blk).AtsLoad = true;
}

void new_game() {
	_G(gameState).clear();

	for (int16 i = 0; i < MAX_MOV_OBJ; i++) {
		_G(gameState).room_m_obj[i].RoomNr = -1;
		_G(gameState).InventSlot[i] = -1;
	}
	for (int16 i = 0; i < MAX_FEST_OBJ; i++)
		_G(gameState).room_s_obj[i].RoomNr = -1;
	for (int16 i = 0; i < MAX_EXIT; i++)
		_G(gameState).room_e_obj[i].RoomNr = -1;

	_G(obj)->load(INVENTORY_IIB, &_G(gameState).room_m_obj[0]);
	_G(obj)->load(INVENTORY_SIB, &_G(gameState).room_s_obj[0]);
	_G(obj)->load(EXIT_EIB, &_G(gameState).room_e_obj[0]);

	_G(obj)->sort();
	for (int16 i = 0; i < _G(obj)->spieler_invnr[0]; i++)
		_G(gameState).InventSlot[i] = _G(obj)->spieler_invnr[i + 1];

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

	_G(room)->loadRoom(&_G(room_blk), _G(room_start_nr), &_G(gameState));
	_G(out)->setPalette(_G(pal));
}

void tidy() {
	free_buffers();
	_G(obj)->free_inv_spr(&_G(inv_spr)[0]);

	delete _G(cur);
	delete _G(mov);
	delete _G(atds);
	delete _G(det);
	delete _G(uhr);
	delete _G(obj);
	delete _G(room);
	delete _G(barriers);
	delete _G(txt);
	delete _G(fx);
	delete _G(in);
	delete _G(out);
	delete _G(mem);

	_G(cur) = nullptr;
	_G(mov) = nullptr;
	_G(atds) = nullptr;
	_G(det) = nullptr;
	_G(uhr) = nullptr;
	_G(obj) = nullptr;
	_G(room) = nullptr;
	_G(barriers) = nullptr;
	_G(txt) = nullptr;
	_G(fx) = nullptr;
	_G(in) = nullptr;
	_G(out) = nullptr;
	_G(mem) = nullptr;
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
