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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge2/cge2.h"

namespace CGE2 {

CGE2Engine::CGE2Engine(OSystem *syst, const ADGameDescription *gameDescription)
	: Engine(syst), _gameDescription(gameDescription) {
}

CGE2Engine::~CGE2Engine() {
}

bool CGE2Engine::hasFeature(EngineFeature f) const {
	return false;
}

bool CGE2Engine::canLoadGameStateCurrently() {
	return false;
}
bool CGE2Engine::canSaveGameStateCurrently() {
	return false;
}

Common::Error CGE2Engine::loadGameState(int slot) {
	warning("STUB: CGE2Engine::loadGameState()");
	return Common::kNoError;
}

Common::Error CGE2Engine::saveGameState(int slot, const Common::String &desc) {
	warning("STUB: CGE2Engine::saveGameState()");
	return Common::kNoError;
}

Common::Error CGE2Engine::run() {
	warning("STUB: CGE2Engine::run()");
	return Common::kNoError;
}

} // End of namespace CGE2
