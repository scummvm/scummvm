/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "build_display.h"
#include "console.h"
#include "debug.h"
#include "header.h"
#include "mem_view.h"
#include "memory.h"
#include "resman.h"
#include "sword2.h"	// (James11aug97) for CloseGame()

// has to be global because a local in Fetch_mem_owner is destroyed on exit
char buf[50];

void Console_mem_display(void) {	// Tony13Aug96
	int pass, found_end, k, j, free = 0;
	_standardHeader	*file_header;
	int scrolls = 0;
	_keyboardEvent ke;

	char inf[][20] = {
		{ "M_null  " },
		{ "M_free  " },
		{ "M_locked" },
		{ "M_float " }
	};

	j = base_mem_block;
	do {
		if (mem_list[j].uid < 65536) {
			file_header = (_standardHeader*) res_man.Res_open(mem_list[j].uid);
			// close immediately so give a true count
			res_man.Res_close(mem_list[j].uid);

			Zdebug("view %d", mem_list[j].uid);

			pass = 0;
			found_end = 0;

			for (k = 0; k < 30; k++) {
				if (file_header->name[k] == 0) {
					found_end = 1;
					break;
				}

				if (file_header->name[k] < ' ' || file_header->name[k] > 'z')
					pass = 1;
			}

			if (file_header->name[0] == 0)
				pass = 1;	// also illegal

			if (!pass && found_end) { // && file_header->fileType < 10)
				Print_to_console("%d %s, size 0x%.5x (%dk %d%%), res %d %s %s, A%d, C%d",
					j, inf[mem_list[j].state],
					mem_list[j].size,
					mem_list[j].size / 1024,
					(mem_list[j].size * 100) / total_free_memory,
					mem_list[j].uid,
					res_man.Fetch_cluster(mem_list[j].uid),
					file_header->name,
					res_man.Fetch_age(mem_list[j].uid),
					res_man.Fetch_count(mem_list[j].uid));
			} else
				Print_to_console(" %d is an illegal resource", mem_list[j].uid);
		} else {
			Print_to_console("%d %s, size 0x%.5x (%dk %d%%), %s",
				j, inf[mem_list[j].state], mem_list[j].size,
				mem_list[j].size / 1024,
				(mem_list[j].size * 100) / total_free_memory,
				Fetch_mem_owner(mem_list[j].uid));
		}

		if (mem_list[j].state == MEM_free)
			free += mem_list[j].size;

		j = mem_list[j].child;

		scrolls++;

		Build_display();

		if (scrolls == 18) {
			Temp_print_to_console("- Press ESC to stop or any other key to continue");
			Build_display();

			do {
				ServiceWindows();
			} while(!KeyWaiting());

			ReadKey(&ke);	//kill the key we just pressed
			if (ke.keycode == 27)	//ESC
				break;

			// clear the Press Esc message ready for the new line
			Clear_console_line();
			scrolls = 0;
		}	
	} while (j != -1);

	Scroll_console();
	Print_to_console("(total memory block 0x%.8x %dk %dMB) %d / %d%% free",
		total_free_memory, total_free_memory / 1024,
		total_free_memory / (1000 * 1024), free,
		(free * 100) / total_free_memory);
}

const char *Fetch_mem_owner(uint32 uid) {	//Tony3June96
	switch (uid) {
	case UID_memman:
		return "MEMMAN";
	case UID_font:
		return "font";
	case UID_temp:
		return "temp ram allocation";
	case UID_decompression_buffer:
		return "decompression buffer";
	case UID_shrink_buffer:
		return "shrink buffer";
	case UID_con_sprite:
		return "console sprite buffer";
	case UID_text_sprite:
		return "text sprite";
	case UID_walk_anim:
		return "walk anim";
	case UID_savegame_buffer:
		return "savegame buffer";
	default:
		sprintf(buf, "<sob> %d?", uid);
		return buf;
	}
}

void Create_mem_string(char *string) {	// James (21oct96 updated 4dec96)
	int blockNo = base_mem_block;
	int blocksUsed = 0;
	int mem_free = 0;
	int mem_locked = 0;
	int mem_floating = 0;
	int memUsed = 0;
	int percent;

	while (blockNo != -1) {
		switch (mem_list[blockNo].state) {
			case MEM_free:
				mem_free++;
				break;

			case MEM_locked:
				mem_locked++;
				memUsed += mem_list[blockNo].size;
				break;

			case MEM_float:
				mem_floating++;
				memUsed += mem_list[blockNo].size;
				break;
		}

		blocksUsed++;
		blockNo = mem_list[blockNo].child;
	}

	percent =  (memUsed * 100) / total_free_memory;

	sprintf(string,
		"locked(%u)+float(%u)+free(%u) = %u/%u blocks (%u%% used)(cur %uk)",
		mem_locked, mem_floating, mem_free, blocksUsed, MAX_mem_blocks,
		percent, (res_man.Res_fetch_useage() / 1024));
}
