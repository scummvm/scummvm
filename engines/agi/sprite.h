/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

#ifndef AGI_SPRITE_H
#define AGI_SPRITE_H

namespace Agi {

int init_sprites(void);
void deinit_sprites(void);
void erase_upd_sprites(void);
void erase_nonupd_sprites(void);
void erase_both(void);
void blit_upd_sprites(void);
void blit_nonupd_sprites(void);
void blit_both(void);
void commit_upd_sprites(void);
void commit_nonupd_sprites(void);
void commit_both(void);
void add_to_pic(int, int, int, int, int, int, int);
void show_obj(int);
void commit_block(int, int, int, int);

}                             // End of namespace Agi

#endif				/* AGI_SPRITE_H */
