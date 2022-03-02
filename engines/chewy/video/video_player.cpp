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
#include "chewy/events.h"
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

//	uint16 x = (g_system->getWidth() - cfoDecoder->getWidth()) / 2;
//	uint16 y = (g_system->getHeight() - cfoDecoder->getHeight()) / 2;
	bool skipVideo = false;
	byte curPalette[256 * 3];
	uint32 curFrame = 0;
	bool keepPlaying = true;

	g_system->getPaletteManager()->grabPalette(curPalette, 0, 256);
	//save_palette(curPalette);
	_G(cur)->hide_cur();

	// Clear events
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
	}

	cfoDecoder->start();

	while (!g_engine->shouldQuit() && !cfoDecoder->endOfVideo() && !skipVideo && keepPlaying) {
		if (cfoDecoder->needsUpdate()) {
			const ::Graphics::Surface *frame = cfoDecoder->decodeNextFrame();
			if (frame) {
				const byte *srcP = (const byte *)frame->getPixels();
				byte *destP = (byte *)g_screen->getPixels();
				Common::copy(srcP, srcP + (SCREEN_WIDTH * SCREEN_HEIGHT), destP);
				g_screen->markAllDirty();

				if (cfoDecoder->hasDirtyPalette())
					g_system->getPaletteManager()->setPalette(cfoDecoder->getPalette(), 0, 256);
					//setScummVMPalette(cfoDecoder->getPalette(), 0, 256);

				keepPlaying = handleCustom(num, curFrame, cfoDecoder);
				curFrame = cfoDecoder->getCurFrame();
				
				g_screen->update();
			}
		}

		g_events->update();

		// FIXME: We ignore mouse events because the game checks
		// for left mouse down, instead of up, so releasing the
		// mouse button results in video skipping
		if (_G(in)->get_switch_code() == Common::KEYCODE_ESCAPE)
			skipVideo = true;

		// Clear any pending keys
		_G(in)->_hotkey = 0;
		_G(kbinfo)._keyCode = '\0';
		_G(kbinfo).scan_code = 0;
	}

	cfoDecoder->close();

	g_system->getPaletteManager()->setPalette(curPalette, 0, 256);
	//setScummVMPalette(curPalette, 0, 256);
	_G(cur)->show_cur();

	delete videoResource;
	delete cfoDecoder;

	return !skipVideo;
}

bool VideoPlayer::handleCustom(uint num, uint frame, CfoDecoder *cfoDecoder) {
	const int16 scrollx = _G(spieler).scrollx;
	const int16 scrolly = _G(spieler).scrolly;

	switch (num) {
	case FCUT_004:
		// Room6::cut_serv1
		return (frame == 40) ? false : true;
	case FCUT_005:
		// Room10::cut_serv
		_G(atds)->print_aad(scrollx, scrolly);
		if (frame == 31)
			start_aad(107, 0);
		break;
	case FCUT_009:
	case FCUT_010:
		// Room11::cut_serv and Room11::cut_serv_2
		if (_G(spieler).R11DoorRightF)
			_G(det)->plot_static_details(0, 0, 0, 0);

		if (_G(spieler).R11DoorRightB)
			_G(det)->plot_static_details(0, 0, 6, 6);

		if (_G(spieler).R6DoorRightB)
			_G(det)->plot_static_details(0, 0, 7, 7);

		if (num == FCUT_010) {
			_G(atds)->print_aad(scrollx, scrolly);
			if (frame == 43)
				start_aad(106, 0);
		}
		break;
	case FCUT_032:
	case FCUT_035:
	case FCUT_036:
	case FCUT_037:
	case FCUT_038:
	case FCUT_039:
	case FCUT_040:
		// Room39::setup_func
		if (!_G(spieler).R39TranslatorUsed)
			return false;

		_G(atds)->print_aad(scrollx, scrolly);

		if (cfoDecoder->endOfVideo() && _G(atds)->aad_get_status() != -1)
			cfoDecoder->rewind();
		break;
	case FCUT_034:
		// Room39::setup_func
		if (!_G(spieler).R39TranslatorUsed)
			return false;

		switch (frame) {
		case 121:
			start_aad(599, -1);
			break;
		case 247:
			start_aad(600, -1);
			break;
		case 267:
			start_aad(601, 0);
			break;
		case 297:
			_G(in)->_hotkey = 1;
			break;
		case 171:
		case 266:
		case 370:
			_G(atds)->stopAad();
			break;
		default:
			break;
		}

		_G(atds)->print_aad(scrollx, scrolly);
		break;
	case FCUT_094:
		// Room87::proc3
		return (frame >= 12) ? false : true;
	case FCUT_112:
		// Room56::proc1
		return (_G(in)->get_switch_code() == 1) ? false : true;
	default:
		return true;
	}

	return true;
}

} // End of namespace Chewy
