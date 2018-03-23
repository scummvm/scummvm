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

#ifndef BLADERUNNER_ITEMS_H
#define BLADERUNNER_ITEMS_H

#include "bladerunner/bladerunner.h"
#include "bladerunner/item.h"

#include "common/array.h"

namespace BladeRunner {

class Items {
	BladeRunnerEngine *_vm;

	Common::Array<Item *> _items;

public:
	Items(BladeRunnerEngine *vm);
	~Items();

	void getXYZ(int itemId, float *x, float *y, float *z) const;
	void getWidthHeight(int itemId, int *width, int *height) const;

	void tick();
	bool addToWorld(int itemId, int animationId, int setId, Vector3 position, int facing, int height, int width, bool isTargetFlag, bool isVisible, bool isPoliceMazeEnemy, bool addToSetFlag);
	bool addToSet(int itemId);
	bool remove(int itemId);

	void setIsTarget(int itemId, bool isTarget);
	bool isTarget(int itemId) const;
	bool isPoliceMazeEnemy(int itemId) const;
	bool isVisible(int itemId) const;
	int findTargetUnderMouse(int mouseX, int mouseY) const;

	BoundingBox *getBoundingBox(int itemId);
	Common::Rect *getScreenRectangle(int itemId);

	void spinInWorld(int itemId);

private:
	int findItem(int itemId) const;
};

} // End of namespace BladeRunner

#endif
