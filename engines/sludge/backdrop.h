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

#ifndef SLUDGE_BACKDROP_H
#define SLUDGE_BACKDROP_H

#include "common/stream.h"

#include "graphics/surface.h"

#include "sludge/variable.h"

namespace Sludge {

/**
 * parallax layers can scroll at different speeds
 * to the background image, giving the illusion of
 * depth to a scene as it moves.
 */
class Parallax {
public:
	struct ParallaxLayer {
		Graphics::Surface surface;
		int speedX, speedY;
		bool wrapS, wrapT;
		uint16 fileNum, fractionX, fractionY;
		int cameraX, cameraY;
	};
	typedef Common::List<ParallaxLayer *> ParallaxLayers;

	Parallax();
	~Parallax();

	void kill();
	bool add(uint16 v, uint16 fracX, uint16 fracY);
	void save(Common::WriteStream *fp);
	void draw();
private:
	ParallaxLayers _parallaxLayers;

	inline int sortOutPCamera(int cX, int fX, int sceneMax, int boxMax) {
		return (fX == 65535) ? (sceneMax ? ((cX * boxMax) / sceneMax) : 0) : ((cX * fX) / 100);
	}
};

} // End of namespace Sludge

#endif
