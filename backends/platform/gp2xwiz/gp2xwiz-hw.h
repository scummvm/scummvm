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

/*
 * GP2X Wiz: Hardware Stuff.
 *
 */

#ifndef GP2XWIZ_HW_H
#define GP2XWIZ_HW_H

/* GP2X Wiz: Main Joystick Mappings */

#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_MENU            (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_L               (10)
#define GP2X_BUTTON_R               (11)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (14)
#define GP2X_BUTTON_Y               (15)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)

namespace WIZ_HW {

#define VOLUME_MIN 0
#define VOLUME_INITIAL 70 /* Default volume if you call mixerMoveVolume(0) without setting the global var. */
#define VOLUME_MAX 100
#define VOLUME_CHANGE_RATE 8
#define VOLUME_NOCHG 0
#define VOLUME_DOWN 1
#define VOLUME_UP 2

extern int volumeLevel;

extern void	deviceInit();
extern void	deviceDeinit();
extern void	mixerMoveVolume(int);

} /* namespace WIZ_HW */

#endif //GP2XWIZ_HW_H
