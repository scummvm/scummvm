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

#include "engines/util.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/graphics/helpers.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#include "sci/util.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/span.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"
#include "video/avi_decoder.h"
#include "video/qt_decoder.h"
#include "sci/video/seq_decoder.h"
#ifdef ENABLE_SCI32
#include "sci/engine/guest_additions.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/video32.h"
#include "sci/video/robot_decoder.h"
#endif

namespace Sci {

void playVideo(Video::VideoDecoder &videoDecoder) {
	videoDecoder.start();

	Common::SpanOwner<SciSpan<byte> > scaleBuffer;
	byte bytesPerPixel = videoDecoder.getPixelFormat().bytesPerPixel;
	uint16 width = videoDecoder.getWidth();
	uint16 height = videoDecoder.getHeight();
	uint16 pitch = videoDecoder.getWidth() * bytesPerPixel;
	uint16 screenWidth = g_sci->_gfxScreen->getDisplayWidth();
	uint16 screenHeight = g_sci->_gfxScreen->getDisplayHeight();
	uint32 numPixels;

	if (screenWidth == 640 && width <= 320 && height <= 240) {
		width *= 2;
		height *= 2;
		pitch *= 2;
		numPixels = width * height * bytesPerPixel;
		scaleBuffer->allocate(numPixels, "video scale buffer");
	}

	uint16 x = (screenWidth - width) / 2;
	uint16 y = (screenHeight - height) / 2;

	bool skipVideo = false;

	if (videoDecoder.hasDirtyPalette()) {
		const byte *palette = videoDecoder.getPalette();
		g_sci->_gfxScreen->setPalette(palette, 0, 255);
	}

	while (!g_engine->shouldQuit() && !videoDecoder.endOfVideo() && !skipVideo) {
		if (videoDecoder.needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder.decodeNextFrame();

			if (frame) {
				Common::Rect rect(x, y, x+width, y+height);
				if (scaleBuffer) {
					const SciSpan<const byte> input((const byte *)frame->getPixels(), frame->w * frame->h * bytesPerPixel);
					// TODO: Probably should do aspect ratio correction in KQ6
					g_sci->_gfxScreen->scale2x(input, *scaleBuffer, videoDecoder.getWidth(), videoDecoder.getHeight(), bytesPerPixel);
					g_sci->_gfxScreen->copyVideoFrameToScreen(scaleBuffer->getUnsafeDataAt(0, pitch * height), pitch, rect, bytesPerPixel == 1);
				} else {
					g_sci->_gfxScreen->copyVideoFrameToScreen((const byte *)frame->getPixels(), frame->pitch, rect, bytesPerPixel == 1);
				}

				if (videoDecoder.hasDirtyPalette()) {
					const byte *palette = videoDecoder.getPalette();
					g_sci->_gfxScreen->setPalette(palette, 0, 255);
				}

				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}
		if (g_sci->getEngineState()->_delayedRestoreGameId != -1)
			skipVideo = true;

		g_system->delayMillis(10);
	}
}

reg_t kShowMovie(EngineState *s, int argc, reg_t *argv) {
	// Hide the cursor if it's showing and then show it again if it was
	// previously visible.
	bool reshowCursor = g_sci->_gfxCursor->isVisible();
	if (reshowCursor)
		g_sci->_gfxCursor->kernelHide();

	uint16 screenWidth = g_system->getWidth();
	uint16 screenHeight = g_system->getHeight();

	Common::ScopedPtr<Video::VideoDecoder> videoDecoder;

	bool switchedGraphicsMode = false;

	if (argv[0].isPointer()) {
		Common::String filename = s->_segMan->getString(argv[0]);

		if (g_sci->getPlatform() == Common::kPlatformMacintosh) {
			// Mac QuickTime
			// The only argument is the string for the video

			// HACK: Switch to 16bpp graphics for Cinepak.
			if (g_system->getScreenFormat().bytesPerPixel == 1) {
				initGraphics(screenWidth, screenHeight, nullptr);
				switchedGraphicsMode = true;
			}

			if (g_system->getScreenFormat().bytesPerPixel == 1) {
				warning("This video requires >8bpp color to be displayed, but could not switch to RGB color mode");
				return NULL_REG;
			}

			videoDecoder.reset(new Video::QuickTimeDecoder());
			if (!videoDecoder->loadFile(filename))
				error("Could not open '%s'", filename.c_str());
		} else {
			// DOS SEQ
			// SEQ's are called with no subops, just the string and delay
			// Time is specified as ticks
			videoDecoder.reset(new SEQDecoder(argv[1].toUint16()));

			if (!videoDecoder->loadFile(filename)) {
				warning("Failed to open movie file %s", filename.c_str());
				videoDecoder.reset();
			}
		}
	} else {
		// Windows AVI
		// TODO: This appears to be some sort of subop. case 0 contains the string
		// for the video, so we'll just play it from there for now.

		switch (argv[0].toUint16()) {
		case 0: {
			Common::String filename = s->_segMan->getString(argv[1]);
			videoDecoder.reset(new Video::AVIDecoder());
			if (!videoDecoder->loadFile(filename.c_str())) {
				warning("Failed to open movie file %s", filename.c_str());
				videoDecoder.reset();
			}
			break;
		}
		default:
			warning("Unhandled SCI kShowMovie subop %d", argv[0].toUint16());
		}
	}

	if (videoDecoder) {
		bool is8bit = videoDecoder->getPixelFormat().bytesPerPixel == 1;

		playVideo(*videoDecoder);

		// HACK: Switch back to 8bpp if we played a true color video.
		// We also won't be copying the screen to the SCI screen...
		if (switchedGraphicsMode)
			initGraphics(screenWidth, screenHeight);
		else if (is8bit) {
			g_sci->_gfxScreen->kernelSyncWithFramebuffer();
			g_sci->_gfxPalette16->kernelSyncScreenPalette();
		}
	}

	if (reshowCursor)
		g_sci->_gfxCursor->kernelShow();

	return s->r_acc;
}

#ifdef ENABLE_SCI32
reg_t kShowMovie32(EngineState *s, int argc, reg_t *argv) {
	Common::String fileName = s->_segMan->getString(argv[0]);
	const int16 numTicks = argv[1].toSint16();
	const int16 x = argc > 3 ? argv[2].toSint16() : 0;
	const int16 y = argc > 3 ? argv[3].toSint16() : 0;

	if (g_sci->getPlatform() == Common::kPlatformMacintosh) {
		g_sci->_video32->getQuickTimePlayer().play(fileName);
	} else {
		g_sci->_video32->getSEQPlayer().play(fileName, numTicks, x, y);
	}

	return s->r_acc;
}

reg_t kRobot(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kRobotOpen(EngineState *s, int argc, reg_t *argv) {
	const GuiResourceId robotId = argv[0].toUint16();
	const reg_t plane = argv[1];
	const int16 priority = argv[2].toSint16();
	const int16 x = argv[3].toSint16();
	const int16 y = argv[4].toSint16();
	const int16 scale = argc > 5 ? argv[5].toSint16() : 128;
	g_sci->_video32->getRobotPlayer().open(robotId, plane, priority, x, y, scale);
	return make_reg(0, 0);
}
reg_t kRobotShowFrame(EngineState *s, int argc, reg_t *argv) {
	const uint16 frameNo = argv[0].toUint16();
	const uint16 newX = argc > 1 ? argv[1].toUint16() : (uint16)RobotDecoder::kUnspecified;
	const uint16 newY = argc > 1 ? argv[2].toUint16() : (uint16)RobotDecoder::kUnspecified;
	g_sci->_video32->getRobotPlayer().showFrame(frameNo, newX, newY, RobotDecoder::kUnspecified);
	return s->r_acc;
}

reg_t kRobotGetFrameSize(EngineState *s, int argc, reg_t *argv) {
	Common::Rect frameRect;
	const uint16 numFramesTotal = g_sci->_video32->getRobotPlayer().getFrameSize(frameRect);

	SciArray *outRect = s->_segMan->lookupArray(argv[0]);
	reg_t values[4] = {
		make_reg(0, frameRect.left),
		make_reg(0, frameRect.top),
		make_reg(0, frameRect.right - 1),
		make_reg(0, frameRect.bottom - 1) };
	outRect->setElements(0, 4, values);

	return make_reg(0, numFramesTotal);
}

reg_t kRobotPlay(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getRobotPlayer().resume();
	return s->r_acc;
}

reg_t kRobotGetIsFinished(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_video32->getRobotPlayer().getStatus() == RobotDecoder::kRobotStatusEnd);
}

reg_t kRobotGetIsInitialized(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_video32->getRobotPlayer().getStatus() != RobotDecoder::kRobotStatusUninitialized);
}

reg_t kRobotClose(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getRobotPlayer().close();
	return s->r_acc;
}

reg_t kRobotGetCue(EngineState *s, int argc, reg_t *argv) {
	writeSelectorValue(s->_segMan, argv[0], SELECTOR(signal), g_sci->_video32->getRobotPlayer().getCue());
	return s->r_acc;
}

reg_t kRobotPause(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getRobotPlayer().pause();
	return s->r_acc;
}

reg_t kRobotGetFrameNo(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_video32->getRobotPlayer().getFrameNo());
}

reg_t kRobotSetPriority(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getRobotPlayer().setPriority(argv[0].toSint16());
	return s->r_acc;
}

reg_t kShowMovieWin(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kShowMovieWinOpen(EngineState *s, int argc, reg_t *argv) {
	// SCI2.1 adds a movie ID to the call, but the movie ID is broken,
	// so just ignore it
	if (getSciVersion() > SCI_VERSION_2) {
		++argv;
		--argc;
	}

	const Common::String fileName = s->_segMan->getString(argv[0]);
	return make_reg(0, g_sci->_video32->getAVIPlayer().open(fileName));
}

reg_t kShowMovieWinInit(EngineState *s, int argc, reg_t *argv) {
	// SCI2.1 adds a movie ID to the call, but the movie ID is broken,
	// so just ignore it
	if (getSciVersion() > SCI_VERSION_2) {
		++argv;
		--argc;
	}

	// argv[0] is a broken x-coordinate
	// argv[1] is a broken y-coordinate
	// argv[2] is an optional broken width
	// argv[3] is an optional broken height
	const bool pixelDouble = argc > 3 && argv[2].toSint16() && argv[3].toSint16();
	return make_reg(0, g_sci->_video32->getAVIPlayer().init(pixelDouble));
}

reg_t kShowMovieWinPlay(EngineState *s, int argc, reg_t *argv) {
	if (getSciVersion() == SCI_VERSION_2) {
		AVIPlayer::EventFlags flags = (AVIPlayer::EventFlags)argv[0].toUint16();
		return make_reg(0, g_sci->_video32->getAVIPlayer().playUntilEvent(flags));
	} else {
		// argv[0] is a broken movie ID
		const int16 from = argc > 2 ? argv[1].toSint16() : 0;
		const int16 to = argc > 2 ? argv[2].toSint16() : 0;
		const int16 showStyle = argc > 3 ? argv[3].toSint16() : 0;
		const bool cue = argc > 4 ? (bool)argv[4].toSint16() : false;
		return make_reg(0, g_sci->_video32->getAVIPlayer().play(from, to, showStyle, cue));
	}
}

reg_t kShowMovieWinClose(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_video32->getAVIPlayer().close());
}

reg_t kShowMovieWinGetDuration(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_video32->getAVIPlayer().getDuration());
}

reg_t kShowMovieWinCue(EngineState *s, int argc, reg_t *argv) {
	// SCI2.1 adds a movie ID to the call, but the movie ID is broken,
	// so just ignore it
	if (getSciVersion() > SCI_VERSION_2) {
		++argv;
		--argc;
	}

	const uint16 frameNo = argv[0].toUint16();
	return make_reg(0, g_sci->_video32->getAVIPlayer().cue(frameNo));
}

reg_t kShowMovieWinPlayUntilEvent(EngineState *s, int argc, reg_t *argv) {
	const int defaultFlags =
		AVIPlayer::kEventFlagEnd |
		AVIPlayer::kEventFlagEscapeKey;

	// argv[0] is the movie number, which is not used by this method
	const AVIPlayer::EventFlags flags = (AVIPlayer::EventFlags)(argc > 1 ? argv[1].toUint16() : defaultFlags);

	return make_reg(0, g_sci->_video32->getAVIPlayer().playUntilEvent(flags));
}

reg_t kShowMovieWinInitDouble(EngineState *s, int argc, reg_t *argv) {
	// argv[0] is a broken movie ID
	// argv[1] is a broken x-coordinate
	// argv[2] is a broken y-coordinate
	return make_reg(0, g_sci->_video32->getAVIPlayer().init(true));
}

reg_t kPlayVMD(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kPlayVMDOpen(EngineState *s, int argc, reg_t *argv) {
	const Common::String fileName = s->_segMan->getString(argv[0]);
	// argv[1] is an optional cache size argument which we do not use
	// const uint16 cacheSize = argc > 1 ? CLIP<int16>(argv[1].toSint16(), 16, 1024) : 0;
	const VMDPlayer::OpenFlags flags = argc > 2 ? (VMDPlayer::OpenFlags)argv[2].toUint16() : VMDPlayer::kOpenFlagNone;

	return make_reg(0, g_sci->_video32->getVMDPlayer().open(fileName, flags));
}

reg_t kPlayVMDInit(EngineState *s, int argc, reg_t *argv) {
	const int16 x = argv[0].toSint16();
	const int16 y = argv[1].toSint16();
	const VMDPlayer::PlayFlags flags = argc > 2 ? (VMDPlayer::PlayFlags)argv[2].toUint16() : VMDPlayer::kPlayFlagNone;
	int16 boostPercent;
	int16 boostStartColor;
	int16 boostEndColor;
	if (argc > 5 && (flags & VMDPlayer::kPlayFlagBoost)) {
		boostPercent = argv[3].toSint16();
		boostStartColor = argv[4].toSint16();
		boostEndColor = argv[5].toSint16();
	} else {
		boostPercent = 0;
		boostStartColor = -1;
		boostEndColor = -1;
	}

	g_sci->_video32->getVMDPlayer().init(x, y, flags, boostPercent, boostStartColor, boostEndColor);

	return make_reg(0, 0);
}

reg_t kPlayVMDClose(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_video32->getVMDPlayer().close());
}

reg_t kPlayVMDIgnorePalettes(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getVMDPlayer().ignorePalettes();
	return s->r_acc;
}

reg_t kPlayVMDGetStatus(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_video32->getVMDPlayer().getStatus());
}

reg_t kPlayVMDPlayUntilEvent(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->_guestAdditions->kPlayDuckPlayVMDHook()) {
		return make_reg(0, VMDPlayer::kEventFlagEnd);
	}

	const VMDPlayer::EventFlags flags = (VMDPlayer::EventFlags)argv[0].toUint16();
	const int16 lastFrameNo = argc > 1 ? argv[1].toSint16() : -1;
	const int16 yieldInterval = argc > 2 ? argv[2].toSint16() : -1;
	return make_reg(0, g_sci->_video32->getVMDPlayer().kernelPlayUntilEvent(flags, lastFrameNo, yieldInterval));
}

reg_t kPlayVMDShowCursor(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getVMDPlayer().setShowCursor((bool)argv[0].toUint16());
	return s->r_acc;
}

reg_t kPlayVMDStartBlob(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getVMDPlayer().deleteBlobs();
	return NULL_REG;
}

reg_t kPlayVMDStopBlobs(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getVMDPlayer().deleteBlobs();
	return NULL_REG;
}

reg_t kPlayVMDAddBlob(EngineState *s, int argc, reg_t *argv) {
	int16 squareSize = argv[0].toSint16();
	int16 top = argv[1].toSint16();
	int16 left = argv[2].toSint16();
	int16 bottom = argv[3].toSint16();
	int16 right = argv[4].toSint16();
	int16 blobNumber = g_sci->_video32->getVMDPlayer().addBlob(squareSize, top, left, bottom, right);
	return make_reg(0, blobNumber);
}

reg_t kPlayVMDDeleteBlob(EngineState *s, int argc, reg_t *argv) {
	int16 blobNumber = argv[0].toSint16();
	g_sci->_video32->getVMDPlayer().deleteBlob(blobNumber);
	return SIGNAL_REG;
}

reg_t kPlayVMDSetBlackoutArea(EngineState *s, int argc, reg_t *argv) {
	const int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
	const int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();

	Common::Rect blackoutArea;
	blackoutArea.left = MAX<int16>(0, argv[0].toSint16());
	blackoutArea.top = MAX<int16>(0, argv[1].toSint16());
	blackoutArea.right = MIN<int16>(scriptWidth, argv[2].toSint16() + 1);
	blackoutArea.bottom = MIN<int16>(scriptHeight, argv[3].toSint16() + 1);
	g_sci->_video32->getVMDPlayer().setBlackoutArea(blackoutArea);
	return s->r_acc;
}

reg_t kPlayVMDRestrictPalette(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getVMDPlayer().restrictPalette(argv[0].toUint16(), argv[1].toUint16());
	return s->r_acc;
}

reg_t kPlayVMDSetPlane(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getVMDPlayer().setPlane(argv[0].toSint16(), argc > 1 ? argv[1] : NULL_REG);
	return s->r_acc;
}

reg_t kPlayDuck(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kPlayDuckPlay(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->_guestAdditions->kPlayDuckPlayVMDHook()) {
		return NULL_REG;
	}
	kPlayDuckOpen(s, argc, argv);
	g_sci->_video32->getDuckPlayer().play(-1);
	g_sci->_video32->getDuckPlayer().close();
	return NULL_REG;
}

reg_t kPlayDuckSetFrameOut(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getDuckPlayer().setDoFrameOut((bool)argv[0].toUint16());
	return NULL_REG;
}

reg_t kPlayDuckOpen(EngineState *s, int argc, reg_t *argv) {
	const GuiResourceId resourceId = argv[0].toUint16();
	const int displayMode = argv[1].toSint16();
	const int16 x = argv[2].toSint16();
	const int16 y = argv[3].toSint16();
	// argv[4] is a cache size argument that we do not use
	g_sci->_video32->getDuckPlayer().open(resourceId, displayMode, x, y);
	return NULL_REG;
}

reg_t kPlayDuckClose(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getDuckPlayer().close();
	return NULL_REG;
}

reg_t kPlayDuckSetVolume(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getDuckPlayer().setVolume(argv[0].toUint16());
	return NULL_REG;
}

#endif

} // End of namespace Sci
