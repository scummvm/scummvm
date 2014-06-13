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

#ifndef BACKENDS_GRAPHICS_OPENGL_OPENGL_H
#define BACKENDS_GRAPHICS_OPENGL_OPENGL_H

// The purpose of this header is to include the OpenGL headers in an uniform
// fashion. A notable example for a non standard port is the Tizen port.

#include "common/scummsys.h"

#ifdef WIN32
#if defined(ARRAYSIZE) && !defined(_WINDOWS_)
#undef ARRAYSIZE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ARRAYSIZE
#endif

// HACK: In case common/util.h has been included already we need to make sure
// to define ARRAYSIZE again in case of Windows.
#if !defined(ARRAYSIZE) && defined(COMMON_UTIL_H)
#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))
#endif

#if defined(TIZEN)
#include <FGraphicsOpengl.h>
using namespace Tizen::Graphics::Opengl;
#elif defined(USE_GLES)
#include <GLES/gl.h>
#elif defined(SDL_BACKEND)
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#endif

#endif
