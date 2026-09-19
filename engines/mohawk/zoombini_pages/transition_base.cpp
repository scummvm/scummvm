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

#include "common/archive.h"
#include "common/scummsys.h"

#include "mohawk/video.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/transition_base.h"
#include "mohawk/zoombini_sound.h"

namespace Mohawk {

ZoombiniTransition::ZoombiniTransition(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType) : ZoombiniPage(vm, ZoombiniPageCategory::kTransition, pageType) {
	_useFadeEffect = false;
}

ZoombiniTransition::~ZoombiniTransition() {
	_vm->clearPageArchives();
}

void ZoombiniTransition::close() {
	_vm->_midi->stop();
	ZoombiniPage::close();
}

ZmbEventHandleResult ZoombiniTransition::onLButtonDown(const Common::Point &absPos, const Common::Point &relPos) {
	(void)absPos;
	(void)relPos;

	close();
	return ZmbEventHandleResult::kConsumed;
}

ZmbEventHandleResult ZoombiniTransition::onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) {
	(void)kbd;
	(void)kbdRepeat;

	close();
	return ZmbEventHandleResult::kConsumed;
}

} // End of namespace Mohawk
