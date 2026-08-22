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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/scenes/wofford_scene.h"

#include "common/debug.h"
#include "common/system.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/milestones.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

// RunWoffordInteractiveMediaScene loads these entries from the Cyber asset
// container. Keep the directory qualified because loose installations also
// contain the unrelated SCENE/WOFFORD1.WAV.
static const char *const kSceneMedia = "cyber/wofford.smk";
static const char *const kAudioCues[4] = {
	"cyber/wofford0.wav", "cyber/wofford1.wav",
	"cyber/wofford2.wav", "cyber/wofford3.wav"
};

static const uint kDefaultCursor = 14;
static const uint kChoiceCursor = 16;
static const uint kBusyCursor = 19;
static const uint kLoopStartFrame = 1;
static const uint kChoiceArmFrame = 20;
// SetAudioTriggerHandlePackedVolume receives stereo value 0x70a270a2 at
// 0x2ad75; each 0x70a2 channel rounds to 88% of the signed 16-bit range.
static const uint kOpeningCueVolume = 88;

static const uint16 kEscapeCommand = 0x1b;
static const uint16 kFirstChoiceCommand = 0x672;
static const uint16 kSecondChoiceCommand = 0x673;
static const uint16 kFailureCommand = 0x7ffd;

// g_woffordControlLayoutRows at 0x2a824 stores the original controls in
// transposed scene coordinates. The first row must retain priority because it
// lies inside the second row's full-presentation rectangle.
static bool isFirstChoicePoint(const Common::Point &point) {
	return point.x >= 205 && point.x < 416 && point.y >= 203 && point.y < 296;
}

static bool isSecondChoicePoint(const Common::Point &point) {
	return point.x >= 16 && point.x < 618 && point.y >= 73 && point.y < 335;
}

} // End of anonymous namespace

WoffordScene::WoffordScene(RipperEngine *engine) : Scene(engine),
		_hoveredControl(-1), _choiceCueArmed(false) {
}

bool WoffordScene::startFollowupCue(uint cue, const char *source) {
	if (cue >= ARRAYSIZE(kAudioCues) || cue == 0)
		return false;
	if (!_engine->getMedia()->playVoiceClip(kAudioCues[cue], _followupCueHandle)) {
		warning("Ripper: could not start Wofford cue '%s'", kAudioCues[cue]);
		return false;
	}
	debugC(2, kDebugAudio,
		"Ripper: started Wofford cue=%u media='%s' source=%s",
		cue, kAudioCues[cue], source);
	return true;
}

void WoffordScene::updateCursor(const Common::Point &point) {
	const bool busy = _engine->getMedia()->isSoundEffectActive(_followupCueHandle);
	int hovered = -1;
	uint cursor = busy ? kBusyCursor : kDefaultCursor;
	if (!busy && _choiceCueArmed) {
		if (isFirstChoicePoint(point))
			hovered = 0;
		else if (isSecondChoicePoint(point))
			hovered = 1;
		if (hovered >= 0)
			cursor = kChoiceCursor;
	}
	if (hovered != _hoveredControl) {
		debugC(2, kDebugCyber,
			"Ripper: Wofford media hover control=%d previous=%d point=%d,%d cursor=%u armed=%d busy=%d",
			hovered, _hoveredControl, point.x, point.y, cursor,
			_choiceCueArmed, busy);
		_hoveredControl = hovered;
	}
	_engine->getCursor()->update(cursor);
	_engine->getCursor()->setVisible(true);
}

uint16 WoffordScene::service(uint frame) {
	uint16 command = 0;
	if (_engine->getInput()->hasPendingKey()) {
		command = _engine->getInput()->consumeKey();
		debugC(3, kDebugInput,
			"Ripper: Wofford media keyboard command=0x%04x frame=%u",
			command, frame);
	}

	// ServiceWoffordMediaSequenceCallback at 0x2aaf5 arms the controls and
	// queues descriptor 1 when the one-based playback frame reaches 0x14.
	if (frame == kChoiceArmFrame && !_choiceCueArmed) {
		_choiceCueArmed = true;
		if (!startFollowupCue(1, "frame-20"))
			return kFailureCommand;
	}

	const MouseState mouse = _engine->getInput()->publishMouseState();
	updateCursor(mouse.position);
	if (command == kEscapeCommand)
		return command;
	if (command != 0 || !_choiceCueArmed ||
			_engine->getMedia()->isSoundEffectActive(_followupCueHandle) ||
			(mouse.pressed & kMouseButtonLeft) == 0)
		return 0;

	if (_hoveredControl == 0)
		command = kFirstChoiceCommand;
	else if (_hoveredControl == 1)
		command = kSecondChoiceCommand;
	if (command == kFirstChoiceCommand) {
		if (!startFollowupCue(2, "control-0x672"))
			return kFailureCommand;
	} else if (command == kSecondChoiceCommand) {
		if (!startFollowupCue(3, "control-0x673"))
			return kFailureCommand;
	}
	if (command != 0) {
		debugC(2, kDebugCyber,
			"Ripper: Wofford media selected command=0x%04x control=%d point=%d,%d frame=%u",
			command, _hoveredControl, mouse.position.x, mouse.position.y, frame);
	}
	return 0;
}

void WoffordScene::stopAllAudio() {
	stopAudio(_openingCueHandle);
	stopAudio(_followupCueHandle);
}

WoffordScene::Result WoffordScene::run(uint completionFlag) {
	// RunWoffordInteractiveMediaScene at 0x2ac04 sets the action argument in
	// the shared milestone bitset before constructing the temporary controls.
	if (!_engine->getMilestones()->set(completionFlag, true,
			"cyber-wofford-media"))
		return kLoadFailed;

	prepare("wofford-media-entry", kDefaultCursor, true);
	g_system->fillScreen(0);
	presentScreen();
	debugC(1, kDebugCyber,
		"Ripper: entered Wofford media scene flag=%u media='%s' loopStart=%u armFrame=%u controls=[205,203,211,93;16,73,602,262]",
		completionFlag, kSceneMedia, kLoopStartFrame, kChoiceArmFrame);
	if (!_engine->getMedia()->playVoiceClip(kAudioCues[0],
			_openingCueHandle, kOpeningCueVolume)) {
		stopAllAudio();
		finish("wofford-media-load-failure", -1, false);
		return kLoadFailed;
	}

	uint16 command = 0;
	const bool played = _engine->getMedia()->playPuzzleSequence(
		kSceneMedia, kLoopStartFrame, this, &command);
	const Result result = (!played && !_engine->shouldQuit()) ||
		command == kFailureCommand ? kLoadFailed : kExited;

	stopAllAudio();
	finish("wofford-media-exit", -1, false);
	debugC(result == kExited ? 1 : 2, kDebugCyber,
		"Ripper: left Wofford media scene result=%d command=0x%04x armed=%d quit=%d",
		result, command, _choiceCueArmed, _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
