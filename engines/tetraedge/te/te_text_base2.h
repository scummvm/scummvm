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

#ifndef TETRAEDGE_TE_TE_TEXT_BASE2_H
#define TETRAEDGE_TE_TE_TEXT_BASE2_H

#include "common/str.h"
#include "common/hashmap.h"

#include "tetraedge/te/te_color.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_i_font.h"
#include "tetraedge/te/te_mesh.h"
#include "tetraedge/te/te_vector2s32.h"

namespace Tetraedge {

class TeTextBase2 {
public:
	TeTextBase2();
	virtual ~TeTextBase2();

	struct  Line {
		uint _startOffset;
		uint _endOffset;
		float _height;
		float _width;
	};

	enum WrapMode {
		WrapModeFixed,
		WrapModeProportional
	};

	void build();
	void clear();
	void clearStyles();
	void clearText();

	TeColor currentColor(uint offset) const;
	TeIntrusivePtr<TeIFont> currentFont(uint offset);
	void draw();
	uint endOfWord(uint i) const;
	void insertNewLine(uint offset);
	bool isASpace(uint offset) const;
	int newLines(uint offset) const;
	int nextNonSpaceChar(uint start);
	void setAlignStyle(TeIFont::AlignStyle style);
	void setColor(uint offset, const TeColor &color);
	void setFont(uint offset, const TeIntrusivePtr<TeIFont> &newfont);
	void setFontSize(int fontSz);
	void setGlobalColor(const TeColor &color);
	void setInterLine(float val);
	void setRect(const TeVector2s32 &rect);
	void setText(const Common::String &newText);
	void setWrapMode(WrapMode &mode);
	TeVector2s32 size();
	void strikethrough(bool val);
	bool strikethrough() const { return _strikethrough; }
	const Common::String &text() const { return _text; }
	WrapMode wrapMode() const { return _wrapMode; }
	const TeVector2s32 &size() const { return _size; }

private:
	void computeNbSpaces(Line &line, uint startOffset, uint endOffset);
	void drawEmptyChar(uint offset);
	void drawLine(TeImage &img, const Common::String &str, int yoffset);

	TeIFont::AlignStyle _alignStyle;
	WrapMode _wrapMode;
	int _fontSize;
	bool _valueWasSet;
	TeColor _globalColor;
	float _interLine;
	TeVector2s32 _drawRect;
	TeVector2s32 _size;
	Common::String _text;
	bool _strikethrough;

	TeMesh *_mesh;

	Common::Array<Common::String> _wrappedLines;

	Common::Array<uint32> _lineBreaks;
	Common::HashMap<uint, TeColor> _colors;
	Common::HashMap<uint, TeIntrusivePtr<TeIFont>> _fonts;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_TEXT_BASE2_H
