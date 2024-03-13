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
#include "chewy/cursor.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/resource.h"
#include "chewy/sound.h"
#include "common/events.h"
#include "graphics/paletteman.h"

namespace Chewy {

bool VideoPlayer::playVideo(uint num, bool stopMusic, bool disposeMusic) {
	CfoDecoder *cfoDecoder = new CfoDecoder(g_engine->_sound, disposeMusic);
	VideoResource *videoResource = new VideoResource("cut.tap");
	Common::SeekableReadStream *videoStream = videoResource->getVideoStream(num);
	_playCount = 0;

	if (stopMusic) {
		g_engine->_sound->stopMusic();
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
	_G(cur)->hideCursor();

	// Clear events
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
	}

	cfoDecoder->start();

	while (!g_engine->shouldQuit() && !cfoDecoder->endOfVideo() && !skipVideo && keepPlaying) {
		if (cfoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = cfoDecoder->decodeNextFrame();
			if (frame) {
				const byte *srcP = (const byte *)frame->getPixels();
				byte *destP = (byte *)g_screen->getPixels();
				Common::copy(srcP, srcP + (SCREEN_WIDTH * SCREEN_HEIGHT), destP);
				g_screen->markAllDirty();

				if (cfoDecoder->hasDirtyPalette())
					g_system->getPaletteManager()->setPalette(cfoDecoder->getPalette(), 0, 256);

				keepPlaying = handleCustom(num, curFrame, cfoDecoder);
				curFrame = cfoDecoder->getCurFrame();
				
				g_screen->update();
			}
		}

		g_events->update();

		// FIXME: We ignore mouse events because the game checks
		// for left mouse down, instead of up, so releasing the
		// mouse button results in video skipping
		if (g_events->getSwitchCode() == Common::KEYCODE_ESCAPE)
			skipVideo = true;

		// Clear any pending keys
		g_events->_kbInfo._keyCode = '\0';
		g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
	}

	cfoDecoder->close();

	g_system->getPaletteManager()->setPalette(curPalette, 0, 256);
	_G(cur)->showCursor();

	delete videoResource;
	delete cfoDecoder;

	return !skipVideo;
}

bool VideoPlayer::handleCustom(uint num, uint frame, CfoDecoder *cfoDecoder) {
	const int16 scrollx = _G(gameState).scrollx;
	const int16 scrolly = _G(gameState).scrolly;

	switch (num) {
	case FCUT_004:
		// Room6::cut_serv1
		return (frame == 40) ? false : true;
	case FCUT_005:
		// Room10::cut_serv
		_G(atds)->print_aad(scrollx, scrolly);
		if (frame == 31)
			start_aad(107, 0, true);
		break;
	case FCUT_009:
	case FCUT_010:
		// Room11::cut_serv and Room11::cut_serv_2
		if (_G(gameState).R11DoorRightF)
			_G(det)->plot_static_details(0, 0, 0, 0);

		if (_G(gameState).R11DoorRightB)
			_G(det)->plot_static_details(0, 0, 6, 6);

		if (_G(gameState).R6DoorRightB)
			_G(det)->plot_static_details(0, 0, 7, 7);

		if (num == FCUT_010) {
			_G(atds)->print_aad(scrollx, scrolly);
			if (frame == 43)
				start_aad(106, 0, true);
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
		if (!_G(gameState).R39TranslatorUsed)
			return false;

		_G(atds)->print_aad(scrollx, scrolly);

		if (cfoDecoder->endOfVideo() && _G(atds)->aadGetStatus() != -1)
			cfoDecoder->rewind();
		break;
	case FCUT_034:
		// Room39::setup_func
		if (!_G(gameState).R39TranslatorUsed)
			return false;

		switch (frame) {
		case 121:
			start_aad(599, -1, true);
			break;
		case 247:
			start_aad(600, -1, true);
			break;
		case 267:
			start_aad(601, 0, true);
			break;
		case 297:
			//_G(in)->_hotkey = 1;
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
	case FCUT_047:
		// Room37::cut_serv1
		if (!_G(gameState).R37RoosterFoughtWithDog) {
			if (!_G(gameState).R37TakenDenturesFromGlass) {
				_G(det)->plot_static_details(scrollx, scrolly, 9, 9);
				_G(det)->plot_static_details(scrollx, scrolly, 11, 11);
				_G(det)->showStaticSpr(11);
			} else {
				_G(det)->plot_static_details(scrollx, scrolly, 8, 8);
				_G(det)->plot_static_details(scrollx, scrolly, 0, 0);
			}
		}

		_G(det)->plot_static_details(scrollx, scrolly, 7, 7);
		_G(det)->plot_static_details(scrollx, scrolly, 14, 14);
		break;
	case FCUT_048: {
		// Room37::cut_serv2
		const int16 STATIC_NR[] = {7, 14, 12, 10};

		_G(det)->showStaticSpr(12);
		_G(det)->showStaticSpr(10);
		for (short i = 0; i < 4; i++)
			_G(det)->plot_static_details(scrollx, scrolly, STATIC_NR[i], STATIC_NR[i]);
		}
		break;
	case FCUT_053:
		if (cfoDecoder->endOfVideo() && _playCount < 3) {
			cfoDecoder->rewind();
			_playCount++;
		}
		break;
	case FCUT_054:
		if (cfoDecoder->endOfVideo() && _playCount < 2) {
			cfoDecoder->rewind();
			_playCount++;
		}
		break;
	case FCUT_055:
	case FCUT_056:
	case FCUT_064:
		// Room28::cut_serv2 (FCUT_055)
		if (num != FCUT_055 || frame < 23) {
			// Room28::cut_serv1 (FCUT_056 / FCUT_064)
			if (_G(gameState).R28LetterBox)
				_G(det)->plot_static_details(0, 0, 8, 9);
			else
				_G(det)->plot_static_details(0, 0, 7, 7);
		}
		break;
	case FCUT_061:
		// Room43::setup_func
		_G(atds)->print_aad(scrollx, scrolly);
		break;
	case FCUT_068:
		// Room51::cut_serv
		_G(det)->plot_static_details(0, 0, 16, 16);
		break;
	case FCUT_069:
		// Room54::cut_serv
		_G(det)->plot_static_details(176, 0, 9, 9);
		break;
	case FCUT_070:
		// Room55::cut_serv
		if (frame < 29)
			_G(det)->plot_static_details(136, 0, 10, 10);
		break;
	case FCUT_078: {
		// Room64::cut_sev
		const int16 spr_nr = _G(chewy_ph)[_G(moveState)[P_CHEWY].Phase * 8 + _G(moveState)[P_CHEWY].PhNr];
		const int16 x = _G(spieler_mi)[P_CHEWY].XyzStart[0] + _G(chewy)->correction[spr_nr * 2] - scrollx;
		const int16 y = _G(spieler_mi)[P_CHEWY].XyzStart[1] + _G(chewy)->correction[spr_nr * 2 + 1] - scrolly;

		calc_zoom(_G(spieler_mi)[P_CHEWY].XyzStart[1], (int16)_G(room)->_roomInfo->_zoomFactor, (int16)_G(room)->_roomInfo->_zoomFactor, &_G(moveState)[P_CHEWY]);
		_G(out)->scale_set(_G(chewy)->image[spr_nr], x, y, _G(moveState)[P_CHEWY].Xzoom, _G(moveState)[P_CHEWY].Yzoom, _G(scr_width));
		}
		break;
	case FCUT_083:
		if (cfoDecoder->endOfVideo() && _playCount < 2) {
			cfoDecoder->rewind();
			_playCount++;
		}
		break;
	case FCUT_089:
		// Room87::proc5
		_G(atds)->print_aad(scrollx, scrolly);
		break;
	case FCUT_094:
		// Room87::proc3
		return (frame >= 12) ? false : true;
	case FCUT_095:
		// Room87::proc5
		_G(atds)->print_aad(scrollx, scrolly);

		if (cfoDecoder->endOfVideo() && _G(atds)->aadGetStatus() != -1)
			cfoDecoder->rewind();
		break;
	case FCUT_107:
		// Room90::proc5
		_G(det)->plot_static_details(scrollx, 0, 3, 3);
		break;
	case FCUT_112:
		// Room56::proc1
		if (cfoDecoder->endOfVideo() && _playCount < 2) {
			cfoDecoder->rewind();
			_playCount++;
		}
		return (g_events->getSwitchCode() == Common::KEYCODE_ESCAPE) ? false : true;
	case FCUT_116:
		if (cfoDecoder->endOfVideo() && _playCount < 6) {
			cfoDecoder->rewind();
			_playCount++;
		}
		break;
	case FCUT_135:
	case FCUT_145:
	case FCUT_142:
	case FCUT_140:
	case FCUT_144:
	case FCUT_134:
	case FCUT_148:
	case FCUT_138:
	case FCUT_143:
	case FCUT_146:
	case FCUT_154:
	case FCUT_139:
	case FCUT_156:
	case FCUT_157:
	case FCUT_147:
	case FCUT_153:
	case FCUT_152:
	case FCUT_141:
	case FCUT_137:
	case FCUT_136:
	case FCUT_151:
	case FCUT_149:
	case FCUT_150:
		// Intro
		_G(atds)->print_aad(scrollx, scrolly);
		if (num == FCUT_135 || num == FCUT_134 || num == FCUT_154 || num == FCUT_156)
			if (_G(atds)->aadGetStatus() != -1)
				_G(out)->raster_col(254, 63, 12, 46);
		if (num == FCUT_137 && frame == 35)
			_G(atds)->stopAad();
		if (num == FCUT_136 && frame == 18)
			_G(atds)->stopAad();
		if (num == FCUT_140 && frame == 15)
			return false;
		if (num == FCUT_144 && frame == 7)
			return false;

		if (num == FCUT_141 || num == FCUT_142 || num == FCUT_143 ||
			num == FCUT_145 || num == FCUT_146 || num == FCUT_152) {
			if (cfoDecoder->endOfVideo() && _G(atds)->aadGetStatus() != -1)
				cfoDecoder->rewind();
		}
		break;
	default:
		return true;
	}

	return true;
}

} // End of namespace Chewy
