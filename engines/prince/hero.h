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
 */

#ifndef PRINCE_HERO_H
#define PRINCE_HERO_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/memstream.h"

#include "graphics/surface.h"
#include "graphics/primitives.h"

namespace Prince {

class Animation;
class PrinceEngine;
class GraphicsMan;
struct InventoryItem;
struct DrawNode;

class Hero {
public:
	static const uint32 kMoveSetSize = 26;
	static const int16 kStepLeftRight = 8;
	static const int16 kStepUpDown = 4;
	static const int16 kHeroShadowZ = 2;

	enum State {
		kHeroStateStay,
		kHeroStateTurn,
		kHeroStateMove,
		kHeroStateBore,
		kHeroStateSpec,
		kHeroStateTalk,
		kHeroStateMvan,
		kHeroStateTran,
		kHeroStateRun,
		kHeroStateDelayMove
	};

	enum Direction {
		kHeroDirLeft = 1,
		kHeroDirRight = 2,
		kHeroDirUp = 3,
		kHeroDirDown = 4
	};

	enum MoveSet {
		kMove_SL,
		kMove_SR,
		kMove_SU,
		kMove_SD,
		kMove_ML,
		kMove_MR,
		kMove_MU,
		kMove_MD,
		kMove_TL,
		kMove_TR,
		kMove_TU,
		kMove_TD,
		kMove_MLU,
		kMove_MLD,
		kMove_MLR,
		kMove_MRU,
		kMove_MRD,
		kMove_MRL,
		kMove_MUL,
		kMove_MUR,
		kMove_MUD,
		kMove_MDL,
		kMove_MDR,
		kMove_MDU,
		kMove_BORED1,
		kMove_BORED2
	};

	// Used instead of offset in getData
	enum AttrId {
		kHeroLastDir = 26,
		kHeroAnimSet = 120
	};

	uint16 getData(AttrId dataId);

	Hero(PrinceEngine *vm, GraphicsMan *graph);
	~Hero();
	bool loadAnimSet(uint32 heroAnimNumber);

	Graphics::Surface *getSurface();

	void setPos(int16 x, int16 y) { _middleX = x; _middleY = y; }
	void setVisible(bool flag) { _visible = flag; }

	void showHero();
	void drawHero();
	void freeZoomedSurface();
	void heroStanding();
	void heroMoveGotIt(int x, int y, int dir);
	int rotateHero(int oldDirection, int newDirection);
	void scrollHero();
	void setScale(int8 zoomBitmapValue);
	int getScaledValue(int size);
	void selectZoom();
	void countDrawPosition();
	Graphics::Surface *zoomSprite(Graphics::Surface *heroFrame);
	void line(int x1, int y1, int x2, int y2);
	void plotPoint(int x, int y);
	static void showHeroShadow(Graphics::Surface *screen, DrawNode *drawNode);
	void drawHeroShadow(Graphics::Surface *heroFrame);
	void freeOldMove();
	void freeHeroAnim();

	uint16 _number;
	uint16 _visible;
	int16 _state;
	int16 _middleX; // middle of X
	int16 _middleY; // lower part of hero
	int16 _moveSetType;

	int16 _frameXSize;
	int16 _frameYSize;
	int16 _scaledFrameXSize;
	int16 _scaledFrameYSize;
	int16 _drawX;
	int16 _drawY;
	int16 _drawZ;

	byte *_coords; // array of coordinates
	byte *_dirTab; // array of directions
	byte *_currCoords; // current coordinations
	byte *_currDirTab; // current direction
	int16 _lastDirection; // previous move direction
	int16 _destDirection;
	int16 _leftRightMainDir; // left or right - dominant direction
	int16 _upDownMainDir; // up or down - dominant direction
	int32 _phase; // Phase animation phase
	int16 _step; // Step x/y step size depends on direction
	int16 _maxBoredom; // stand still timeout
	int16 _boredomTime; // Boredom current boredom time in frames
	uint16 _boreNum; // Bore anim frame
	int16 _talkTime; // TalkTime time of talk anim
	Animation *_specAnim; // additional anim
	Graphics::Surface *_zoomedHeroSurface;

	uint16 _currHeight; // height of current anim phase

	Common::Array<byte> _inventory; // Inventory array of items
	Common::Array<byte> _inventory2; // Inventory2 array of items
	// Font subtitiles font
	int _color; // subtitles color
	uint32 _animSetNr; // number of animation set
	Common::Array<Animation *> _moveSet; // MoveAnims MoveSet

	uint32 _moveDelay;
	uint32 _shadMinus;

private:
	PrinceEngine *_vm;
	GraphicsMan *_graph;
};

} // End of namespace Prince

#endif
