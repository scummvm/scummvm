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

#include "bladerunner/outtake.h"

#include "bladerunner/bladerunner.h"
#include "bladerunner/chapters.h"
#include "bladerunner/subtitles.h"
#include "bladerunner/vqa_player.h"
#include "bladerunner/time.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/game_info.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"

namespace BladeRunner {

OuttakePlayer::OuttakePlayer(BladeRunnerEngine *vm) {
	_vm = vm;
	_surfaceVideo.create(_vm->_surfaceBack.w, _vm->_surfaceBack.h, _vm->_surfaceBack.format);
}

OuttakePlayer::~OuttakePlayer() {
	_surfaceVideo.free();
}

void OuttakePlayer::play(const Common::String &name, bool noLocalization, int container) {
	Common::String oldOuttakeFile;
	Common::String newOuttakeFile;
	if (container > 0) {
		oldOuttakeFile = Common::String::format("OUTTAKE%d.MIX", _vm->_chapters->currentResourceId());
		newOuttakeFile = Common::String::format("OUTTAKE%d.MIX", container);

		if (_vm->isArchiveOpen(oldOuttakeFile)
		    && _vm->_chapters->currentResourceId() != container) {
			_vm->closeArchive(oldOuttakeFile);
		}

		if (!_vm->isArchiveOpen(newOuttakeFile)) {
			_vm->openArchive(newOuttakeFile);
		}
	}

	_vm->playerLosesControl();

	Common::String resNameNoVQASuffix = name;
	if (!noLocalization) {
		resNameNoVQASuffix = resNameNoVQASuffix + "_" + _vm->_languageCode;
	}

	VQAPlayer vqaPlayer(_vm, &_surfaceVideo, resNameNoVQASuffix + ".VQA"); // in original game _surfaceFront is used here, but for proper subtitles rendering we need separate surface

	vqaPlayer.open();

	// NOTE: The VQAPlayer::open():
	// - calls VQAPlayer::close(), and that calls VQADecoder::close(), at its start to cleanup any previous state.
	// - later it calls _decoder.loadStream() which also calls VQADecoder::close().
	// Since VQADecoder::close() cleans up (deletes) the VQPTable,
	// we need to do the allocation for it and loading of its values here, *after* vqaPlayer.open().
	// The parsing of the VQP files is needed for the case of the non-interactive Blade Runner demo
	// and the support of other old VQA files that have accompanying VQP files (eg. for demo reels for other Westwood games)
	if (container == -2) {
		// container value: -2 indicates potential existence of VQP file
		if (!vqaPlayer.loadVQPTable(resNameNoVQASuffix + ".VQP")) {
			debug("Unable to load VQP table");
		}
	}

	_vm->_vqaIsPlaying = true;
	_vm->_vqaStopIsRequested = false;

	while (!_vm->_vqaStopIsRequested && !_vm->shouldQuit()) {
		_vm->handleEvents();

		if (!_vm->_windowIsActive) {
			continue;
		}

		int frame = vqaPlayer.update();
		blit(_surfaceVideo, _vm->_surfaceFront); // This helps to make subtitles disappear properly, if the video is rendered in separate surface and then pushed to the front surface
		if (frame == -3) { // end of video
			if (_vm->_cutContent && resNameNoVQASuffix.equals("FLYTRU_E")) {
				_vm->_ambientSounds->removeAllNonLoopingSounds(true);
				_vm->_ambientSounds->removeAllLoopingSounds(1u);
			}
			break;
		}

		if (frame >= 0) {
			_vm->_subtitles->loadOuttakeSubsText(resNameNoVQASuffix, frame);
			_vm->_subtitles->tickOuttakes(_vm->_surfaceFront);
			_vm->blitToScreen(_vm->_surfaceFront);
			if (_vm->_cutContent && resNameNoVQASuffix.equals("FLYTRU_E")) {
				// This FLYTRU_E outtake has 150 frames
				//
				// We can have at most kLoopingSounds (3) looping ambient tracks
				// Outtakes in general use a specific Mixer Sound Type (ie. VQAPlayer::kVQASoundType)
				//      so the sounds here should conform too.
				//     (see VQAPlayer::update())
				//
				// No need for _ambientSounds->tick() since it's not required for looping ambient tracks,
				// nor for explicitly played Sounds (via playSound())
				// It is harmless however, so it could remain.
				//_vm->_ambientSounds->tick();
				switch (frame) {
				case 0:
					_vm->_ambientSounds->addLoopingSound(kSfxLABAMB1,   95, 0, 0u, VQAPlayer::kVQASoundType);
					_vm->_ambientSounds->addLoopingSound(kSfxROOFAIR1, 100, 0, 0u, VQAPlayer::kVQASoundType);
					_vm->_ambientSounds->addLoopingSound(kSfxPSPA6,     74, 0, 1u, VQAPlayer::kVQASoundType);
					break;
				case 18:
					_vm->_ambientSounds->playSound(kSfxSPIN2A, 100, 90, 20, 99, VQAPlayer::kVQASoundType);
					break;
				case 24:
					_vm->_ambientSounds->playSound(kSfxSWEEP4, 45, 90, 20, 99, VQAPlayer::kVQASoundType);
					break;
				case 32:
					if (_vm->_rnd.getRandomNumberRng(1, 5) < 4) 
						_vm->_ambientSounds->playSound(kSfxTHNDER3, 82, -20, -20, 99, VQAPlayer::kVQASoundType);
					break;
				case 41:
					_vm->_ambientSounds->playSound(kSfxMUSVOL8, 22, 46, 46, 99, VQAPlayer::kVQASoundType);
					break;
				case 52:
					if (_vm->_rnd.getRandomNumberRng(1, 4) < 4) 
						_vm->_ambientSounds->playSound(kSfxTHNDR3, 90, 10, 10, 89, VQAPlayer::kVQASoundType);
					break;
				case 78:
					if (_vm->_rnd.getRandomNumberRng(1, 5) < 5) 
						_vm->_ambientSounds->playSound(kSfxSIREN2, 62, -60, 45, 99, VQAPlayer::kVQASoundType);
					break;
				case 105:
					_vm->_ambientSounds->playSound(kSfxSWEEP3, 22, 20, 95, 99, VQAPlayer::kVQASoundType);
					break;
				case 112:
					if (_vm->_rnd.getRandomNumberRng(1, 5) < 4) 
						_vm->_ambientSounds->playSound(kSfxTHNDER4, 95, -20, -20, 99, VQAPlayer::kVQASoundType);
					break;
				}
			}
		}
	}

	if ((_vm->_vqaStopIsRequested || _vm->shouldQuit())
		&& _vm->_cutContent && resNameNoVQASuffix.equals("FLYTRU_E")) {
		_vm->_ambientSounds->removeAllNonLoopingSounds(true);
		_vm->_ambientSounds->removeAllLoopingSounds(0u);
		_vm->_audioPlayer->stopAll();
	}

	_vm->_vqaIsPlaying = false;
	_vm->_vqaStopIsRequested = false;
	vqaPlayer.close();

	_vm->playerGainsControl();

	if (container > 0) {
		if (_vm->isArchiveOpen(newOuttakeFile)
		    && _vm->_chapters->currentResourceId() != container) {
			_vm->closeArchive(newOuttakeFile);
		}

		if (!_vm->isArchiveOpen(oldOuttakeFile)) {
			_vm->openArchive(oldOuttakeFile);
		}
	}
}

} // End of namespace BladeRunner
