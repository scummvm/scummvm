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

#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "engines/stark/gfx/driver.h"

namespace Stark {

/*
enum StarkGameID {
	GID_TLJ = 0,
	GID_DREAM
};
*/

enum StarkGameFeatures {
	GF_DVD =  (1 << 31)
};

class StarkEngine : public Engine {
public:
	StarkEngine(OSystem *syst, const ADGameDescription *gameDesc);
	virtual ~StarkEngine();

protected:
	// Engine APIs
	virtual Common::Error run();

private:
	void mainLoop();
	void updateDisplayScene();

	GfxDriver *_gfx;

	const ADGameDescription *_gameDescription;
};

} // End of namespace Stark

#endif // STARK_H
