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
#include "zvision/text/subtitles.h"
#include "zvision/video/rlf_decoder.h"
#include "zvision/video/zork_avi_decoder.h"

namespace ZVision {

Video::VideoDecoder *ZVision::loadAnimation(const Common::String &fileName) {
	Common::String tmpFileName = fileName;
	tmpFileName.toLowercase();
	Video::VideoDecoder *animation = NULL;

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
		error("Unknown suffix for animation %s", fileName.c_str());

	Common::File *_file = getSearchManager()->openFile(tmpFileName);
	if (!_file)
		error("Error opening %s", tmpFileName.c_str());

	bool loaded = animation->loadStream(_file);
	if (!loaded)
		error("Error loading animation %s", tmpFileName.c_str());

	return animation;
}

void ZVision::playVideo(Video::VideoDecoder &vid, const Common::Rect &destRect, bool skippable, Subtitle *sub) {
	Common::Rect dst = destRect;
	// If destRect is empty, no specific scaling was requested. However, we may choose to do scaling anyway
	if (dst.isEmpty())
		dst = Common::Rect(vid.getWidth(), vid.getHeight());

	Graphics::Surface *scaled = NULL;

	if (vid.getWidth() != dst.width() || vid.getHeight() != dst.height()) {
		scaled = new Graphics::Surface;
		scaled->create(dst.width(), dst.height(), vid.getPixelFormat());
	}

	uint16 x = _workingWindow.left + dst.left;
	uint16 y = _workingWindow.top + dst.top;
	uint16 finalWidth = dst.width() < _workingWindow.width() ? dst.width() : _workingWindow.width();
	uint16 finalHeight = dst.height() < _workingWindow.height() ? dst.height() : _workingWindow.height();
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
			if (sub && showSubs)
				sub->process(vid.getCurFrame());

			if (frame) {
				if (scaled) {
					_renderManager->scaleBuffer(frame->getPixels(), scaled->getPixels(), frame->w, frame->h, frame->format.bytesPerPixel, scaled->w, scaled->h);
					frame = scaled;
				}
				Common::Rect rect = Common::Rect(x, y, x + finalWidth, y + finalHeight);
				_renderManager->copyToScreen(*frame, rect, 0, 0);
				_renderManager->processSubs(0);
			}
		}

		// Always update the screen so the mouse continues to render
		_system->updateScreen();

		_system->delayMillis(vid.getTimeToNextFrame() / 2);
	}

	_cutscenesKeymap->setEnabled(false);
	_gameKeymap->setEnabled(true);

	_videoIsPlaying = false;
	_clock.start();

	if (scaled) {
		scaled->free();
		delete scaled;
	}
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
