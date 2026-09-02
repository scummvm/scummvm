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

#include "mohawk/console.h"
#include "mohawk/mohawk.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "mohawk/zoombini.h"
#include "mohawk/zoombini_graphics.h"
#include "mohawk/zoombini_pages/dialog_base.h"
#include "mohawk/zoombini_random.h"
#include "mohawk/zoombini_sound.h"
#include "mohawk/zoombini_state.h"

namespace Mohawk {

ZoombiniDialog::ZoombiniDialog(MohawkEngine_Zoombini *vm, ZoombiniPageType pageType) : ZoombiniPage(vm, ZoombiniPageCategory::kDialog, pageType) {
	_useFadeEffect = false;

	// Backup current back screen
	_vm->_gfx->createScreen(_capturedBackScreen);
	_vm->_gfx->captureScreen(ZoombiniGraphics::kBackScreen, &_capturedBackScreen);
}

ZoombiniDialog::~ZoombiniDialog() {
	// Restore captured back screen
	_vm->_gfx->copyToScreen(ZoombiniGraphics::kBackScreen, _capturedBackScreen);
}

ZoombiniDialog::DialogKeyAction ZoombiniDialog::classifyDialogKey(const Common::KeyState &kbd) {
	switch (kbd.keycode) {
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		return kDialogKeyAccept;
	case Common::KEYCODE_ESCAPE:
		return kDialogKeyCancel;
	default:
		return kDialogKeyNone;
	}
}

void ZoombiniDialog::resetLongButtonStates() {
	for (Common::StableMap<uint32, ButtonState>::iterator it = _longButtonStateMap.begin(); it != _longButtonStateMap.end(); it++)
		it->second.reset();
}

void ZoombiniDialog::setBackgroundBitmap() {
	// Capture current composed screen as a background
	_vm->_gfx->captureComposedScreen(ZoombiniGraphics::kBackScreen);
	scheduleForceRedraw();
}

ZmbEventHandleResult ZoombiniDialog::onKeyDown(const Common::KeyState &kbd, bool kbdRepeat) {
	const ZmbEventHandleResult result = ZoombiniPage::onKeyDown(kbd, kbdRepeat);
	if (result == ZmbEventHandleResult::kConsumed)
		return result;

	if (!kbdRepeat && kbd.hasFlags(Common::KBD_CTRL) && kbd.keycode == Common::KEYCODE_q) {
		close();
		Engine::quitGame();
		return ZmbEventHandleResult::kConsumed;
	}

	return result;
}

} // End of namespace Mohawk
