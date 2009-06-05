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
 */

#ifndef ASYLUM_ENGINE_H
#define ASYLUM_ENGINE_H

#include "engines/engine.h"

namespace Asylum {

class AsylumEngine: public Engine {
public:
	
	AsylumEngine(OSystem *system, Common::Language language);
	virtual ~AsylumEngine();

	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;	
private:
	Common::Language _language;
	Common::RandomSource _rnd;
};

} // namespace Asylum

#endif
