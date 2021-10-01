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

#include "groovie/logic/clangame.h"

namespace Groovie {

// This a list of files for background music. This list is hardcoded in the Clandestiny player.
const char *kClanMusicFiles[] = {"mbf_arb1", "mbf_arm1", "mbf_bal1", "mbf_c2p2", "act18mus", "act15mus", "act21mus",
										  "act05mus", "act04mus", "act23mus", "act17mus", "act03mus", "act06mus", "act19mus",
										  "act07mus", "mbf_mne1", "act24mus", "act24mus", "act14mus", "act20mus", "act15mus",
										  "act13mus", "act08mus", "mbf_uph1", "mbf_uph1", "act19mus", "mbf_bol1", "mbf_cbk1",
										  "mbf_glf1", "mbf_bro1", "mbf_c1r1", "mbf_c1r1", "mbf_c1r1", "mbf_c1r1", "mbf_c2r1",
										  "mbf_c2r1", "mbf_c2r1", "mbf_c2r1", "mbf_c3r1", "mbf_c3r1", "mbf_c3r1", "mbf_c4r1",
										  "mbf_c4r1", "mbf_c1p2", "mbf_c3p3", "mbf_c1p3", "mbf_bro1", "mbf_c1p1", "act17mus",
										  "mbf_c2p2", "mbf_c2p1", "act10mus", "mbf_c1p1", "mbf_mne1", "mbf_c3p3", "act17mus",
										  "mbf_c3p2", "mbf_c3p1", "act25mus", "mbf_c4p2", "mbf_c4p1"};

// Gets the filename of the background music file.
const char *ClanGame::getClanMusicFilename(int musicId) {
	return kClanMusicFiles[musicId];
}

} // namespace Groovie
