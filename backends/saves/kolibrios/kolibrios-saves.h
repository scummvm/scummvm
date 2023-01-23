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

#if !defined(BACKEND_KOLIBRIOS_SAVES_H) && !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)
#define BACKEND_KOLIBRIOS_SAVES_H

#include "backends/saves/default/default-saves.h"

#if !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)
/**
 * Customization of the DefaultSaveFileManager for KolibriOS.
 * The only two differences are that the default constructor sets
 * up the savepath based on HOME, and that checkPath tries to
 * create the savedir, if missing, via the mkdir() syscall.
 */
class KolibriOSSaveFileManager : public DefaultSaveFileManager {
public:
	KolibriOSSaveFileManager(const Common::Path& writeablePath);
};
#endif

#endif
