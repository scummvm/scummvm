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

#ifndef ULTIMA4_H
#define ULTIMA4_H

#include "ultima/shared/engine/ultima.h"

namespace Ultima {
namespace Ultima4 {

class Ultima4Engine : public Shared::UltimaEngine {
protected:
	// Engine APIs
	Common::Error run() override;

	bool initialize() override;
public:
	Ultima4Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~Ultima4Engine() override;

	/**
	 * Returns true if a savegame can be loaded
	 */
	bool canLoadGameStateCurrently(bool isAutosave = false) override {
		return false;
	}

	/**
	 * Returns true if the game can be saved
	 */
	bool canSaveGameStateCurrently(bool isAutosave = false) override {
		return false;
	}
};


} // End of namespace Ultima4
} // End of namespace Ultima

#endif
