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
#include <string.h>
#include "common/scummsys.h"
#include "sky/skydefs.h"
#include "sky/sky.h"

#define intro_text_width    128

#define fn_a_pal    60080
#define fn_1a_log   60081
#define fn_1a   60082
#define fn_1b   60083
#define fn_1c   60084
#define fn_1d   60085
#define fn_1e   60086
#define fn_4a   60087
#define fn_4b_log   60088
#define fn_4b   60089
#define fn_4c_log   60090
#define fn_4c   60091
#define fn_5_pal    60092
#define fn_5_log    60093
#define fn_5    60094
#define fn_6_pal    60095
#define fn_6_log    60096
#define fn_6a   60097
#define fn_6b   60098

#ifdef short_intro_start
#define virgin_time_1   3
#define viring_time_2   3
#define rev_time    8
#define gibb_time   6
#else
#define virgin_time_1   (3 * 50)
#define virgin_time_2   ((3 * 50) + 8)
#define rev_time    ((8 * 50) + 8)
#define gibb_time   ((6 * 50) + 8)
#endif

void prepare_text(void);
void show_intro_text(void);
void remove_text(void);
void intro_fx(void);
void intro_vol(void);

uint8 *seq1a_data;
uint8 *seq1b_data;
uint8 *seq1c_data;
uint8 *seq1d_data;
uint8 *seq1e_data;
uint8 *seq4a_data;
uint8 *seq4b_data;
uint8 *seq4c_data;
uint8 *seq5_data;
uint8 *seq6a_data;
uint8 *seq6b_data;

uint8 *vga_data;
uint8 *diff_data;

uint8 *work_base;
uint8 *work_screen;
uint8 *work_screen_end;

uint8 *intro_text_space;    //space for storing text messages
uint8 *intro_text_save; //save screen data here

uint8 *vga_pointer;
uint8 *diff_pointer;

uint32 no_frames;   //number of frames in scrolling intro
uint32 frame_counter;

#define ic_prepare_text 0
#define ic_show_text    1
#define ic_remove_text  2
#define ic_make_sound   3
#define ic_fx_volume    4

typedef void (*pfc)(void);
pfc command_routines[] = { &prepare_text, &show_intro_text, &remove_text, &intro_fx, &intro_vol };

uint32 cockpit_commands[] = 
{
	1000, //do straight away
	ic_prepare_text,
	77,
	220,
	ic_show_text, //radar detects jamming signal
	20,
	160,
	105,
	ic_remove_text,
	81,
	105,
	ic_show_text, //well switch to override you fool
	170,
	86,
	35,
	ic_remove_text,
	35,
	ic_prepare_text,
	477,
	35,
	ic_show_text,
	30,
	160,
	3,
	ic_remove_text
};

uint32 zero_commands[] = { 0 };

uint32 anim5_commands[] = 
{
	31,
	ic_make_sound,
	2,
	127,
	0
};

uint32 anim4a_commands[] = 
{
	136,
	ic_make_sound,
	1,
	70,
	90,
	ic_fx_volume,
	80,
	50,
	ic_fx_volume,
	90,
	5,
	ic_fx_volume,
	100,
	0
};

uint32 anim4c_commands[] =
{
	1000,
	ic_fx_volume,
	100,
	25,
	ic_fx_volume,
	110,
	15,
	ic_fx_volume,
	120,
	4,
	ic_fx_volume,
	127,
	0
};

uint32 anim6a_commands[] =
{
	1000,
	ic_prepare_text,
	478,
	13,
	ic_show_text,
	175,
	155,
	0
};

uint32 anim6b_commands[] = 
{
	131,
	ic_remove_text,
	131,
	ic_prepare_text,
	479,
	74,
	ic_show_text,
	175,
	155,
	45,
	ic_remove_text,
	45,
	ic_prepare_text,
	162,
	44,
	ic_show_text,
	175,
	155,
	4,
	ic_remove_text,
	0
};

uint32 *command_pointer = (uint32 *)zero_commands;

void SkyState::initVirgin() {
	
	_tempPal = (uint8 *)loadFile(60111, NULL);
	if (_tempPal != NULL)
		setPalette(_tempPal);

	_workScreen = (uint8 *)loadFile(60110, NULL);

	if (_workScreen != NULL)
		showScreen();

	// free the memory that was malloc'ed indirectly via load_file
	free(_workScreen);
	free(_tempPal);
}

void SkyState::intro(void) {

	_workScreen = (uint8 *)loadFile(60112, NULL); //while virgin screen is up, load rev screen
	_tempPal = (uint8 *)loadFile(60113, NULL);

	//loadSectionMusic(0);
	
	delay(3000); //keep virgin screen up for 3 seconds

	//if (!_isCDVersion)
	//	fn_start_music();
	
	delay(3000); //and another 3 seconds.
	fn_fade_down(0); //remove virgin screen
	showScreen();
	paletteFadeUp(_tempPal);
	free (_tempPal);
	free (_workScreen);
	
	//while rev is up, load gibbons screen
	_workScreen = (uint8 *)loadFile(60114, NULL);
	_tempPal = (uint8 *)loadFile(60115, NULL);

	intro_text_space = (uint8 *)malloc(10000);
	intro_text_save = (uint8 *)malloc(10000);

	//get_text(77);
	
	delay(8000); // keep revolution up for 8 seconds

	fn_fade_down(0);
	showScreen();
	paletteFadeUp(_tempPal);

	free (_tempPal);
	free (_workScreen);

	if (_isCDVersion)
		doCDIntro();
	
	
}

void SkyState::showScreen(void) {
	
	_system->copy_rect(_workScreen, 320, 0, 0, 320, 200);
	_system->update_screen();
}

void prepare_text(void)
{

}

void show_intro_text(void)
{

}

void remove_text(void)
{

}

void intro_fx(void)
{

}

void intro_vol(void)
{

}
