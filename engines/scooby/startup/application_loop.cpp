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

#include "application_loop.h"

#include "intro_sequence.h"
#include "scooby/rendering/frame_clock.h"
#include "scooby/runtime/session_controller.h"
#include "scooby/runtime/session_exit.h"

namespace Scooby {

void ApplicationLoop::run() {
	while (!_host.shouldClose()) {
		RuntimeState state;
		DeterministicRandom random;
		initializeRuntimeState(state, random);
		IntroSequence intro(_rom, _scene);
		FrameClock clock;
		while (!_host.shouldClose() && !intro.isComplete()) {
			clock.waitForNextFrame();
			if (_host.isIntroSkipPressed()) {
				intro.skipCurrentStep();
			} else {
				intro.advanceFrame();
			}

			_scene.render(_frame);
			_host.present(_frame);
		}

		if (_host.shouldClose()) {
			return;
		}

		SessionController session(_rom, _scene, _frame, _host, clock, state, random);
		_scene.setInterfaceScroll(0);
		if (session.runGame() == SessionExit::HostClosed) {
			return;
		}
	}
}

void ApplicationLoop::initializeRuntimeState(RuntimeState &state,
											 DeterministicRandom &random) {
	// Ghidra 0x0000A248-0x0000A255: native execution waits until the VDP command port is ready. Logical
	// scene operations complete synchronously, so there is no pending hardware state to poll here.

	// Ghidra 0x0000A256-0x0000A267: clear the episode selection and install the startup random seed.
	state.EpisodeIndex = 0;
	random.initializeStartupSeed();

	// Ghidra 0x0000A268-0x0000A271: preserve the distinct RNG call boundary. Zero replaces the hardware V
	// counter because only the discarded scaled result depends on it; the persisted seed does not.
	(void)random.scaleNextRandomValue(0);

	// Ghidra 0x0000A272-0x0000A279: clear the mixed startup/menu flags.
	state.InitializationFlags = 0;

	// Native control then falls through to the separately recovered ResetRuntimeState entry at
	// 0x0000A27A.
	state.resetRuntimeState(_rom);
}
} // namespace Scooby