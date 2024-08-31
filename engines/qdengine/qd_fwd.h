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

#ifndef QDENGINE_QD_FWD_H
#define QDENGINE_QD_FWD_H

#include "common/str.h"
#include "common/std/list.h"
#include "common/std/vector.h"

namespace Common {
class SeekableReadStream;
}

namespace QDEngine {

class qdGameScene;
typedef Std::list<qdGameScene *> qdGameSceneList;

class qdGameEnd;
typedef Std::list<qdGameEnd *> qdGameEndList;

class qdVideo;
typedef Std::list<qdVideo *> qdVideoList;

class qdTriggerChain;
typedef Std::list<qdTriggerChain *> qdTriggerChainList;

class qdGameObject;
typedef Std::list<qdGameObject *> qdGameObjectList;

class qdMiniGame;
typedef Std::list<qdMiniGame *> qdMiniGameList;

class qdCounter;
typedef Std::list<qdCounter *> qdCounterList;

class qdGridZone;
typedef Std::list<qdGridZone *> qdGridZoneList;

class qdMusicTrack;
typedef Std::list<qdMusicTrack *> qdMusicTrackList;

class qdCondition;
typedef Std::list<qdCondition *> qdConditionList;

class qdSound;
typedef Std::list<qdSound *> qdSoundList;

class qdAnimation;
typedef Std::list<qdAnimation *> qdAnimationList;

class qdAnimationSet;
typedef Std::list<qdAnimationSet *> qdAnimationSetList;

class qdInventory;
typedef Std::list<qdInventory *> qdInventoryList;

class qdTriggerChain;
typedef Std::list<qdTriggerChain *> qdTriggerChainList;

class qdGameObjectState;
class qdGameObjectStateStatic;
class qdGameObjectStateWalk;
class qdGameObjectStateMask;
typedef Std::vector<qdGameObjectState *> qdGameObjectStateVector;

class qdGameObjectMouse;
class qdGameObjectMoving;
class qdNamedObject;
class qdNamedObjectReference;
class qdScaleInfo;

class qdInterfaceButton;

class qdGameDispatcher;

class qdFontInfo;
typedef Std::list<qdFontInfo *> qdFontInfoList;

} // namespace QDEngine

#endif // QDENGINE_QD_FWD_H
