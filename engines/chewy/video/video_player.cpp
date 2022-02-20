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

#include "chewy/video/cfo_decoder.h"
#include "chewy/video/video_player.h"
#include "chewy/globals.h"
#include "chewy/resource.h"
#include "chewy/sound.h"
#include "common/events.h"
#include "graphics/palette.h"

namespace Chewy {

bool VideoPlayer::playVideo(uint num, bool stopMusic) {
	CfoDecoder *cfoDecoder = new CfoDecoder(g_engine->_sound);
	VideoResource *videoResource = new VideoResource("cut.tap");
	Common::SeekableReadStream *videoStream = videoResource->getVideoStream(num);

	if (stopMusic) {
		_G(sndPlayer)->stopMod();
		_G(currentSong) = -1;
	}

	if (!cfoDecoder->loadStream(videoStream)) {
		delete videoResource;
		delete cfoDecoder;
		return false;
	}

	uint16 x = (g_system->getWidth() - cfoDecoder->getWidth()) / 2;
	uint16 y = (g_system->getHeight() - cfoDecoder->getHeight()) / 2;
	bool skipVideo = false;
	byte curPalette[256 * 3];
	uint32 curFrame = 0;

	g_system->getPaletteManager()->grabPalette(curPalette, 0, 256);
	//save_palette(curPalette);
	_G(cur)->hide_cur();

	// Clear events
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
	}

	cfoDecoder->start();

	while (!g_engine->shouldQuit() && !cfoDecoder->endOfVideo() && !skipVideo && handleCustom(num, curFrame)) {
		if (cfoDecoder->needsUpdate()) {
			const ::Graphics::Surface *frame = cfoDecoder->decodeNextFrame();
			if (frame) {
				g_system->copyRectToScreen(frame->getPixels(), frame->pitch, x, y, frame->w, frame->h);
				curFrame = cfoDecoder->getCurFrame();

				if (cfoDecoder->hasDirtyPalette())
					g_system->getPaletteManager()->setPalette(cfoDecoder->getPalette(), 0, 256);
				//setScummVMPalette(cfoDecoder->getPalette(), 0, 256);

				g_system->updateScreen();
			}
		}

		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}

		g_system->delayMillis(10);
	}

	cfoDecoder->close();

	g_system->getPaletteManager()->setPalette(curPalette, 0, 256);
	//setScummVMPalette(curPalette, 0, 256);
	_G(cur)->show_cur();

	delete videoResource;
	delete cfoDecoder;

	return !skipVideo;
}

bool VideoPlayer::handleCustom(uint num, uint frame) {
	switch (num) {
	case FCUT_004:
		// Room6::cut_serv1
		return (frame == 40) ? false : true;
	case FCUT_005:
		// Room10::cut_serv
		// TODO: The text functions print to an overlay buffer
		_G(atds)->print_aad(_G(spieler).scrollx, _G(spieler).scrolly);
		if (frame == 31) {
			start_aad(107, 0);
			//TextEntryList *text = _G(txt)->getDialog(107, 0);
			//g_engine->_sound->playSpeech(text->front().speechId);
			//delete text;
		}
		break;
	case FCUT_094:
		//Room87::proc3
		return (frame >= 12) ? false : true;
	case FCUT_112:
		//Room56::proc1
		return (_G(in)->get_switch_code() == 1) ? false : true;
	default:
		return true;
	}

	return true;
}

} // End of namespace Chewy
