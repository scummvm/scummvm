/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "sky/sound.h"
#include "sky/struc.h"
#include "sky/logic.h"

#define SOUND_FILE_BASE 60203
#define MAX_FX_NUMBER 393
#define SFXF_START_DELAY 0x80
#define SFXF_SAVE 0x20

#if !defined(__GNUC__)
#pragma START_PACK_STRUCTS
#endif

struct RoomList {
	uint8 room;
	uint8 adlibVolume;
	uint8 rolandVolume;
} GCC_PACK;

struct Sfx {
	uint8 soundNo;
	uint8 flags;
	RoomList roomList[10];
} GCC_PACK;

#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif

uint16 SkySound::_speechConvertTable[8] = {
	0,									//;Text numbers to file numbers
	600,								//; 553 lines in section 0
	600+500,							//; 488 lines in section 1
	600+500+1330,						//;1303 lines in section 2
	600+500+1330+950,					//; 922 lines in section 3
	600+500+1330+950+1150,				//;1140 lines in section 4
	600+500+1330+950+1150+550,			//; 531 lines in section 5
	600+500+1330+950+1150+550+150,		//; 150 lines in section 6
};


static Sfx fx_null = {
	0,
	0,
	{
		{ 200,127,127 },
		{ 255,0,0 }
	}
};

static Sfx fx_level_3_ping = {
	1,
	0,
	{
		{ 28,63,63 },
		{ 29,63,63 },
		{ 31,63,63 },
		{ 255,0,0 },
	}
};

static Sfx fx_factory_sound = {
	1,
	SFXF_SAVE,
	{
		{ 255,30,30 },
	}
};

static Sfx fx_crowbar_plaster = {
	1,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_masonry_fall = {
	1,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_prise_brick = {
	2,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_rope_creak = {
	2,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_ping = {
	3,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_force_fire_door = {
	3,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_brick_hit_foster = {
	3,
	10+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_brick_hit_plank = {
	3,
	8+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_rm3_lift_moving = {
	4,
	SFXF_SAVE,
	{
		{ 3,127,127 },
		{ 2,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_weld = {
	4,
	0,
	{
		{ 15,127,127 },
		{ 7,127,127 },
		{ 6,60,60 },
		{ 12,60,60 },
		{ 13,60,60 },
		{ 255,0,0 },
	}
};

static Sfx fx_weld12 = {
	4,
	0,
	{
		{ 12,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_spray_on_skin = {
	4,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_plank_vibrating = {
	4,
	6+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_press_bang = {
	5,
	0,
	{
		{ 0,50,100 },
		{ 255,0,0 },
	}
};

static Sfx fx_spanner_clunk = {
	5,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_break_crystals = {
	5,
	0,
	{
		{ 96,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_press_hiss = {
	6,
	0,
	{
		{ 0,40,40 },
		{ 255,0,0 },
	}
};

static Sfx fx_open_door = {
	6,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_open_lamb_door = {
	6,
	0,
	{
		{ 20,127,127 },
		{ 21,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_splash = {
	6,
	22+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_disintegrate = {
	7,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_buzzer = {
	7,
	4+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_lathe = {
	7,
	SFXF_SAVE,
	{
		{ 4,60,60 },
		{ 2,20,20 },
		{ 255,0,0 },
	}
};

static Sfx fx_hit_crowbar_brick = {
	7,
	9+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_hello_helga = {
	8,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_statue_on_armour = {
	8,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_lift_alarm = {
	8,
	SFXF_SAVE,
	{
		{ 2,63,63 },
		{ 255,0,0 },
	}
};

static Sfx fx_drop_crowbar = {
	8,
	5+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_byee_helga = {
	9,
	3+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_shed_door_creak = {
	10,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_explosion = {
	10,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_fire_crackle_in_pit = {
	 9,
	SFXF_SAVE,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_remove_bar_grill = {
	10,
	7+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_grill_creak = {
	10,
	43+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_steam1 = {
	11,
	SFXF_SAVE,
	{
		{ 18,20,20 },
		{ 255,0,0 },
	}
};

static Sfx fx_steam2 = {
	11,
	SFXF_SAVE,
	{
		{ 18,63,63 },
		{ 255,0,0 },
	}
};

static Sfx fx_steam3 = {
	11,
	SFXF_SAVE,
	{
		{ 18,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_crowbar_wooden = {
	11,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_helmet_down_3 = {
	11,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_guard_fall = {
	11,
	4,
	{
		{ 255,127,127 },
	}
};

#if 0
static Sfx fx_furnace = {
	11,
	0,
	{
		{ 3,90,90 },
		{ 255,0,0 },
	}
};
#endif

static Sfx fx_fall_thru_box = {
	12,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_lazer = {
	12,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_scanner = {
	12,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_helmet_up_3 = {
	12,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_liquid_bubble = {
	12,
	SFXF_SAVE,
	{
		{ 80,127,127 },
		{ 72,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_liquid_drip = {
	13,
	6+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_goo_drip = {
	13,
	5+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_comp_bleeps = {
	13,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_use_crowbar_grill = {
	13,
	34+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_helmet_grind = {
	14,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_lift_moving = {
	14,
	SFXF_SAVE,
	{
		{ 7,127,127 },
		{ 29,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_use_secateurs = {
	14,
	18+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_hit_joey1 = {
	14,
	7+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_hit_joey2 = {
	14,
	13+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_dani_phone_ring = {
	15,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_sc74_pod_down = {
	15,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_phone = {
	15,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_25_weld = {
	15,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_lift_open_7 = {
	15,
	0,
	{
		{ 7,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_lift_close_7 = {
	16,
	0,
	{
		{ 7,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_s2_helmet = {
	16,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_hiss_in_nitrogen = {
	16,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_dog_yap_indoors = {
	16,
	0,
	{
		{ 38,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_dog_yap_outdoors = {
	16,
	0,
	{
		{ 31,127,127 },
		{ 30,40,40 },
		{ 32,40,40 },
		{ 33,40,40 },
		{ 255,0,0 },
	}
};

static Sfx fx_locker_creak_open = {
	17,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_big_tent_gurgle = {
	17,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_wind_howl = {
	17,
	SFXF_SAVE,
	{
		{ 1,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_lift_open_29 = {
	17,
	0,
	{
		{ 29,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_lift_arrive_7 = {
	17,
	0,
	{
		{ 7,63,63 },
		{ 255,0,0 },
	}
};

static Sfx fx_lift_close_29 = {
	18,
	0,
	{
		{ 29,127,127 },
		{ 28,127,127 },
		{ 255,0,0 },
	}
};

static Sfx fx_shaft_industrial_noise = {
	18,
	SFXF_SAVE,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_gall_drop = {
	18,
	29+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_door_slam_under = {
	19,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_reichs_fish = {
	19,
	SFXF_SAVE,
	{
		{ 255,60,60 },
	}
};

static Sfx fx_judges_gavel1 = {
	19,
	13+SFXF_START_DELAY,
	{
		{ 255,60,60 },
	}
};

static Sfx fx_judges_gavel2 = {
	19,
	16+SFXF_START_DELAY,
	{
		{ 255,90,90 },
	}
};

static Sfx fx_judges_gavel3 = {
	19,
	19+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_wind_3 = {
	20,
	SFXF_SAVE,
	{
		{ 255,60,60 },
	}
};

static Sfx fx_fact_sensor = {
	20,
	SFXF_SAVE,
	{
		{ 255,60,60 },
	}
};

static Sfx fx_medi_stab_gall = {
	20,
	17+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_computer_3 = {
	21,
	SFXF_SAVE,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_timber_cracking = {
	21,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_anchor_fall = {
	22,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_elevator_4 = {
	22,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_star_trek_2 = {
	22,
	SFXF_SAVE,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_lift_closing = {
	23,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_heartbeat = {
	23,
	11+SFXF_START_DELAY,
	{
		{ 67,60,60 },
		{ 68,60,60 },
		{ 69,60,60 },
		{ 77,20,20 },
		{ 78,50,50 },
		{ 79,70,70 },
		{ 80,127,127 },
		{ 81,60,60 },
		{ 255,0,0 },
	}
};

static Sfx fx_pos_key = {
	25,
	2+SFXF_START_DELAY,
	{
		{ 255,127,127 },
	}
};

static Sfx fx_neg_key = {
	26,
	2+SFXF_START_DELAY,
	{
		{ 255,100,100 },
	}
};

static Sfx fx_orifice_swallow_drip = {
	28,
	0,
	{
		{ 255,127,127 },
	}
};

static Sfx *musicList[] = {
	&fx_press_bang, // 256 banging of the press
	&fx_press_hiss, // 257 hissing press
	&fx_wind_howl, // 258 howling wind
	&fx_spanner_clunk, // 259 spanner in works
	&fx_reichs_fish, // 260 Reichs fish
	&fx_explosion, // 261 panel blows open
	&fx_wind_3, // 262 single steam
	&fx_open_door, // 263 general open door
	&fx_open_lamb_door, // 264 lamb door opens
	&fx_comp_bleeps, // 265 scanner bleeps
	&fx_helmet_down_3, // 266
	&fx_helmet_up_3, // 267
	&fx_helmet_grind, // 268
	&fx_lift_close_29, // 269 rm 29 lift closes
	&fx_lift_open_29, // 270 rm 29 lift opens
	&fx_computer_3, // 271 rm 29 lift arrives
	&fx_level_3_ping, // 272 background noise in room 4
	&fx_lift_alarm, // 273 loader alarm
	&fx_null, // 274 furnace room background noise
	&fx_rm3_lift_moving, // 275 lift moving in room 3
	&fx_lathe, // 276 jobsworth lathe
	&fx_factory_sound, // 277 factory background sound
	&fx_weld, // 278 do some welding
	&fx_lift_close_7, // 279 rm 7 lift closes
	&fx_lift_open_7, // 280 rm 7 lift opens
	&fx_lift_arrive_7, // 281 rm 7 lift arrives
	&fx_lift_moving, // 282 lift moving
	&fx_scanner, // 283 scanner operating
	&fx_force_fire_door, // 284 Force fire door open
	&fx_null, // 285 General door creak
	&fx_phone, // 286 telephone
	&fx_lazer, // 287 lazer
	&fx_lazer, // 288 lazer
	&fx_anchor_fall, // 289 electric   ;not used on amiga
	&fx_weld12, // 290 welding in room 12 (not joey)
	&fx_hello_helga, // 291 helga appears
	&fx_byee_helga, // 292 helga disapears
	&fx_null, // 293 smash through window               ;doesn't exist
	&fx_pos_key, // 294
	&fx_neg_key, // 295
	&fx_s2_helmet, // 296 ;helmet down section 2
	&fx_s2_helmet, // 297 ;  "      up    "    "
	&fx_lift_arrive_7, // 298 ;security door room 7
	&fx_null, // 299
	&fx_rope_creak, // 300
	&fx_crowbar_wooden, // 301
	&fx_fall_thru_box, // 302
	&fx_use_crowbar_grill, // 303
	&fx_use_secateurs, // 304
	&fx_grill_creak, // 305
	&fx_timber_cracking, // 306
	&fx_masonry_fall, // 307
	&fx_masonry_fall, // 308
	&fx_crowbar_plaster, // 309
	&fx_prise_brick, // 310
	&fx_brick_hit_foster, // 311
	&fx_spray_on_skin, // 312
	&fx_hit_crowbar_brick, // 313
	&fx_drop_crowbar, // 314
	&fx_fire_crackle_in_pit, // 315
	&fx_remove_bar_grill, // 316
	&fx_liquid_bubble, // 317
	&fx_liquid_drip, // 318
	&fx_guard_fall, // 319
	&fx_sc74_pod_down, // 320
	&fx_hiss_in_nitrogen, // 321
	&fx_null, // 322
	&fx_hit_joey1, // 323
	&fx_hit_joey2, // 324
	&fx_medi_stab_gall, // 325
	&fx_gall_drop, // 326
	&fx_null, // 327
	&fx_null, // 328
	&fx_null, // 329
	&fx_big_tent_gurgle, // 330
	&fx_null, // 331
	&fx_orifice_swallow_drip, // 332
	&fx_brick_hit_plank, // 333
	&fx_goo_drip, // 334
	&fx_plank_vibrating, // 335
	&fx_splash, // 336
	&fx_buzzer, // 337
	&fx_shed_door_creak, // 338
	&fx_dog_yap_outdoors, // 339
	&fx_dani_phone_ring, // 340
	&fx_locker_creak_open, // 341
	&fx_judges_gavel1, // 342
	&fx_dog_yap_indoors, // 343
	&fx_brick_hit_plank, // 344
	&fx_brick_hit_plank, // 345
	&fx_shaft_industrial_noise, // 346
	&fx_judges_gavel2, // 347
	&fx_judges_gavel3, // 348
	&fx_elevator_4, // 349
	&fx_lift_closing, // 350
	&fx_null, // 351
	&fx_null, // 352
	&fx_sc74_pod_down, // 353
	&fx_null, // 354
	&fx_null, // 355
	&fx_heartbeat, // 356
	&fx_star_trek_2, // 357
	&fx_null, // 358
	&fx_null, // 359
	&fx_null, // 350
	&fx_null, // 361
	&fx_null, // 362
	&fx_null, // 363
	&fx_null, // 364
	&fx_null, // 365
	&fx_break_crystals, // 366
	&fx_disintegrate, // 367
	&fx_statue_on_armour, // 368
	&fx_null, // 369
	&fx_null, // 360
	&fx_ping, // 371
	&fx_null, // 372
	&fx_door_slam_under, // 373
	&fx_null, // 374
	&fx_null, // 375
	&fx_null, // 376
	&fx_null, // 377
	&fx_null, // 378
	&fx_null, // 379
	&fx_steam1, // 380
	&fx_steam2, // 381
	&fx_steam2, // 382
	&fx_steam3, // 383
	&fx_null, // 384
	&fx_null, // 385
	&fx_fact_sensor, // 386            Sensor in Potts' room
	&fx_null, // 387
	&fx_null, // 388
	&fx_null, // 389
	&fx_null, // 390
	&fx_null, // 391
	&fx_null, // 392
	&fx_null, // 393
	&fx_25_weld // 394            my anchor weld bodge
};

SkySound::SkySound(SoundMixer *mixer, SkyDisk *pDisk) {
	_skyDisk = pDisk;
	_soundData = NULL;
	_mixer = mixer;
	_voiceHandle = 0;
	_effectHandle = 0;
	_bgSoundHandle = 0;
	_ingameSound = 0;
	_ingameSpeech = 0;
	_sfxPaused = false;
}

SkySound::~SkySound(void) {

	_mixer->stopAll();
	if (_soundData) free(_soundData);
}

int SkySound::playVoice(byte *sound, uint32 size) {

	return playSound(sound, size, &_voiceHandle);
}


int SkySound::playBgSound(byte *sound, uint32 size) {

	size -= 512; //Hack to get rid of the annoying pop at the end of some bg sounds 
	return playSound(sound, size, &_bgSoundHandle);
}

int SkySound::playSound(byte *sound, uint32 size, PlayingSoundHandle *handle) {

	byte flags = 0;
	flags |= SoundMixer::FLAG_UNSIGNED|SoundMixer::FLAG_AUTOFREE;
	size -= sizeof(struct dataFileHeader);
	byte *buffer = (byte *)malloc(size); 
	memcpy(buffer, sound+sizeof(struct dataFileHeader), size);	
	
	return _mixer->playRaw(handle, buffer, size, 11025, flags);
}

void SkySound::loadSection(uint8 pSection) {

	if (_ingameSound) _mixer->stop(_ingameSound - 1);
	_ingameSound = 0;
	if (_soundData) free(_soundData);
	_soundData = _skyDisk->loadFile(pSection * 4 + SOUND_FILE_BASE, NULL);
	if ((_soundData[0x7E] != 0x3C) || (_soundData[0xA5] != 0x8D) || (_soundData[0xA6] != 0x1E) ||
		(_soundData[0xAD] != 0x8D) || (_soundData[0xAE] != 0x36))
		error("Unknown sounddriver version!\n");
	_soundsTotal = _soundData[0x7F];
	uint16 sRateTabOfs = (_soundData[0xA8] << 8) | _soundData[0xA7];
	_sfxBaseOfs = (_soundData[0xB0] << 8) | _soundData[0xAF];
	_sampleRates = _soundData + sRateTabOfs;
	_sfxInfo = _soundData + _sfxBaseOfs;
}

void SkySound::playSound(uint16 sound, uint16 volume) {

	if (!_soundData) {
		warning("SkySound::playSound(%04X, %04X) called with a section having been loaded.\n", sound, volume);
		return;
	}
	
	if (sound > _soundsTotal) {
		if (sound & 0x80) warning("SkySound::playSound(%04X, %04X) not implemented.\n", sound, volume);
		else warning("SkySound::playSound(%04X, %04X) ignored.\n", sound, volume);
		return ;
	}

	volume = ((volume & 0x7F) + 1) << 1;
	sound &= 0xFF;
	
	// note: all those tables are big endian. Don't ask me why. *sigh*
	uint16 sampleRate = (_sampleRates[sound << 2] << 8) | _sampleRates[(sound << 2) | 1];
	uint16 dataOfs = ((_sfxInfo[sound << 3] << 8) | _sfxInfo[(sound << 3) | 1]) << 4;
	dataOfs += _sfxBaseOfs;
	uint16 dataSize = (_sfxInfo[(sound << 3) | 2] << 8) | _sfxInfo[(sound << 3) | 3];
	uint16 dataLoop = (_sfxInfo[(sound << 3) | 6] << 8) | _sfxInfo[(sound << 3) | 7];

	byte flags = SoundMixer::FLAG_UNSIGNED;
	if (dataSize == dataLoop) {
		//flags |= SoundMixer::FLAG_LOOP;
	}
	
	if (_ingameSound > 0) _mixer->stop(_ingameSound - 1);
	_mixer->setVolume(volume);
	_mixer->playRaw(&_ingameSound, _soundData + dataOfs, dataSize, sampleRate, flags);
}

void SkySound::fnPauseFx(void) {

	if (_ingameSound) _mixer->stop(_ingameSound - 1);
	_sfxPaused = true;
}

bool SkySound::fnStartFx(uint32 sound) {
	if (sound < 256 || sound > MAX_FX_NUMBER || _sfxPaused || (SkyState::_systemVars.systemFlags & SF_FX_OFF))
		return true;

	uint8 screen = (uint8)(SkyLogic::_scriptVariables[SCREEN] & 0xff);
	if (sound == 278 && screen == 25) // is this weld in room 25
		sound= 394;

	sound &= ~(1 << 8);

	Sfx *sfx = musicList[sound];
	RoomList *roomList = sfx->roomList;

	int i = 0;
	if (roomList[i].room != 0xff) // if room list empty then do all rooms
		while (roomList[i].room != screen) { // check rooms
			i++;
			if (roomList[i].room == 0xff)	
				return true;
		}

	// get fx volume

	uint8 volume = 0x40; // start with max vol

	if (!SkyState::isCDVersion()) {
		// as long as we can't set the volume for sfx without changing the speech volume,
		// we're better off not setting it at all.
		if (SkyState::_systemVars.systemFlags & SF_SBLASTER)
			volume = roomList[i].adlibVolume;
		if (SkyState::_systemVars.systemFlags & SF_ROLAND)
		 	volume = roomList[i].rolandVolume;
	}

	// Check the flags, the sound may come on after a delay.
	if (sfx->flags & SFXF_START_DELAY) {
		// queue sound
	}

	//if (sfx->flags & SFXF_SAVE)
	//	save_fx_0[stfx_cur_chn] = sound;

	playSound(sfx->soundNo, volume);
	return true;
}

bool SkySound::startSpeech(uint16 textNum) {
    
	if (!(SkyState::_systemVars.systemFlags & SF_ALLOW_SPEECH))
		return false;
	uint16 speechFileNum = _speechConvertTable[textNum >> 12] + (textNum & 0xFFF);
	
	uint8 *speechData = _skyDisk->loadFile(speechFileNum + 50000, NULL);
	if (!speechData) {
		debug(9,"File %d (speechFile %d from section %d) wasn't found!\n", speechFileNum + 50000, textNum & 0xFFF, textNum >> 12);
		return false;
	}

	uint32 speechSize = ((dataFileHeader*)speechData)->s_tot_size;
	uint8 *playBuffer = (uint8*)malloc(speechSize);
	memcpy(playBuffer, speechData + sizeof(dataFileHeader), speechSize);

	free(speechData);
	_skyDisk->flushPrefetched();

    // TODO: implement pre_after_table_area to find and prefetch file for next speech
	
	_mixer->playRaw(&_ingameSpeech, playBuffer, speechSize - 64, 11025, SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE);
	return true;
}
