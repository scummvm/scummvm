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

#include "pink/pink.h"
#include "pink/objects/actors/audio_info_pda_button.h"
#include "pink/objects/actors/lead_actor.h"
#include "pink/objects/pages/page.h"

namespace Pink {

void AudioInfoPDAButton::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "AudioInfoPDAButton: _name = %s", _name.c_str());
	for (uint i = 0; i < _actions.size(); ++i) {
		_actions[i]->toConsole();
	}
}

void AudioInfoPDAButton::onMouseOver(Common::Point point, CursorMgr *mgr)  {
	mgr->setCursor(kClickableFirstFrameCursor, point, Common::String());
}

void AudioInfoPDAButton::onMouseOverWithItem(Common::Point point, const Common::String &itemName, CursorMgr *cursorMgr) {
	onMouseOver(point, cursorMgr);
}

void AudioInfoPDAButton::onLeftClickMessage() {
	AudioInfoMgr *audioInfoMgr = _page->getLeadActor()->getAudioInfoMgr();
	audioInfoMgr->onLeftClick();
}

} // End of namespace Pink
