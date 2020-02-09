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

// TODO: I couldn't come up with a better name than 'Module' so far

#ifndef NEVERHOOD_MODULE_H
#define NEVERHOOD_MODULE_H

#include "neverhood/neverhood.h"
#include "neverhood/background.h"
#include "neverhood/entity.h"
#include "neverhood/graphics.h"
#include "neverhood/mouse.h"
#include "neverhood/palette.h"
#include "neverhood/screen.h"

namespace Neverhood {

class NavigationScene;

enum SceneType {
	kSceneTypeNormal,
	kSceneTypeSmacker,
	kSceneTypeNavigation
};

class Module : public Entity {
public:
	Module(NeverhoodEngine *vm, Module *parentModule);
	~Module() override;
	void draw() override;
	SceneType getSceneType() { return _sceneType; }

	Entity *_childObject;
protected:
	Module *_parentModule;
	bool _done;
	uint32 _moduleResult;
	SceneType _sceneType;
	int _navigationAreaType;
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	NavigationScene *navigationScene();
	void createNavigationScene(uint32 navigationListId, int navigationIndex, const byte *itemsTypes = NULL);
	void createSmackerScene(uint32 fileHash, bool doubleSurface, bool canSkip, bool canAbort);
	void createSmackerScene(const uint32 *fileHashList, bool doubleSurface, bool canSkip, bool canAbort);
	void createStaticScene(uint32 backgroundFileHash, uint32 cursorFileHash);
	void createDemoScene();
	bool updateChild();
	void leaveModule(uint32 result);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MODULE_H */
