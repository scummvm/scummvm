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

#ifndef CHEWY_SCENE_H
#define CHEWY_SCENE_H

#include "chewy/chewy.h"

namespace Chewy {

struct SceneInfo;

class Scene {
public:
	Scene(ChewyEngine *vm);
	virtual ~Scene();

	void change(uint scene);
	void draw();
	void updateMouse(Common::Point coords);
	void mouseClick(Common::Point coords);
	uint getCurScene() const {
		return _curScene;
	}

private:
	void loadSceneInfo();

	ChewyEngine *_vm;
	uint _curScene;
	SceneInfo *_sceneInfo;
};

} // End of namespace Chewy

#endif
