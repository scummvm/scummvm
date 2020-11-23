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

//=============================================================================
//
// OpenGL includes and definitions for various platforms
//
//=============================================================================

#include "ags/shared/core/platform.h"

#if AGS_PLATFORM_OS_WINDOWS
#include "ags/lib/allegro.h"
//include <winalleg.h>
//include <allegro/platform/aintwin.h>

#include "ags/shared/glad/glad.h"
#include "ags/shared/glad/glad_wgl.h"

#elif AGS_PLATFORM_OS_LINUX
#include "ags/lib/allegro.h"
//include <xalleg.h>
//include <X11/Xatom.h>

#include "ags/shared/glad/glad.h"
#include "ags/shared/glad/glad_glx.h"

#elif AGS_PLATFORM_OS_ANDROID

//include <GLES/gl.h>
//include <GLES2/gl2.h>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

// TODO: we probably should not use GLExt since we use GLES2
//include <GLES/glext.h>

#define HDC void*
#define HGLRC void*
#define HWND void*
#define HINSTANCE void*

#elif AGS_PLATFORM_OS_IOS

//include <OpenGLES/ES1/gl.h>
//include <OpenGLES/ES2/gl.h>

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

//include <OpenGLES/ES1/glext.h>

#define HDC void*
#define HGLRC void*
#define HWND void*
#define HINSTANCE void*

#else

#error "opengl: unsupported platform"

#endif

#ifndef GLAPI
#define GLAD_GL_VERSION_2_0 (0)
#endif
