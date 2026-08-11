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

#include "backends/keymapper/keymap.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"
#if defined(USE_MPEG2) && defined(USE_A52)
#include "video/mpegps_decoder.h"
#endif
#include "zvision/zvision.h"
#include "zvision/core/clock.h"
#include "zvision/file/file_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/sound/volume_manager.h"
#include "zvision/text/subtitle_manager.h"
#include "zvision/video/rlf_decoder.h"
#include "zvision/video/zork_avi_decoder.h"

namespace ZVision {

Video::VideoDecoder *ZVision::loadAnimation(const Common::Path &fileName) {
	debugC(5, kDebugVideo, "loadAnimation()");
	Common::String tmpFileName = fileName.baseName();
	tmpFileName.toLowercase();
	Video::VideoDecoder *animation = NULL;

	debugC(1, kDebugVideo, "Loading animation %s", fileName.toString().c_str());

	if (tmpFileName.hasSuffix(".rlf"))
		animation = new RLFDecoder();
	else if (tmpFileName.hasSuffix(".avi"))
		animation = new ZorkAVIDecoder();
#if defined(USE_MPEG2) && defined(USE_A52)
	else if (tmpFileName.hasSuffix(".vob")) {
 		double amplification = getVolumeManager()->getVobAmplification(tmpFileName);
		animation = new Video::MPEGPSDecoder(amplification);
	}
#endif
	else
		error("Unknown suffix for animation %s", fileName.toString().c_str());

	Common::File *file = getFileManager()->open(fileName);
	if (!file)
		error("Error opening %s", fileName.toString().c_str());

	bool loaded = animation->loadStream(file);
	if (!loaded)
		error("Error loading animation %s", fileName.toString().c_str());

	debugC(5, kDebugVideo, "~loadAnimation()");
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

void ZVision::playVideo(Video::VideoDecoder &vid, Common::Rect dstRect, bool skippable, uint16 sub, Common::Rect srcRect) {
	Common::Rect frameArea = Common::Rect(vid.getWidth(), vid.getHeight());
	Common::Rect workingArea = _renderManager->getWorkingArea();
	// If dstRect is empty, no specific scaling was requested. However, we may choose to do scaling anyway
	bool scaled = false;
	workingArea.moveTo(0, 0); // Set local origin system in this scope to origin of working area

	debugC(1, kDebugVideo, "Playing video, source %d,%d,%d,%d, at destination %d,%d,%d,%d", srcRect.left, srcRect.top, srcRect.right, srcRect.bottom, dstRect.left, dstRect.top, dstRect.right, dstRect.bottom);

	if (dstRect.isEmpty())
		dstRect = frameArea;
	dstRect.clip(workingArea);

	debugC(2, kDebugVideo, "Clipped dstRect = %d,%d,%d,%d", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom);

	if (srcRect.isEmpty())
		srcRect = frameArea;
	else
		srcRect.clip(frameArea);

	debugC(2, kDebugVideo, "Clipped srcRect = %d,%d,%d,%d", srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);

	Graphics::ManagedSurface &outSurface = _renderManager->getVidSurface(dstRect);
	dstRect.moveTo(0, 0);
	dstRect.clip(Common::Rect(outSurface.w, outSurface.h));

	debugC(2, kDebugVideo, "dstRect clipped with outSurface = %d,%d,%d,%d", dstRect.left, dstRect.top, dstRect.right, dstRect.bottom);

	debugC(1, kDebugVideo, "Final size %d x %d, at working window coordinates %d, %d", srcRect.width(), srcRect.height(), dstRect.left, dstRect.top);
	if (srcRect.width() != dstRect.width() || srcRect.height() != dstRect.height()) {
		debugC(1, kDebugVideo, "Video will be scaled from %dx%d to %dx%d", srcRect.width(), srcRect.height(), dstRect.width(), dstRect.height());
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
					if (ConfMan.hasKey("confirm_exit") && ConfMan.getBool("confirm_exit")) {
						if (quit(true, true))
							vid.stop();
					}
					else {
						quit(false);
						vid.stop();
					}
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
				_renderManager->renderSceneToScreen(true, true, true); // Redraw text area to clean background of subtitles for videos that don't fill entire working area, e.g, Nemesis sarcophagi
				if (scaled) {
					debugC(8, kDebugVideo, "Scaled blit from area %d x %d to video output surface at output surface position %d, %d", srcRect.width(), srcRect.height(), dstRect.left, dstRect.top);
					outSurface.blitFrom(*frame, srcRect, dstRect);
				} else {
					debugC(8, kDebugVideo, "Simple blit from area %d x %d to video output surface at output surface position %d, %d", srcRect.width(), srcRect.height(), dstRect.left, dstRect.top);
					outSurface.simpleBlitFrom(*frame, srcRect, dstRect.origin());
				}
				_subtitleManager->process(0);
			}
		}

		// Always update the screen so the mouse continues to render & video does not skip
		_renderManager->renderSceneToScreen(true, true, false);

		_system->delayMillis(vid.getTimeToNextFrame() / 2); // Exponentially decaying delay
	}
	
	vid.close();	// Ensure resources are freed.
	
	_cutscenesKeymap->setEnabled(false);
	_gameKeymap->setEnabled(true);

	_videoIsPlaying = false;
	_clock.start();

	debugC(1, kDebugVideo, "Video playback complete");
}

} // End of namespace ZVision
