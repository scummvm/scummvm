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
 * Based on the original sources
 *   https://github.com/Croquetx/thecolony
 * Copyright (C) 1988, David A. Smith
 *
 * Distributed under Apache Version 2.0 License
 *
 */

#ifndef COLONY_RENDERER_OPENGL_SHADERS_H
#define COLONY_RENDERER_OPENGL_SHADERS_H

#include "common/scummsys.h"

namespace Colony {

class Renderer;

// Factory for the programmable-pipeline OpenGL renderer. Returns nullptr
// when USE_OPENGL_SHADERS is not defined at build time, so callers can
// fall back to the fixed-function renderer.
Renderer *createOpenGLShaderRenderer(OSystem *system, int width, int height);

} // End of namespace Colony

#endif
