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
 *
 *              Originally written by Syn9 in FreeBASIC with SDL
 *              http://syn9.thehideoutgames.com/index_backup.php
 *
 *            Ported to plain C for GCW-Zero handheld by Dmitry Smagin
 *                http://github.com/dmitrysmagin/griffon_legend
 *
 *
 *                 Programming/Graphics: Daniel "Syn9" Kennedy
 *                     Music/Sound effects: David Turner
 *
 *                   Beta testing and gameplay design help:
 *                    Deleter, Cha0s, Aether Fox, and Kiz
 *
 */

#include "common/system.h"

#include "griffon/griffon.h"

namespace Griffon {

#define POLL_AND_CHECK_QUIT() 		if (g_system->getEventManager()->pollEvent(_event)) { \
		if (_event.type == Common::EVENT_QUIT) { \
			_shouldQuit = true; \
			return; \
		} \
	}

#define CHECK_QUIT() 		if (_event.type == Common::EVENT_QUIT) { \
		_shouldQuit = true; \
		return; \
	}

const char *story[48] = {
	"The Griffon Legend",
	"http://syn9.thehideoutgames.com/",
	"",
	"Programming/Graphics: Daniel Kennedy",
	"Music/Sound effects: David Turner",
	"",
	"Porting to GCW-Zero: Dmitry Smagin",
	"",
	"",
	"Story",
	"Ever since I was a child",
	"I remember being told the",
	"Legend of the Griffon Knights,",
	"who rid the world of the",
	"Dragon Empire.  These great",
	"heroes inspired us to become",
	"knights as well.",
	" ",
	"Now, 500 years after the war",
	"ended, the Dragons have",
	"returned.  Cities are falling",
	"from the lack of knights to",
	"protect them.",
	" ",
	"We never saw it coming.",
	" ",
	"And now, here I am, making",
	"my way into the lower town",
	"of Fidelis, a small city on",
	"the main continent. The rest",
	"of my men have died over",
	"the last couple days from",
	"aerial attacks.",
	" ",
	"We believed we could find",
	"shelter here, only to find",
	"every last griffon dead,",
	"the town burned to the ground,",
	"and transformed into a garrison",
	"for the Dragon forces.",
	" ",
	"In these dark times, I try to",
	"draw strength from the stories",
	"of those knights that risked",
	"everything to protect their homeland,",
	" ",
	"and hope that I can die",
	"with that honor as well."
};

const char *story2[27] = {
	"After the fall of Margrave Gradius,",
	"All the dragons, struck with panic,",
	"evacuated the city immediately.",
	" ",
	"It\'s funny how without a leader",
	"everyone is so weak.",
	" ",
	" ",
	"But yet another leader will rise,",
	"and another city will fall.",
	" ",
	" ",
	"I should return home to Asherton",
	"It\'s time to leave this place",
	"and cleanse this blood stained",
	"life of mine.",
	" ",
	"No one should have to see as much",
	"death as I have.",
	" ",
	" ",
	"Before, I said that I wanted",
	"to die an honorable death.",
	" ",
	"Now I say that I have lived an",
	"honorable life,",
	"and I am free to die as I please."
};

void GriffonEngine::showLogos() {
	_ticks = g_system->getMillis();
	int ticks1 = _ticks;

	do {
		float y = 255.0;
		if (_ticks < ticks1 + 1000) {
			y = 255.0 * ((float)(_ticks - ticks1) / 1000.0);
			y = CLIP<float>(y, 0.0, 255.0);
		}

		if (_ticks > ticks1 + 3000) {
			y = 255.0 - 255.0 * ((float)(_ticks - ticks1 - 3000.0) / 1000.0);
			y = CLIP<float>(y, 0.0, 255.0);
		}

		_videoBuffer->fillRect(Common::Rect(0, 0, 320, 240), 0);
		_logosImg->blit(*_videoBuffer, 0, 0, Graphics::FLIP_NONE, nullptr, TS_ARGB((int)y, (int)y, (int)y, (int)y));

		g_system->copyRectToScreen(_videoBuffer->getPixels(), _videoBuffer->pitch, 0, 0, _videoBuffer->w, _videoBuffer->h);
		g_system->updateScreen();

		if (g_system->getEventManager()->pollEvent(_event)) {
			if (_event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START && _event.customType == kGriffonMenu)
				return;

			CHECK_QUIT();
		}

		_ticksPassed = _ticks;
		_ticks = g_system->getMillis();

		_ticksPassed = _ticks - _ticksPassed;
		_fpsr = (float)_ticksPassed / 24;

		_fp++;
		if (_ticks > _nextTicks) {
			_nextTicks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		g_system->delayMillis(10);
		if (_ticks > ticks1 + 4000)
			break;
	} while (!_shouldQuit);
}

void GriffonEngine::intro() {
	_videoBuffer2->fillRect(Common::Rect(0, 0, _videoBuffer2->w, _videoBuffer2->h), 0);
	_videoBuffer3->fillRect(Common::Rect(0, 0, _videoBuffer3->w, _videoBuffer3->h), 0);

	_ticks = g_system->getMillis();

	_videoBuffer->blit(*_videoBuffer3);
	_videoBuffer->blit(*_videoBuffer2);

	_fpsr = 0.0;
	int y = 140;

	if (config.music) {
		haltSoundChannel(-1);
		_musicChannel = playSound(_musicEndOfGame, true);
		setChannelVolume(_musicChannel, 0);
	}

	_secsInGame = 0;
	_secStart = 0;

	bool ldStop = false;
	bool speedUp = false;
	int cnt = 0;
	float xofs = 0.0;
	float ld = 0.0;
	do {
		Common::Rect rc;

		ld += 4 * _fpsr;
		if ((int)ld > config.musicVol)
			ld = config.musicVol;
		if (!ldStop) {
			setChannelVolume(_musicChannel, (int)ld);
			if ((int)ld == config.musicVol)
				ldStop = true;
		}

		rc.left = -xofs;
		rc.top = 0;

		_titleImg->blit(*_videoBuffer, rc.left, rc.top);

		rc.left = -xofs + 320;
		rc.top = 0;

		_titleImg->blit(*_videoBuffer, rc.left, rc.top);

		if (++cnt >= 6) {
			cnt = 0;
			y--;
		}

		for (int i = 0; i <= 37; i++) {
			int yy = y + i * 10;
			if (yy > -8 && yy < 240) {
				int x = 160 - strlen(story[i]) * 4;
				drawString(_videoBuffer, story[i], x, yy, 4);
			}

			if (yy < 10 && i == 37)
				return;
		}

		g_system->copyRectToScreen(_videoBuffer->getPixels(), _videoBuffer->pitch, 0, 0, _videoBuffer->w, _videoBuffer->h);
		g_system->updateScreen();

		_ticksPassed = _ticks;
		_ticks = g_system->getMillis();

		_ticksPassed = _ticks - _ticksPassed;
		_fpsr = (float)_ticksPassed / 24.0;

		_fp++;
		if (_ticks > _nextTicks) {
			_nextTicks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		float add = 0.5 * _fpsr;
		if (add > 1)
			add = 1;
		xofs += add;
		if (xofs >= 320)
			xofs -= 320;

		if (g_system->getEventManager()->pollEvent(_event)) {
			if (_event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				if (_event.customType == kGriffonCutsceneSpeedUp) {
					speedUp = true;
					cnt = 6;
				}
				else if (_event.customType == kGriffonMenu)
					return;
			} else if (_event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
				if (_event.customType == kGriffonCutsceneSpeedUp)
					speedUp = false;
			}

			CHECK_QUIT();
		} else if (speedUp)
			cnt = 6;

		g_system->delayMillis(10);
	} while (!_shouldQuit);
}

void GriffonEngine::endOfGame() {
	float xofs = 0;
	_ticks = g_system->getMillis();

	float spd = 0.2f;

	if (config.music) {
		haltSoundChannel(-1);
		_musicChannel = playSound(_musicEndOfGame, true);
		setChannelVolume(_musicChannel, 0);
	}

	int ticks1 = _ticks;
	int ya = 0;

	float ld = 0;
	bool ldstop = false; // CHECKME: Check if actually used

	do {
		ld += 4 * _fpsr;
		if (ld > config.musicVol)
			ld = config.musicVol;
		if (!ldstop) {
			setChannelVolume(_musicChannel, (int)ld);
			if ((int)ld == config.musicVol)
				ldstop = true;
		}

		ya = 0;
		if (_ticks < ticks1 + 1500) {
			ya = (255 * (_ticks - ticks1)) / 1500;
			ya = CLIP(ya, 0, 255);
		} else
			break;

		_videoBuffer2->fillRect(Common::Rect(0, 0, _videoBuffer2->w, _videoBuffer2->h), 0);
		_videoBuffer->setAlpha(255 - ya);
		_videoBuffer->blit(*_videoBuffer2);

		g_system->copyRectToScreen(_videoBuffer2->getPixels(), _videoBuffer2->pitch, 0, 0, _videoBuffer2->w, _videoBuffer2->h);

		POLL_AND_CHECK_QUIT();

		g_system->updateScreen();
		g_system->delayMillis(10);

		_ticksPassed = _ticks;
		_ticks = g_system->getMillis();

		_ticksPassed = _ticks - _ticksPassed;
		_fpsr = (float)_ticksPassed / 24;

		_fp++;
		if (_ticks > _nextTicks) {
			_nextTicks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}
	} while (1);

	ticks1 = _ticks;
	ya = 0;
	float y = 140;

	do {
		Common::Rect rc;

		rc.left = -xofs;
		rc.top = 0;

		_titleImg->blit(*_videoBuffer, rc.left, rc.top);

		rc.left = -xofs + 320;
		rc.top = 0;

		_titleImg->blit(*_videoBuffer, rc.left, rc.top);

		y = y - spd * _fpsr;
		for (int i = 0; i <= 26; i++) {
			int yy = y + i * 10;
			if (yy > -8 && yy < 240) {
				int x = 160 - strlen(story2[i]) * 4;
				drawString(_videoBuffer, story2[i], x, yy, 4);
			}

			if (yy < 10 && i == 25)
				break;
		}

		ya = 255;
		if (_ticks < ticks1 + 1000) {
			ya = 255 * (_ticks - ticks1) / 1000;
			ya = CLIP(ya, 0, 255);
		}

		_videoBuffer->setAlpha(ya);
		g_system->copyRectToScreen(_videoBuffer->getPixels(), _videoBuffer->pitch, 0, 0, _videoBuffer->w, _videoBuffer->h);

		_ticksPassed = _ticks;
		_ticks = g_system->getMillis();

		_ticksPassed = _ticks - _ticksPassed;
		_fpsr = (float)_ticksPassed / 24;

		_fp++;
		if (_ticks > _nextTicks) {
			_nextTicks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		float add = 0.5 * _fpsr;
		if (add > 1)
			add = 1;
		xofs += add;
		if (xofs >= 320)
			xofs -= 320;

		if (g_system->getEventManager()->pollEvent(_event)) {
			if (_event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				if (_event.customType == kGriffonCutsceneSpeedUp)
					spd = 1.0f;
				else if (_event.customType == kGriffonMenu)
					break;
			} else if (_event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
				if (_event.customType == kGriffonCutsceneSpeedUp)
					spd = 0.2f;
			}

			CHECK_QUIT();
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	} while (1);


	ticks1 = _ticks;
	int y1 = 0;

	_videoBuffer2->copyRectToSurface(_videoBuffer->getPixels(), _videoBuffer->pitch, 0, 0, _videoBuffer->w, _videoBuffer->h);

	do {
		if (_ticks < ticks1 + 1500) {
			y1 = 255 * (_ticks - ticks1) / 1500;
			y1 = CLIP(y1, 0, 255);
		} else
			break;

		_videoBuffer->fillRect(Common::Rect(0, 0, _videoBuffer->w, _videoBuffer->h), 0);

		_videoBuffer->setAlpha(y1);
		_videoBuffer2->blit(*_videoBuffer3);
		_videoBuffer->blit(*_videoBuffer3);

		g_system->copyRectToScreen(_videoBuffer3->getPixels(), _videoBuffer3->pitch, 0, 0, _videoBuffer3->w, _videoBuffer3->h);
		g_system->updateScreen();

		POLL_AND_CHECK_QUIT();

		g_system->delayMillis(10);

		_ticksPassed = _ticks;
		_ticks = g_system->getMillis();

		_ticksPassed = _ticks - _ticksPassed;
		_fpsr = (float)_ticksPassed / 24;

		_fp++;
		if (_ticks > _nextTicks) {
			_nextTicks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}
	} while (1);


	int keywait = 2000 + _ticks;

	ticks1 = _ticks;
	y1 = 0;
	do {

		_videoBuffer->copyRectToSurface(_theEndImg->getPixels(), _theEndImg->pitch, 0, 0, _theEndImg->w, _theEndImg->h);

		y1 = 255;
		if (_ticks < ticks1 + 1000) {
			y1 = 255 * (_ticks - ticks1) / 1000;
			y1 = CLIP(y1, 0, 255);
		}

		_videoBuffer->setAlpha(y1);

		_videoBuffer2->fillRect(Common::Rect(0, 0, _videoBuffer2->w, _videoBuffer2->h), 0);
		_videoBuffer->blit(*_videoBuffer2);

		g_system->copyRectToScreen(_videoBuffer2->getPixels(), _videoBuffer2->pitch, 0, 0, _videoBuffer2->w, _videoBuffer2->h);

		g_system->updateScreen();
		g_system->delayMillis(10);

		_ticksPassed = _ticks;
		_ticks = g_system->getMillis();

		_ticksPassed = _ticks - _ticksPassed;
		_fpsr = (float)_ticksPassed / 24;

		_fp++;
		if (_ticks > _nextTicks) {
			_nextTicks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		if (g_system->getEventManager()->pollEvent(_event)) {
			CHECK_QUIT();

			if ((_event.type == Common::EVENT_KEYDOWN || _event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) && keywait < _ticks)
				break;
		}

	} while (1);

	_videoBuffer2->fillRect(Common::Rect(0, 0, _videoBuffer2->w, _videoBuffer2->h), 0);
	_videoBuffer3->fillRect(Common::Rect(0, 0, _videoBuffer3->w, _videoBuffer3->h), 0);

	theEnd();
}

void GriffonEngine::theEnd() {
	for (int i = 0; i < kMaxFloat; i++) {
		_floatText[i].framesLeft = 0;
		_floatIcon[i].framesLeft = 0;
	}

	for (float y = 0; y < 100; y += _fpsr) {
		_videoBuffer2->fillRect(Common::Rect(0, 0, _videoBuffer2->w, _videoBuffer2->h), 0);
		_videoBuffer->setAlpha((int)((100.0 - y) / 100 * 255));
		_videoBuffer->blit(*_videoBuffer2);

		g_system->copyRectToScreen(_videoBuffer2->getPixels(), _videoBuffer2->pitch, 0, 0, _videoBuffer2->w, _videoBuffer2->h);
		g_system->updateScreen();

		POLL_AND_CHECK_QUIT();

		g_system->delayMillis(10);

		_ticksPassed = _ticks;
		_ticks = g_system->getMillis();

		_ticksPassed = _ticks - _ticksPassed;
		_fpsr = (float)_ticksPassed / 24.0;

		_fp++;
		if (_ticks > _nextTicks) {
			_nextTicks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}
	}
}


} // end of namespace Griffon
