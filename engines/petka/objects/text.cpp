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

	Common::ScopedPtr<Graphics::Font> font(Graphics::loadTTFFontFromArchive("FreeSans.ttf", 18));
	Common::Rect rect = font->getBoundingBox(text, 0, 0, 0);
	rect.right += 10;
	rect.bottom += 4;

	_rect = Common::Rect((640 - rect.width()) / 2, 479 - rect.height(), 639 - (640 - rect.width()) / 2, 479);
	Graphics::Surface *s = g_vm->resMgr()->findOrCreateSurface(-2, _rect.width(), _rect.height());
	font->drawString(s, text, 0, 0, s->w, textColor);

	drawOutline(s, outlineColor);
}

void QText::draw() {
	const Graphics::Surface *s = g_vm->resMgr()->loadBitmap(-2);
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

QTextPhrase::QTextPhrase(const Common::U32String &text, uint16 textColor, uint16 outlineColor)
	: QText(text, textColor, outlineColor), _phrase(text), _time(0) {}

void QTextPhrase::draw() {
	if (g_vm->getQSystem()->_panelInterface->_subtitles) {
		QText::draw();
	}
}

void QTextPhrase::update(int time) {
	DialogInterface &dialog = g_vm->getQSystem()->_mainInterface->_dialog;
	_time += time;
	Sound *sound = dialog.findSound();
	if (sound) {
		if (!sound->isPlaying()) {
			_time = 0;
			dialog.next(-1);
		}
	} else if (_time > _phrase.size() * 30 + 1000 || !g_vm->getQSystem()->_panelInterface->_subtitles) {
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

	FlicDecoder *flc = g_vm->resMgr()->loadFlic(6008);
	flc->setFrame(frame);

	const Graphics::Surface *frameS = flc->getCurrentFrame();
	Graphics::Surface *s = g_vm->resMgr()->findOrCreateSurface(-2, 640, 480);

	Graphics::Surface *convS = frameS->convertTo(s->format, flc->getPalette());
	s->copyRectToSurface(*convS, 0, 0, _rect);
	convS->free();
	delete convS;


	Common::Rect textArea(160, 275, 598, 376);
	Common::ScopedPtr<Graphics::Font> font(Graphics::loadTTFFontFromArchive("FreeSans.ttf", 16));

	font->drawString(s, desc, textArea.left, textArea.top, textArea.width(), 0);
}

void QTextDescription::onClick(Common::Point p) {
	g_vm->getQSystem()->_mainInterface->removeTextDescription();
}

void QTextDescription::draw() {
	Graphics::Surface *s = g_vm->resMgr()->loadBitmap(-2);
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(6008);
	g_vm->videoSystem()->transBlitFrom(*s, flc->getTransColor(s->format));
	// todo dirty rects
}

QTextChoice::QTextChoice(const Common::Array<Common::U32String> &choices, uint16 color, uint16 selectedColor) {
	_activeChoice = 0;
	_choiceColor = color;
	_selectedColor = selectedColor;
	_choices = choices;

	int w = 0;
	int h = 0;
	Common::ScopedPtr<Graphics::Font> font(Graphics::loadTTFFontFromArchive("FreeSans.ttf", 18));
	_rects.resize(choices.size());
	for (uint i = 0; i < _choices.size(); ++i) {
		_rects[i] = font->getBoundingBox(_choices[i]);
		if (_rects[i].width() > w)
			w = _rects[i].width();
		h += _rects[i].height();
	}

	_rect = Common::Rect((640 - w) / 2, 479 - h, 639 - (640 - w) / 2, 479);

	Graphics::Surface *s = g_vm->resMgr()->findOrCreateSurface(-2, w, h);

	int y = 0;
	for (uint i = 0; i < _choices.size(); ++i) {
		font->drawString(s, _choices[i], 0, y, s->w, color);
		_rects[i].moveTo(0, y);
		y += _rects[i].height();
	}
}

void QTextChoice::onMouseMove(Common::Point p) {
	p.x = p.x - _rect.left;
	p.y = p.y - _rect.top;
	uint newChoice;
	for (newChoice = 0; newChoice < _rects.size(); ++newChoice) {
		if (_rects[newChoice].contains(p)) {
			break;
		}
	}

	if (newChoice != _activeChoice) {
		Graphics::Surface *s = g_vm->resMgr()->loadBitmap(-2);
		Common::ScopedPtr<Graphics::Font> font(Graphics::loadTTFFontFromArchive("FreeSans.ttf", 18));

		if (_activeChoice < _choices.size()) {
			font->drawString(s, _choices[_activeChoice], _rects[_activeChoice].left, _rects[_activeChoice].top, _rects[_activeChoice].width(), _choiceColor);
		}

		if (newChoice < _choices.size()) {
			font->drawString(s, _choices[newChoice], _rects[newChoice].left, _rects[newChoice].top, _rects[newChoice].width(), _selectedColor);
		}

		_activeChoice = newChoice;
	}
}

void QTextChoice::onClick(Common::Point p) {
	if (_activeChoice < _choices.size()) {
		g_vm->getQSystem()->_mainInterface->_dialog.next(_activeChoice);
	}
}

} // End of namespace Petka
