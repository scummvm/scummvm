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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HAROLD_BCB_CINEMA_TABLES_H
#define HAROLD_BCB_CINEMA_TABLES_H

#include "harold/bcb/ui.h"

namespace BCB {

struct CinemaButton {
	StaticRect source;
	int16 destinationX;
	int16 destinationY;
};

struct CinemaMovie {
	uint16 disc;
	uint16 number;
};

struct CinemaPlaylist {
	uint16 firstMovie;
	uint16 movieCount;
};

extern const char *const kCinemaPicturePaths[3];
extern const CinemaButton kCinemaButtons[26];
extern const uint16 kCinemaDialogSizes[4][2];
extern const StaticRect kCinemaBackgroundItems[6];
extern const StaticRect kCinemaPlaylistItems[8];
extern const StaticRect kCinemaPlaybackItems[3];
extern const CinemaMovie kCinemaMovies[];
extern const CinemaPlaylist kCinemaPlaylists[78];

} // namespace BCB

#endif
