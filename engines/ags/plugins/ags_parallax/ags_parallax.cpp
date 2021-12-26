/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "ags/plugins/ags_parallax/ags_parallax.h"
#include "ags/shared/core/platform.h"
#include "common/endian.h"

namespace AGS3 {
namespace Plugins {
namespace AGSParallax {

const unsigned int Magic = 0xCAFE0000;
const unsigned int Version = 2;
const unsigned int SaveMagic = Magic + Version;

const char *AGSParallax::AGS_GetPluginName() {
	return "Parallax plugin recreation";
}

void AGSParallax::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	if (_engine->version < 13)
		_engine->AbortGame("Engine interface is too old, need newer version of AGS.");

	SCRIPT_METHOD(pxDrawSprite, AGSParallax::pxDrawSprite);
	SCRIPT_METHOD(pxDeleteSprite, AGSParallax::pxDeleteSprite);

	_engine->RequestEventHook(AGSE_PREGUIDRAW);
	_engine->RequestEventHook(AGSE_PRESCREENDRAW);
	_engine->RequestEventHook(AGSE_ENTERROOM);
	_engine->RequestEventHook(AGSE_SAVEGAME);
	_engine->RequestEventHook(AGSE_RESTOREGAME);
}

int64 AGSParallax::AGS_EngineOnEvent(int event, NumberPtr data) {
	if (event == AGSE_PREGUIDRAW) {
		Draw(true);
	} else if (event == AGSE_PRESCREENDRAW) {
		// Get screen size and then draw
		_engine->GetScreenDimensions(&_screenWidth, &_screenHeight, &_screenColorDepth);
		Draw(false);
	} else if (event == AGSE_ENTERROOM) {
		// Reset all _sprites
		clear();
	} else if (event == AGSE_RESTOREGAME) {
		Serializer s(_engine, data, true);
		syncGame(s);
	} else if (event == AGSE_SAVEGAME) {
		Serializer s(_engine, data, false);
		syncGame(s);
	}

	return 0;
}

void AGSParallax::clear() {
	for (int i = 0; i < MAX_SPRITES; i++) {
		_sprites[i] = Sprite();
		_sprites[i].slot = -1;
	}

	_enabled = false;
}

void AGSParallax::syncGame(Serializer &s) {
	int saveVersion = SaveMagic;
	s.syncAsInt(saveVersion);
	if ((uint)saveVersion != SaveMagic) {
		_engine->AbortGame("ags_parallax: bad save.");
		return;
	}

	for (int i = 0; i < MAX_SPRITES; ++i)
		_sprites[i].syncGame(s);
	s.syncAsBool(_enabled);
}

void AGSParallax::Draw(bool foreground) {
	if (!_enabled)
		return;

	BITMAP *bmp;
	int i;

	int32 offsetX = 0;
	int32 offsetY = 0;
	_engine->ViewportToRoom(&offsetX, &offsetY);

	for (i = 0; i < MAX_SPRITES; i++) {
		if (_sprites[i].slot > -1) {
			if (foreground) {
				if (_sprites[i].speed > 0) {
					bmp = _engine->GetSpriteGraphic(_sprites[i].slot);
					if (bmp)
						_engine->BlitBitmap(_sprites[i].x - offsetX - (_sprites[i].speed * offsetX / 100), _sprites[i].y, bmp, 1);
				}
			} else {
				if (_sprites[i].speed <= 0) {
					bmp = _engine->GetSpriteGraphic(_sprites[i].slot);
					if (bmp)
						_engine->BlitBitmap(_sprites[i].x - offsetX - (_sprites[i].speed * offsetX / 1000), _sprites[i].y, bmp, 1);
				}
			}
		}
	}
}

void AGSParallax::pxDrawSprite(ScriptMethodParams &params) {
	PARAMS5(int, id, int, x, int, y, int, slot, int, speed);

	if ((id < 0) || (id >= MAX_SPRITES))
		return;

	if ((speed < -MAX_SPEED) || (speed > MAX_SPEED))
		speed = 0;

	_sprites[id].x = x;
	_sprites[id].y = y;
	_sprites[id].slot = slot;
	_sprites[id].speed = speed;

	_engine->RoomToViewport(&_sprites[id].x, &_sprites[id].y);

	_enabled = true;
}


void AGSParallax::pxDeleteSprite(ScriptMethodParams &params) {
	PARAMS1(int, id);

	if ((id < 0) || (id >= MAX_SPRITES))
		return;

	_sprites[id].slot = -1;
}

/*------------------------------------------------------------------*/

void Sprite::syncGame(Serializer &s) {
	s.syncAsInt(x);
	s.syncAsInt(y);
	s.syncAsInt(slot);
	s.syncAsInt(speed);
}

} // namespace AGSParallax
} // namespace Plugins
} // namespace AGS3
