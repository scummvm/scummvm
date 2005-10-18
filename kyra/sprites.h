/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */

#ifndef KYRASPRITES_H
#define KYRASPRITES_H

namespace Kyra {

#define MAX_NUM_SPRITES 50
#define MAX_NUM_ANIMS 11

struct Sprite {
	uint16 x;
	uint16 y;
	uint16 width;
	uint16 height;
};

struct Anim {
	uint8 *script;
	uint16 length;
	int16 x;
	int16 y;
	bool flipX;
	int8 sprite;
	uint8 *loopStart;
	uint16 loopsLeft;
	uint8 *reentry;
	uint32 nextRun;
	bool play;
};

class Sprites {
public:

	Sprites(KyraEngine *engine, OSystem *system);
	~Sprites();

	void doAnims();
	void loadDAT(const char* filename);
	Sprite getSprite(uint8 spriteID);
	void drawSprites(uint8 srcPage, uint8 dstPage);
	
	void enableAnim(uint8 anim) { _anims[anim].play = true; }
	void disableAnim(uint8 anim) { _anims[anim].play = false; }

protected:
	KyraEngine *_engine;
	Resource *_res;
	OSystem *_system;
	Screen *_screen;
	Sprite _sprites[MAX_NUM_SPRITES];
	uint8 *_dat;
	Anim _anims[MAX_NUM_ANIMS];
	Common::RandomSource _rnd;
	uint8 _animDelay;
};

} // End of namespace Kyra

#endif
