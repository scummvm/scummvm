/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef STARK_XMG_H
#define STARK_XMG_H

#include "engines/stark/sceneelement.h"

#include "common/stream.h"

namespace Graphics {
struct Surface;
}

namespace Stark {

/**
 * XMG (still image) decoder and renderer
 */
class SceneElementXMG : public SceneElement2D {
private:
	SceneElementXMG();

public:
	~SceneElementXMG();
	static SceneElementXMG *load(Common::ReadStream *stream);

	void render(GfxDriver *gfx) override;

private:
	Graphics::Surface *_surface;
};

} // End of namespace Stark

#endif // STARK_XMG_H
