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

#include "ripper/media.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"

#include "ripper/detection.h"
#include "ripper/display.h"
#include "ripper/input.h"
#include "ripper/media/presentation_text.h"
#include "ripper/ripper.h"
#include "ripper/script.h"
#include "ripper/settings.h"

namespace Ripper {

namespace {

static const int kScenePresentationTop = 50;

SmackerPlaybackPlan makeWacInterfaceSequencePlan(const Common::Rect &centerBounds,
		uint loopStartFrame, MediaSequenceCallback *callback, uint16 *command) {
	SmackerPlaybackPlan plan;
	plan.placement.centerBounds = centerBounds;
	plan.palette.patchInterfacePalette = false;
	plan.palette.patchWacMediaPalette = true;
	plan.loop.loopStartFrame = loopStartFrame;
	plan.callback.sequenceCallback = callback;
	plan.callback.sequenceCommand = command;
	return plan;
}

} // End of anonymous namespace

bool MediaPlayer::play(const Common::String &path, bool allowEscSpace, int x, int y,
		bool sceneViewport, const Common::String &presentationText) {
	RipperEngine::SaveLoadScope saveLoad(*_engine);
	// Route WAV entries through blocking audio before either video
	// path. That loop always permits Escape, independent of AVI controls.
	if (path.hasSuffixIgnoreCase(".wav")) {
		debugC(2, kDebugAudio,
			"Ripper: dispatching media presentation '%s' as blocking audio",
			path.c_str());
		const bool result = playBlockingAudio(path, true, presentationText,
			sceneViewport ? kScenePresentationTop : 0);
		_input->drainKeys();
		return result;
	}

	Common::String source;
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		openSource(path, kSourceDirectFile, source));
	if (!stream) {
		warning("Ripper: could not open media '%s'", path.c_str());
		return false;
	}

	const MediaFormat format = detectMediaFormat(*stream);
	const bool isSmacker = format == kMediaFormatSmacker;
	const bool isIavf = format == kMediaFormatIavf;
	const bool restoreIavfDisplay = isIavf && allowEscSpace;
	IndexedDisplaySnapshot displayContext;
	const bool displayContextCaptured = restoreIavfDisplay && displayContext.capture();
	const bool rememberIavfPalette = !displayContextCaptured;
	if (isIavf) {
		debugC(2, kDebugVideo,
			"Ripper: IAVF display policy media='%s' keyboardControls=%d restore=%d captured=%d rememberPalette=%d size=%dx%d",
			path.c_str(), allowEscSpace, restoreIavfDisplay, displayContextCaptured,
			rememberIavfPalette, displayContext.bounds().width(),
			displayContext.bounds().height());
	}

	bool result = false;
	if (isSmacker) {
		// Direct scene Smackers use a 640x300 page whose logical y=0 is physical
		// y=50 in ScummVM's 640x400 framebuffer.
		const int originY = sceneViewport ? kScenePresentationTop : 0;
		SmackerPlaybackPlan plan;
		plan.input.allowEscSpace = allowEscSpace;
		plan.placement.x = x;
		plan.placement.y = y;
		plan.placement.originY = originY;
		result = playValidatedSmacker(stream.release(), path, "presentation", plan);
	} else if (isIavf) {
		const bool hasPresentationText = !presentationText.empty();
		const int originY = hasPresentationText ? 0 :
			(sceneViewport ? kScenePresentationTop : 0);
		const int presentationX = hasPresentationText ? -1 : x;
		const int presentationY = hasPresentationText ? -1 : y;
		PresentationTextMediaCallback textCallback(_engine, _input,
			presentationText, originY, _engine->getSettings()->getVideoMode());
		uint16 textCommand = 0;
		const uint textDisplayScale = hasPresentationText &&
			_engine->getSettings()->getVideoMode() < 2 ? 1 : 0;
		// Restoring an indexed page also requires its source palette; ACCESED.AVI
		// must not replace the palette used to rebuild the surrounding WAC page.
		result = playIavf(*stream, path, allowEscSpace, presentationX,
			presentationY, originY, false,
			rememberIavfPalette, textDisplayScale,
			hasPresentationText ? &textCallback : nullptr,
			hasPresentationText ? &textCommand : nullptr);
		if (result && hasPresentationText && textCommand == 0 &&
				!_engine->shouldQuit())
			result = textCallback.waitForDismissal();
	} else {
		warning("Ripper: unsupported media format for '%s' format=%s",
			path.c_str(), mediaFormatName(format));
	}
	if (displayContextCaptured && !_engine->shouldQuit()) {
		const bool restored = displayContext.restore();
		debugC(restored ? 2 : 1, kDebugVideo,
			"Ripper: restored script media display context media='%s' success=%d size=%dx%d",
			path.c_str(), restored, displayContext.bounds().width(),
			displayContext.bounds().height());
		if (!restored)
			result = false;
	} else if (isIavf && !restoreIavfDisplay && result && !_engine->shouldQuit()) {
		debugC(2, kDebugVideo,
			"Ripper: retained final IAVF display media='%s' keyboardControls=%d",
			path.c_str(), allowEscSpace);
	}

	_input->drainKeys();
	return result;
}

bool MediaPlayer::playWacMedia(const Common::String &path, int x, int y) {
	debugC(1, kDebugVideo,
		"Ripper: entering WAC media presentation media='%s' position=%d,%d palette=10..149",
		path.c_str(), x, y);
	SmackerPlaybackPlan plan;
	plan.placement.x = x;
	plan.placement.y = y;
	plan.palette.patchInterfacePalette = false;
	plan.palette.patchWacMediaPalette = true;
	Common::String source;
	const bool result = playValidatedSmacker(
		openSource(path, kSourcePuzzleLibrary, source), path, "WAC", plan);
	_input->drainKeys();
	return result;
}

bool MediaPlayer::playWacPresentation(const Common::String &path, int x, int y) {
	debugC(1, kDebugVideo,
		"Ripper: entering WAC packetized presentation media='%s' position=%d,%d scale=1",
		path.c_str(), x, y);
	Common::String source;
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		openSource(path, kSourceDirectFile, source));
	if (!stream) {
		warning("Ripper: could not open WAC packetized presentation '%s'", path.c_str());
		return false;
	}
	if (detectMediaFormat(*stream) != kMediaFormatIavf) {
		warning("Ripper: unsupported WAC presentation format for '%s'", path.c_str());
		return false;
	}

	IndexedDisplaySnapshot displayContext;
	const bool captured = displayContext.capture();
	bool result = playIavf(*stream, path, true, x, y, 0, false, !captured, 1);
	if (captured && !_engine->shouldQuit()) {
		const bool restored = displayContext.restore();
		debugC(restored ? 2 : 1, kDebugVideo,
			"Ripper: restored WAC packetized presentation display media='%s' success=%d size=%dx%d",
			path.c_str(), restored, displayContext.bounds().width(),
			displayContext.bounds().height());
		if (!restored)
			result = false;
	}
	_input->drainKeys();
	return result;
}

bool MediaPlayer::playWacInterfaceSequence(const Common::String &path,
		const Common::Rect &centerBounds,
		uint loopStartFrame, MediaSequenceCallback *callback, uint16 *command) {
	debugC(1, kDebugVideo,
		"Ripper: entering WAC interface sequence media='%s' centerBounds=%d,%d,%d,%d palette=10..149 loopStartFrame=%u callback=%d",
		path.c_str(), centerBounds.left, centerBounds.top, centerBounds.right,
		centerBounds.bottom, loopStartFrame, callback != nullptr);
	const SmackerPlaybackPlan plan = makeWacInterfaceSequencePlan(
		centerBounds, loopStartFrame, callback, command);
	Common::String source;
	return playValidatedSmacker(
		openSource(path, kSourceInterfaceLibrary, source), path,
		"WAC interface sequence", plan);
}

bool MediaPlayer::playWacInterfaceSequenceStream(Common::SeekableReadStream *stream,
		const Common::String &name, const Common::Rect &centerBounds,
		uint loopStartFrame, MediaSequenceCallback *callback, uint16 *command) {
	debugC(1, kDebugVideo,
		"Ripper: entering streamed WAC interface sequence media='%s' centerBounds=%d,%d,%d,%d palette=10..149 loopStartFrame=%u callback=%d",
		name.c_str(), centerBounds.left, centerBounds.top, centerBounds.right,
		centerBounds.bottom, loopStartFrame, callback != nullptr);
	const SmackerPlaybackPlan plan = makeWacInterfaceSequencePlan(
		centerBounds, loopStartFrame, callback, command);
	return playValidatedSmacker(stream, name,
		"streamed WAC interface sequence", plan);
}

bool MediaPlayer::playInterfaceSequence(const Common::String &path, int x, int y,
		Common::Array<byte> &sourcePalette) {
	debugC(1, kDebugVideo,
		"Ripper: entering interface media presentation media='%s' position=%d,%d transparent=first-pixel",
		path.c_str(), x, y);
	SmackerPlaybackPlan plan;
	plan.placement.x = x;
	plan.placement.y = y;
	plan.rendering.transparentFirstPixel = true;
	plan.rendering.retainFinalTransparentFrame = true;
	plan.palette.patchInterfacePalette = false;
	plan.palette.preserveDisplayPalette = true;
	plan.palette.sourcePalette = &sourcePalette;
	plan.palette.rememberVideoPalette = false;
	Common::String source;
	return playValidatedSmacker(
		openSource(path, kSourceInterfaceLibrary, source), path,
		"interface sequence", plan);
}

bool MediaPlayer::playPuzzleSequence(const Common::String &path, uint loopStartFrame,
		MediaSequenceCallback *callback, uint16 *command) {
	debugC(1, kDebugVideo,
		"Ripper: entering puzzle Smacker sequence media='%s' loopStartFrame=%u callback=%d",
		path.c_str(), loopStartFrame, callback != nullptr);
	SmackerPlaybackPlan plan;
	plan.placement.x = 0;
	plan.placement.y = 0;
	plan.placement.originY = kScenePresentationTop;
	plan.loop.loopStartFrame = loopStartFrame;
	plan.callback.sequenceCallback = callback;
	plan.callback.sequenceCommand = command;
	Common::String source;
	return playValidatedSmacker(
		openSource(path, kSourceDirectFile, source), path,
		"puzzle sequence", plan);
}

bool MediaPlayer::playPuzzleSequenceStream(Common::SeekableReadStream *stream,
		const Common::String &name, int x, int y, uint loopStartFrame,
		MediaSequenceCallback *callback, uint16 *command) {
	debugC(1, kDebugVideo,
		"Ripper: entering archived puzzle Smacker sequence media='%s' "
		"position=%d,%d loopStartFrame=%u callback=%d",
		name.c_str(), x, y, loopStartFrame, callback != nullptr);
	SmackerPlaybackPlan plan;
	plan.placement.x = x;
	plan.placement.y = y;
	plan.placement.originY = kScenePresentationTop;
	plan.input.serviceSceneUi = true;
	plan.loop.loopStartFrame = loopStartFrame;
	plan.callback.sequenceCallback = callback;
	plan.callback.sequenceCommand = command;
	return playValidatedSmacker(stream, name,
		"archived puzzle sequence", plan);
}

bool MediaPlayer::playSceneStream(Common::SeekableReadStream *stream,
		const Common::String &name, int x, int y, bool allowEscSpace) {
	Common::ScopedPtr<Common::SeekableReadStream> ownedStream(stream);
	if (!ownedStream)
		return false;
	const MediaFormat format = detectMediaFormat(*ownedStream);
	if (format != kMediaFormatSmacker && format != kMediaFormatIavf) {
		warning("Ripper: unsupported archived scene media '%s' format=%s",
			name.c_str(), mediaFormatName(format));
		return false;
	}

	debugC(1, kDebugVideo,
		"Ripper: entering archived scene presentation media='%s' "
		"mode=%s position=%d,%d controls=%d",
		name.c_str(), mediaFormatName(format), x, y, allowEscSpace);
	bool result = false;
	if (format == kMediaFormatSmacker) {
		SmackerPlaybackPlan plan;
		plan.input.allowEscSpace = allowEscSpace;
		plan.placement.x = x;
		plan.placement.y = y;
		plan.placement.originY = kScenePresentationTop;
		result = playValidatedSmacker(ownedStream.release(), name,
			"archived scene", plan);
	} else {
		IndexedDisplaySnapshot displayContext;
		const bool captured = displayContext.capture();
		result = playIavf(*ownedStream, name, allowEscSpace, x, y,
			kScenePresentationTop, false, !captured);
		if (captured && !_engine->shouldQuit()) {
			const bool restored = displayContext.restore();
			debugC(restored ? 2 : 1, kDebugVideo,
				"Ripper: restored archived scene display media='%s' "
				"success=%d size=%dx%d",
				name.c_str(), restored, displayContext.bounds().width(),
				displayContext.bounds().height());
			if (!restored)
				result = false;
		}
	}
	_input->drainKeys();
	return result;
}

bool MediaPlayer::playPuzzleSequenceSegment(const Common::String &path, uint firstFrame,
		uint lastFrame, int x, int y, MediaSequenceCallback *callback, uint16 *command,
		uint boundedLoopStartFrame) {
	debugC(1, kDebugVideo,
		"Ripper: entering puzzle Smacker segment media='%s' frames=%u..%u loopStart=%d position=%d,%d callback=%d",
		path.c_str(), firstFrame, lastFrame,
		boundedLoopStartFrame == 0xffffffff ? -1 : (int)boundedLoopStartFrame,
		x, y, callback != nullptr);
	SmackerPlaybackPlan plan;
	plan.placement.x = x;
	plan.placement.y = y;
	plan.placement.originY = kScenePresentationTop;
	plan.callback.sequenceCallback = callback;
	plan.callback.sequenceCommand = command;
	plan.frames.firstFrame = firstFrame;
	plan.frames.lastFrame = lastFrame;
	plan.loop.boundedLoopStartFrame = boundedLoopStartFrame;
	Common::String source;
	return playValidatedSmacker(
		openSource(path, kSourceDirectFile, source), path,
		"puzzle segment", plan);
}

bool MediaPlayer::playCombatSequence(const Common::String &path,
		MediaSequenceCallback *callback, uint16 *command) {
	return playScaledInteractiveSequence(path, "combat", callback, command);
}

bool MediaPlayer::playShootingGallerySequence(const Common::String &path,
		MediaSequenceCallback *callback, uint16 *command) {
	return playScaledInteractiveSequence(path, "shooting-gallery", callback, command);
}

bool MediaPlayer::playBlobShooterSequence(const Common::String &path,
		MediaSequenceCallback *callback, uint16 *command) {
	// Advance KJ.SMK continuously while its local actor loop remains active.
	// Reconstruct frame one when the short background presentation wraps.
	return playScaledInteractiveSequence(path, "blob-shooter", callback, command, 1);
}

bool MediaPlayer::playScaledInteractiveSequence(const Common::String &path,
		const char *description, MediaSequenceCallback *callback, uint16 *command,
		uint loopStartFrame) {
	debugC(1, kDebugVideo,
		"Ripper: entering %s Smacker sequence media='%s' callback=%d",
		description, path.c_str(), callback != nullptr);
	SmackerPlaybackPlan plan;
	plan.placement.x = 0;
	plan.placement.y = 0;
	plan.placement.displayScale = kAutoPacketizedDisplayScale;
	plan.palette.patchInterfacePalette = true;
	plan.loop.loopStartFrame = loopStartFrame;
	plan.callback.sequenceCallback = callback;
	plan.callback.sequenceCommand = command;
	plan.palette.rememberVideoPalette = false;
	Common::String source;
	return playValidatedSmacker(
		openSource(path, kSourceDirectFile, source), path,
		description, plan);
}

bool MediaPlayer::playTransparentSmackerOverlay(const Common::String &path, int x, int y) {
	debugC(1, kDebugVideo,
		"Ripper: entering transparent Smacker overlay media='%s' position=%d,%d",
		path.c_str(), x, y);
	SmackerPlaybackPlan plan;
	plan.placement.x = x;
	plan.placement.y = y;
	plan.placement.originY = kScenePresentationTop;
	plan.rendering.transparentFirstPixel = true;
	Common::String source;
	return playValidatedSmacker(
		openSource(path, kSourceDirectFile, source), path,
		"transparent overlay", plan);
}

bool MediaPlayer::playInteractiveIavf(const Common::String &path,
		MediaSequenceCallback *callback, uint16 *command) {
	Common::String source;
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		openSource(path, kSourceDirectFile, source));
	if (!stream) {
		warning("Ripper: could not open interactive IAVF media '%s'", path.c_str());
		return false;
	}
	if (detectMediaFormat(*stream) != kMediaFormatIavf) {
		warning("Ripper: interactive media '%s' is not IAVF", path.c_str());
		return false;
	}

	debugC(1, kDebugVideo,
		"Ripper: entering interactive IAVF presentation media='%s' callback=%d",
		path.c_str(), callback != nullptr);
	const bool result = playIavf(*stream, path, false, -1, -1, 0, false,
		true, kAutoPacketizedDisplayScale, callback, command);
	_input->drainKeys();
	return result;
}

bool MediaPlayer::playScene(const Common::String &path, int x, int y, bool firstFrameOnly,
		bool loopUntilInput, bool allowEscSpace, MediaSequenceCallback *callback,
		uint16 *command) {
	debugC(1, kDebugVideo,
		"Ripper: entering scene presentation media='%s' firstFrameOnly=%d loopUntilInput=%d controls=%d callback=%d",
		path.c_str(), firstFrameOnly, loopUntilInput, allowEscSpace, callback != nullptr);
	Common::String source;
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		openSource(path, kSourceConfiguredPath, source));
	if (!stream) {
		warning("Ripper: could not open scene media '%s'", path.c_str());
		return false;
	}

	const uint frameLimit = firstFrameOnly ? 1 : 0;
	const MediaFormat format = detectMediaFormat(*stream);
	const bool isSmacker = format == kMediaFormatSmacker;
	const bool isIavf = format == kMediaFormatIavf;
	debugC(2, kDebugVideo,
		"Ripper: scene media '%s' mode=%s scriptPosition=%d,%d originY=%d controls=%d",
		path.c_str(), firstFrameOnly ? "first-frame-preview" : "sequence", x, y,
		kScenePresentationTop, allowEscSpace);
	bool result = false;
	_stopSceneOnMouse = loopUntilInput && !firstFrameOnly;
	const bool loop = loopUntilInput && !firstFrameOnly;
	uint pass = 0;
	do {
		const bool repeatedLoopPass = pass++ != 0;
		if (repeatedLoopPass) {
			stream.reset(openSource(path, kSourceConfiguredPath, source));
			if (!stream) {
				warning("Ripper: could not reopen looping scene media '%s'", path.c_str());
				result = false;
				break;
			}
		}
	if (isSmacker) {
		SmackerPlaybackPlan plan;
		plan.input.allowEscSpace = allowEscSpace;
		plan.placement.x = x;
		plan.placement.y = y;
		plan.frames.frameLimit = frameLimit;
		plan.placement.originY = kScenePresentationTop;
		plan.input.serviceSceneUi = true;
		plan.loop.loopFromStart = loop;
		plan.callback.sequenceCallback = callback;
		plan.callback.sequenceCommand = command;
		result = playValidatedSmacker(stream.release(), path, "scene", plan);
		if (!result && _stopSceneOnMouse && _input->peekMouseState().pressed != 0) {
			result = true;
			debugC(1, kDebugVideo,
				"Ripper: interactive scene media stopped by mouse; returning to hotspot polling");
		}
	} else if (isIavf && !firstFrameOnly) {
		if (callback) {
			warning("Ripper: scene callback is unsupported for packetized media '%s'",
				path.c_str());
			result = false;
			break;
		}
		result = playIavf(*stream, path, allowEscSpace, x, y,
			kScenePresentationTop, true, true);
		stream.reset();
	} else {
		warning("Ripper: unsupported scene media mode for '%s' format=%s",
			path.c_str(), mediaFormatName(format));
		stream.reset();
	}
	} while (loop && isIavf && !_input->peekMouseState().pressed &&
		!_engine->shouldQuit() && result &&
		!_engine->getScripts()->hasPendingSceneTransition());
	_input->drainKeys();
	_stopSceneOnMouse = false;
	return result;
}

} // End of namespace Ripper
