/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef STARK_H
#define STARK_H

#include "engines/engine.h"

#include "engines/stark/archive.h"
#include "engines/stark/xrc.h"

namespace Stark {

enum StarkGameID {
	GID_TLJ = 0,
	GID_DREAM
};

enum StarkGameFeatures {
	GF_DEMO = (1 << 0),
	GF_DVD =  (1 << 1)
};

struct StarkGameDescription;

class StarkEngine : public Engine {
public:
	StarkEngine(OSystem *syst, const StarkGameDescription *gameDesc);
	virtual ~StarkEngine();

	int getGameID() const;
	uint16 getVersion() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;

	const StarkGameDescription *_gameDescription;
	
protected:
	// Engine APIs
	virtual Common::Error run();
	
private:
	XARCArchive _xArchive;
	
};

} // end of namespace Stark

#endif
