#include <io.h>
#include <fcntl.h>
#include "common.h"
#include "savegame.h"
#include "resdata.h"
#include "script.h"
#include "cga.h"
#include "room.h"

void SaveRestartGame(void)
{
	/*TODO*/
}

void RestartGame(void)
{
	/*
	while(!LoadFile("CLEAR.BIN", save_start))
		AskDisk2();
	*/

	script_byte_vars.cur_spot_flags = 0xFF;
	script_byte_vars.load_flag = 2;
	/*Restart();*/

	/*TODO*/
}

#define CGA_SAVE_BEG_OFS 0x751E
#define CGA_SAVE_END_OFS 0x9D5D

#define CGA_SAVE_WORD_VARS_OFS 0x7844
#define CGA_SAVE_BYTE_VARS_OFS 0x79B6
#define CGA_SAVE_INVENTORY_OFS 0x75E5
#define CGA_SAVE_ZONES_OFS     0x7A27
#define CGA_SAVE_PERS_OFS      0x7765
#define CGA_SAVE_STACK_OFS     0x7562

int LoadScena(void)
{
	int f;
	int rlen;

	script_byte_vars.game_paused = 1;


	f = open("SCENAx.BIN", O_RDONLY | O_BINARY);
	if(f == -1)
	{
		script_byte_vars.game_paused = 0;
		return 1;	/*error*/
	}
	/*
	Save format:
	  vars memory (751E-9D5D)
	  frontbuffer (0x3FFF bytes)
      backbuffer  (0x3FFF bytes)


	*/


	/*TODO*/


	rlen = read(f, backbuffer, 0x3FFF);
	if(rlen != 0x3FFF)
		goto error;

	CGA_BackBufferToRealFull();
	SelectPalette();

	rlen = read(f, backbuffer, 0x3FFF);
	if(rlen != 0x3FFF)
		goto error;

	/*re-initialize sprites list*/
	BackupSpotsImages();

	close(f);
	script_byte_vars.game_paused = 0;
	return 0;

error:;
	close(f);
	script_byte_vars.game_paused = 0;
	return 1;
}

#define SAVEADDR(value, base, nativesize, origsize, origbase)	\
	LE16(((((unsigned char*)(value)) - (unsigned char*)(base)) / nativesize) * origsize + origbase)

int SaveScena(void)
{
	int f;
	int wlen;
	unsigned short ofs16;

	script_byte_vars.game_paused = 1;
	BlitSpritesToBackBuffer();

	f = open("SCENAx.BIN", O_CREAT | O_WRONLY | O_BINARY);
	if(f == -1)
	{
		script_byte_vars.game_paused = 0;
		return 1;	/*error*/
	}

	ofs16 = SAVEADDR(script_vars[ScrPool0_WordVars0], &script_word_vars, 2, 2, CGA_SAVE_WORD_VARS_OFS);
	wlen = write(f, &ofs16, 2); if(wlen != 2) goto error;

	ofs16 = SAVEADDR(script_vars[ScrPool1_WordVars1], &script_word_vars, 2, 2, CGA_SAVE_WORD_VARS_OFS);
	wlen = write(f, &ofs16, 2); if(wlen != 2) goto error;

	ofs16 = SAVEADDR(script_vars[ScrPool2_ByteVars], &script_byte_vars, 1, 1, CGA_SAVE_BYTE_VARS_OFS);
	wlen = write(f, &ofs16, 2); if(wlen != 2) goto error;

	ofs16 = SAVEADDR(script_vars[ScrPool3_CurrentItem], inventory_items, sizeof(item_t), sizeof(item_t), CGA_SAVE_INVENTORY_OFS);
	wlen = write(f, &ofs16, 2); if(wlen != 2) goto error;

	ofs16 = SAVEADDR(script_vars[ScrPool4_ZoneSpots], zones_data, 1, 1, CGA_SAVE_ZONES_OFS);
	wlen = write(f, &ofs16, 2); if(wlen != 2) goto error;

	ofs16 = SAVEADDR(script_vars[ScrPool5_Persons], pers_list, 1, 1, CGA_SAVE_PERS_OFS);
	wlen = write(f, &ofs16, 2); if(wlen != 2) goto error;

	ofs16 = SAVEADDR(script_vars[ScrPool6_Inventory], inventory_items, sizeof(item_t), sizeof(item_t), CGA_SAVE_INVENTORY_OFS);
	wlen = write(f, &ofs16, 2); if(wlen != 2) goto error;

	ofs16 = SAVEADDR(script_vars[ScrPool7_Inventory38], inventory_items, sizeof(item_t), sizeof(item_t), CGA_SAVE_INVENTORY_OFS);
	wlen = write(f, &ofs16, 2); if(wlen != 2) goto error;

	ofs16 = SAVEADDR(script_vars[ScrPool8_CurrentPers], pers_list, 1, 1, CGA_SAVE_PERS_OFS);
	wlen = write(f, &ofs16, 2); if(wlen != 2) goto error;


	/*TODO*/




	wlen = write(f, frontbuffer, 0x3FFF);
	if(wlen != 0x3FFF)
		goto error;

	wlen = write(f, backbuffer, 0x3FFF);
	if(wlen != 0x3FFF)
		goto error;

	close(f);
	script_byte_vars.game_paused = 0;
	return 0;

error:;
	close(f);
	script_byte_vars.game_paused = 0;
	return 1;
}
