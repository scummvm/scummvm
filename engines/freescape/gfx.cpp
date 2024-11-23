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

#include "common/config-manager.h"
#include "common/system.h"

#include "gui/message.h"
#include "engines/util.h"

#if defined(USE_OPENGL_GAME) || defined(USE_OPENGL_SHADERS) || defined(USE_GLES2)
#include "graphics/opengl/context.h"
#endif

#include "freescape/gfx.h"
#include "freescape/objects/object.h"

namespace Freescape {

const Graphics::PixelFormat getRGBAPixelFormat() {
#ifdef SCUMM_BIG_ENDIAN
		return Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
		return Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
}

Renderer::Renderer(int screenW, int screenH, Common::RenderMode renderMode, bool authenticGraphics) {
	_screenW = screenW;
	_screenH = screenH;
	_currentPixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	_palettePixelFormat = Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0);
	_keyColor = -1;
	_inkColor = -1;
	_paperColor = -1;
	_underFireBackgroundColor = -1;
	_palette = nullptr;
	_colorMap = nullptr;
	_colorRemaps = nullptr;
	_renderMode = renderMode;
	_isAccelerated = false;
	_authenticGraphics = authenticGraphics;

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 128; j++) {
			_stipples[i][j] = 0;
		}
		_colorPair[i] = 0;
	}

	_scale = 1;
}

Renderer::~Renderer() {}

extern byte getCPCPixel(byte cpc_byte, int index, bool mode0);

byte getCPCStipple(byte cpc_byte, int back, int fore) {
	int c0 = getCPCPixel(cpc_byte, 0, true);
	assert(c0 == back || c0 == fore);
	int c1 = getCPCPixel(cpc_byte, 1, true);
	assert(c1 == back || c1 == fore);
	int c2 = getCPCPixel(cpc_byte, 2, true);
	assert(c2 == back || c2 == fore);
	int c3 = getCPCPixel(cpc_byte, 3, true);
	assert(c3 == back || c3 == fore);

	byte st = 0;
	if (c0 == fore)
		st = st | 0x3;

	if (c1 == fore)
		st = st | (0x3 << 2);

	if (c2 == fore)
		st = st | (0x3 << 4);

	if (c3 == fore)
		st = st | (0x3 << 6);

	return st;
}

byte getCGAPixel(byte x, int index) {
	if (index == 0)
		return (x >> 0) & 0x3;
	else if (index == 1)
		return (x >> 2) & 0x3;
	else if (index == 2)
		return (x >> 4) & 0x3;
	else if (index == 3)
		return (x >> 6) & 0x3;
	else
		error("Invalid index %d requested", index);
}

byte getCGAStipple(byte x, int back, int fore) {
	int c0 = getCGAPixel(x, 0);
	assert(c0 == back || c0 == fore || back == fore);
	int c1 = getCGAPixel(x, 1);
	assert(c1 == back || c1 == fore || back == fore);
	int c2 = getCGAPixel(x, 2);
	assert(c2 == back || c2 == fore || back == fore);
	int c3 = getCGAPixel(x, 3);
	assert(c3 == back || c3 == fore || back == fore);

	byte st = 0;
	if (c0 == fore)
		st = st | 0x3;

	if (c1 == fore)
		st = st | (0x3 << 2);

	if (c2 == fore)
		st = st | (0x3 << 4);

	if (c3 == fore)
		st = st |  (0x3 << 6);

	return st;
}

void Renderer::clearColorPairArray() {
	for (int i = 0; i < 16; i++)
		_colorPair[i] = 0;
}

void Renderer::fillColorPairArray() {
	for (int i = 4; i < 15; i++) {
		byte *entry = (*_colorMap)[i];
		int c1;
		if (_renderMode == Common::kRenderCGA)
			c1 = getCGAPixel(entry[0], 0);
		else if (_renderMode == Common::kRenderCPC)
			c1 = getCPCPixel(entry[0], 0, true);
		else
			error("Not implemented");

		int c2 = -1;

		for (int j = 0; j < 4; j++) {
			int k, c;
			for (k = 0; k < 4; k++) {
				if (_renderMode == Common::kRenderCGA)
					c = getCGAPixel(entry[j], k);
				else if (_renderMode == Common::kRenderCPC)
					c = getCPCPixel(entry[j], k, true);
				else
					error("Not implemented");
				if (c1 != c) {
					c2 = c;
					break;
				}
			}
			if (k != 4)
				break;
		}
		assert(c2 >= 0);
		assert((c1 < 16) & (c2 < 16));
		_colorPair[i] = byte(c1) | (byte(c2) << 4);
	}
}


uint16 duplicate_bits(uint8 byte) {
    uint16 result = 0;

    for (int i = 0; i < 8; i++) {
        // Extract the bit at position i
        uint8 bit = (byte >> i) & 1;
        // Duplicate the bit
        uint16 duplicated_bits = (bit << 1) | bit;
        // Position the duplicated bits in the appropriate place in the result
        result |= (duplicated_bits << (2 * i));
    }

    return result;
}


void Renderer::scaleStipplePattern(byte originalPattern[128], byte newPattern[128]) {
    // Initialize the new pattern to all 0
    memset(newPattern, 0, 128);

    for (int i = 0; i < 64; i++) {
		// Duplicate the bits of the original pattern
		uint16 duplicated_bits = duplicate_bits(originalPattern[i]);
		// Position the duplicated bits in the appropriate place in the new pattern
		newPattern[2 * i] = (duplicated_bits >> 8) & 0xff;
		newPattern[2 * i + 1] = duplicated_bits & 0xff;
	}
}

void Renderer::setColorMap(ColorMap *colorMap_) {
	_colorMap = colorMap_;
	if (_renderMode == Common::kRenderZX || _renderMode == Common::kRenderHercG) {
		for (int i = 0; i < 15; i++) {
			byte *entry = (*_colorMap)[i];
			for (int j = 0; j < 128; j++)
				_stipples[i][j] = entry[(j / 4) % 4];
		}
	} else if (_renderMode == Common::kRenderCPC) {
		fillColorPairArray();
		for (int i = 4; i < 15; i++) {
			byte pair = _colorPair[i];
			byte c1 = pair & 0xf;
			byte c2 = (pair >> 4) & 0xf;
			byte *entry = (*_colorMap)[i];
			for (int j = 0; j < 128; j++)
				_stipples[i][j] = getCPCStipple(entry[(j / 8) % 4], c1, c2) ;
		}
	} else if (_renderMode == Common::kRenderCGA) {
		fillColorPairArray();
		for (int i = 4; i < 15; i++) {
			byte pair = _colorPair[i];
			byte c1 = pair & 0xf;
			byte c2 = (pair >> 4) & 0xf;
			byte *entry = (*_colorMap)[i];
			for (int j = 0; j < 128; j++)
				_stipples[i][j] = getCGAStipple(entry[(j / 8) % 4], c1, c2) ;
		}
	}

	if (_isAccelerated && _authenticGraphics) {
		for (int i = 1; i <= 14; i++) {
			scaleStipplePattern(_stipples[i], _stipples[15]);
			memcpy(_stipples[i], _stipples[15], 128);
			scaleStipplePattern(_stipples[i], _stipples[15]);
			memcpy(_stipples[i], _stipples[15], 128);
		}
	}
}

void Renderer::readFromPalette(uint8 index, uint8 &r, uint8 &g, uint8 &b) {
	r = _palette[3 * index + 0];
	g = _palette[3 * index + 1];
	b = _palette[3 * index + 2];
}

uint8 Renderer::indexFromColor(uint8 r, uint8 g, uint8 b) {
	for (int i = 0; i < 16; i++) {
		if (r == _palette[3 * i + 0] && g == _palette[3 * i + 1] && b == _palette[3 * i + 2])
			return i;
	}
	warning("color %x %x %x not found", r, g, b);
	return 0;
}

void Renderer::setColorRemaps(ColorReMap *colorRemaps) {
	_colorRemaps = colorRemaps;

	if (_renderMode == Common::kRenderZX) {
		for (auto &it : *_colorRemaps) {
			if (it._key == 1)
				_paperColor = it._value;
			else if (it._key == 3)
				_inkColor = it._value;
		}
	}
}

bool Renderer::getRGBAtCGA(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2, byte *&stipple) {
	if (index == _keyColor)
		return false;

	assert (_renderMode == Common::kRenderCGA);
	if (index <= 4) { // Solid colors
		stipple = nullptr;
		readFromPalette(index - 1, r1, g1, b1);
		r2 = r1;
		g2 = g1;
		b2 = b1;
		return true;
	}

	stipple = (byte *)_stipples[index - 1];
	byte pair = _colorPair[index - 1];
	byte c1 = pair & 0xf;
	byte c2 = (pair >> 4) & 0xf;
	readFromPalette(c1, r1, g1, b1);
	readFromPalette(c2, r2, g2, b2);
	if (r1 == r2 && g1 == g2 && b1 == b2) {
		stipple = nullptr;
	}
	return true;
}


void Renderer::extractC64Indexes(uint8 cm1, uint8 cm2, uint8 &i1, uint8 &i2) {
	if (cm1 == 0xaa && cm2 == 0x5a) {
		i1 = 2;
		i2 = 3;
	} else if (cm1 == 0x4f && cm2 == 0x46) {
		i1 = 0;
		i2 = 2;
	} else if (cm1 == 0x56 && cm2 == 0x45) {
		i1 = 0;
		i2 = 1;
	} else if (cm1 == 0xa0 && cm2 == 0x55) {
		i1 = 1;
		i2 = 3;
	} else if (cm1 == 0x4c && cm2 == 0x54) {
		i1 = 1;
		i2 = 2;
	} else if (cm1 == 0x41 && cm2 == 0x52) {
		i1 = 0;
		i2 = 3;
// Covered by the default of i1 = 0, i2 = 0
#if 0
	} else if (cm1 == 0x5a && cm2 == 0xa5) {
		i1 = 0;
		i2 = 0;
	} else if (cm1 == 0xbb && cm2 == 0xee) {
		i1 = 0;
		i2 = 0;
	} else if (cm1 == 0x5f && cm2 == 0xaf) {
		i1 = 0;
		i2 = 0;
	} else if (cm1 == 0xfb && cm2 == 0xfe) {
		i1 = 0;
		i2 = 0;
#endif
	} else {
		i1 = 0;
		i2 = 0;
	}
}


bool Renderer::getRGBAtC64(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2) {
	if (index == _keyColor)
		return false;

	if (index <= 4) { // Solid colors
		selectColorFromFourColorPalette(index - 1, r1, g1, b1);
		r2 = r1;
		g2 = g1;
		b2 = b1;
		return true;
	}

	uint8 i1, i2;
	byte *entry = (*_colorMap)[index - 1];
	uint8 cm1 = *(entry);
	entry++;
	uint8 cm2 = *(entry);

	extractC64Indexes(cm1, cm2, i1, i2);
	selectColorFromFourColorPalette(i1, r1, g1, b1);
	selectColorFromFourColorPalette(i2, r2, g2, b2);
	return true;
}

bool Renderer::getRGBAtZX(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2, byte *&stipple) {
	if (index == _keyColor)
		return false;

	byte *entry = (*_colorMap)[index - 1];
	if (entry[0] == 0 && entry[1] == 0 && entry[2] == 0 && entry[3] == 0) {
		readFromPalette(_paperColor, r1, g1, b1);
		readFromPalette(_paperColor, r2, g2, b2);
		stipple = nullptr;
		return true;
	}

	if (entry[0] == 0xff && entry[1] == 0xff && entry[2] == 0xff && entry[3] == 0xff) {
		readFromPalette(_inkColor, r1, g1, b1);
		readFromPalette(_inkColor, r2, g2, b2);
		stipple = nullptr;
		return true;
	}

	stipple = (byte *)_stipples[index - 1];

	readFromPalette(_paperColor, r1, g1, b1);
	readFromPalette(_inkColor, r2, g2, b2);
	if (r1 == r2 && g1 == g2 && b1 == g2) {
		stipple = nullptr;
	}
	return true;
}

bool Renderer::getRGBAtHercules(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2, byte *&stipple) {
	if (index == _keyColor)
		return false;

	byte *entry = (*_colorMap)[index - 1];
	if (entry[0] == 0 && entry[1] == 0 && entry[2] == 0 && entry[3] == 0) {
		readFromPalette(0, r1, g1, b1);
		readFromPalette(0, r2, g2, b2);
		return true;
	}

	if (entry[0] == 0xff && entry[1] == 0xff && entry[2] == 0xff && entry[3] == 0xff) {
		readFromPalette(1, r1, g1, b1);
		readFromPalette(1, r2, g2, b2);
		return true;
	}

	stipple = (byte *)_stipples[index - 1];
	readFromPalette(0, r1, g1, b1);
	readFromPalette(1, r2, g2, b2);
	return true;
}


void Renderer::selectColorFromFourColorPalette(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1) {
	if (index == 0) {
		r1 = 0;
		g1 = 0;
		b1 = 0;
	} else if (index == 1) {
		readFromPalette(_underFireBackgroundColor, r1, g1, b1);
	} else if (index == 2) {
		readFromPalette(_paperColor, r1, g1, b1);
	} else if (index == 3) {
		readFromPalette(_inkColor, r1, g1, b1);
	} else
		error("Invalid color");
}

bool Renderer::getRGBAtCPC(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2, byte *&stipple) {
	if (index == _keyColor)
		return false;

	if (_colorRemaps && _colorRemaps->contains(index)) {
		index = (*_colorRemaps)[index];
		if (index == 0) {
			r1 = g1 = b1 = 0;
			r2 = r1;
			g2 = g1;
			b2 = b1;
			stipple = nullptr;
			return true;
		}
		readFromPalette(index, r1, g1, b1);
		r2 = r1;
		g2 = g1;
		b2 = b1;
		stipple = nullptr;
		return true;
	}

	assert (_renderMode == Common::kRenderCPC);
	if (index <= 4) { // Solid colors
		selectColorFromFourColorPalette(index - 1, r1, g1, b1);
		r2 = r1;
		g2 = g1;
		b2 = b1;
		stipple = nullptr;
		return true;
	}

	stipple = (byte *)_stipples[index - 1];
	byte *entry = (*_colorMap)[index - 1];
	uint8 i1 = getCPCPixel(entry[0], 0, true);
	uint8 i2 = getCPCPixel(entry[0], 1, true);
	selectColorFromFourColorPalette(i1, r1, g1, b1);
	selectColorFromFourColorPalette(i2, r2, g2, b2);
	if (r1 == r2 && g1 == g2 && b1 == b2) {
		stipple = nullptr;
	}
	return true;
}

uint8 Renderer::mapEGAColor(uint8 index) {
	byte *entry = (*_colorMap)[index - 1];
	uint8 color = 0;
	uint8 acc = 1;
	for (int i = 0; i < 4; i++) {
		byte be = *entry;
		assert (be == 0 || be == 0xff);
		if (be == 0xff)
			color = color + acc;

		acc = acc << 1;
		entry++;
	}
	assert(color < 16);
	return color;
}

bool Renderer::getRGBAtEGA(uint8 index, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2) {
	uint8 color;
	if (_colorPair[index] > 0) {
		color = mapEGAColor(_colorPair[index] & 0xf);
		readFromPalette(color, r1, g1, b1);
		color = mapEGAColor(_colorPair[index] >> 4);
		readFromPalette(color, r2, g2, b2);
	} else {
		color = mapEGAColor(index);

		if (_colorRemaps && _colorRemaps->contains(color)) {
			color = (*_colorRemaps)[color];
		}

		readFromPalette(color, r1, g1, b1);
		r2 = r1;
		g2 = g1;
		b2 = b1;
	}
	return true;
}

bool Renderer::getRGBAt(uint8 index, uint8 ecolor, uint8 &r1, uint8 &g1, uint8 &b1, uint8 &r2, uint8 &g2, uint8 &b2, byte *&stipple) {
	if (index == _keyColor && ecolor == 0)
		return false;

	if (index == 0 && ecolor == 0) {
		readFromPalette(0, r1, g1, b1);
		r2 = r1;
		g2 = g1;
		b2 = b1;
		return true;
	}

	if (_renderMode == Common::kRenderAmiga || _renderMode == Common::kRenderAtariST) {
		if (_colorPair[index] > 0) {
			int color = 0;
			color = _colorPair[index] & 0xf;
			readFromPalette(color, r1, g1, b1);
			color = _colorPair[index] >> 4;
			readFromPalette(color, r2, g2, b2);
			return true;
		} else if (_colorRemaps && _colorRemaps->contains(index)) {
			int color = (*_colorRemaps)[index];
			_texturePixelFormat.colorToRGB(color, r1, g1, b1);
		} else
			readFromPalette(index, r1, g1, b1);

		if (ecolor > 0)
			readFromPalette(ecolor, r2, g2, b2);
		else {
			r2 = r1;
			g2 = g1;
			b2 = b1;
		}

		return true;
	} else if (_renderMode == Common::kRenderEGA)
		return getRGBAtEGA(index, r1, g1, b1, r2, g2, b2);
	else if (_renderMode == Common::kRenderC64)
		return getRGBAtC64(index, r1, g1, b1, r2, g2, b2);
	else if (_renderMode == Common::kRenderCGA)
		return getRGBAtCGA(index, r1, g1, b1, r2, g2, b2, stipple);
	else if (_renderMode == Common::kRenderCPC)
		return getRGBAtCPC(index, r1, g1, b1, r2, g2, b2, stipple);
	else if (_renderMode == Common::kRenderZX)
		return getRGBAtZX(index, r1, g1, b1, r2, g2, b2, stipple);
	else if (_renderMode == Common::kRenderHercG)
		return getRGBAtHercules(index, r1, g1, b1, r2, g2, b2, stipple);


	error("Invalid or unsupported render mode");
}

void Renderer::flipVertical(Graphics::Surface *s) {
	for (int y = 0; y < s->h / 2; ++y) {
		// Flip the lines
		byte *line1P = (byte *)s->getBasePtr(0, y);
		byte *line2P = (byte *)s->getBasePtr(0, s->h - y - 1);

		for (int x = 0; x < s->pitch; ++x)
			SWAP(line1P[x], line2P[x]);
	}
}

Graphics::Surface *Renderer::convertImageFormatIfNecessary(Graphics::ManagedSurface *msurface) {
	if (!msurface)
		return nullptr;

	Graphics::Surface *surface = new Graphics::Surface();
	surface->copyFrom(msurface->rawSurface());
	byte *palette = (byte *)malloc(sizeof(byte) * 16 * 3);
	msurface->grabPalette(palette, 0, 16); // Maximum should be 16 colours
	surface->convertToInPlace(_texturePixelFormat, palette, 16);
	free(palette);
	return surface;
}

Common::Rect Renderer::viewport() const {
	return _screenViewport;
}

bool Renderer::computeScreenViewport() {
	int32 screenWidth = g_system->getWidth();
	int32 screenHeight = g_system->getHeight();

	Common::Rect viewport;
	if (g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection)) {
			// Aspect ratio correction
			int32 viewportWidth = MIN<int32>(screenWidth, screenHeight * float(4) / 3);
			int32 viewportHeight = MIN<int32>(screenHeight, screenWidth * float(3) / 3);
			viewport = Common::Rect(viewportWidth, viewportHeight);

			// Pillarboxing
			viewport.translate((screenWidth - viewportWidth) / 2,
				(screenHeight - viewportHeight) / 2);
	} else {
			// Aspect ratio correction disabled, just stretch
			viewport = Common::Rect(screenWidth, screenHeight);
	}

	if (viewport == _screenViewport) {
		return false;
	}

	_screenViewport = viewport;
	return true;
}

void Renderer::renderPyramid(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<float> *ordinates, Common::Array<uint8> *colours, Common::Array<uint8> *ecolours, int type) {
	Math::Vector3d vertices[8] = { origin, origin, origin, origin, origin, origin, origin, origin };
	switch (type) {
	default:
		error("Invalid pyramid type: %d", type);
	case kEastPyramidType:
		vertices[0] += Math::Vector3d(0, 0, size.z());
		vertices[1] += Math::Vector3d(0, size.y(), size.z());
		vertices[2] += Math::Vector3d(0, size.y(), 0);

		vertices[4] += Math::Vector3d(size.x(), (*ordinates)[0], (*ordinates)[3]);
		vertices[5] += Math::Vector3d(size.x(), (*ordinates)[2], (*ordinates)[3]);
		vertices[6] += Math::Vector3d(size.x(), (*ordinates)[2], (*ordinates)[1]);
		vertices[7] += Math::Vector3d(size.x(), (*ordinates)[0], (*ordinates)[1]);
		break;
	case kWestPyramidType:

		vertices[0] += Math::Vector3d(size.x(), 0, 0);
		vertices[1] += Math::Vector3d(size.x(), size.y(), 0);
		vertices[2] += Math::Vector3d(size.x(), size.y(), size.z());
		vertices[3] += Math::Vector3d(size.x(), 0, size.z());

		vertices[4] += Math::Vector3d(0, (*ordinates)[0], (*ordinates)[1]);
		vertices[5] += Math::Vector3d(0, (*ordinates)[2], (*ordinates)[1]);
		vertices[6] += Math::Vector3d(0, (*ordinates)[2], (*ordinates)[3]);
		vertices[7] += Math::Vector3d(0, (*ordinates)[0], (*ordinates)[3]);
		break;

	case kUpPyramidType:
		vertices[1] += Math::Vector3d(size.x(), 0, 0);
		vertices[2] += Math::Vector3d(size.x(), 0, size.z());
		vertices[3] += Math::Vector3d(0, 0, size.z());

		vertices[4] += Math::Vector3d((*ordinates)[0], size.y(), (*ordinates)[1]);
		vertices[5] += Math::Vector3d((*ordinates)[2], size.y(), (*ordinates)[1]);
		vertices[6] += Math::Vector3d((*ordinates)[2], size.y(), (*ordinates)[3]);
		vertices[7] += Math::Vector3d((*ordinates)[0], size.y(), (*ordinates)[3]);
		break;

	case kDownPyramidType:

		vertices[0] += Math::Vector3d(size.x(), size.y(), 0);
		vertices[1] += Math::Vector3d(0, size.y(), 0);
		vertices[2] += Math::Vector3d(0, size.y(), size.z());
		vertices[3] += Math::Vector3d(size.x(), size.y(), size.z());

		vertices[4] += Math::Vector3d((*ordinates)[2], 0, (*ordinates)[1]);
		vertices[5] += Math::Vector3d((*ordinates)[0], 0, (*ordinates)[1]);
		vertices[6] += Math::Vector3d((*ordinates)[0], 0, (*ordinates)[3]);
		vertices[7] += Math::Vector3d((*ordinates)[2], 0, (*ordinates)[3]);
		break;

	case kNorthPyramidType:
		vertices[0] += Math::Vector3d(0, size.y(), 0);
		vertices[1] += Math::Vector3d(size.x(), size.y(), 0);
		vertices[2] += Math::Vector3d(size.x(), 0, 0);

		vertices[4] += Math::Vector3d((*ordinates)[0], (*ordinates)[3], size.z());
		vertices[5] += Math::Vector3d((*ordinates)[2], (*ordinates)[3], size.z());
		vertices[6] += Math::Vector3d((*ordinates)[2], (*ordinates)[1], size.z());
		vertices[7] += Math::Vector3d((*ordinates)[0], (*ordinates)[1], size.z());
		break;
	case kSouthPyramidType:
		vertices[0] += Math::Vector3d(0, 0, size.z());
		vertices[1] += Math::Vector3d(size.x(), 0, size.z());
		vertices[2] += Math::Vector3d(size.x(), size.y(), size.z());

		vertices[3] += Math::Vector3d(0, size.y(), size.z());
		vertices[4] += Math::Vector3d((*ordinates)[0], (*ordinates)[1], 0);
		vertices[5] += Math::Vector3d((*ordinates)[2], (*ordinates)[1], 0);
		vertices[6] += Math::Vector3d((*ordinates)[2], (*ordinates)[3], 0);
		vertices[7] += Math::Vector3d((*ordinates)[0], (*ordinates)[3], 0);
		break;
	}

	Common::Array<Math::Vector3d> face;
	byte *stipple = nullptr;
	uint8 r1, g1, b1, r2, g2, b2;
	uint color = (*colours)[0];
	uint ecolor = ecolours ? (*ecolours)[0] : 0;

	if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
		setStippleData(stipple);
		useColor(r1, g1, b1);

		face.push_back(vertices[4]);
		face.push_back(vertices[5]);
		face.push_back(vertices[1]);
		face.push_back(vertices[0]);

		renderFace(face);

		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(face);
			useStipple(false);
		}

		face.clear();
	}

	color = (*colours)[1];
	ecolor = ecolours ? (*ecolours)[1] : 0;

	if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
		setStippleData(stipple);
		useColor(r1, g1, b1);

		face.push_back(vertices[5]);
		face.push_back(vertices[6]);
		face.push_back(vertices[2]);
		face.push_back(vertices[1]);

		renderFace(face);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(face);
			useStipple(false);
		}

		face.clear();
	}
	color = (*colours)[2];
	ecolor = ecolours ? (*ecolours)[2] : 0;

	if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
		setStippleData(stipple);
		useColor(r1, g1, b1);

		face.push_back(vertices[6]);
		face.push_back(vertices[7]);
		face.push_back(vertices[3]);
		face.push_back(vertices[2]);
		renderFace(face);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(face);
			useStipple(false);
		}

		face.clear();
	}

	color = (*colours)[3];
	ecolor = ecolours ? (*ecolours)[3] : 0;

	if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
		setStippleData(stipple);
		useColor(r1, g1, b1);

		face.push_back(vertices[7]);
		face.push_back(vertices[4]);
		face.push_back(vertices[0]);
		face.push_back(vertices[3]);

		renderFace(face);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(face);
			useStipple(false);
		}

		face.clear();
	}

	color = (*colours)[4];
	ecolor = ecolours ? (*ecolours)[4] : 0;

	if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
		setStippleData(stipple);
		useColor(r1, g1, b1);

		face.push_back(vertices[0]);
		face.push_back(vertices[1]);
		face.push_back(vertices[2]);
		face.push_back(vertices[3]);
		renderFace(face);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(face);
			useStipple(false);
		}

		face.clear();
	}

	color = (*colours)[5];
	ecolor = ecolours ? (*ecolours)[5] : 0;

	if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
		setStippleData(stipple);
		useColor(r1, g1, b1);

		face.push_back(vertices[7]);
		face.push_back(vertices[6]);
		face.push_back(vertices[5]);
		face.push_back(vertices[4]);
		renderFace(face);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(face);
			useStipple(false);
		}
	}
}

void Renderer::renderCube(const Math::Vector3d &originalOrigin, const Math::Vector3d &size, Common::Array<uint8> *colours, Common::Array<uint8> *ecolours, float offset) {
	Math::Vector3d origin = originalOrigin;

	byte *stipple = nullptr;
	uint8 r1, g1, b1, r2, g2, b2;
	Common::Array<Math::Vector3d> face;

	uint color = (*colours)[0];
	uint ecolor = ecolours ? (*ecolours)[0] : 0;

	if (size.x() <= 1) {
		origin.x() += offset;
	} else if (size.y() <= 1) {
		origin.y() += offset;
	} else if (size.z() <= 1) {
		origin.z() += offset;
	}

	if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
		setStippleData(stipple);
		useColor(r1, g1, b1);
		face.push_back(origin);
		face.push_back(Math::Vector3d(origin.x(), origin.y(), origin.z() + size.z()));
		face.push_back(Math::Vector3d(origin.x(), origin.y() + size.y(), origin.z() + size.z()));
		face.push_back(Math::Vector3d(origin.x(), origin.y() + size.y(), origin.z()));
		renderFace(face);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(face);
			useStipple(false);
		}
	}

	color = (*colours)[1];
	ecolor = ecolours ? (*ecolours)[1] : 0;

	if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
		setStippleData(stipple);
		useColor(r1, g1, b1);
		face.clear();
		face.push_back(Math::Vector3d(origin.x() + size.x(), origin.y() + size.y(), origin.z()));
		face.push_back(Math::Vector3d(origin.x() + size.x(), origin.y() + size.y(), origin.z() + size.z()));
		face.push_back(Math::Vector3d(origin.x() + size.x(), origin.y(), origin.z() + size.z()));
		face.push_back(Math::Vector3d(origin.x() + size.x(), origin.y(), origin.z()));
		renderFace(face);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(face);
			useStipple(false);
		}
	}

	color = (*colours)[2];
	ecolor = ecolours ? (*ecolours)[2] : 0;

	if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
		setStippleData(stipple);
		useColor(r1, g1, b1);
		face.clear();
		face.push_back(Math::Vector3d(origin.x() + size.x(), origin.y(), origin.z()));
		face.push_back(Math::Vector3d(origin.x() + size.x(), origin.y(), origin.z() + size.z()));
		face.push_back(Math::Vector3d(origin.x(), origin.y(), origin.z() + size.z()));
		face.push_back(Math::Vector3d(origin.x(), origin.y(), origin.z()));
		renderFace(face);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(face);
			useStipple(false);
		}
	}

	color = (*colours)[3];
	ecolor = ecolours ? (*ecolours)[3] : 0;

	if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
		setStippleData(stipple);
		useColor(r1, g1, b1);
		face.clear();
		face.push_back(Math::Vector3d(origin.x(), origin.y() + size.y(), origin.z()));
		face.push_back(Math::Vector3d(origin.x(), origin.y() + size.y(), origin.z() + size.z()));
		face.push_back(Math::Vector3d(origin.x() + size.x(), origin.y() + size.y(), origin.z() + size.z()));
		face.push_back(Math::Vector3d(origin.x() + size.x(), origin.y() + size.y(), origin.z()));
		renderFace(face);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(face);
			useStipple(false);
		}
	}

	color = (*colours)[4];
	ecolor = ecolours ? (*ecolours)[4] : 0;

	if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
		setStippleData(stipple);
		useColor(r1, g1, b1);
		face.clear();
		face.push_back(Math::Vector3d(origin.x(), origin.y() + size.y(), origin.z()));
		face.push_back(Math::Vector3d(origin.x() + size.x(), origin.y() + size.y(), origin.z()));
		face.push_back(Math::Vector3d(origin.x() + size.x(), origin.y(), origin.z()));
		face.push_back(origin);
		renderFace(face);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(face);
			useStipple(false);
		}
	}

	color = (*colours)[5];
	ecolor = ecolours ? (*ecolours)[5] : 0;

	if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
		setStippleData(stipple);
		useColor(r1, g1, b1);
		face.clear();
		face.push_back(Math::Vector3d(origin.x(), origin.y(), origin.z() + size.z()));
		face.push_back(Math::Vector3d(origin.x() + size.x(), origin.y(), origin.z() + size.z()));
		face.push_back(Math::Vector3d(origin.x() + size.x(), origin.y() + size.y(), origin.z() + size.z()));
		face.push_back(Math::Vector3d(origin.x(), origin.y() + size.y(), origin.z() + size.z()));
		renderFace(face);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(face);
			useStipple(false);
		}
	}
}

void Renderer::renderRectangle(const Math::Vector3d &originalOrigin, const Math::Vector3d &originalSize, Common::Array<uint8> *colours, Common::Array<uint8> *ecolours, float offset) {

	Math::Vector3d size = originalSize;
	Math::Vector3d origin = originalOrigin;

	if (!_isAccelerated)
		polygonOffset(true);

	if (size.x() > 0 && size.y() > 0 && size.z() > 0) {
		/* According to https://www.shdon.com/freescape/
		If the bounding box is has all non-zero dimensions
		and is thus a cube, the rectangle is rendered as a
		slope at an angle with the plane of the polygon being
		parallel to the X axis (its lower edge extends from
		the base corner along the positive X direction).
		In that case, when the player is at a Z coordinate
		greater than (i.e. north of) the base corner,
		it is rendered in the front face material, otherwise it
		is rendered in the back face material. This implies
		that the engine does its material selection as though
		it were a rectangle perpendicular to the Z axis.
		TODO: fix this case.
		*/
		if (size.x() <= size.y() && size.x() <= size.z())
			size.x() = 0;
		else if (size.y() <= size.x() && size.y() <= size.z())
			size.y() = 0;
		else if (size.z() <= size.x() && size.z() <= size.y())
			size.z() = 0;
		else
			error("Invalid size!");
	}

	float dx, dy, dz;
	uint8 r1, g1, b1, r2, g2, b2;
	byte *stipple = nullptr;
	Common::Array<Math::Vector3d> vertices;
	uint color = 0;
	uint ecolor = 0;

	if (size.x() == 0) {
		origin.x() += offset;
	} else if (size.y() == 0) {
		origin.y() += offset;
	} else if (size.z() == 0) {
		origin.z() += offset;
	}

	for (int i = 0; i < 2; i++) {

		color = (*colours)[i];
		ecolor = ecolours ? (*ecolours)[i] : 0;

		if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
			setStippleData(stipple);
			useColor(r1, g1, b1);
			vertices.clear();
			vertices.push_back(Math::Vector3d(origin.x(), origin.y(), origin.z()));

			dx = dy = dz = 0.0;
			if (size.x() == 0) {
				dy = size.y();
			} else if (size.y() == 0) {
				dx = size.x();
			} else if (size.z() == 0) {
				dx = size.x();
			}

			vertices.push_back(Math::Vector3d(origin.x() + dx, origin.y() + dy, origin.z() + dz));
			vertices.push_back(Math::Vector3d(origin.x() + size.x(), origin.y() + size.y(), origin.z() + size.z()));
			vertices.push_back(Math::Vector3d(origin.x(), origin.y(), origin.z()));

			dx = dy = dz = 0.0;
			if (size.x() == 0) {
				dz = size.z();
			} else if (size.y() == 0) {
				dz = size.z();
			} else if (size.z() == 0) {
				dy = size.y();
			}

			vertices.push_back(Math::Vector3d(origin.x() + dx, origin.y() + dy, origin.z() + dz));
			vertices.push_back(Math::Vector3d(origin.x() + size.x(), origin.y() + size.y(), origin.z() + size.z()));
			renderFace(vertices);
			if (r1 != r2 || g1 != g2 || b1 != b2) {
				useStipple(true);
				useColor(r2, g2, b2);
				renderFace(vertices);
				useStipple(false);
			}
		}
	}
	polygonOffset(false);
}

void Renderer::renderPolygon(const Math::Vector3d &origin, const Math::Vector3d &size, const Common::Array<float> *originalOrdinates, Common::Array<uint8> *colours, Common::Array<uint8> *ecolours, float offset) {
	Common::Array<float> *ordinates = new Common::Array<float>(*originalOrdinates);

	uint8 r1, g1, b1, r2, g2, b2;
	byte *stipple = nullptr;
	if (ordinates->size() % 3 > 0 && ordinates->size() > 0)
		error("Invalid polygon with size %f %f %f and ordinates %d", size.x(), size.y(), size.z(), ordinates->size());

	Common::Array<Math::Vector3d> vertices;

	uint color = 0;
	uint ecolor = 0;

	if (ordinates->size() == 6) { // Line
		polygonOffset(true);
		color = (*colours)[0];
		ecolor = ecolours ? (*ecolours)[0] : 0;

		assert(getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)); // It will never return false?
		setStippleData(stipple);
		useColor(r1, g1, b1);
		for (uint i = 0; i < ordinates->size(); i = i + 3)
			vertices.push_back(Math::Vector3d((*ordinates)[i], (*ordinates)[i + 1], (*ordinates)[i + 2]));
		renderFace(vertices);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(vertices);
			useStipple(false);
		}

		vertices.clear();
		color = (*colours)[1];
		ecolor = ecolours ? (*ecolours)[1] : 0;

		assert(getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)); // It will never return false?
		setStippleData(stipple);
		useColor(r1, g1, b1);
		for (int i = ordinates->size(); i > 0; i = i - 3)
			vertices.push_back(Math::Vector3d((*ordinates)[i - 3], (*ordinates)[i - 2], (*ordinates)[i - 1]));
		renderFace(vertices);
		if (r1 != r2 || g1 != g2 || b1 != b2) {
			useStipple(true);
			useColor(r2, g2, b2);
			renderFace(vertices);
			useStipple(false);
		}
		polygonOffset(false);
	} else {

		if (!_isAccelerated)
			polygonOffset(true);

		if (size.x() == 0) {
			for (int i = 0; i < int(ordinates->size()); i++) {
				if (i % 3 == 0)
					(*ordinates)[i] += (offset);
			}
		} else if (size.y() == 0) {
			for (int i = 0; i < int(ordinates->size()); i++) {
				if (i % 3 == 1)
					(*ordinates)[i] += (offset);
			}
		} else if (size.z() == 0) {
			for (int i = 0; i < int(ordinates->size()); i++) {
				if (i % 3 == 2)
					(*ordinates)[i] += (offset);
			}
		}

		color = (*colours)[0];
		ecolor = ecolours ? (*ecolours)[0] : 0;

		if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
			setStippleData(stipple);
			useColor(r1, g1, b1);
			for (uint i = 0; i < ordinates->size(); i = i + 3) {
				vertices.push_back(Math::Vector3d((*ordinates)[i], (*ordinates)[i + 1], (*ordinates)[i + 2]));
			}
			renderFace(vertices);
			if (r1 != r2 || g1 != g2 || b1 != b2) {
				useStipple(true);
				useColor(r2, g2, b2);
				renderFace(vertices);
				useStipple(false);
			}
		}
		vertices.clear();
		color = (*colours)[1];
		ecolor = ecolours ? (*ecolours)[1] : 0;

		if (getRGBAt(color, ecolor, r1, g1, b1, r2, g2, b2, stipple)) {
			setStippleData(stipple);
			useColor(r1, g1, b1);
			for (int i = ordinates->size(); i > 0; i = i - 3) {
				vertices.push_back(Math::Vector3d((*ordinates)[i - 3], (*ordinates)[i - 2], (*ordinates)[i - 1]));
			}
			renderFace(vertices);
			if (r1 != r2 || g1 != g2 || b1 != b2) {
				useStipple(true);
				useColor(r2, g2, b2);
				renderFace(vertices);
				useStipple(false);
			}
		}
	}

	polygonOffset(false);
	delete(ordinates);
}

void Renderer::drawBackground(uint8 color) {
	uint8 r1, g1, b1;
	uint8 r2, g2, b2;

	if (_colorRemaps && _colorRemaps->contains(color)) {
		color = (*_colorRemaps)[color];
		readFromPalette(color, r1, g1, b1);
		clear(r1, g1, b1);
		return;
	}

	if (color == 0) {
		clear(0, 0, 0);
		return;
	}

	byte *stipple = nullptr;

	getRGBAt(color, 0, r1, g1, b1, r2, g2, b2, stipple);
	clear(r1, g1, b1);
}

void Renderer::drawEclipse(byte color1, byte color2, float progress) {
	Math::Vector3d sunPosition(-5000, 1200, 0);
	float radius = 400.0;
	drawCelestialBody(sunPosition, radius, color1);
	Math::Vector3d moonPosition(-5000, 1200, 800 * progress);
	drawCelestialBody(moonPosition, radius, color2);
}

Graphics::RendererType determinateRenderType() {
	Common::String rendererConfig = ConfMan.get("renderer");
	Graphics::RendererType desiredRendererType = Graphics::Renderer::parseTypeCode(rendererConfig);
	Graphics::RendererType matchingRendererType = Graphics::Renderer::getBestMatchingAvailableType(desiredRendererType,
#if defined(USE_OPENGL_GAME)
						Graphics::kRendererTypeOpenGL |
#endif
#if defined(USE_OPENGL_SHADERS)
			Graphics::kRendererTypeOpenGLShaders |
#endif
#if defined(USE_TINYGL)
						Graphics::kRendererTypeTinyGL |
#endif
						0);

	if (matchingRendererType != desiredRendererType && desiredRendererType != Graphics::kRendererTypeDefault) {
		// Display a warning if unable to use the desired renderer
		warning("Unable to create a '%s' renderer", rendererConfig.c_str());
	}

	#if defined(USE_OPENGL_GAME) && !defined(USE_GLES2)
		if (matchingRendererType == Graphics::kRendererTypeOpenGL)
			return matchingRendererType;
	#endif

	#if defined(USE_OPENGL_SHADERS)
		if (matchingRendererType == Graphics::kRendererTypeOpenGLShaders)
			return matchingRendererType;
	#endif

	#if defined(USE_TINYGL)
		return Graphics::kRendererTypeTinyGL;
	#endif

	return Graphics::kRendererTypeDefault;
}

Renderer *createRenderer(int screenW, int screenH, Common::RenderMode renderMode, bool authenticGraphics) {
	Graphics::PixelFormat pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	Graphics::RendererType rendererType = determinateRenderType();

	bool isAccelerated = rendererType != Graphics::kRendererTypeTinyGL;

	if (isAccelerated) {
		initGraphics3d(screenW, screenH);
	} else {
		initGraphics(screenW, screenH, &pixelFormat);
	}

	#if defined(USE_OPENGL_GAME) && !defined(USE_GLES2)
		if (rendererType == Graphics::kRendererTypeOpenGL) {
			return CreateGfxOpenGL(screenW, screenH, renderMode, authenticGraphics);
		}
	#endif

	#if defined(USE_OPENGL_SHADERS)
		if (rendererType == Graphics::kRendererTypeOpenGLShaders) {
			return CreateGfxOpenGLShader(screenW, screenH, renderMode, authenticGraphics);
		}
	#endif

	#if defined(USE_TINYGL)
	if (rendererType == Graphics::kRendererTypeTinyGL) {
		// TinyGL graphics are always authentic
		return CreateGfxTinyGL(screenW, screenH, renderMode);
	}
	#endif

	GUI::MessageDialog dialog("No available renderers enabled");
	// TODO: improve message with other renders
	dialog.runModal();
	return nullptr;
}

} // End of namespace Freescape
