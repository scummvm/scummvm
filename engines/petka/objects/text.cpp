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

#include "graphics/fonts/ttf.h"
#include "graphics/fontman.h"

#include "petka/q_manager.h"
#include "petka/petka.h"
#include "petka/video.h"
#include "petka/objects/text.h"
#include "petka/q_system.h"
#include "petka/interfaces/panel.h"
#include "petka/sound.h"
#include "petka/interfaces/main.h"
#include "petka/flc.h"

namespace Petka {

QText::QText(const Common::U32String &text, uint16 textColor, uint16 outlineColor) {
	_resourceId = -2;
	_z = 3000;

	auto *font = g_vm->getTextFont();

	Common::Rect rect = calculateBoundingBoxForText(text, *font);
	rect.right += 10;
	rect.bottom += 4;

	_rect = Common::Rect((640 - rect.width()) / 2, 479 - rect.height(), 639 - (640 - rect.width()) / 2, 479);
	Graphics::Surface *s = g_vm->resMgr()->getSurface(-2, rect.width(), rect.height());

	drawText(*s, 0, 630, text, textColor, *font, Graphics::kTextAlignCenter);
	drawOutline(s, outlineColor);
}

void QText::draw() {
	const Graphics::Surface *s = g_vm->resMgr()->getSurface(-2);
	if (s) {
		g_vm->videoSystem()->transBlitFrom(*s, Common::Point((640 - s->w) / 2, 479 - s->h));
	}
}

const Common::Rect &QText::getRect() {
	return _rect;
}

void QText::drawOutline(Graphics::Surface *s, uint16 color) {
	for (int y = 0; y < s->h; ++y) {
		for (int x = 1; x < s->w - 1; ++x) {
			uint16 *pixel = (uint16 *)s->getBasePtr(x, y);
			if (*pixel && *pixel != color) {
				if (!pixel[-1])
					pixel[-1] = (uint16)color;
				if (!pixel[1])
					pixel[1] = (uint16)color;
			}
		}
	}

	for (int x = 0; x < s->w; ++x) {
		for (int y = 0; y < s->h - 1; ++y) {
			uint16 *pixel = (uint16 *)s->getBasePtr(x, y);
			if (*pixel && *pixel != color) {
				pixel = (uint16 *)s->getBasePtr(x, y - 1);
				if (*pixel == 0)
					*pixel = color;
				pixel = (uint16 *)s->getBasePtr(x, y + 1);
				if (*pixel == 0)
					*pixel = color;
			}
		}
	}
}

QText::QText() {
	_resourceId = -2;
	_z = 3000;
}

void QText::update(int) {
	g_vm->videoSystem()->addDirtyRect(_rect);
}

QTextPhrase::QTextPhrase(const Common::U32String &text, uint16 textColor, uint16 outlineColor)
	: QText(text, textColor, outlineColor), _phrase(text), _time(0) {}

void QTextPhrase::draw() {
	if (g_vm->getQSystem()->_panelInterface->showSubtitles()) {
		QText::draw();
	}
}

void QTextPhrase::update(int time) {
	DialogInterface &dialog = g_vm->getQSystem()->_mainInterface->_dialog;
	_time += time;
	QText::update(time);
	Sound *sound = dialog.findSound();
	if (sound) {
		if (!sound->isPlaying()) {
			_time = 0;
			dialog.next(-1);
		}
	} else if (_time > _phrase.size() * 30 + 1000 || !g_vm->getQSystem()->_panelInterface->showSubtitles()) {
		_time = 0;
		dialog.next(-1);
	}
}

void QTextPhrase::onClick(Common::Point p) {
	DialogInterface &dialog = g_vm->getQSystem()->_mainInterface->_dialog;
	dialog.next(-1);
}

QTextDescription::QTextDescription(const Common::U32String &desc, uint32 frame) {
	_z = 999;
	_resourceId = -2;
	_rect = Common::Rect(0, 0, 640, 480);

	FlicDecoder *flc = g_vm->resMgr()->getFlic(6008);
	flc->setFrame(frame);

	const Graphics::Surface *frameS = flc->getCurrentFrame();
	Graphics::Surface *s = g_vm->resMgr()->getSurface(-2, 640, 480);

	Graphics::Surface *convS = frameS->convertTo(s->format, flc->getPalette());
	s->copyRectToSurface(*convS, 0, 0, _rect);
	convS->free();
	delete convS;

	Common::Rect textArea(160, 275, 598, 376);
	auto *font = g_vm->getDescriptionFont();
	auto textSurface = s->getSubArea(textArea);

	drawText(textSurface, 0, textArea.width(), desc, 0, *font, Graphics::kTextAlignLeft);

	g_vm->videoSystem()->addDirtyRect(_rect);
}

void QTextDescription::onClick(Common::Point p) {
	g_vm->getQSystem()->_mainInterface->removeTextDescription();
}

void QTextDescription::draw() {
	QManager *resMgr = g_vm->resMgr();
	VideoSystem *videoSys = g_vm->videoSystem();
	Graphics::Surface *s = resMgr->getSurface(-2);
	FlicDecoder *flc = resMgr->getFlic(6008);

	for (auto dirty : videoSys->rects()) {
		videoSys->transBlitFrom(*s, dirty, dirty, flc->getTransColor(s->format));
	}
}

QTextChoice::QTextChoice(const Common::Array<Common::U32String> &choices, uint16 color, uint16 outlineColor, uint16 selectedColor) {
	_activeChoice = 0;
	_choiceColor = color;
	_outlineColor = outlineColor;
	_selectedColor = selectedColor;
	_choices = choices;

	int w = 0;
	int h = 0;

	auto *font = g_vm->getTextFont();

	_rects.resize(choices.size());
	for (uint i = 0; i < _choices.size(); ++i) {
		_rects[i] = calculateBoundingBoxForText(_choices[i], *font);
		w = MAX<int>(w, _rects[i].width());
		_rects[i].setWidth(w);
		_rects[i].setHeight(font->getFontHeight());
		h += font->getFontHeight();
	}

	w += 10;
	h += 4;

	_rect = Common::Rect((640 - w) / 2, 479 - h, 639 - (640 - w) / 2, 479);

	Graphics::Surface *s = g_vm->resMgr()->getSurface(-2, w, h);

	int y = 0;
	for (uint i = 0; i < _choices.size(); ++i) {
		drawText(*s, y, 630, _choices[i], _choiceColor, *font, Graphics::TextAlign::kTextAlignLeft);

		_rects[i].moveTo(0, y);
		y += font->getFontHeight();
	}
	drawOutline(s, outlineColor);
}

void QTextChoice::onMouseMove(Common::Point p) {
	p.x = p.x - _rect.left - g_vm->getQSystem()->_xOffset;
	p.y = p.y - _rect.top;
	uint newChoice;
	for (newChoice = 0; newChoice < _rects.size(); ++newChoice) {
		if (_rects[newChoice].contains(p)) {
			break;
		}
	}

	if (newChoice != _activeChoice) {
		Graphics::Surface *s = g_vm->resMgr()->getSurface(-2);
		auto *font = g_vm->getTextFont();

		s->fillRect(Common::Rect(s->w, s->h), 0);
		for (uint i = 0; i < _choices.size(); ++i) {
			uint color = (i == newChoice) ? _selectedColor : _choiceColor;
			drawText(*s, _rects[i].top, 630, _choices[i], color, *font, Graphics::kTextAlignLeft);
		}
		drawOutline(s, _outlineColor);
		_activeChoice = newChoice;
	}
}

void QTextChoice::onClick(Common::Point p) {
	if (_activeChoice < _choices.size()) {
		g_vm->getQSystem()->_mainInterface->_dialog.next(_activeChoice);
	}
}

Common::Rect QText::calculateBoundingBoxForText(const Common::U32String &text, Graphics::Font &font) {
	if (text.empty())
		return {};

	Common::Array<Common::U32String> lines;
	font.wordWrapText(text, 630, lines);

	Common::Rect rect = font.getBoundingBox(lines[0]);
	rect.setHeight(font.getFontHeight());
	for (uint j = 1; j < lines.size(); ++j) {
		auto box = font.getBoundingBox(lines[j]);
		rect.setHeight(rect.height() + font.getFontHeight());
		if (box.width() > rect.width())
			rect.setWidth(box.width());
	}

	return rect;
}

void QText::drawText(Graphics::Surface &s, int y, int maxWidth, const Common::U32String &text, uint color, Graphics::Font &font, Graphics::TextAlign alignment) {
	Common::Array<Common::U32String> lines;
	font.wordWrapText(text, maxWidth, lines);

	int h = 0;
	for (uint i = 0; i < lines.size(); ++i) {
		font.drawString(&s, lines[i], 0, y + h, s.w, color, alignment);
		h += font.getFontHeight();
	}
}

} // End of namespace Petka
