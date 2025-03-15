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
 * aint32 with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_MAZE_DOOM_H
#define HODJNPODJ_MAZE_DOOM_H

#include "bagel/hodjnpodj/views/minigame_view.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

class MazeDoom : public MinigameView {
private:
	const char *_upBmp, *_downBmp,
		*_leftBmp, *_rightBmp;
	GfxSurface _background;

	void setupBitmaps();
	void loadBackground();

public:
	MazeDoom();
	virtual ~MazeDoom() {}

	bool msgOpen(const OpenMessage &msg) override;
	void draw() override;
};

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel

#endif
