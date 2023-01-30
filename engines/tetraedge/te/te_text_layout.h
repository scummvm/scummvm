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

#ifndef TETRAEDGE_TE_TE_TEXT_LAYOUT_H
#define TETRAEDGE_TE_TE_TEXT_LAYOUT_H

#include "tetraedge/te/te_layout.h"
#include "tetraedge/te/te_i_text_layout.h"
#include "tetraedge/te/te_text_base2.h"

namespace Tetraedge {

class TeTextLayout : public TeITextLayout, public TeLayout {
public:
	TeTextLayout();

	virtual ~TeTextLayout();

	void clear() {
		_base.clear();
	}

	void draw() override;
	void setText(const Common::String &val) override;
	void setInterLine(float val) override;
	void setWrapMode(TeTextBase2::WrapMode mode) override;
	void setTextSizeType(int type) override;
	void setTextSizeProportionalToWidth(int val) override;
	void strikethrough(bool val);
	bool strikethrough() const;
	const Common::String &text() const;
	const TeVector2s32 &textSize() const;
	int textSizeProportionalToWidth() const { return _textSizeProportionalToWidth; }
	int textSizeType() const { return _textSizeType; }
	void updateSize() override;
	TeTextBase2::WrapMode wrapMode() const { return _base.wrapMode(); }

private:
	int _textSizeType;
	int _textSizeProportionalToWidth;
	int _baseFontSize;

	TeTextBase2 _base;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_TEXT_LAYOUT_H
