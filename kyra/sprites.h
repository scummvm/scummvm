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
	uint16 width;
	uint16 height;
	uint16 width2;
	uint16 height2;
	uint16 unk1;
	uint16 drawY;
	uint16 unk2;
	uint8 *background;
};

class Sprites {
public:

	Sprites(KyraEngine *engine, OSystem *system);
	~Sprites();

	void updateSceneAnims();
	void setupSceneAnims();
	void loadDAT(const char *filename, SceneExits &exits);
	void loadSceneShapes();
	
	Anim _anims[MAX_NUM_ANIMS];
	AnimObject *_animObjects;
	uint8 *_sceneShapes[50];

	void refreshSceneAnimObject(uint8 animNum, uint8 shapeNum, uint16 x, uint16 y, bool flipX, bool unkFlag);

protected:
	void freeSceneShapes();

	KyraEngine *_engine;
	Resource *_res;
	OSystem *_system;
	Screen *_screen;
	uint8 *_dat;
	Common::RandomSource _rnd;
	uint8 _animDelay;
	uint8 *_spriteDefStart;
};

} // End of namespace Kyra

#endif
