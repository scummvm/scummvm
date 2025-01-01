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

#include "tetraedge/te/te_font2.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_3d_object2.h"
#include "tetraedge/te/te_core.h"

namespace Tetraedge {

bool operator==(const KernChars &l, const KernChars &r) {
	return l._c1 == r._c1 && l._c2 == r._c2;
}

TeFont2::TeFont2() : _numChars(0), _hasKernData(false), _maxHeight(0) {
}

TeFont2::~TeFont2() {
}

bool TeFont2::load(const Common::Path &path) {
	if (_loadedPath == path)
		return true; // already open

	TeCore *core = g_engine->getCore();
	TetraedgeFSNode node = core->findFile(path);
	return load(node);
}

bool TeFont2::load(const TetraedgeFSNode &node) {
	const Common::Path path = node.getPath();

	unload();
	setAccessName(path);
	_loadedPath = path;

	if (!node.exists()) {
		warning("TeFont2::load: Can't read from %s", node.toString().c_str());
		return false;
	}

	Common::ScopedPtr<Common::SeekableReadStream> file(node.createReadStream());

	if (!Te3DObject2::loadAndCheckFourCC(*file, "TESF")) {
		warning("TeFont2::load: Invalid magic in %s", node.toString().c_str());
		return false;
	}

	_numChars = file->readUint32LE();
	if (_numChars > 65535)
		error("TeFont2::load: improbable number of char points %d", _numChars);
	TeVector2s32::deserialize(*file, _somePt);
	TeVector3f32::deserialize(*file, _someVec);
	_hasKernData = (file->readByte() != 0);
	if (_hasKernData) {
		uint32 numKernData = file->readUint32LE();
		if (numKernData > 10000)
			error("TeFont2::load: improbable number of kerning points %d", numKernData);
		for (uint32 i = 0; i < numKernData; i++) {
			KernChars kc;
			TeVector3f32 vec;
			kc._c1 = file->readUint32LE();
			kc._c2 = file->readUint32LE();
			vec.x() = file->readFloatLE();
			vec.y() = file->readFloatLE();
			_kernData[kc] = vec;
			//debug("KernChars: '%c'-'%c' (%.2f, %.2f)", (char)kc._c1, (char)kc._c2, vec.x(), vec.y());
		}
	}

	for (uint32 i = 0; i < _numChars; i++) {
		GlyphData2 g;
		g._xSz = file->readFloatLE();
		g._ySz = file->readFloatLE();
		_maxHeight = MAX(_maxHeight, g._ySz);
		g._xOff = file->readFloatLE();
		g._yOff = file->readFloatLE();
		g._xAdvance = file->readFloatLE();
		// TODO: What are these other floats?
		for (uint j = 0; j < 3; j++)
			g._floats[j] = file->readFloatLE();
		g._vec.x() = file->readFloatLE();
		g._vec.y() = file->readFloatLE();
		_glyphs.push_back(g);
		uint32 charNo = file->readUint32LE();
		_uintArray.push_back(charNo);
		/*
		if (i >= 35 && i <= 127)
			debug("Char data %c: sz (%.1f %.1f) off (%.1f %.1f) xadv %.1f %.1f %.1f %.1f texloc (%.1f, %.1f) %d", (char)i,
				g._xSz, g._ySz, g._xOff, g._yOff, g._xAdvance, g._floats[0], g._floats[1], g._floats[2],
				g._vec.x(), g._vec.y(), charNo);
		*/
	}

	if (!_texture.load(*file, "png")) {
		warning("Invalid png data in %s", node.toString().c_str());
		return false;
	}

	return true;
}

void TeFont2::unload() {
	_texture.free();
	_uintArray.clear();
	_glyphs.clear();
	_kernData.clear();
	_numChars = 0;
}

int TeFont2::getFontHeight() const {
	return _maxHeight;
}

int TeFont2::getMaxCharWidth() const {
	int maxWidth = 0;
	for (auto &glyph : _glyphs) {
		maxWidth = MAX(maxWidth, (int)glyph._xAdvance);
	}
	return maxWidth;
}

int TeFont2::getCharWidth(uint32 chr) const {
	if (chr >= _glyphs.size())
		return 0;
	return (int)_glyphs[chr]._xAdvance;
}

void TeFont2::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	if (chr >= _glyphs.size())
		return;

	if (_texture.empty())
		error("Call to TeFont2::drawChar but no texture loaded");
	const GlyphData2 g = _glyphs[chr];

	if (!g._xSz || !g._ySz)
		return;

	Common::Rect srcRect;
	srcRect.left = (int)g._vec.x();
	srcRect.top = _texture.h - (int)g._vec.y() - g._ySz;
	srcRect.right = srcRect.left + g._xSz;
	srcRect.bottom = srcRect.top + g._ySz;

	int dstX = x + g._xOff;
	int dstY = _maxHeight - g._yOff;

	if (dstX + srcRect.width() > dst->w)
		srcRect.right = srcRect.left + (dst->w - dstX);
	if (dstY + srcRect.height() > dst->h)
		srcRect.bottom = srcRect.top + (dst->h - dstY);

	debug("TeFont2::drawChar %c (%d, %d) from (%d,%d-%d,%d)", chr, x, y,
			srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);

	dst->copyRectToSurface(_texture, dstX, dstY, srcRect);
}

int TeFont2::getKerningOffset(uint32 left, uint32 right) const {
	KernChars kc { left, right };
	if (_kernData.contains(kc))
		return (int)_kernData[kc].x();
	return 0;
}

TeVector3f32 TeFont2::kerning(uint pxSize, uint isocode1, uint isocode2) {
	KernChars kc { isocode1, isocode2 };
	if (_kernData.contains(kc))
		return _kernData[kc];
	return TeVector3f32();
}

Common::Rect TeFont2::getBBox(const Common::String &str, int fontSize) {
	Common::Rect rect;
	for (uint i = 0; i < str.size(); i++) {
		uint c = str[i];
		if (c >= _glyphs.size())
			continue;
		const GlyphData2 &g = _glyphs[c];
		rect.top = MIN(rect.top, (int16)-g._yOff);
		rect.bottom = MAX(rect.bottom, (int16)(-g._yOff + g._ySz));
		rect.right += g._xAdvance;
		if (i < str.size() - 1) {
			rect.right += kerning(fontSize, c, str[i+1]).x();
		}
	}
	return rect;
}

Common::Rect TeFont2::getBoundingBox(uint32 chr) const {
	if (chr > _glyphs.size())
		return Common::Rect();

	Common::Rect rect;
	rect.left = (int)_glyphs[chr]._xOff;
	rect.right = rect.left + _glyphs[chr]._xSz;
	rect.top = _maxHeight - _glyphs[chr]._yOff;
	rect.bottom = _maxHeight;
	return rect;
}

float TeFont2::height(uint pxSize) {
	return _maxHeight;
}

Graphics::Font *TeFont2::getAtSize(uint size) {
	return this;
}


} // end namespace Tetraedge
