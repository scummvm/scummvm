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

#include <list>
#include <string>
#include <vector>


namespace QDEngine {

class qdGameScene;
typedef std::list<qdGameScene *> qdGameSceneList;

class qdGameEnd;
typedef std::list<qdGameEnd *> qdGameEndList;

class qdVideo;
typedef std::list<qdVideo *> qdVideoList;

class qdTriggerChain;
typedef std::list<qdTriggerChain *> qdTriggerChainList;

class qdGameObject;
typedef std::list<qdGameObject *> qdGameObjectList;

class qdMiniGame;
typedef std::list<qdMiniGame *> qdMiniGameList;

class qdCounter;
typedef std::list<qdCounter *> qdCounterList;

class qdGridZone;
typedef std::list<qdGridZone *> qdGridZoneList;

class qdMusicTrack;
typedef std::list<qdMusicTrack *> qdMusicTrackList;

class qdCondition;
typedef std::list<qdCondition *> qdConditionList;

class qdSound;
typedef std::list<qdSound *> qdSoundList;

class qdAnimation;
typedef std::list<qdAnimation *> qdAnimationList;

class qdAnimationSet;
typedef std::list<qdAnimationSet *> qdAnimationSetList;

class qdInventory;
typedef std::list<qdInventory *> qdInventoryList;

class qdTriggerChain;
typedef std::list<qdTriggerChain *> qdTriggerChainList;

class qdGameObjectState;
class qdGameObjectStateStatic;
class qdGameObjectStateWalk;
class qdGameObjectStateMask;
typedef std::vector<qdGameObjectState *> qdGameObjectStateVector;

class qdGameObjectMouse;
class qdGameObjectMoving;
class qdNamedObject;
class qdNamedObjectReference;
class qdScaleInfo;

class qdInterfaceButton;

class qdGameDispatcher;

typedef std::list<std::string> qdFileNameList;

class qdFontInfo;
typedef std::list<qdFontInfo *> qdFontInfoList;

} // namespace QDEngine

#endif // QDENGINE_QD_FWD_H
