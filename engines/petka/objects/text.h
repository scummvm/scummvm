/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef PETKA_TEXT_H
#define PETKA_TEXT_H

#include "common/rect.h"
#include "common/ustr.h"
#include "common/str-array.h"

#include "graphics/font.h"

#include "petka/objects/object.h"

namespace Petka {

class QText : public QVisibleObject {
public:
	QText(const Common::U32String &text, uint16 textColor, uint16 outlineColor);

	void draw();
	void update(int time);
	const Common::Rect &getRect();

protected:
	QText();

	static void drawOutline(Graphics::Surface *surface, uint16 color);
	static Common::Rect calculateBoundingBoxForText(const Common::U32String &text, Graphics::Font &font);
	static void drawText(Graphics::Surface &s, int y, int maxWidth, const Common::U32String &text, uint color, Graphics::Font &font);

protected:
	Common::Rect _rect;
};

class QTextPhrase : public QText {
public:
	QTextPhrase(const Common::U32String &phrase, uint16 textColor, uint16 outlineColor);

	void draw() override;
	void update(int time) override;
	void onClick(Common::Point p) override;
	bool isInPoint(Common::Point p) override { return true; }

private:
	Common::U32String _phrase;
	uint _time;
};

class QTextDescription : public QText {
public:
	QTextDescription(const Common::U32String &desc, uint32 frame);

	void draw() override;
	void onClick(Common::Point p) override;
	bool isInPoint(Common::Point p) override { return true; }
	void update(int t) override {}
};

class QTextChoice : public QText {
public:
	QTextChoice(const Common::Array<Common::U32String> &choices, uint16 color, uint16 selectedColor);

	void onMouseMove(Common::Point p) override;
	void onClick(Common::Point p) override;
	bool isInPoint(Common::Point p) override { return true; }

private:
	Common::Array<Common::Rect> _rects;
	Common::Array<Common::U32String> _choices;
	uint _activeChoice;
	uint16 _choiceColor;
	uint16 _selectedColor;
};

} // End of namespace Petka

#endif
