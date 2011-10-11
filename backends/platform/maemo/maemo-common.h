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
 */

#if defined(MAEMO)

#ifndef PLATFORM_SDL_MAEMO_COMMON_H
#define PLATFORM_SDL_MAEMO_COMMON_H

enum MaemoModelType {
	kMaemoModelTypeN800 = 1,
	kMaemoModelTypeN810 = 2,
	kMaemoModelTypeN900 = 4,
	kMaemoModelTypeInvalid = 0
};

struct MaemoModel {
	const char *hwId;
	MaemoModelType modelType;
	const char *hwAlias;
	bool hwKeyboard;
};

static const MaemoModel maemoModels[] = {
	{"RX-34", kMaemoModelTypeN800, "N800", false},
	{"RX-44", kMaemoModelTypeN810, "N810", true},
	{"RX-48", kMaemoModelTypeN810, "N810W", true},
	{"RX-51", kMaemoModelTypeN900, "N900", true},
	{0, kMaemoModelTypeInvalid, 0, true}
};


#endif // ifndef PLATFORM_SDL_MAEMO_COMMON_H

#endif // if defined(MAEMO)
