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

#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/graphics/helpers.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/gfxdrivers.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#include "sci/util.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/span.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
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

	if (screenWidth == 640 && width <= 320 && height <= 240) {
		width *= 2;
		height *= 2;
		pitch *= 2;
		uint32 numPixels = width * height * bytesPerPixel;
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
					g_sci->_gfxScreen->copyVideoFrameToScreen(scaleBuffer->getUnsafeDataAt(0, pitch * height), pitch, rect);
				} else {
					g_sci->_gfxScreen->copyVideoFrameToScreen((const byte *)frame->getPixels(), frame->pitch, rect);
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
	reg_t retval = s->r_acc;

	// Hide the cursor if it's showing and then show it again if it was
	// previously visible.
	bool reshowCursor = g_sci->_gfxCursor->isVisible();
	if (reshowCursor)
		g_sci->_gfxCursor->kernelHide();

	Common::ScopedPtr<Video::VideoDecoder> videoDecoder;

	bool switchedGraphicsMode = false;
	bool syncLastFrame = true;

	if (argv[0].isPointer()) {
		Common::Path filename(s->_segMan->getString(argv[0]));

		if (g_sci->getPlatform() == Common::kPlatformMacintosh) {
			// Mac QuickTime: the only argument is the string for the video
			videoDecoder.reset(new Video::QuickTimeDecoder());
			if (!videoDecoder->loadFile(filename)) {
				warning("Could not open '%s'", filename.toString().c_str());
				return NULL_REG;
			}

			Graphics::PixelFormat screenFormat = g_system->getScreenFormat();

			if (videoDecoder->getPixelFormat() != screenFormat) {
				// Attempt to switch to a screen format with higher bpp
				const Common::List<Graphics::PixelFormat> supportedFormats = g_system->getSupportedFormats();
				Common::List<Graphics::PixelFormat>::const_iterator it;
				for (it = supportedFormats.begin(); it != supportedFormats.end(); ++it) {
					if (it->bytesPerPixel >= videoDecoder->getPixelFormat().bytesPerPixel) {
						screenFormat = *it;
						break;
					}
				}
			}

			if (screenFormat.isCLUT8()) {
				// We got an indexed screen format, so dither the QuickTime video.
				uint8 palette[256 * 3];
				g_sci->_gfxScreen->grabPalette(palette, 0, 256);
				videoDecoder->setDitheringPalette(palette);
			} else {
				// Init the screen again with an RGB source format.
				// This is needed so that the GFX driver is aware that we'll be
				// sending RGB instead of paletted graphics.
				g_sci->_gfxScreen->gfxDriver()->initScreen(&screenFormat);
				videoDecoder->setOutputPixelFormat(g_system->getScreenFormat());
			}

			// Switch back to the normal screen format, once the QT video is done playing.
			// This ensures that the source graphics are in paletted format, but the screen
			// can be either in paletted or RGB format, if the user has checked the RGB
			// mode checkbox.
			switchedGraphicsMode = true;
			// Never sync the last frame for QT movies
			syncLastFrame = false;
		} else {
			// DOS SEQ
			// SEQ's are called with no subops, just the string and delay
			// Time is specified as ticks
			videoDecoder.reset(new SEQDecoder(argv[1].toUint16()));

			if (!videoDecoder->loadFile(filename)) {
				warning("Failed to open movie file %s", filename.toString().c_str());
				videoDecoder.reset();
			}
		}
	} else {
		// Windows AVI: Only used by KQ6 CD for the Sierra logo and intro cartoon.
		// The first parameter is a subop. Some of the subops set the accumulator.
		// The interpreter implements subops 0-6. KQ6 only calls 0, 1, 2, 6.
		// Subop 0: Open movie file
		// Subop 1: Setup movie playback rectangle
		// Subop 2: Play movie
		// Subop 6: Close movie file
		// We just play it on opcode 0, since the config parameters that are passed
		// to opcodes 1 and 2 aren't properly used anyway (the video will be centered,
		// regardless of any x, y, width and height settings).
		// Using any other opcode than 0 would also require unblocking the engine
		// after the movie playback like this (with <pauseToken> being the second
		// argument passed to opcode 2):
		// invokeSelector(s, <pauseToken>, g_sci->getKernel()->findSelector("cue"), argc, argv);
		switch (argv[0].toUint16()) {
		case 0: {
			Common::String filename = s->_segMan->getString(argv[1]);
			// For KQ6, this changes the vertical 200/440 upscaling to 200/400, since this is the expected behavior. Also,
			// the calculation of the scaled x/y coordinates works slightly differently compared to the normal gfx rendering.
			g_sci->_gfxScreen->gfxDriver()->setFlags(GfxDriver::kMovieMode);
			videoDecoder.reset(new Video::AVIDecoder());
			if (!videoDecoder->loadFile(filename.c_str())) {
				warning("Failed to open movie file %s", filename.c_str());
				videoDecoder.reset();
			}
			syncLastFrame = false;
			retval = TRUE_REG;
			break;
		}
		default:
			debug(kDebugLevelVideo, "Unhandled kShowMovie subop %d", argv[0].toUint16());
		}
	}

	if (videoDecoder) {
		if (videoDecoder->getPixelFormat().bytesPerPixel > 1)
			syncLastFrame = false;

		playVideo(*videoDecoder);

		// Switch back to 8bpp if we played a true color video.
		// We also won't be copying the screen to the SCI screen.
		if (switchedGraphicsMode)
			g_sci->_gfxScreen->gfxDriver()->initScreen();
		else if (syncLastFrame) {
			g_sci->_gfxScreen->kernelSyncWithFramebuffer();
			g_sci->_gfxPalette16->kernelSyncScreenPalette();
		}

		g_sci->_gfxScreen->gfxDriver()->clearFlags(GfxDriver::kMovieMode);
	}

	if (reshowCursor)
		g_sci->_gfxCursor->kernelShow();

	return retval;
}

#ifdef ENABLE_SCI32
reg_t kShowMovie32(EngineState *s, int argc, reg_t *argv) {
	Common::Path fileName(s->_segMan->getString(argv[0]));
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
		//--argc;
	}

	const Common::Path fileName(s->_segMan->getString(argv[0]));
	return make_reg(0, g_sci->_video32->getAVIPlayer().open(fileName));
}

reg_t kShowMovieWinInit(EngineState *s, int argc, reg_t *argv) {
	// SCI2.1 adds a movie ID to the call, but the movie ID is broken,
	// so just ignore it
	if (getSciVersion() > SCI_VERSION_2) {
		++argv;
		//--argc;
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
		//--argc;
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
	const Common::Path fileName(s->_segMan->getString(argv[0]));
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
