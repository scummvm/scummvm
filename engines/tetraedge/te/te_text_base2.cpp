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

//#define TETRAEDGE_DUMP_RENDERED_FONTS

#ifdef TETRAEDGE_DUMP_RENDERED_FONTS
#include "image/png.h"
#endif

#include "tetraedge/te/te_text_base2.h"

namespace Tetraedge {

TeTextBase2::TeTextBase2() : _drawRect(0, 0), _size(0, 0),
_alignStyle(TeIFont::AlignLeft), _interLine(0.0f), _globalColor(0xff, 0xff, 0xff, 0xff),
_wrapMode(WrapModeFixed), _strikethrough(false), _fontSize(10), _valueWasSet(true) {
	_mesh = TeMesh::makeInstance();
	_mesh->setglTexEnvBlend();
	_mesh->setShouldDraw(true);
}

TeTextBase2::~TeTextBase2() {
	delete _mesh;
}

#ifdef TETRAEDGE_DUMP_RENDERED_FONTS
static int dumpCount = 0;
#endif

void TeTextBase2::build() {
	if (!_text.size() || !_fontSize)
		return;

	TeIntrusivePtr<TeIFont> font = _fonts[0];
	if (!font.get()) {
		warning("[TeTextBase2::build()] Warning : font missing");
		return;
	}

	_valueWasSet = false;
	_size = TeVector2s32(0, 0);
	_wrappedLines.clear();

	Common::Array<uint32> endPts = _lineBreaks;
	uint32 npos = Common::String::npos;
	endPts.push_back(npos);
	uint32 start = 0;
	for (uint32 end : endPts) {
		Common::Array<Common::String> lines;
		Common::String txt = _text.substr(start, end - start);
		if (txt.find(' ') != Common::String::npos)
			font->wordWrapText(txt, _fontSize, _drawRect._x, lines);
		else
			lines.push_back(txt);
		_wrappedLines.push_back(lines);
		start = end;
	}

	Common::Array<float> lineoffsets;
	float lineHeight = font->getHeight(_fontSize);
	float height = 0;
	for (const Common::String &line : _wrappedLines) {
		if (_alignStyle == TeIFont::AlignJustify) {
			warning("TODO: Implement TeTextBase2::computeNbSpaces for Justify");
			//computeNbSpaces(&line, offset, line.endOffset);
		}
		Common::Rect lineSize = font->getBBox(line, _fontSize);
		if (lineSize.right > _size._x)
			_size._x = lineSize.right;

		lineoffsets.push_back(height);
		height += lineHeight + _interLine;
	}

	// Round up to the nearest 2 pixels so it centres in the
	// area without a half pixel offset.
	_size._y = (((int)ceilf(height) + 1) / 2) * 2;
	_size._x = ((_size._x + 1) / 2) * 2;

	TeImage img;
	Common::SharedPtr<TePalette> nullpal;
	img.createImg(_size._x, _size._y, nullpal, TeImage::RGBA8);
	// fill with global color, alpha 0 so that the font anti-aliasing blends
	// to the right color (see eg, the cellphone display)
	img.fill(_globalColor.r(), _globalColor.g(), _globalColor.b(), 0);

	for (uint i = 0; i < _wrappedLines.size(); i++) {
		drawLine(img, _wrappedLines[i], lineoffsets[i]);
	}

	TeIntrusivePtr<Te3DTexture> texture = Te3DTexture::makeInstance();
	texture->load(img);

#ifdef TETRAEDGE_DUMP_RENDERED_FONTS
	Common::DumpFile dumpFile;
	dumpFile.open(Common::String::format("/tmp/rendered-font-dump-%04d.png", dumpCount));
	dumpCount++;
	Image::writePNG(dumpFile, img);
	dumpFile.close();
#endif

	_mesh->setConf(4, 4, TeMesh::MeshMode_TriangleStrip, 0, 0);
	_mesh->defaultMaterial(texture);
	_mesh->setglTexEnvBlend();
	_mesh->setShouldDraw(true);
	_mesh->setColor(_globalColor);
	_mesh->setVertex(0, TeVector3f32(_size._x * -0.5f, _size._y * -0.5f, 0.0f));
	_mesh->setTextureUV(0, TeVector2f32(0, 1));
	_mesh->setNormal(0, TeVector3f32(0.0f, 0.0f, 1.0f));
	_mesh->setColor(0, _globalColor);
	_mesh->setVertex(1, TeVector3f32(_size._x * 0.5f, _size._y * -0.5f, 0.0f));
	_mesh->setTextureUV(1, TeVector2f32(1, 1));
	_mesh->setNormal(1, TeVector3f32(0.0f, 0.0f, 1.0f));
	_mesh->setColor(1, _globalColor);
	_mesh->setVertex(2, TeVector3f32(_size._x * 0.5f, _size._y * 0.5f, 0.0f));
	_mesh->setTextureUV(2, TeVector2f32(1, 0));
	_mesh->setNormal(2, TeVector3f32(0.0f, 0.0f, 1.0f));
	_mesh->setColor(2, _globalColor);
	_mesh->setVertex(3, TeVector3f32(_size._x * -0.5f, _size._y * 0.5f, 0.0f));
	_mesh->setTextureUV(3, TeVector2f32(0, 0));
	_mesh->setNormal(3, TeVector3f32(0.0f, 0.0f, 1.0f));
	_mesh->setColor(3, _globalColor);
	_mesh->setIndex(0, 0);
	_mesh->setIndex(1, 1);
	_mesh->setIndex(2, 3);
	_mesh->setIndex(3, 2);
	_mesh->setHasAlpha(true);
}

void TeTextBase2::clear() {
	clearText();
	clearStyles();
	_valueWasSet = true;
}

void TeTextBase2::clearStyles() {
	_lineBreaks.clear();
	_fonts.clear();
	_colors.clear();
	_valueWasSet = true;
}

void TeTextBase2::clearText() {
	_text.clear();
	_valueWasSet = true;
}

void TeTextBase2::computeNbSpaces(Line &line, uint startOffset, uint endOffset) {
	// only needed if we implement Justify
	error("TODO: Implement TeTextBase2::computeNbSpaces");
}

TeColor TeTextBase2::currentColor(uint offset) const {
	if (_colors.size() == 0)
		return _globalColor;
	int closest_off = -1;
	TeColor result;
	// Find closest without going over.
	for (auto &pair : _colors) {
		if ((int)pair._key > closest_off && pair._key <= offset) {
			result = pair._value;
			closest_off = pair._key;
		}
	}
	if (closest_off == -1)
		return _globalColor;
	return result;
}

TeIntrusivePtr<TeIFont> TeTextBase2::currentFont(uint offset) {
	if (_fonts.size() == 0)
		return TeIntrusivePtr<TeIFont>();
	int closest_off = -1;
	TeIntrusivePtr<TeIFont> result;
	// Find closest without going over.
	for (auto &pair : _fonts) {
		if ((int)pair._key > closest_off && pair._key <= offset) {
			result = pair._value;
			closest_off = pair._key;
		}
	}
	if (closest_off == -1)
		return TeIntrusivePtr<TeIFont>();
	return result;
}

void TeTextBase2::draw() {
	if (_text.empty() || (_drawRect._x <= 0 && _wrapMode == WrapModeFixed))
		return;

	if (_valueWasSet)
		build();

	_mesh->draw();

	//if (_strikethrough)
	//	warning("TODO: Implement TeTextBase2::draw strikethrough support");
}

void TeTextBase2::drawEmptyChar(uint offset) {
	error("TODO: Implement TeTextBase2::drawEmptychar");
}

void TeTextBase2::drawLine(TeImage &img, const Common::String &str, int yoffset) {
	TeIntrusivePtr<TeIFont> font = _fonts[0];

	// Note: We draw this with black because the global color will be applied on
	// the mesh.
	font->draw(img, str, _fontSize, yoffset, TeColor(0, 0, 0, 255), _alignStyle);
}

uint TeTextBase2::endOfWord(uint offset) const {
	while (offset < _text.size() && !newLines(offset) && !isASpace(offset))
		offset++;
	return offset;
}

void TeTextBase2::insertNewLine(uint offset) {
	_lineBreaks.push_back(offset);
}

bool TeTextBase2::isASpace(uint offset) const {
	char c = _text[offset];
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

int TeTextBase2::newLines(uint offset) const {
	int result = 0;
	for (uint off : _lineBreaks) {
		if (off == offset)
			result++;
	}
	return result;
}

int TeTextBase2::nextNonSpaceChar(uint offset) {
	while (isASpace(offset))
		offset++;
	return offset; // TODO: or offset - 1?
}

void TeTextBase2::setAlignStyle(TeIFont::AlignStyle style) {
	_alignStyle = style;
	_valueWasSet = true;
}

void TeTextBase2::setColor(uint offset, const TeColor &color) {
	_colors.setVal(offset, color);
	_valueWasSet = true;
}

void TeTextBase2::setFont(uint offset, const TeIntrusivePtr<TeIFont> &newfont) {
	_fonts.setVal(offset, newfont);
	_valueWasSet = true;
}

void TeTextBase2::setFontSize(int newSize) {
	if (_fontSize != newSize) {
		_fontSize = newSize;
		_valueWasSet = true;
	}
}

void TeTextBase2::setGlobalColor(const TeColor &color) {
	_globalColor = color;
	_valueWasSet = true;
}

void TeTextBase2::setInterLine(float val) {
	_interLine = val;
	_valueWasSet = true;
}

void TeTextBase2::setRect(const TeVector2s32 &rect) {
	_drawRect = rect;
	_valueWasSet = true;
}

void TeTextBase2::setText(const Common::String &newText) {
	_valueWasSet = true;
	_text = newText;
	//int len = newText.size();
	//_mesh.setConf(len * 4, len * 6, TeMesh::MeshMode_Triangles, 1, len * 2);
}

void TeTextBase2::setWrapMode(TeTextBase2::WrapMode &mode) {
	_wrapMode = mode;
	_valueWasSet = true;
}

TeVector2s32 TeTextBase2::size() {
	if (_valueWasSet)
		build();
	return _size;
}

void TeTextBase2::strikethrough(bool val) {
	if (_strikethrough != val) {
		_strikethrough = val;
		_valueWasSet = true;
	}
	if (val) {
		warning("TODO: Implement TeTextBase2::draw strikethrough support");
	}
}


} // end namespace Tetraedge
