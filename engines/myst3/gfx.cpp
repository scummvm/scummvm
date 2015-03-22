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

#if defined(WIN32)
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE
#endif

#include "engines/myst3/gfx.h"

#include "graphics/surface.h"

#include "math/glmath.h"

namespace Myst3 {

BaseRenderer::BaseRenderer(OSystem *system)
		: _system(system), _font(NULL) { }

BaseRenderer::~BaseRenderer() {
}

void BaseRenderer::initFont(const Graphics::Surface *surface) {
	_font = createTexture(surface);
}

void BaseRenderer::freeFont() {
	if (_font) {
		freeTexture(_font);
		_font = nullptr;
	}
}

Common::Rect BaseRenderer::getFontCharacterRect(uint8 character) {
	uint index = 0;

	if (character == ' ')
		index = 0;
	else if (character >= '0' && character <= '9')
		index = 1 + character - '0';
	else if (character >= 'A' && character <= 'Z')
		index = 1 + 10 + character - 'A';
	else if (character == '|')
		index = 1 + 10 + 26;

	return Common::Rect(16 * index, 0, 16 * (index + 1), 32);
}

Common::Rect BaseRenderer::viewport() const {
	return _screenViewport;
}

Common::Rect BaseRenderer::frameViewport() const {
	Common::Rect screen = viewport();

	Common::Rect frame = Common::Rect(screen.width(), screen.height() * kFrameHeight / kOriginalHeight);
	frame.translate(screen.left, screen.top + screen.height() * kBottomBorderHeight / kOriginalHeight);

	return frame;
}

void BaseRenderer::computeScreenViewport() {
	int32 screenWidth = _system->getWidth();
	int32 screenHeight = _system->getHeight();

	if (_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection)) {
		// Aspect ratio correction
		int32 viewportWidth = MIN<int32>(screenWidth, screenHeight * kOriginalWidth / kOriginalHeight);
		int32 viewportHeight = MIN<int32>(screenHeight, screenWidth * kOriginalHeight / kOriginalWidth);
		_screenViewport = Common::Rect(viewportWidth, viewportHeight);

		// Pillarboxing
		_screenViewport.translate((screenWidth - viewportWidth) / 2,
			(screenHeight - viewportHeight) / 2);
	} else {
		// Aspect ratio correction disabled, just stretch
		_screenViewport = Common::Rect(screenWidth, screenHeight);
	}
}

Common::Point BaseRenderer::frameCenter() const {
	Common::Rect screen = viewport();
	Common::Rect frame = frameViewport();
	return Common::Point((frame.left + frame.right) / 2, screen.top + screen.bottom - (frame.top + frame.bottom) / 2);
}

Math::Matrix4 BaseRenderer::makeProjectionMatrix(float fov) const {
	static const float nearClipPlane = 1.0;
	static const float farClipPlane = 10000.0;

	float aspectRatio = kOriginalWidth / (float) kFrameHeight;

	float xmaxValue = nearClipPlane * tan(fov * M_PI / 360.0);
	float ymaxValue = xmaxValue / aspectRatio;

	return Math::makeFrustumMatrix(-xmaxValue, xmaxValue, -ymaxValue, ymaxValue, nearClipPlane, farClipPlane);
}

void BaseRenderer::flipVertical(Graphics::Surface *s) {
	for (int y = 0; y < s->h / 2; ++y) {
		// Flip the lines
		byte *line1P = (byte *)s->getBasePtr(0, y);
		byte *line2P = (byte *)s->getBasePtr(0, s->h - y - 1);

		for (int x = 0; x < s->pitch; ++x)
			SWAP(line1P[x], line2P[x]);
	}
}

} // End of namespace Myst3
