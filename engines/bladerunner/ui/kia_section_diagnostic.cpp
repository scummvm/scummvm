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

const int KIASectionDiagnostic::kTextColors[] = { 0x0000, 0x0821, 0x1061, 0x1C82, 0x24C2, 0x2CE3, 0x3524, 0x4145, 0x4586, 0x4DC7, 0x5609, 0x5E4B, 0x668C, 0x6EEE, 0x7730, 0x7B92 };

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
	int timeNow = _vm->_time->currentSystem();

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
				_vm->_mainFont->drawColor(text, surface, 320 - _vm->_mainFont->getTextWidth(text) / 2, y, kTextColors[colorIndex]);
			}
		}
	}

	// Timing fixed for 60Hz by ScummVM team
	if (timeNow - _timeLast > 1000 / 60) {
		++_offset;
		if (_offset > kLineHeight * _text->getCount() + 366) {
			_offset = 0;
		}
		_timeLast = timeNow;
	}
}

} // End of namespace BladeRunner
