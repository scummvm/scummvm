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

#include "common/system.h"
#include "common/config-manager.h"
#include "common/macresman.h"

#include "engines/engine.h"

#if 0
#include "graphics/maccursor.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/surface.h"
#endif

#include "scumm/scumm.h"
#include "scumm/detection.h"
#include "scumm/macgui/macgui_impl.h"
#include "scumm/macgui/macgui_v6.h"

#if 0
#include "scumm/music.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"
#endif

namespace Scumm {

// ===========================================================================
// The Mac SCUMM v6 (and later) GUI.
// ===========================================================================

MacV6Gui::MacV6Gui(ScummEngine *vm, const Common::Path &resourceFile) : MacGuiImpl(vm, resourceFile) {
}

const Graphics::Font *MacV6Gui::getFontByScummId(int32 id) {
	// V5 games do not use CharsetRendererMac
	return nullptr;
}

bool MacV6Gui::getFontParams(FontId fontId, int &id, int &size, int &slant) const {
	return false;
}

bool MacV6Gui::handleMenu(int id, Common::String &name) {
	if (MacGuiImpl::handleMenu(id, name))
		return true;

	return false;
}

void MacV6Gui::runAboutDialog() {
}

bool MacV6Gui::runOptionsDialog() {
	return false;
}

void MacV6Gui::resetAfterLoad() {
	reset();
}

bool MacV6Gui::handleEvent(Common::Event event) {
	if (MacGuiImpl::handleEvent(event))
		return true;

	if (_vm->isPaused())
		return false;

	if (_vm->_userPut <= 0)
		return false;

	return false;
}

} // End of namespace Scumm
