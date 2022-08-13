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
#include "tetraedge/te/te_font3.h"
#include "tetraedge/te/te_intrusive_ptr.h"
#include "tetraedge/te/te_mesh.h"
#include "tetraedge/te/te_vector2s32.h"

namespace Tetraedge {

class TeTextBase2 {
public:
	TeTextBase2();

	struct  Line {
		unsigned int _startOffset;
		unsigned int _endOffset;
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

	TeColor currentColor(unsigned int offset) const;
	TeIntrusivePtr<TeFont3> currentFont(unsigned int offset);
	void draw();
	unsigned int endOfWord(unsigned int i) const;
	void insertNewLine(unsigned int offset);
	bool isASpace(unsigned int offset) const;
	int newLines(unsigned int offset) const;
	int nextNonSpaceChar(unsigned int start);
	void setAlignStyle(TeFont3::AlignStyle style);
	void setColor(unsigned int offset, const TeColor &color);
	void setFont(unsigned int offset, const TeIntrusivePtr<TeFont3> &newfont);
	void setFontSize(unsigned long fontSz);
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
	void computeNbSpaces(Line &line, unsigned int startOffset, unsigned int endOffset);
	void drawEmptyChar(unsigned int offset);
	void drawLine(TeImage &img, const Common::String &str, int yoffset);

	TeFont3::AlignStyle _alignStyle;
	WrapMode _wrapMode;
	unsigned long _fontSize;
	bool _valueWasSet;
	TeColor _globalColor;
	float _interLine;
	TeVector2s32 _drawRect;
	TeVector2s32 _size;
	Common::String _text;
	bool _strikethrough;

	TeMesh _mesh;

	Common::Array<Common::String> _wrappedLines;

	Common::Array<unsigned int> _lineBreaks;
	Common::HashMap<unsigned int, TeColor> _colors;
	Common::HashMap<unsigned int, TeIntrusivePtr<TeFont3>> _fonts;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_TEXT_BASE2_H
