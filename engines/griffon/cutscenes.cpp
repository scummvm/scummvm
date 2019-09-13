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
#include "griffon/config.h"

namespace Griffon {

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

		_videobuffer->fillRect(Common::Rect(0, 0, 320, 240), 0);
		_logosimg->blit(*_videobuffer, 0, 0, Graphics::FLIP_NONE, nullptr, TS_ARGB((int)y, (int)y, (int)y, (int)y));

		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_QUIT)
			_shouldQuit = true;

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		g_system->delayMillis(10);
		if (_ticks > ticks1 + 4000)
			break;
	} while (!_shouldQuit);
}

void GriffonEngine::intro() {
	_videobuffer2->fillRect(Common::Rect(0, 0, _videobuffer2->w, _videobuffer2->h), 0);
	_videobuffer3->fillRect(Common::Rect(0, 0, _videobuffer3->w, _videobuffer3->h), 0);

	_ticks = g_system->getMillis();

	_videobuffer->blit(*_videobuffer3);
	_videobuffer->blit(*_videobuffer2);

	_fpsr = 0.0;
	int y = 140;

	if (config.music) {
		haltSoundChannel(-1);
		_musicchannel = playSound(_mendofgame, true);
		setChannelVolume(_musicchannel, 0);
	}

	_secsingame = 0;
	_secstart = 0;

	bool ldstop = false;
	int cnt = 0;
	float xofs = 0.0;
	float ld = 0.0;
	do {
		Common::Rect rc;

		ld += 4 * _fpsr;
		if ((int)ld > config.musicvol)
			ld = config.musicvol;
		if (!ldstop) {
			setChannelVolume(_musicchannel, (int)ld);
			if ((int)ld == config.musicvol)
				ldstop = true;
		}

		rc.left = -xofs;
		rc.top = 0;

		_titleimg->blit(*_videobuffer, rc.left, rc.top);

		rc.left = -xofs + 320;
		rc.top = 0;

		_titleimg->blit(*_videobuffer, rc.left, rc.top);

		if (++cnt >= 6) {
			cnt = 0;
			y--;
		}

		for (int i = 0; i <= 37; i++) {
			int yy = y + i * 10;
			if (yy > -8 && yy < 240) {
				int x = 160 - strlen(story[i]) * 4;
				drawString(_videobuffer, story[i], x, yy, 4);
			}

			if (yy < 10 && i == 47)
				return;
		}

		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24.0;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		float add = 0.5 * _fpsr;
		if (add > 1)
			add = 1;
		xofs += add;
		if (xofs >= 320)
			xofs = xofs - 320;

		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_KEYDOWN)
			cnt = 6;
		if (_event.kbd.keycode == Common::KEYCODE_ESCAPE)
			return;

		if (_event.type == Common::EVENT_QUIT)
			_shouldQuit = true;

		g_system->delayMillis(10);
	} while (!_shouldQuit);
}

void GriffonEngine::endOfGame() {
	float xofs = 0;
	_ticks = g_system->getMillis();

	float spd = 0.2f;

	if (config.music) {
		haltSoundChannel(-1);
		_musicchannel = playSound(_mendofgame, true);
		setChannelVolume(_musicchannel, 0);
	}

	int ticks1 = _ticks;
	int ya = 0;

	_videobuffer2->fillRect(Common::Rect(0, 0, _videobuffer2->w, _videobuffer2->h), 0);
	_videobuffer3->fillRect(Common::Rect(0, 0, _videobuffer3->w, _videobuffer3->h), 0);
	_videobuffer2->copyRectToSurface(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);

	float ld = 0;
	bool ldstop = false; // CHECKME: Check if actually used

	do {
		ld = ld + 4 * _fpsr;
		if (ld > config.musicvol)
			ld = config.musicvol;
		if (!ldstop) {
			setChannelVolume(_musicchannel, (int)ld);
			if ((int)ld == config.musicvol)
				ldstop = true;
		}

		ya = 0;
		if (_ticks < ticks1 + 1500) {
			ya = (255 * (_ticks - ticks1)) / 1500;
			ya = CLIP(ya, 0, 255);
		} else
			break;

		_videobuffer->fillRect(Common::Rect(0, 0, _videobuffer->w, _videobuffer->h), 0);

		_videobuffer->setAlpha(ya);
		_videobuffer3->copyRectToSurface(_videobuffer2->getPixels(), _videobuffer2->pitch, 0, 0, _videobuffer2->w, _videobuffer2->h);
		_videobuffer3->copyRectToSurface(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);

		g_system->copyRectToScreen(_videobuffer3->getPixels(), _videobuffer3->pitch, 0, 0, _videobuffer3->w, _videobuffer3->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
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

		_titleimg->blit(*_videobuffer, rc.left, rc.top);

		rc.left = -xofs + 320;
		rc.top = 0;

		_titleimg->blit(*_videobuffer, rc.left, rc.top);

		y = y - spd * _fpsr;
		for (int i = 0; i <= 26; i++) {
			int yy = y + i * 10;
			if (yy > -8 && yy < 240) {
				int x = 160 - strlen(story2[i]) * 4;
				drawString(_videobuffer, story2[i], x, yy, 4);
			}

			if (yy < 10 && i == 25)
				break;
		}

		ya = 255;
		if (_ticks < ticks1 + 1000) {
			ya = 255 * (_ticks - ticks1) / 1000;
			ya = CLIP(ya, 0, 255);
		}

		_videobuffer->setAlpha(ya);
		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		float add = 0.5 * _fpsr;
		if (add > 1)
			add = 1;
		xofs = xofs + add;
		if (xofs >= 320)
			xofs = xofs - 320;

		if (_event.type == Common::EVENT_KEYDOWN)
			spd = 1.0f;
		if (_event.type == Common::EVENT_KEYUP)
			spd = 0.2f;

		if (_event.kbd.keycode == Common::KEYCODE_ESCAPE)
			break;
	} while (1);


	ticks1 = _ticks;
	int y1 = 0;

	_videobuffer2->copyRectToSurface(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);

	do {
		if (_ticks < ticks1 + 1500) {
			y1 = 255 * (_ticks - ticks1) / 1500;
			y1 = CLIP(y1, 0, 255);
		}
		else
			break;

		_videobuffer->fillRect(Common::Rect(0, 0, _videobuffer->w, _videobuffer->h), 0);

		_videobuffer->setAlpha(y1);
		_videobuffer2->blit(*_videobuffer3);
		_videobuffer->blit(*_videobuffer3);

		g_system->copyRectToScreen(_videobuffer3->getPixels(), _videobuffer3->pitch, 0, 0, _videobuffer3->w, _videobuffer3->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}
	} while (1);


	int keywait = 2000 + _ticks;

	ticks1 = _ticks;
	y1 = 0;
	do {

		_videobuffer->copyRectToSurface(_theendimg->getPixels(), _theendimg->pitch, 0, 0, _theendimg->w, _theendimg->h);

		y1 = 255;
		if (_ticks < ticks1 + 1000) {
			y1 = 255 * (_ticks - ticks1) / 1000;
			y1 = CLIP(y1, 0, 255);
		}

		_videobuffer->setAlpha(y1);
		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}

		g_system->getEventManager()->pollEvent(_event);

		if (_event.type == Common::EVENT_KEYDOWN && keywait < _ticks)
			break;

	} while (1);

	_videobuffer2->fillRect(Common::Rect(0, 0, _videobuffer2->w, _videobuffer2->h), 0);
	_videobuffer3->fillRect(Common::Rect(0, 0, _videobuffer3->w, _videobuffer3->h), 0);

	theEnd();

}

void GriffonEngine::theEnd() {
	for (int i = 0; i < kMaxFloat; i++) {
		_floattext[i][0] = 0;
		_floaticon[i][0] = 0;
	}

	for (float y = 0; y < 100; y += _fpsr) {
		_videobuffer->setAlpha((int)y);
		_videobuffer->fillRect(Common::Rect(0, 0, _videobuffer->w, _videobuffer->h), 0);
		g_system->copyRectToScreen(_videobuffer->getPixels(), _videobuffer->pitch, 0, 0, _videobuffer->w, _videobuffer->h);
		g_system->updateScreen();

		g_system->getEventManager()->pollEvent(_event);
		g_system->delayMillis(10);

		_tickspassed = _ticks;
		_ticks = g_system->getMillis();

		_tickspassed = _ticks - _tickspassed;
		_fpsr = (float)_tickspassed / 24.0;

		_fp++;
		if (_ticks > _nextticks) {
			_nextticks = _ticks + 1000;
			_fps = _fp;
			_fp = 0;
		}
	}

	title(0);
}


} // end of namespace Griffon
