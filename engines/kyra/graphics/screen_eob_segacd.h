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

#ifndef KYRA_SCREEN_EOB_SEGACD_H
#define KYRA_SCREEN_EOB_SEGACD_H

#ifdef ENABLE_EOB

#include "kyra/graphics/screen_eob.h"
#include "graphics/segagfx.h"

namespace Kyra {

class SCDRenderer : public Graphics::SegaRenderer {
public:
	SCDRenderer(Screen_EoB *screen);
	~SCDRenderer();

	void loadStreamToVRAM(Common::SeekableReadStream *in, uint16 addr, bool compressedData = false);
	void fillRectWithTiles(int vramArea, int x, int y, int w, int h, uint16 nameTblEntry, bool incr = false, bool topToBottom = false, const uint16 *patternTable = 0);

	void renderToPage(int destPageNum, int renderBlockX = -1, int renderBlockY = -1, int renderBlockWidth = -1, int renderBlockHeight = -1, bool spritesOnly = false);

private:
	Screen_EoB *_screen;
};

class SegaAnimator {
public:
	SegaAnimator(SCDRenderer *renderer);
	~SegaAnimator();

	void initSprite(int id, int16 x, int16 y, uint16 nameTbl, uint16 hw);
	void clearSprites();
	void moveMorphSprite(int id, uint16 nameTbl, int16 addX, int16 addY);
	void moveSprites(int id, uint16 num, int16 addX, int16 addY);
	void moveSprites2(int id, uint16 num, int16 addX, int16 addY);

	void update();

private:
	struct Sprite {
		int16 x;
		int16 y;
		uint16 nameTbl;
		uint16 hw;
	};

	uint16 *_tempBuffer;
	Sprite *_sprites;
	SCDRenderer *_renderer;
	bool _needUpdate;
};

class ScrollManager {
public:
	ScrollManager(SCDRenderer *renderer);
	~ScrollManager();

	void setVScrollTimers(uint16 destA, int incrA, int delayA, uint16 destB, int incrB, int delayB);
	void setHScrollTimers(uint16 destA, int incrA, int delayA, uint16 destB, int incrB, int delayB);
	void updateScrollTimers();
	void fastForward();

private:
	struct ScrollTimer {
		ScrollTimer() : _offsCur(0), _offsDest(0), _incr(0), _delay(0), _timer(0) {}
		int16 _offsCur;
		int16 _offsDest;
		int16 _incr;
		int16 _delay;
		int16 _timer;
	};

	ScrollTimer *_vScrollTimers;
	ScrollTimer *_hScrollTimers;
	SCDRenderer *_renderer;
};

} // End of namespace Kyra

#endif // ENABLE_EOB

#endif
