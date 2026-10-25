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

#include "graphics/framelimiter.h"
#include "intro_sequence.h"
#include "scooby/runtime/session_controller.h"
#include "scooby/scooby.h"
#include "scooby/sound/gems_audio.h"
#include "scooby/runtime/save_game_data.h"

namespace Scooby {

void ApplicationLoop::run() {
	while (!g_engine->shouldQuit()) {
		 _state = RuntimeState{};
		DeterministicRandom random;
		initializeRuntimeState(_state, random);
		// Ghidra RunStartupSequence 0x000008E2-0x000008FF: every (re)start reloads and starts the
		// GEMS Z80 driver with the cartridge's sound banks.
		gemsInit();
		// Ghidra RunStartupSequence 0x00000904-0x00000915: g_lResumeAudioCommand = 0x2F, then PlayAudioCommand
		// starts that title song before the intro, so it plays under the logos and continues into the main menu.
		// RunAdventure's InitializeAudioDriver stops it when a session (including a loaded save) begins.
		_state.ResumeAudioCommand = 0x2F;
		gemsStartSong(_state.ResumeAudioCommand);
		Graphics::FrameLimiter limiter(g_system, kFramesPerSecond); // NTSC - 60Hz
		if (!_state._saveGameData.shouldLoadGameData) {
			IntroSequence intro(_rom, _scene);
			// Managed-only mouse support: no pointer during the title intro (mouse_input.h, Mode::Intro).
			MouseModeScope mouseMode(_input.mouse(), MouseInput::Mode::Intro);
			while (!g_engine->shouldQuit() && !intro.isComplete()) {
				limiter.delayBeforeSwap();
				if (_input.isIntroSkipPressed()) {
					intro.skipCurrentStep();
				} else {
					intro.advanceFrame();
				}

				_scene.render(_frame);
				_frame.present();
				limiter.startFrame();
			}
		}

		if (g_engine->shouldQuit()) {
			return;
		}

		SessionController session(_rom, _scene, _frame, _input, limiter, _state, random);
		_scene.setInterfaceScroll(0);
		if (session.runGame() == SessionExit::HostClosed) {
			return;
		}
	}
}

Common::Error ApplicationLoop::syncGame(Common::Serializer &s) {
	// Managed-only save games: the complete persistent world (runtime/save_game_data.h).
	if (s.isLoading()) {
		SaveGameData loaded;
		if (!loaded.sync(s)) {
			return Common::Error(Common::kReadingFailed, "Incompatible or damaged Scooby-Doo Mystery save");
		}

		// The running session sees the flag and restarts; initializeRuntimeState then hands the data over.
		_loadGameData = loaded;
		_loadGameData.shouldLoadGameData = true;
		_state._saveGameData.shouldLoadGameData = true;
		debug(1, "Scooby: loaded save for episode %d, room %d", _loadGameData.episodeIdx, _loadGameData.roomId);
	} else {
		SaveGameData snapshot;
		snapshot.capture(_state);
		if (!snapshot.sync(s)) {
			return Common::Error(Common::kWritingFailed);
		}

		debug(1, "Scooby: saved episode %d, room %d", snapshot.episodeIdx, snapshot.roomId);
	}

	return Common::kNoError;
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

	if (_loadGameData.shouldLoadGameData) {
		state._saveGameData = _loadGameData;
		_loadGameData.shouldLoadGameData = false;
	}
}
} // namespace Scooby
