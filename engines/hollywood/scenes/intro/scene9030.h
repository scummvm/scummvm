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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE9030_H
#define HOLLYWOOD_SCENES_INTRO_SCENE9030_H

#include "common/array.h"

#include "hollywood/scenes/intro/intro_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9030 : public IntroSceneBase {
public:
	Scene9030(HollywoodEngine *vm, const IndexedSurfaceBuffer *previousFramebuffer = nullptr,
		const Common::Array<byte> *previousPalette = nullptr);

	bool play();

private:
	bool load();
	uint presentRowOffset() const override;
	void clearFinalSweepBand(uint rowOffset, uint sweepOffset, byte bandWidth);
	void finish();

	const IndexedSurfaceBuffer *_previousFramebuffer;
	const Common::Array<byte> *_previousPalette;
	uint _rowOffset;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9030_H
