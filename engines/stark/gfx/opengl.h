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

#ifndef STARK_GFX_OPENGL_H
#define STARK_GFX_OPENGL_H

#include "common/rect.h"
#include "common/system.h"

#ifdef USE_OPENGL

#include "engines/stark/gfx/driver.h"

namespace Stark {
namespace Gfx {

class OpenGLDriver : public Driver {
public:
	OpenGLDriver();
	~OpenGLDriver();

	void setupScreen(int screenW, int screenH, bool fullscreen);

	void setGameViewport() override;
	void setScreenViewport() override;

	void setupCamera(const Math::Matrix4 &projection, const Math::Matrix4 &view) override;

	void clearScreen();
	void flipBuffer();

	Texture *createTexture(const Graphics::Surface *surface = nullptr, const byte *palette = nullptr) override;
	VisualActor *createActorRenderer() override;

	void drawSurface(const Texture *texture, const Common::Point &dest) override;

	void set3DMode();

private:
	void start2DMode();
	void end2DMode();

	Common::Rect _viewport;
};

} // End of namespace Gfx
} // End of namespace Stark

#endif // USE_OPENGL

#endif // STARK_GFX_OPENGL_H
