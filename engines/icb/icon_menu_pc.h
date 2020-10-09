/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ICB_ICONMENU_PC_H_INCLUDED
#define ICB_ICONMENU_PC_H_INCLUDED

namespace ICB {

#define ICON_X_SIZE 60

#define ICON_Y_SIZE 40

// And this defines the space between them when we draw them.
#define ICON_SPACING 4

// This is the default row to draw the chooser at.
#define ICON_MENU_PIXEL_X ICON_SPACING

#define ICON_MENU_PIXEL_Y 420

// These position the flashing icon when medipacks and ammo clips are added.
#define ICON_ADDING_X 570
#define ICON_ADDING_Y 420

// These define the sizes of some commonly used rectangles.
extern LRECT ICON_BITMAP_RECT;
extern LRECT ICON_ADDING_RECT;

} // End of namespace ICB

#endif // #ifndef ICONMENU_PC_H_INCLUDED
