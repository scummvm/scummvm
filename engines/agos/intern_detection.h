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

// Game detection - related enums, taken from agos/intern.h

namespace AGOS {

enum GameIds {
	GID_PN,
	GID_ELVIRA1,
	GID_ELVIRA2,
	GID_WAXWORKS,

	GID_SIMON1,
	GID_SIMON1DOS,
	GID_SIMON1CD32,

	GID_SIMON2,

	GID_FEEBLEFILES,

	GID_DIMP,
	GID_JUMBLE,
	GID_PUZZLE,
	GID_SWAMPY
};

enum GameFeatures {
	GF_TALKIE           = 1 << 0,
	GF_OLD_BUNDLE       = 1 << 1,
	GF_CRUNCHED         = 1 << 2,
	GF_CRUNCHED_GAMEPC  = 1 << 3,
	GF_ZLIBCOMP         = 1 << 4,
	GF_32COLOR          = 1 << 5,
	GF_EGA              = 1 << 6,
	GF_PLANAR           = 1 << 7,
	GF_DEMO             = 1 << 8,
	GF_PACKED           = 1 << 9,
	GF_BROKEN_FF_RATING = 1 << 10,
	GF_WAVSFX           = 1 << 11
};

enum GameFileTypes {
	GAME_BASEFILE = 1 << 0,
	GAME_ICONFILE = 1 << 1,
	GAME_GMEFILE  = 1 << 2,
	GAME_MENUFILE = 1 << 3,
	GAME_STRFILE  = 1 << 4,
	GAME_RMSLFILE = 1 << 5,
	GAME_STATFILE = 1 << 6,
	GAME_TBLFILE  = 1 << 7,
	GAME_XTBLFILE = 1 << 8,
	GAME_RESTFILE = 1 << 9,
	GAME_TEXTFILE = 1 << 10,
	GAME_VGAFILE  = 1 << 11,

	GAME_GFXIDXFILE = 1 << 12
};


} // End of namespace AGOS
