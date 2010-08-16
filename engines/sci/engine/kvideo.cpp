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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/util.h"
#include "sci/engine/state.h"
#include "sci/graphics/helpers.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#include "graphics/cursorman.h"
#include "graphics/video/avi_decoder.h"
#include "graphics/video/qt_decoder.h"
#include "sci/video/seq_decoder.h"
#ifdef ENABLE_SCI32
#include "graphics/video/coktel_decoder.h"
#endif

namespace Sci {

void playVideo(Graphics::VideoDecoder *videoDecoder) {
	if (!videoDecoder)
		return;

	byte *scaleBuffer = 0;
	uint16 width = videoDecoder->getWidth();
	uint16 height = videoDecoder->getHeight();
	uint16 screenWidth = g_system->getWidth();
	uint16 screenHeight = g_system->getHeight();

	if (screenWidth == 640 && width <= 320 && height <= 240) {
		assert(videoDecoder->getPixelFormat().bytesPerPixel == 1);
		width *= 2;
		height *= 2;
		scaleBuffer = new byte[width * height];
	}

	uint16 x = (screenWidth - width) / 2;
	uint16 y = (screenHeight - height) / 2;
	bool skipVideo = false;

	if (videoDecoder->hasDirtyPalette())
		videoDecoder->setSystemPalette();

	while (!g_engine->shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			Graphics::Surface *frame = videoDecoder->decodeNextFrame();
			if (frame) {
				if (scaleBuffer) {
					// TODO: Probably should do aspect ratio correction in e.g. GK1 Windows 
					g_sci->_gfxScreen->scale2x((byte *)frame->pixels, scaleBuffer, videoDecoder->getWidth(), videoDecoder->getHeight());
					g_system->copyRectToScreen(scaleBuffer, width, x, y, width, height);
				} else
					g_system->copyRectToScreen((byte *)frame->pixels, frame->pitch, x, y, width, height);

				if (videoDecoder->hasDirtyPalette())
					videoDecoder->setSystemPalette();

				g_system->updateScreen();
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

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
		
	Graphics::VideoDecoder *videoDecoder = 0;

	if (argv[0].segment != 0) {
		Common::String filename = s->_segMan->getString(argv[0]);

		if (g_sci->getPlatform() == Common::kPlatformMacintosh) {
			// Mac QuickTime
			// The only argument is the string for the video

			// HACK: Switch to 16bpp graphics for Cinepak.
			initGraphics(screenWidth, screenHeight, screenWidth > 320, NULL);

			if (g_system->getScreenFormat().bytesPerPixel == 1) {
				error("This video requires >8bpp color to be displayed, but could not switch to RGB color mode.");
				return NULL_REG;
			}

			videoDecoder = new Graphics::QuickTimeDecoder();
			if (!videoDecoder->loadFile(filename))
				error("Could not open '%s'", filename.c_str());
		} else {
			// DOS SEQ
			// SEQ's are called with no subops, just the string and delay
			SeqDecoder *seqDecoder = new SeqDecoder();
			seqDecoder->setFrameDelay(argv[1].toUint16()); // Time between frames in ticks
			videoDecoder = seqDecoder;

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
		if (getSciVersion() >= SCI_VERSION_2_1) {
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
			videoDecoder = new Graphics::AviDecoder(g_system->getMixer());

			if (!videoDecoder->loadFile(filename.c_str())) {
				warning("Failed to open movie file %s", filename.c_str());
				delete videoDecoder;
				videoDecoder = 0;
			}
			break;
		}
		default:
			warning("Unhandled SCI kShowMovie subop %d", argv[1].toUint16());
		}
	}

	if (videoDecoder) {
		playVideo(videoDecoder);

		// HACK: Switch back to 8bpp if we played a QuickTime video.
		// We also won't be copying the screen to the SCI screen...
		if (g_system->getScreenFormat().bytesPerPixel != 1)
			initGraphics(screenWidth, screenHeight, screenWidth > 320);
		else {
			g_sci->_gfxScreen->kernelSyncWithFramebuffer();
			g_sci->_gfxPalette->kernelSyncScreenPalette();
		}
	}

	if (reshowCursor)
		g_sci->_gfxCursor->kernelShow();

	return s->r_acc;
}

#ifdef ENABLE_SCI32

reg_t kPlayVMD(EngineState *s, int argc, reg_t *argv) {
	uint16 operation = argv[0].toUint16();
	Graphics::VideoDecoder *videoDecoder = 0;
	bool reshowCursor = g_sci->_gfxCursor->isVisible();
	Common::String fileName, warningMsg;

	switch (operation) {
	case 0:	// init
		// This is actually meant to init the video file, but we play it instead
		fileName = s->_segMan->derefString(argv[1]);
		// TODO: argv[2] (usually null). When it exists, it points to an "Event" object,
		// that holds no data initially (e.g. in the intro of Phantasmagoria 1 demo).
		// Perhaps it's meant for syncing
		if (argv[2] != NULL_REG)
			warning("kPlayVMD: third parameter isn't 0 (it's %04x:%04x - %s)", PRINT_REG(argv[2]), s->_segMan->getObjectName(argv[2]));

		videoDecoder = new Graphics::VMDDecoder(g_system->getMixer());

		if (!videoDecoder->loadFile(fileName)) {
			warning("Could not open VMD %s", fileName.c_str());
			break;
		}

		if (reshowCursor)
			g_sci->_gfxCursor->kernelHide();

		playVideo(videoDecoder);

		if (reshowCursor)
			g_sci->_gfxCursor->kernelShow();
		break;
	case 1:
	{
		// Set VMD parameters. Called with a maximum of 6 parameters:
		//
		// x, y, flags, gammaBoost, gammaFirst, gammaLast
		//
		// Flags are as follows:
		// bit 0		doubled
		// bit 1		"drop frames"?
		// bit 2		insert black lines
		// bit 3		unknown
		// bit 4		gamma correction
		// bit 5		hold black frame
		// bit 6		hold last frame
		// bit 7		unknown
		// bit 8		stretch

		// gammaBoost boosts palette colors in the range gammaFirst to
		// gammaLast, but only if bit 4 in flags is set. Percent value such that
		// 0% = no amplification These three parameters are optional if bit 4 is
		// clear. Also note that the x, y parameters play subtle games if used
		// with subfx 21. The subtleness has to do with creation of temporary
		// planes and positioning relative to such planes.

		int flags = argv[3].offset;
		Common::String flagspec;

		if (argc > 3) {
			if (flags & 1)
				flagspec += "doubled ";
			if (flags & 2)
				flagspec += "dropframes ";
			if (flags & 4)
				flagspec += "blacklines ";
			if (flags & 8)
				flagspec += "bit3 ";
			if (flags & 16)
				flagspec += "gammaboost ";
			if (flags & 32)
				flagspec += "holdblack ";
			if (flags & 64)
				flagspec += "holdlast ";
			if (flags & 128)
				flagspec += "bit7 ";
			if (flags & 256)
				flagspec += "stretch";

			warning("VMDFlags: %s", flagspec.c_str());
		}

		warning("x, y: %d, %d", argv[1].offset, argv[2].offset);

		if (argc > 4 && flags & 16)
			warning("gammaBoost: %d%% between palette entries %d and %d", argv[4].offset, argv[5].offset, argv[6].offset);
		break;
	}
	case 6:
		// Play, perhaps? Or stop? This is the last call made, and takes no extra parameters
	case 14:
		// Takes an additional integer parameter (e.g. 3)
	case 16:
		// Takes an additional parameter, usually 0
	case 21:
		// Looks to be setting the video size and position. Called with 4 extra integer
		// parameters (e.g. 86, 41, 235, 106)
	default:
		warningMsg = Common::String::printf("PlayVMD - unsupported subop %d. Params: %d (", operation, argc);

		for (int i = 0; i < argc; i++) {
			warningMsg +=  Common::String::printf("%04x:%04x", PRINT_REG(argv[i]));
			warningMsg += (i == argc - 1 ? ")" : ", ");
		}

		warning("%s", warningMsg.c_str());
		break;
	}

	return s->r_acc;
}

#endif

} // End of namespace Sci
