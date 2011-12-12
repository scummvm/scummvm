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

namespace Maemo {

enum ModelType {
	kModelTypeN800 = 1,
	kModelTypeN810 = 2,
	kModelTypeN900 = 4,
	kModelTypeInvalid = 0
};

struct Model {
	const char *hwId;
	ModelType modelType;
	const char *hwAlias;
	bool hwKeyboard;
};

static const Model models[] = {
	{"RX-34", kModelTypeN800, "N800", false},
	{"RX-44", kModelTypeN810, "N810", true},
	{"RX-48", kModelTypeN810, "N810W", true},
	{"RX-51", kModelTypeN900, "N900", true},
	{0, kModelTypeInvalid, 0, true}
};

} // namespace Maemo

#endif // ifndef PLATFORM_SDL_MAEMO_COMMON_H

#endif // if defined(MAEMO)
