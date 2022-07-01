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

#include "chewy/dialogs/options.h"
#include "chewy/cursor.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/mcga_graphics.h"
#include "chewy/sound.h"

#include "common/config-manager.h"

namespace Chewy {
namespace Dialogs {

#define SURIMY_START 0
#define SURIMY_END 7
#define SCHNULLER 8
#define SCHNULL_BAND 22
#define MUND_START 9
#define MUND_END 11
#define SCHNULL_OFF 23
#define TDISP_START 12
#define TDISP_END 13
#define TDISP_EIN 20
#define TDISP_AUS 21
#define MUSIC_OFF 24
#define MUSIC_ON1 25
#define MUSIC_ON2 26
#define EXIT 27

void Options::execute(TafInfo *ti) {
	long akt_clock = 0, stop_clock = 0;
	_G(room)->load_tgp(0, &_G(room_blk), GBOOK_TGP, false, GBOOK);
	_G(out)->setPointer(_G(workptr));
	_G(out)->map_spr2screen(_G(ablage)[_G(room_blk).AkAblage], 0, 0);
	_G(out)->setPointer((byte *)g_screen->getPixels());

	_G(room)->set_ak_pal(&_G(room_blk));
	_G(fx)->blende1(_G(workptr), _G(pal), 0, 0);
	_G(out)->setPointer(_G(workptr));
	int16 key = 0;
	int16 surimy_ani = SURIMY_START;
	int16 mund_ani = MUND_START;
	int16 mund_delay = 3;
	int16 mund_count = mund_delay;
	_G(FrameSpeed) = 0;
	int16 delay_count = _G(gameState).DelaySpeed;
	//warning("stop_clock = (clock() / CLK_TCK) + 1;");
	while (key != Common::KEYCODE_ESCAPE) {
		_G(out)->map_spr2screen(_G(ablage)[_G(room_blk).AkAblage], 0, 0);
		++_G(FrameSpeed);
		//warning("akt_clock = clock() / CLK_TCK;");
		if (akt_clock >= stop_clock) {
			//TmpFrame = _G(FrameSpeed);
			_G(gameState).DelaySpeed = (_G(FrameSpeed) >> 1) / _G(gameState).FramesPerSecond;

			_G(FrameSpeed) = 0;
			//warning("stop_clock = (clock() / CLK_TCK) + 1;");
		}

		_G(out)->spriteSet(ti->image[surimy_ani], 18 + ti->correction[surimy_ani << 1],
			8 + ti->correction[(surimy_ani << 1) + 1], 0);
		short bar_off = (_G(gameState).FramesPerSecond - 6) * 16;
		_G(out)->boxFill(33 + bar_off, 65, 33 + 17 + bar_off, 65 + 8, 0);
		Common::String fps = Common::String::format("%d", _G(gameState).FramesPerSecond << 1);
		_G(out)->printxy(36 + bar_off, 65, 255, 300, 0, fps.c_str());

		if (g_engine->_sound->speechEnabled()) {
			_G(out)->spriteSet(ti->image[mund_ani],
				18 + ti->correction[mund_ani << 1],
				8 + ti->correction[(mund_ani << 1) + 1], 0);
			_G(out)->spriteSet(ti->image[SCHNULL_OFF],
				18 + ti->correction[SCHNULL_OFF << 1],
				8 + ti->correction[(SCHNULL_OFF << 1) + 1], 0);
		} else {
			_G(out)->spriteSet(ti->image[SCHNULLER],
				18 + ti->correction[SCHNULLER << 1],
				8 + ti->correction[(SCHNULLER << 1) + 1], 0);
			_G(out)->spriteSet(ti->image[SCHNULL_BAND],
				18 + ti->correction[SCHNULL_BAND << 1],
				8 + ti->correction[(SCHNULL_BAND << 1) + 1], 0);
		}

		const int soundVolume = MAX(1, g_engine->_sound->getSoundVolume() * 32 / Audio::Mixer::kMaxMixerVolume);
		_G(out)->pop_box(32 - 2, 104 - 12, 42 + 4, 136 + 2, 192, 183, 182);
		_G(out)->printxy(32 + 3, 104 - 10, 15, 300, 0, "S");
		_G(out)->boxFill(33, 136 - soundVolume, 42, 136, 15);

		const int musicVolume = MAX(1, g_engine->_sound->getMusicVolume() * 32 / Audio::Mixer::kMaxMixerVolume);
		_G(out)->pop_box(52 - 2, 104 - 12, 62 + 4, 136 + 2, 192, 183, 182);
		_G(out)->printxy(52 + 3, 104 - 10, 31, 300, 0, "M");
		_G(out)->boxFill(53, 136 - musicVolume, 62, 136, 31);

		if (g_engine->_sound->musicEnabled()) {
			_G(out)->spriteSet(ti->image[MUSIC_ON1],
				18 + ti->correction[MUSIC_ON1 << 1],
				8 + ti->correction[(MUSIC_ON1 << 1) + 1], 0);
			_G(out)->spriteSet(ti->image[MUSIC_ON2],
				18 + ti->correction[MUSIC_ON2 << 1],
				8 + ti->correction[(MUSIC_ON2 << 1) + 1], 0);
		} else
			_G(out)->spriteSet(ti->image[MUSIC_OFF],
				18 + ti->correction[MUSIC_OFF << 1],
				8 + ti->correction[(MUSIC_OFF << 1) + 1], 0);

		if (g_engine->_sound->soundEnabled()) {
			_G(out)->spriteSet(ti->image[TDISP_START],
				ti->correction[TDISP_START << 1],
				ti->correction[(TDISP_START << 1) + 1], 0);
			_G(out)->spriteSet(ti->image[TDISP_EIN],
				18 + ti->correction[TDISP_EIN << 1],
				8 + ti->correction[(TDISP_EIN << 1) + 1], 0);
		} else {
			_G(out)->spriteSet(ti->image[TDISP_END],
				ti->correction[TDISP_END << 1],
				ti->correction[(TDISP_END << 1) + 1], 0);
			_G(out)->spriteSet(ti->image[TDISP_AUS],
				18 + ti->correction[TDISP_AUS << 1],
				8 + ti->correction[(TDISP_AUS << 1) + 1], 0);
		}

		_G(out)->spriteSet(ti->image[EXIT],
			18 + ti->correction[EXIT << 1],
			8 + ti->correction[(EXIT << 1) + 1], 0);

		key = _G(in)->getSwitchCode();
		if ((_G(minfo).button == 1) || (key == Common::KEYCODE_RETURN)) {
			// TODO This line breaks screen updates. Not sure what it does
			// otherwise; options screen seems to work without it.
			//WAIT_TASTE_LOS

			int16 rect = _G(in)->findHotspot(_G(optionHotspots));
			switch (rect) {
			case 0: // Hamster wheel arrow left (speed down)
				if (_G(gameState).FramesPerSecond > 6)
					--_G(gameState).FramesPerSecond;
				break;
			case 1: // Hamster wheel arrow right (speed up)
				if (_G(gameState).FramesPerSecond < 10)
					++_G(gameState).FramesPerSecond;
				break;
			case 2: // Mouth (speech/subtitles)
				if (!g_engine->_sound->speechEnabled()) {
					g_engine->_sound->toggleSubtitles(false);
					g_engine->_sound->toggleSpeech(true);
				} else {
					g_engine->_sound->toggleSubtitles(true);
					g_engine->_sound->toggleSpeech(false);
				}
				g_engine->syncSoundSettings();
				break;
			case 3:
			case 4: // Speaker (sound)
				if (g_engine->_sound->soundEnabled()) {
					g_engine->_sound->toggleSound(false);
					_G(det)->disable_room_sound();
				} else {
					g_engine->_sound->toggleSound(true);
					_G(det)->enable_room_sound();
				}
				g_engine->syncSoundSettings();
				break;
			case 5: // Guitarist (music)
				if (g_engine->_sound->musicEnabled()) {
					g_engine->_sound->toggleMusic(false);
					g_engine->_sound->stopMusic();
				} else {
					g_engine->_sound->toggleMusic(true);
					g_engine->_sound->playRoomMusic(_G(gameState)._personRoomNr[P_CHEWY]);
				}
				g_engine->syncSoundSettings();
				break;
			case 6: // Door (exit)
				key = Common::KEYCODE_ESCAPE;
				break;
			case 7: // S volume gauge
				g_engine->_sound->setSoundVolume(MIN(32, 136 - g_events->_mousePos.y) * Audio::Mixer::kMaxMixerVolume / 32);
				g_engine->syncSoundSettings();
				break;
			case 8: // M volume gauge
				g_engine->_sound->setMusicVolume(MIN(32, 136 - g_events->_mousePos.y) * Audio::Mixer::kMaxMixerVolume / 32);
				g_engine->syncSoundSettings();
				break;

			default:
				break;
			}
			_G(minfo).button = 0;
		}
		switch (key) {

		case Common::KEYCODE_UP:
			_G(cur)->move(g_events->_mousePos.x, --g_events->_mousePos.y);
			break;

		case Common::KEYCODE_DOWN:
			_G(cur)->move(g_events->_mousePos.x, ++g_events->_mousePos.y);
			break;

		case Common::KEYCODE_LEFT:
			_G(cur)->move(--g_events->_mousePos.x, g_events->_mousePos.y);
			break;

		case Common::KEYCODE_RIGHT:
			_G(cur)->move(++g_events->_mousePos.x, g_events->_mousePos.y);
			break;

		default:
			break;
		}

		_G(cur)->updateCursor();
		_G(out)->copyToScreen();
		EVENTS_UPDATE;
		SHOULD_QUIT_RETURN;

		if (!delay_count) {
			if (surimy_ani < SURIMY_END)
				++surimy_ani;
			else
				surimy_ani = SURIMY_START;
			if (mund_count > 0)
				--mund_count;
			else {
				if (mund_ani < MUND_END)
					++mund_ani;
				else
					mund_ani = MUND_START;
				mund_count = mund_delay;
			}
			delay_count = _G(gameState).DelaySpeed;
		} else
			--delay_count;
	}

	_G(room)->load_tgp(1, &_G(room_blk), GBOOK_TGP, false, GBOOK);
	_G(out)->setPointer(_G(workptr));
	_G(out)->map_spr2screen(_G(ablage)[_G(room_blk).AkAblage], 0, 0);
	_G(out)->setPointer((byte *)g_screen->getPixels());
	_G(room)->set_ak_pal(&_G(room_blk));
	_G(fx)->blende1(_G(workptr), _G(pal), 0, 0);
	_G(out)->setPointer(_G(workptr));
}

} // namespace Dialogs
} // namespace Chewy
