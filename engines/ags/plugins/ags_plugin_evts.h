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

//=============================================================================
//
// AGS Plugin interface header file.
// Engine events definition.
//
// If you are writing a plugin, include agsplugin.h instead.
//
//=============================================================================

#ifndef AGS_PLUGINS_AGS_PLUGIN_EVTS_H
#define AGS_PLUGINS_AGS_PLUGIN_EVTS_H

namespace AGS3 {

// Below are interface 3 and later
#define AGSE_KEYPRESS        1
#define AGSE_MOUSECLICK      2
#define AGSE_POSTSCREENDRAW  4
// Below are interface 4 and later
#define AGSE_PRESCREENDRAW   8
// Below are interface 5 and later
#define AGSE_SAVEGAME        0x10
#define AGSE_RESTOREGAME     0x20
// Below are interface 6 and later
#define AGSE_PREGUIDRAW      0x40
#define AGSE_LEAVEROOM       0x80
#define AGSE_ENTERROOM       0x100
#define AGSE_TRANSITIONIN    0x200
#define AGSE_TRANSITIONOUT   0x400
// Below are interface 12 and later
#define AGSE_FINALSCREENDRAW 0x800
#define AGSE_TRANSLATETEXT   0x1000
// Below are interface 13 and later
#define AGSE_SCRIPTDEBUG     0x2000
#define AGSE_AUDIODECODE     0x4000 // obsolete, no longer supported
// Below are interface 18 and later
#define AGSE_SPRITELOAD      0x8000
// Below are interface 21 and later
#define AGSE_PRERENDER       0x10000
// Below are interface 24 and later
#define AGSE_PRESAVEGAME     0x20000
#define AGSE_POSTRESTOREGAME 0x40000
// Below are interface 26 and later
#define AGSE_POSTROOMDRAW    0x80000
#define AGSE_TOOHIGH         0x100000

} // namespace AGS3

#endif
