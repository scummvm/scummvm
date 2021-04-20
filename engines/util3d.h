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

#ifndef ENGINES_UTIL3D_H
#define ENGINES_UTIL3D_H

#include "common/config-manager.h"
#include "common/system.h"

#include "engines/util.h"

#include "graphics/renderer.h"
#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
#include "graphics/opengl/context.h"
#endif


/**
 * @defgroup engines_util Util3d
 * @ingroup engines
 *
 * @brief Various utility functions related to 3d engines only.
 *
 * @{
 */

/**
 * Template code for 3d engine render init.
 * Determines render type by checking settings, features, shader support, etc.
 *
 * Calls initGraphics(width, height, nullptr) for software mode.
 * Calls initGraphics3d(width, height) otherwise.
 *
 * Returns one of CreateOpenGLS(arg) / CreateOpenGL(arg) / CreateTinyGL(arg).
 * Shows an error, if result is nullptr.
 */
template<class T, class A>
T *createSelectedRender(
	int width,
	int height,
#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
	T *(*CreateOpenGLS)(A *),
#endif
#if defined(USE_OPENGL_GAME) && !defined(USE_GLES2)
	T *(*CreateOpenGL)(A *),
#endif
	T *(*CreateTinyGL)(A *),
	A *arg
) {
	Common::String rendererConfig = ConfMan.get("renderer");
	Graphics::RendererType desiredRendererType = Graphics::parseRendererTypeCode(rendererConfig);
	Graphics::RendererType matchingRendererType = Graphics::getBestMatchingAvailableRendererType(desiredRendererType);

	bool isAccelerated = matchingRendererType != Graphics::kRendererTypeTinyGL;

	if (isAccelerated) {
		initGraphics3d(width, height);
	} else {
		initGraphics(width, height, nullptr);
	}

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
	bool backendCapableOpenGL = g_system->hasFeature(OSystem::kFeatureOpenGLForGame);
#endif

#if defined(USE_OPENGL_GAME)
	// Check the OpenGL context actually supports shaders
	if (backendCapableOpenGL && matchingRendererType == Graphics::kRendererTypeOpenGLShaders && !OpenGLContext.shadersSupported) {
		matchingRendererType = Graphics::kRendererTypeOpenGL;
	}
#endif

	if (matchingRendererType != desiredRendererType && desiredRendererType != Graphics::kRendererTypeDefault) {
		// Display a warning if unable to use the desired renderer
		warning("Unable to create a '%s' renderer", rendererConfig.c_str());
	}

	T *result = nullptr;

#if defined(USE_GLES2) || defined(USE_OPENGL_SHADERS)
	if (backendCapableOpenGL && matchingRendererType == Graphics::kRendererTypeOpenGLShaders) {
		result = CreateOpenGLS(arg);
	}
#endif

#if defined(USE_OPENGL_GAME) && !defined(USE_GLES2)
	if (backendCapableOpenGL && matchingRendererType == Graphics::kRendererTypeOpenGL) {
		result = CreateOpenGL(arg);
	}
#endif

	if (matchingRendererType == Graphics::kRendererTypeTinyGL) {
		result = CreateTinyGL(arg);
	}

	if (!result) {
		error("Unable to create a '%s' renderer", rendererConfig.c_str());
	}

	return result;
}

/** @} */
#endif
