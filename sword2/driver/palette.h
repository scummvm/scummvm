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

//=============================================================================
//
//	Filename	:	palette.h
//	Created		:	8th November 1996
//	By			:	P.R.Porter
//
//	Summary		:	This include file defines links to all data which is
//					defined in the palette.c module, but can be accessed by
//					other parts of the driver96 library.
//
//
//=============================================================================


#ifndef PALETTE_H
#define PALETTE_H

namespace Sword2 {

extern uint8 paletteMatch[64*64*64];

extern int32 RestorePalette(void);
extern void FadeServer();

} // End of namespace Sword2

#endif
