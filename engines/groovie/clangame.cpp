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

#include "groovie/clangame.h"

namespace Groovie {

// This a list of files for background music. These list is hard-coded in the TLC player.
const char *kClanMusicFiles[] = {"mbf_arb1.mpg", "mbf_arm1.mpg", "mbf_bal1.mpg", "mbf_c2p2.mpg", "act18mus.mpg", "act15mus.mpg", "act21mus.mpg",
										  "act05mus.mpg", "act04mus.mpg", "act23mus.mpg", "act17mus.mpg", "act03mus.mpg", "act06mus.mpg", "act19mus.mpg",
										  "act07mus.mpg", "mbf_mne1.mpg", "act24mus.mpg", "act24mus.mpg", "act14mus.mpg", "act20mus.mpg", "act15mus.mpg",
										  "act13mus.mpg", "act08mus.mpg", "mbf_uph1.mpg", "mbf_uph1.mpg", "act19mus.mpg", "mbf_bol1.mpg", "mbf_cbk1.mpg",
										  "mbf_glf1.mpg", "mbf_bro1.mpg", "mbf_c1r1.mpg", "mbf_c1r1.mpg", "mbf_c1r1.mpg", "mbf_c1r1.mpg", "mbf_c2r1.mpg",
										  "mbf_c2r1.mpg", "mbf_c2r1.mpg", "mbf_c2r1.mpg", "mbf_c3r1.mpg", "mbf_c3r1.mpg", "mbf_c3r1.mpg", "mbf_c4r1.mpg",
										  "mbf_c4r1.mpg", "mbf_c1p2.mpg", "mbf_c3p3.mpg", "mbf_c1p3.mpg", "mbf_bro1.mpg", "mbf_c1p1.mpg", "act17mus.mpg",
										  "mbf_c2p2.mpg", "mbf_c2p1.mpg", "act10mus.mpg", "mbf_c1p1.mpg", "mbf_mne1.mpg", "mbf_c3p3.mpg", "act17mus.mpg",
										  "mbf_c3p2.mpg", "mbf_c3p1.mpg", "act25mus.mpg", "mbf_c4p2.mpg", "mbf_c4p1.mpg"};

// Gets the filename of the background music file.
const char *ClanGame::getClanMusicFilename(int musicId) {
	return kClanMusicFiles[musicId];
}

} // namespace Groovie
