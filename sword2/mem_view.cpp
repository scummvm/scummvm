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
#include "bs2/sword2.h"
#include "bs2/build_display.h"
#include "bs2/console.h"
#include "bs2/header.h"
#include "bs2/memory.h"
#include "bs2/resman.h"

namespace Sword2 {

// has to be global because a local in Fetch_mem_owner is destroyed on exit
char buf[50];

void MemoryManager::displayMemory(void) {
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

	j = _baseMemBlock;
	do {
		if (_memList[j].uid < 65536) {
			file_header = (_standardHeader*) res_man.open(_memList[j].uid);
			// close immediately so give a true count
			res_man.close(_memList[j].uid);

			debug(5, "view %d", _memList[j].uid);

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
					j, inf[_memList[j].state],
					_memList[j].size,
					_memList[j].size / 1024,
					(_memList[j].size * 100) / _totalFreeMemory,
					_memList[j].uid,
					res_man.fetchCluster(_memList[j].uid),
					file_header->name,
					res_man.fetchAge(_memList[j].uid),
					res_man.fetchCount(_memList[j].uid));
			} else
				Print_to_console(" %d is an illegal resource", _memList[j].uid);
		} else {
			Print_to_console("%d %s, size 0x%.5x (%dk %d%%), %s",
				j, inf[_memList[j].state], _memList[j].size,
				_memList[j].size / 1024,
				(_memList[j].size * 100) / _totalFreeMemory,
				fetchOwner(_memList[j].uid));
		}

		if (_memList[j].state == MEM_free)
			free += _memList[j].size;

		j = _memList[j].child;

		scrolls++;

		Build_display();

		if (scrolls == 18) {
			Temp_print_to_console("- Press ESC to stop or any other key to continue");
			Build_display();

			do {
				g_display->updateDisplay();
			} while (!KeyWaiting());

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
		_totalFreeMemory, _totalFreeMemory / 1024,
		_totalFreeMemory / (1000 * 1024), free,
		(free * 100) / _totalFreeMemory);
}

const char *MemoryManager::fetchOwner(uint32 uid) {
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

void MemoryManager::memoryString(char *string) {
	int blockNo = _baseMemBlock;
	int blocksUsed = 0;
	int mem_free = 0;
	int mem_locked = 0;
	int mem_floating = 0;
	int memUsed = 0;
	int percent;

	while (blockNo != -1) {
		switch (_memList[blockNo].state) {
			case MEM_free:
				mem_free++;
				break;

			case MEM_locked:
				mem_locked++;
				memUsed += _memList[blockNo].size;
				break;

			case MEM_float:
				mem_floating++;
				memUsed += _memList[blockNo].size;
				break;
		}

		blocksUsed++;
		blockNo = _memList[blockNo].child;
	}

	percent =  (memUsed * 100) / _totalFreeMemory;

	sprintf(string,
		"locked(%u)+float(%u)+free(%u) = %u/%u blocks (%u%% used)(cur %uk)",
		mem_locked, mem_floating, mem_free, blocksUsed, MAX_mem_blocks,
		percent, (res_man.fetchUsage() / 1024));
}

} // End of namespace Sword2
