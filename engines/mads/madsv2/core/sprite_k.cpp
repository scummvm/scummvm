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

#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/loader.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/sprite.h"
#include "mads/madsv2/core/keys.h"

namespace MADS {
namespace MADSV2 {

int kidney = false;

void dont_frag_the_palette(void) {
	/* this will tell sprite_free to not execute pal_deallocate. This will */
	/* make it so the colors are not freed and you won't get the */
	/* palette fragging. */

	kidney = true;
}

void go_ahead_and_frag_the_palette(void) {
	kidney = false;
}

void fastcall sprite_free(SeriesPtr *series, int free_memory) {
	if (*series == NULL)
		goto done;

	if ((*series)->pack_by_sprite) {
		if ((*series)->page_info->paging_source == LOADER_DISK) {
			delete (*series)->page_info->handle;
		}
		if ((*series)->arena != NULL) {
			if (free_memory)
				mem_free((*series)->arena);
		}
	}

	if (kidney) {
		/* release the flag - dont deallocate the colors from the list */
		if ((*series)->color_handle) pal_deallocate((*series)->color_handle);
		/* flag_used[(*series)->color_handle] = false; */
	} else {
		/* deallocate the colors from the list */
		if ((*series)->color_handle) pal_deallocate((*series)->color_handle);
	}

	if (free_memory)
		mem_free(*series);
	*series = NULL;

done:
	;
}

} // namespace MADSV2
} // namespace MADS
