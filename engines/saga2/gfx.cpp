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

#include "graphics/surface.h"

#include "saga2/saga2.h"

#include "saga2/gfx.h"

namespace Saga2 {

Renderer::Renderer() {
	for (int i = 0; i < kMaxBackBufferSources; ++i) {
		_savedBackBuffers[i] = nullptr;
	}
}

Renderer::~Renderer() {
	for (int i = 0; i < kMaxBackBufferSources; i++) {
		if (_savedBackBuffers[i]) {
			delete[] _savedBackBuffers[i];
			_savedBackBuffers[i] = nullptr;
		}
	}
}

void Renderer::saveBackBuffer(BackBufferSource source) {
	if (source >= 0 && source < kMaxBackBufferSources) {
		if (_savedBackBuffers[source])
			removeSavedBackBuffer(source);

		Graphics::Surface *surf = g_system->lockScreen();
		int size = surf->w * surf->h;

		_savedBackBuffers[source] = new byte[size];
		memcpy(_savedBackBuffers[source], surf->getPixels(), size);

		g_system->unlockScreen();
	}
}

void Renderer::popSavedBackBuffer(BackBufferSource source) {
	restoreSavedBackBuffer(source);
	removeSavedBackBuffer(source);
}

void Renderer::restoreSavedBackBuffer(BackBufferSource source) {
	if (source >= 0 && source < kMaxBackBufferSources) {
		if (_savedBackBuffers[source]) {
			Graphics::Surface *surf = g_system->lockScreen();
			int size = surf->w * surf->h;

			memcpy(surf->getBasePtr(0, 0), _savedBackBuffers[source], size);

			g_system->unlockScreen();
		}
	}
}

void Renderer::removeSavedBackBuffer(BackBufferSource source) {
	if (source >= 0 && source < kMaxBackBufferSources) {
		if (_savedBackBuffers[source])
			delete[] _savedBackBuffers[source];

		_savedBackBuffers[source] = nullptr;
	}
}

bool Renderer::hasSavedBackBuffer(BackBufferSource source) {
	return (source >= 0 && source < kMaxBackBufferSources) && _savedBackBuffers[source];
}

} // end of namespace Saga2
