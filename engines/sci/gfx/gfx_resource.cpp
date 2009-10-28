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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"	// for INCLUDE_OLDGFX
#ifdef INCLUDE_OLDGFX

#include "sci/gfx/gfx_system.h"
#include "sci/gfx/gfx_resource.h"
#include "sci/gfx/gfx_tools.h"

namespace Sci {

static void gfxr_free_loop(gfxr_loop_t *loop) {
	int i;

	if (loop->cels) {
		for (i = 0; i < loop->cels_nr; i++)
			if (loop->cels[i])
				gfx_free_pixmap(loop->cels[i]);

		free(loop->cels);
	}
}

void gfxr_free_view(gfxr_view_t *view) {
	int i;

	if (view->palette)
		view->palette->free();

	if (view->loops) {
		for (i = 0; i < view->loops_nr; i++)
			gfxr_free_loop(view->loops + i);

		free(view->loops);
	}
	free(view);
}

void gfxr_free_pic(gfxr_pic_t *pic) {
	gfx_free_pixmap(pic->visual_map);
	gfx_free_pixmap(pic->priority_map);
	gfx_free_pixmap(pic->control_map);
	pic->visual_map = NULL;
	pic->priority_map = NULL;
	pic->control_map = NULL;
	if (pic->priorityTable)
		free(pic->priorityTable);
	pic->priorityTable = NULL;
	if (pic->undithered_buffer)
		free(pic->undithered_buffer);
	pic->undithered_buffer = 0;
	free(pic);
}

} // End of namespace Sci

#endif
