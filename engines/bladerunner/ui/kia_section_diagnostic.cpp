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

#include "bladerunner/ui/kia_section_diagnostic.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/font.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/ui/kia.h"

namespace BladeRunner {

const Color256 KIASectionDiagnostic::kTextColors[] = {
	{ 0, 0, 0 },
	{ 16, 8, 8 },
	{ 32, 24, 8 },
	{ 56, 32, 16 },
	{ 72, 48, 16 },
	{ 88, 56, 24 },
	{ 104, 72, 32 },
	{ 128, 80, 40 },
	{ 136, 96, 48 },
	{ 152, 112, 56 },
	{ 168, 128, 72 },
	{ 184, 144, 88 },
	{ 200, 160, 96 },
	{ 216, 184, 112 },
	{ 232, 200, 128 },
	{ 240, 224, 144 }
};

KIASectionDiagnostic::KIASectionDiagnostic(BladeRunnerEngine *vm) : KIASectionBase(vm) {
	_text     = nullptr;
	_offset   = 0;
	_timeLast = 0;
}

void KIASectionDiagnostic::open() {
	_text = new TextResource(_vm);
	if (!_text->open("KIACRED")) {
		return;
	}
	_vm->_kia->playActorDialogue(kActorRunciter, 140);
	_offset = 0;
	_timeLast = _vm->_time->currentSystem();
}

void KIASectionDiagnostic::close() {
	delete _text;
}

void KIASectionDiagnostic::draw(Graphics::Surface &surface) {
	uint32 timeNow = _vm->_time->currentSystem();

	for (int i = 0; i < _text->getCount(); ++i) {
		int y = kLineHeight * i + 366 - _offset;
		if (y >= 150 && y < 366) {
			int colorIndex = 15;
			if (y < 182) {
				colorIndex = (y - 150) / 2;
			} else if (y >= 334) {
				colorIndex = (365 - y) / 2;
			}

			const char *text = _text->getText(i);
			if (text) {
				_vm->_mainFont->drawString(&surface, text, 320 - _vm->_mainFont->getStringWidth(text) / 2, y, surface.w, surface.format.RGBToColor(kTextColors[colorIndex].r, kTextColors[colorIndex].g, kTextColors[colorIndex].b));
			}
		}
	}

	// Timing fixed for 60Hz by ScummVM team
	// unsigned difference is intentional
	if (timeNow - _timeLast > (1000u / 60u)) {
		++_offset;
		if (_offset > kLineHeight * _text->getCount() + 366) {
			_offset = 0;
		}
		_timeLast = timeNow;
	}
}

} // End of namespace BladeRunner
