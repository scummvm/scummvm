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
#include "bladerunner/subtitles.h"
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

	Common::String resName = name;
	if (!noLocalization) {
		resName = resName + "_" + _vm->_languageCode;
	}
	Common::String resNameNoVQASuffix = resName;
	resName = resName + ".VQA";

	VQAPlayer vqa_player(_vm, &_vm->_surfaceBack); // surfaceBack is needed here for subtitles rendering properly, original was _surfaceFront here

	vqa_player.open();

	_vm->_mixer->stopAll();
	while (!_vm->shouldQuit()) {
		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event))
			if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)
				return;

		int frame = vqa_player.update();
		blit(_vm->_surfaceBack, _vm->_surfaceFront); // This helps to make subtitles disappear properly, if the video is rendered in surface back and then pushed to the front surface
		if (frame == -3)
			break;

		if (frame >= 0) {
			_vm->_subtitles->getOuttakeSubsText(resNameNoVQASuffix, frame);
			_vm->_subtitles->tickOuttakes(_vm->_surfaceFront);
			_vm->blitToScreen(_vm->_surfaceFront);
		}

		_vm->_system->delayMillis(10);
	}
}

} // End of namespace BladeRunner
