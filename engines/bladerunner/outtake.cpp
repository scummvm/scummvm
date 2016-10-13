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

#include "bladerunner/outtake.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/vqa_player.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

namespace BladeRunner {

void OuttakePlayer::play(const Common::String &name, bool noLocalization, int container) {
	if (container > 0) {
		debug("OuttakePlayer::play TODO");
		return;
	}

	Common::String resName;
	if (noLocalization)
		resName = name + ".VQA";
	else
		resName = name + "_E.VQA";

	VQAPlayer vqa_player(_vm);

	vqa_player.open(resName);

	_vm->_mixer->stopAll();
	while (!_vm->shouldQuit()) {
		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event))
			if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)
				return;

		int frame = vqa_player.update();
		if (frame == -3)
			break;

		if (frame >= 0) {
			const Graphics::Surface *surface = vqa_player.getSurface();
			_vm->_system->copyRectToScreen((const byte *)surface->getBasePtr(0, 0), surface->pitch, 0, 0, 640, 480);
			_vm->_system->updateScreen();
		}

		_vm->_system->delayMillis(10);
	}
}

} // End of namespace BladeRunner
