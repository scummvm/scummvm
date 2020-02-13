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

#ifndef NEVERHOOD_NAVIGATIONSCENE_H
#define NEVERHOOD_NAVIGATIONSCENE_H

#include "neverhood/neverhood.h"
#include "neverhood/resourceman.h"
#include "neverhood/scene.h"
#include "neverhood/smackerplayer.h"

namespace Neverhood {

class NavigationScene : public Scene {
public:
	NavigationScene(NeverhoodEngine *vm, Module *parentModule, uint32 navigationListId, int navigationIndex, const byte *itemsTypes);
	~NavigationScene() override;
	int getNavigationAreaType();
	int getNavigationIndex() const { return _navigationIndex; }
	bool isWalkingForward() const { return _isWalkingForward; }
	bool isTurning() const { return _isTurning; }
	int getFrameNumber() const { return _smackerPlayer->getFrameNumber(); }
	uint32 getNavigationListId() const { return _navigationListId; }
protected:
	SmackerPlayer *_smackerPlayer;
	bool _smackerDone;
	NavigationList *_navigationList;
	uint32 _navigationListId;	// used for debugging
	int _navigationIndex;
	uint32 _smackerFileHash;
	bool _interactive;
	bool _isWalkingForward;
	bool _isTurning;
	bool _leaveSceneAfter;
	const byte *_itemsTypes;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createMouseCursor();
	void handleNavigation(const NPoint &mousePos);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_NAVIGATIONSCENE_H */
