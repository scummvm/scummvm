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
#include "common/events.h"
#include "common/keyboard.h"
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
#include "sci/graphics/frameout.h"
#include "sci/graphics/video32.h"
#include "sci/video/robot_decoder.h"
#endif

namespace Sci {

void playVideo(Video::VideoDecoder *videoDecoder, VideoState videoState) {
	if (!videoDecoder)
		return;

	videoDecoder->start();

	byte *scaleBuffer = 0;
	byte bytesPerPixel = videoDecoder->getPixelFormat().bytesPerPixel;
	uint16 width = videoDecoder->getWidth();
	uint16 height = videoDecoder->getHeight();
	uint16 pitch = videoDecoder->getWidth() * bytesPerPixel;
	uint16 screenWidth = g_sci->_gfxScreen->getDisplayWidth();
	uint16 screenHeight = g_sci->_gfxScreen->getDisplayHeight();

	videoState.fileName.toLowercase();
	bool isVMD = videoState.fileName.hasSuffix(".vmd");

	if (screenWidth == 640 && width <= 320 && height <= 240 && ((videoState.flags & kDoubled) || !isVMD)) {
		width *= 2;
		height *= 2;
		pitch *= 2;
		scaleBuffer = new byte[width * height * bytesPerPixel];
	}

	uint16 x, y;

	// Sanity check...
	if (videoState.x > 0 && videoState.y > 0 && isVMD) {
		x = videoState.x;
		y = videoState.y;

		if (x + width > screenWidth || y + height > screenHeight) {
			// Happens in the Lighthouse demo
			warning("VMD video won't fit on screen, centering it instead");
			x = (screenWidth - width) / 2;
			y = (screenHeight - height) / 2;
		}
	} else {
		x = (screenWidth - width) / 2;
		y = (screenHeight - height) / 2;
	}

	bool skipVideo = false;
	EngineState *s = g_sci->getEngineState();

	if (videoDecoder->hasDirtyPalette()) {
		const byte *palette = videoDecoder->getPalette() + s->_vmdPalStart * 3;
		g_system->getPaletteManager()->setPalette(palette, s->_vmdPalStart, s->_vmdPalEnd - s->_vmdPalStart);
	}

	while (!g_engine->shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();

			if (frame) {
				if (scaleBuffer) {
					// TODO: Probably should do aspect ratio correction in e.g. GK1 Windows
					g_sci->_gfxScreen->scale2x((const byte *)frame->getPixels(), scaleBuffer, videoDecoder->getWidth(), videoDecoder->getHeight(), bytesPerPixel);
					g_system->copyRectToScreen(scaleBuffer, pitch, x, y, width, height);
				} else {
					g_system->copyRectToScreen(frame->getPixels(), frame->pitch, x, y, width, height);
				}

				if (videoDecoder->hasDirtyPalette()) {
					const byte *palette = videoDecoder->getPalette() + s->_vmdPalStart * 3;
					g_system->getPaletteManager()->setPalette(palette, s->_vmdPalStart, s->_vmdPalEnd - s->_vmdPalStart);
				}

				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}
		if (g_sci->getEngineState()->_delayedRestoreGame)
			skipVideo = true;

		g_system->delayMillis(10);
	}

	delete[] scaleBuffer;
	delete videoDecoder;
}

reg_t kShowMovie(EngineState *s, int argc, reg_t *argv) {
	// Hide the cursor if it's showing and then show it again if it was
	// previously visible.
	bool reshowCursor = g_sci->_gfxCursor->isVisible();
	if (reshowCursor)
		g_sci->_gfxCursor->kernelHide();

	uint16 screenWidth = g_system->getWidth();
	uint16 screenHeight = g_system->getHeight();

	Video::VideoDecoder *videoDecoder = 0;

	if (argv[0].getSegment() != 0) {
		Common::String filename = s->_segMan->getString(argv[0]);

		if (g_sci->getPlatform() == Common::kPlatformMacintosh) {
			// Mac QuickTime
			// The only argument is the string for the video

			// HACK: Switch to 16bpp graphics for Cinepak.
			initGraphics(screenWidth, screenHeight, screenWidth > 320, NULL);

			if (g_system->getScreenFormat().bytesPerPixel == 1) {
				warning("This video requires >8bpp color to be displayed, but could not switch to RGB color mode");
				return NULL_REG;
			}

			videoDecoder = new Video::QuickTimeDecoder();
			if (!videoDecoder->loadFile(filename))
				error("Could not open '%s'", filename.c_str());
		} else {
			// DOS SEQ
			// SEQ's are called with no subops, just the string and delay
			// Time is specified as ticks
			videoDecoder = new SEQDecoder(argv[1].toUint16());

			if (!videoDecoder->loadFile(filename)) {
				warning("Failed to open movie file %s", filename.c_str());
				delete videoDecoder;
				videoDecoder = 0;
			}
		}
	} else {
		// Windows AVI
		// TODO: This appears to be some sort of subop. case 0 contains the string
		// for the video, so we'll just play it from there for now.

#ifdef ENABLE_SCI32
		if (getSciVersion() >= SCI_VERSION_2_1_EARLY) {
			// SCI2.1 always has argv[0] as 1, the rest of the arguments seem to
			// follow SCI1.1/2.
			if (argv[0].toUint16() != 1)
				error("SCI2.1 kShowMovie argv[0] not 1");
			argv++;
			argc--;
		}
#endif
		switch (argv[0].toUint16()) {
		case 0: {
			Common::String filename = s->_segMan->getString(argv[1]);
			videoDecoder = new Video::AVIDecoder();

			if (filename.equalsIgnoreCase("gk2a.avi")) {
				// HACK: Switch to 16bpp graphics for Indeo3.
				// The only known movie to do use this codec is the GK2 demo trailer
				// If another video turns up that uses Indeo, we may have to add a better
				// check.
				initGraphics(screenWidth, screenHeight, screenWidth > 320, NULL);

				if (g_system->getScreenFormat().bytesPerPixel == 1) {
					warning("This video requires >8bpp color to be displayed, but could not switch to RGB color mode");
					return NULL_REG;
				}
			}

			if (!videoDecoder->loadFile(filename.c_str())) {
				warning("Failed to open movie file %s", filename.c_str());
				delete videoDecoder;
				videoDecoder = 0;
			} else {
				s->_videoState.fileName = filename;
			}
			break;
		}
		default:
			warning("Unhandled SCI kShowMovie subop %d", argv[0].toUint16());
		}
	}

	if (videoDecoder) {
		playVideo(videoDecoder, s->_videoState);

		// HACK: Switch back to 8bpp if we played a true color video.
		// We also won't be copying the screen to the SCI screen...
		if (g_system->getScreenFormat().bytesPerPixel != 1)
			initGraphics(screenWidth, screenHeight, screenWidth > 320);
		else if (getSciVersion() < SCI_VERSION_2) {
			g_sci->_gfxScreen->kernelSyncWithFramebuffer();
			g_sci->_gfxPalette16->kernelSyncScreenPalette();
		}
	}

	if (reshowCursor)
		g_sci->_gfxCursor->kernelShow();

	return s->r_acc;
}

#ifdef ENABLE_SCI32

reg_t kRobot(EngineState *s, int argc, reg_t *argv) {
	int16 subop = argv[0].toUint16();

	switch (subop) {
	case 0: { // init
		int id = argv[1].toUint16();
		reg_t obj = argv[2];
		int16 flag = argv[3].toSint16();
		int16 x = argv[4].toUint16();
		int16 y = argv[5].toUint16();
		warning("kRobot(init), id %d, obj %04x:%04x, flag %d, x=%d, y=%d", id, PRINT_REG(obj), flag, x, y);
		g_sci->_robotDecoder->load(id);
		g_sci->_robotDecoder->start();
		g_sci->_robotDecoder->setPos(x, y);
		}
		break;
	case 1:	// LSL6 hires (startup)
		// TODO
		return NULL_REG;	// an integer is expected
	case 4: {	// start - we don't really have a use for this one
			//int id = argv[1].toUint16();
			//warning("kRobot(start), id %d", id);
		}
		break;
	case 7:	// unknown, called e.g. by Phantasmagoria
		warning("kRobot(%d)", subop);
		break;
	case 8: // sync
		//if (true) {	// debug: automatically skip all robot videos
		if (g_sci->_robotDecoder->endOfVideo()) {
			g_sci->_robotDecoder->close();
			// Signal the engine scripts that the video is done
			writeSelector(s->_segMan, argv[1], SELECTOR(signal), SIGNAL_REG);
		} else {
			writeSelector(s->_segMan, argv[1], SELECTOR(signal), NULL_REG);
		}
		break;
	default:
		warning("kRobot(%d)", subop);
		break;
	}

	return s->r_acc;
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

reg_t kPlayVMDPlayUntilEvent(EngineState *s, int argc, reg_t *argv) {
	const VMDPlayer::EventFlags flags = (VMDPlayer::EventFlags)argv[0].toUint16();
	const int16 lastFrameNo = argc > 1 ? argv[1].toSint16() : -1;
	const int16 yieldInterval = argc > 2 ? argv[2].toSint16() : -1;
	return make_reg(0, g_sci->_video32->getVMDPlayer().kernelPlayUntilEvent(flags, lastFrameNo, yieldInterval));
}

reg_t kPlayVMDShowCursor(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getVMDPlayer().setShowCursor((bool)argv[0].toUint16());
	return s->r_acc;
}

reg_t kPlayVMDSetBlackoutArea(EngineState *s, int argc, reg_t *argv) {
	const int16 scriptWidth = g_sci->_gfxFrameout->getCurrentBuffer().scriptWidth;
	const int16 scriptHeight = g_sci->_gfxFrameout->getCurrentBuffer().scriptHeight;

	Common::Rect blackoutArea;
	blackoutArea.left = MAX((int16)0, argv[0].toSint16());
	blackoutArea.top = MAX((int16)0, argv[1].toSint16());
	blackoutArea.right = MIN(scriptWidth, (int16)(argv[2].toSint16() + 1));
	blackoutArea.bottom = MIN(scriptHeight, (int16)(argv[3].toSint16() + 1));
	g_sci->_video32->getVMDPlayer().setBlackoutArea(blackoutArea);
	return s->r_acc;
}

reg_t kPlayVMDRestrictPalette(EngineState *s, int argc, reg_t *argv) {
	g_sci->_video32->getVMDPlayer().restrictPalette(argv[0].toUint16(), argv[1].toUint16());
	return s->r_acc;
}

reg_t kPlayDuck(EngineState *s, int argc, reg_t *argv) {
	uint16 operation = argv[0].toUint16();
	Video::VideoDecoder *videoDecoder = 0;
	bool reshowCursor = g_sci->_gfxCursor->isVisible();

	switch (operation) {
	case 1:	// Play
		// 6 params
		s->_videoState.reset();
		s->_videoState.fileName = Common::String::format("%d.duk", argv[1].toUint16());

		videoDecoder = new Video::AVIDecoder();

		if (!videoDecoder->loadFile(s->_videoState.fileName)) {
			warning("Could not open Duck %s", s->_videoState.fileName.c_str());
			break;
		}

		if (reshowCursor)
			g_sci->_gfxCursor->kernelHide();

		{
		// Duck videos are 16bpp, so we need to change the active pixel format
		int oldWidth = g_system->getWidth();
		int oldHeight = g_system->getHeight();
		Common::List<Graphics::PixelFormat> formats;
		formats.push_back(videoDecoder->getPixelFormat());
		initGraphics(640, 480, true, formats);

		if (g_system->getScreenFormat().bytesPerPixel != videoDecoder->getPixelFormat().bytesPerPixel)
			error("Could not switch screen format for the duck video");

		playVideo(videoDecoder, s->_videoState);

		// Switch back to 8bpp
		initGraphics(oldWidth, oldHeight, oldWidth > 320);
		}

		if (reshowCursor)
			g_sci->_gfxCursor->kernelShow();
		break;
	default:
		kStub(s, argc, argv);
		break;
	}

	return s->r_acc;
}

#endif

} // End of namespace Sci
