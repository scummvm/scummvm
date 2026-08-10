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

// Renderer interface for Hopkins WBASE.

#ifndef HOPKINS_BASE_RENDERER_H
#define HOPKINS_BASE_RENDERER_H

#include "hopkins/base_engine.h"

#include "common/array.h"
#include "common/scummsys.h"

namespace Hopkins {

/**
 * Renderer seam for WBASE. The software implementation is the fidelity
 * reference; a future backend may consume the same ACK state without
 * changing simulation or resource handling.
 */
class BaseRenderer {
public:
	virtual ~BaseRenderer() {}
	virtual void render(const BaseEngine &engine, byte *framebuffer) = 0;

	static BaseRenderer *createSoftware();
};

} // End of namespace Hopkins

#endif // HOPKINS_BASE_RENDERER_H
