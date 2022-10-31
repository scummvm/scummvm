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

#include "backends/graphics/opengl/pipelines/pipeline.h"
#include "backends/graphics/opengl/framebuffer.h"

namespace OpenGL {

Pipeline *Pipeline::activePipeline = nullptr;

Pipeline::Pipeline()
	: _activeFramebuffer(nullptr) {
}

void Pipeline::activate() {
	if (activePipeline == this) {
		return;
	}

	if (activePipeline) {
		activePipeline->deactivate();
	}

	activePipeline = this;

	if (_activeFramebuffer) {
		_activeFramebuffer->activate(this);
	}

	activateInternal();
}

void Pipeline::deactivate() {
	assert(isActive());

	deactivateInternal();

	if (_activeFramebuffer) {
		_activeFramebuffer->deactivate();
	}

	activePipeline = nullptr;
}

Framebuffer *Pipeline::setFramebuffer(Framebuffer *framebuffer) {
	Framebuffer *oldFramebuffer = _activeFramebuffer;
	if (isActive() && oldFramebuffer) {
		oldFramebuffer->deactivate();
	}

	_activeFramebuffer = framebuffer;
	if (isActive() && _activeFramebuffer) {
		_activeFramebuffer->activate(this);
	}

	return oldFramebuffer;
}

} // End of namespace OpenGL
