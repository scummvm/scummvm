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

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_i_loc.h"
#include "tetraedge/te/te_resource_manager.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_text_layout.h"
#include "tetraedge/te/te_text_layout_xml_parser.h"
#include "tetraedge/te/te_font3.h"
#include "tetraedge/te/te_font2.h"

namespace Tetraedge {

TeTextLayout::TeTextLayout(): _textSizeProportionalToWidth(0),
_textSizeType(0), _baseFontSize(10) {
}

TeTextLayout::~TeTextLayout() {
}

void TeTextLayout::setInterLine(float val) {
	_base.setInterLine(val);
}

void TeTextLayout::draw() {
	if (!worldVisible())
		return;

	updateSize();
	TeMatrix4x4 matrix = worldTransformationMatrix();
	TeRenderer *renderer = g_engine->getRenderer();
	renderer->pushMatrix();
	renderer->loadMatrix(matrix);
	_base.draw();
	renderer->popMatrix();
	TeLayout::draw();

}

static TeIFont::AlignStyle _alignNameToEnum(const Common::String &name) {
	if (name == "left")
		return TeIFont::AlignLeft;
	else if (name == "right")
		return TeIFont::AlignRight;
	else if (name == "justify")
		return TeIFont::AlignJustify;
	else if (name =="center")
		return TeIFont::AlignCenter;
	warning("Unknown text align style: %s", name.c_str());
	return TeIFont::AlignLeft;
}

void TeTextLayout::setText(const Common::String &val) {
	if (!val.size()) {
		clear();
		_sizeChanged = true;
		return;
	}

	TeILoc *loc = g_engine->getCore()->loc();

	Common::String replaced = val;
	size_t bstart = replaced.find("$(");
	while (bstart != Common::String::npos) {
		size_t bend = replaced.find(")", bstart);
		if (bend == Common::String::npos)
			break;
		const Common::String *replacement = loc->text(replaced.substr(bstart + 2, bend - bstart - 2));
		if (replacement) {
			/* Workaround: Syberia 2 on Switch has strings
			   <Forward> and <Backwards> via replacement
			   in xml embed in lua. Escape < and >. */
			Common::String escaped = *replacement;
			Common::replace(escaped, "<", "&lt;");
			Common::replace(escaped, ">", "&gt;");
			replaced.replace(bstart, bend - bstart + 1, escaped);
		}
		bstart = replaced.find("$(", bstart + 1);
	}

	//
	// WORKAROUND: The Syberia credits xml has an unmatched "</t>" at the end..
	// just delete it.
	//
	// Note there is another workaround for the credits xml in the parser.
	//
	size_t tagstart = replaced.find("</t>");
	if (tagstart != Common::String::npos)
		replaced.replace(tagstart, 4, "    ");

	const Common::String xmlDocStr = Common::String::format("<?xml version=\"1.0\" encoding=\"UTF-8\"?><document>%s</document>", replaced.c_str());

	TeTextLayoutXmlParser parser;
	parser.setAllowText();

	if (!parser.loadBuffer((const byte *)(xmlDocStr.c_str()), xmlDocStr.size()))
		error("TeTextLayout::load: failed to load xml.");

	if (!parser.parse())
		error("TeTextLayout::load: failed to parse xml.");

	_base.setText(parser.textContent());
	_base.setGlobalColor(parser.color());
	if (parser.fontSize())
		_baseFontSize = parser.fontSize();

	if (!parser.fontFile().empty()) {
		Common::Path fontPath(parser.fontFile());
		TetraedgeFSNode fontNode = g_engine->getCore()->findFile(fontPath);
		TeIntrusivePtr<TeIFont> font;
		if (parser.fontFile().hasSuffixIgnoreCase(".ttf"))
			font = g_engine->getResourceManager()->getResource<TeFont3>(fontNode).get();
		else
			font = g_engine->getResourceManager()->getResource<TeFont2>(fontNode).get();
		_base.setFont(0, font);
	}
	if (parser.style().size())
		_base.setAlignStyle(_alignNameToEnum(parser.style()));
	for (uint offset : parser.lineBreaks())
		_base.insertNewLine(offset);
	_sizeChanged = true;
}

void TeTextLayout::setTextSizeType(int type) {
	if (_textSizeType != type) {
		_textSizeType = type;
		_sizeChanged = true;
	}
}

void TeTextLayout::setTextSizeProportionalToWidth(int val) {
	if (_textSizeProportionalToWidth != val) {
		_textSizeProportionalToWidth = val;
		_sizeChanged = true;
	}
}

void TeTextLayout::setWrapMode(TeTextBase2::WrapMode mode) {
	_base.setWrapMode(mode);
}

void TeTextLayout::strikethrough(bool val) {
	_base.strikethrough(val);
}

bool TeTextLayout::strikethrough() const {
	return _base.strikethrough();
}

const Common::String &TeTextLayout::text() const {
	return _base.text();
}

const TeVector2s32 &TeTextLayout::textSize() const {
	return _base.size();
}

void TeTextLayout::updateSize() {
	if (!_sizeChanged)
		return;

	TeLayout::updateSize();

	const TeMatrix4x4 transform = worldTransformationMatrix();

	const TeVector3f32 v1 = transform * TeVector3f32(0, 0, 0);
	const TeVector3f32 v2 = transform * TeVector3f32(1, 0, 0);
	const TeVector3f32 v3 = transform * TeVector3f32(0, 1, 0);

	const TeVector3f32 transformVec((v2 - v1).length(), (v3 - v1).length(), 1.0);
	const TeVector3f32 thisSize = size();
	const TeVector3f32 textSize = thisSize * transformVec;
	const TeVector2s32 textSizeI(textSize.x(), textSize.y());
	_base.setRect(textSizeI);

	float newFontSize = 0;
	if (_textSizeType == 0 || (_textSizeType == 1 && _textSizeProportionalToWidth == 0)) {
		newFontSize = _baseFontSize;
	} else if (_textSizeType == 1) {
		newFontSize = (thisSize.x() / _textSizeProportionalToWidth) * _baseFontSize;
	}

	newFontSize *= transformVec.y();

	_base.setFontSize(newFontSize);
	_base.build();

	TeVector3f32 userSz = userSize();
	const TeVector2s32 baseSz = _base.size();

	if (sizeType() == RELATIVE_TO_PARENT && parent()) {
		if (wrapMode() != TeTextBase2::WrapModeFixed) {
			if (parent()->xSize() != 0.0)
				userSz.x() = ((float)baseSz._x / transformVec.y()) / parent()->xSize();
		}
		if (parent()->ySize() != 0.0)
			userSz.y() = ((float)baseSz._y / transformVec.y()) / parent()->ySize();
	} else if (sizeType() == ABSOLUTE) {
		if (wrapMode() != TeTextBase2::WrapModeFixed) {
			userSz.x() = baseSz._x / transformVec.y();
		}
		userSz.y() = baseSz._y / transformVec.y();
	}

	setSize(userSz);
}

} // end namespace Tetraedge
