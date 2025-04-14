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

#include "common/scummsys.h"
#include "common/system.h"
#include "video/video_decoder.h"
#if defined(USE_MPEG2) && defined(USE_A52)
#include "video/mpegps_decoder.h"
#endif
#include "engines/util.h"
#include "graphics/surface.h"

#include "backends/keymapper/keymap.h"

#include "zvision/zvision.h"
#include "zvision/core/clock.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/text/subtitle_manager.h"
#include "zvision/video/rlf_decoder.h"
#include "zvision/video/zork_avi_decoder.h"

namespace ZVision {

Video::VideoDecoder *ZVision::loadAnimation(const Common::Path &fileName) {
	Common::String tmpFileName = fileName.baseName();
	tmpFileName.toLowercase();
	Video::VideoDecoder *animation = NULL;

	debug(1, "Loading animation %s", fileName.toString().c_str());

	if (tmpFileName.hasSuffix(".rlf"))
		animation = new RLFDecoder();
	else if (tmpFileName.hasSuffix(".avi"))
		animation = new ZorkAVIDecoder();
#if defined(USE_MPEG2) && defined(USE_A52)
	else if (tmpFileName.hasSuffix(".vob")) {
 		double amplification = getVobAmplification(tmpFileName);
		animation = new Video::MPEGPSDecoder(amplification);
	}
#endif
	else
		error("Unknown suffix for animation %s", fileName.toString().c_str());

	Common::File *_file = getSearchManager()->openFile(fileName);
	if (!_file)
		error("Error opening %s", fileName.toString().c_str());

	bool loaded = animation->loadStream(_file);
	if (!loaded)
		error("Error loading animation %s", fileName.toString().c_str());

	return animation;
}

/**
 * Play video at specified location.
 *
 * Pauses clock & normal game loop for duration of video; will still update & render subtitles & cursor.
 *
 * @param vid       Source video
 * @param dstRect   Rectangle to play video into, defined relative to working window origin; video will scale to rectangle automatically.
 * @param skippable Allow video to be skipped
 * @param sub       Subtitle associated with video
 * @param srcRect   Rectangle within video frame, defined relative to frame origin, to blit to output.  Only used for removing baked-in letterboxing in ZGI DVD HD videos
 */

void ZVision::playVideo(Video::VideoDecoder &vid, const Common::Rect &dstRect, bool skippable, uint16 sub, const Common::Rect &srcRect) {
	Common::Rect _dstRect = dstRect;
	Common::Rect _srcRect = srcRect;
	Common::Rect _frameArea = Common::Rect(vid.getWidth(), vid.getHeight());
	Common::Rect _workingArea = _renderManager->getWorkingArea();
	// If dstRect is empty, no specific scaling was requested. However, we may choose to do scaling anyway
	bool scaled = false;
	_workingArea.moveTo(0, 0); //Set local origin system in this scope to origin of working area

	debug(1, "Playing video, source %d,%d,%d,%d, at destination %d,%d,%d,%d", _srcRect.left, _srcRect.top, _srcRect.right, _srcRect.bottom, _dstRect.left, _dstRect.top, _dstRect.right, _dstRect.bottom);

	if (_dstRect.isEmpty())
		_dstRect = _frameArea;
	_dstRect.clip(_workingArea);

	debug(2, "Clipped dstRect = %d,%d,%d,%d", _dstRect.left, _dstRect.top, _dstRect.right, _dstRect.bottom);

	if (_srcRect.isEmpty())
		_srcRect = _frameArea;
	else
		_srcRect.clip(_frameArea);

	debug(2, "Clipped srcRect = %d,%d,%d,%d", _srcRect.left, _srcRect.top, _srcRect.right, _srcRect.bottom);

	Graphics::ManagedSurface &outSurface = _renderManager->getVidSurface(_dstRect);
	_dstRect.moveTo(0, 0);
	_dstRect.clip(Common::Rect(outSurface.w, outSurface.h));

	debug(2, "dstRect clipped with outSurface = %d,%d,%d,%d", _dstRect.left, _dstRect.top, _dstRect.right, _dstRect.bottom);

	debug(1, "Final size %d x %d, at working window coordinates %d, %d", _srcRect.width(), _srcRect.height(), _dstRect.left, _dstRect.top);
	if (_srcRect.width() != _dstRect.width() || _srcRect.height() != _dstRect.height()) {
		debug(1, "Video will be scaled from %dx%d to %dx%d", _srcRect.width(), _srcRect.height(), _dstRect.width(), _dstRect.height());
		scaled = true;
	}

	bool showSubs = (_scriptManager->getStateValue(StateKey_Subtitles) == 1);

	_clock.stop();
	vid.start();
	_videoIsPlaying = true;

	_cutscenesKeymap->setEnabled(true);
	_gameKeymap->setEnabled(false);

	// Only continue while the video is still playing
	while (!shouldQuit() && !vid.endOfVideo() && vid.isPlaying()) {
		// Check for engine quit and video stop key presses
		while (_eventMan->pollEvent(_event)) {
			switch (_event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				switch ((ZVisionAction)_event.customType) {
				case kZVisionActionQuit:
					quitGame();
					break;
				case kZVisionActionSkipCutscene:
					if (skippable) {
						vid.stop();
					}
					break;
				default:
					break;
				}
			default:
				break;
			}
		}

		if (vid.needsUpdate()) {
			const Graphics::Surface *frame = vid.decodeNextFrame();
			if (showSubs && sub > 0)
				_subtitleManager->update(vid.getCurFrame(), sub);

			if (frame) {
				_renderManager->renderSceneToScreen(true, true, true); //Redraw text area to clean background of subtitles for videos that don't fill entire working area, e.g, Nemesis sarcophagi
				if (scaled) {
					debug(8, "Scaled blit from area %d x %d to video output surface at output surface position %d, %d", _srcRect.width(), _srcRect.height(), _dstRect.left, _dstRect.top);
					outSurface.blitFrom(*frame, _srcRect, _dstRect);
				} else {
					debug(8, "Simple blit from area %d x %d to video output surface at output surface position %d, %d", _srcRect.width(), _srcRect.height(), _dstRect.left, _dstRect.top);
					outSurface.simpleBlitFrom(*frame, _srcRect, _dstRect.origin());
				}
				_subtitleManager->process(0);
			}
		}

		// Always update the screen so the mouse continues to render & video does not skip
		_renderManager->renderSceneToScreen(true, true, false);

		_system->delayMillis(vid.getTimeToNextFrame() / 2); //Exponentially decaying delay
	}

	_cutscenesKeymap->setEnabled(false);
	_gameKeymap->setEnabled(true);

	_videoIsPlaying = false;
	_clock.start();

	debug(1, "Video playback complete");
}

double ZVision::getVobAmplification(Common::String fileName) const {
	// For some reason, we get much lower volume in the hi-res videos than
	// in the low-res ones. So we artificially boost the volume. This is an
	// approximation, but I've tried to match the old volumes reasonably
	// well.
	//
	// Some of these will cause audio clipping. Hopefully not enough to be
	// noticeable.
	double amplification = 0.0;
	if (fileName == "em00d011.vob") {
		// The finale.
		amplification = 10.0;
	} else if (fileName == "em00d021.vob") {
		// Jack's escape and arrival at Flathead Mesa.
		amplification = 9.0;
	} else if (fileName == "em00d032.vob") {
		// The Grand Inquisitor's speech.
		amplification = 11.0;
	} else if (fileName == "em00d122.vob") {
		// Jack orders you to the radio tower.
		amplification = 17.0;
	} else if (fileName == "em3ed012.vob") {
		// The Grand Inquisitor gets the Coconut of Quendor.
		amplification = 12.0;
	} else if (fileName == "g000d101.vob") {
		// Griff gets captured.
		amplification = 11.0;
	} else if (fileName == "g000d111.vob") {
		// Brog gets totemized. The music seems to be mixed much softer
		// in this than in the low-resolution version.
		amplification = 12.0;
	} else if (fileName == "g000d122.vob") {
		// Lucy gets captured.
		amplification = 14.0;
	} else if (fileName == "g000d302.vob") {
		// The Grand Inquisitor visits Jack in his cell.
		amplification = 13.0;
	} else if (fileName == "g000d312.vob") {
		// You get captured.
		amplification = 14.0;
	} else if (fileName == "g000d411.vob") {
		// Propaganda On Parade. No need to make it as loud as the
		// low-resolution version.
		amplification = 11.0;
	} else if (fileName == "pe1ed012.vob") {
		// Jack lets you in with the lantern.
		amplification = 14.0;
	} else if (fileName.hasPrefix("pe1ed")) {
		// Jack answers the door. Several different ways.
		amplification = 17.0;
	} else if (fileName == "pe5ed052.vob") {
		// You get killed by the guards
		amplification = 12.0;
	} else if (fileName == "pe6ed012.vob") {
		// Jack gets captured by the guards
		amplification = 17.0;
	} else if (fileName == "pp1ed022.vob") {
		// Jack examines the lantern
		amplification = 10.0;
	} else if (fileName == "qb1ed012.vob") {
		// Lucy gets invited to the back room
		amplification = 17.0;
	} else if (fileName.hasPrefix("qe1ed")) {
		// Floyd answers the door. Several different ways.
		amplification = 17.0;
	} else if (fileName == "qs1ed011.vob") {
		// Jack explains the rules of the game.
		amplification = 16.0;
	} else if (fileName == "qs1ed021.vob") {
		// Jack loses the game.
		amplification = 14.0;
	} else if (fileName == "uc1gd012.vob") {
		// Y'Gael appears.
		amplification = 12.0;
	} else if (fileName == "ue1ud012.vob") {
		// Jack gets totemized... or what?
		amplification = 12.0;
	} else if (fileName == "ue2qd012.vob") {
		// Jack agrees to totemization.
		amplification = 10.0;
	} else if (fileName == "g000d981.vob") {
		// The Enterprise logo. Has no low-res version. Its volume is
		// louder than the other logo animations.
		amplification = 6.2;
	} else if (fileName.hasPrefix("g000d")) {
		// The Dolby Digital and Activision logos. They have no low-res
		// versions, but I've used the low-resolution Activision logo
		// (slightly different) as reference.
		amplification = 8.5;
	}
	return amplification;
}

} // End of namespace ZVision
