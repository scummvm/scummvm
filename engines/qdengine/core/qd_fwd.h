#ifndef __QD_FWD_H__
#define __QD_FWD_H__

class qdGameScene;
typedef std::list<qdGameScene*> qdGameSceneList;

class qdGameEnd;
typedef std::list<qdGameEnd*> qdGameEndList;

class qdVideo;
typedef std::list<qdVideo*> qdVideoList;

class qdTriggerChain;
typedef std::list<qdTriggerChain*> qdTriggerChainList;

class qdGameObject;
typedef std::list<qdGameObject*> qdGameObjectList;

class qdMiniGame;
typedef std::list<qdMiniGame*> qdMiniGameList;

class qdCounter;
typedef std::list<qdCounter*> qdCounterList;

class qdGridZone;
typedef std::list<qdGridZone*> qdGridZoneList;

class qdMusicTrack;
typedef std::list<qdMusicTrack*> qdMusicTrackList;

class qdCondition;
typedef std::list<qdCondition*> qdConditionList;

class qdSound;
typedef std::list<qdSound*> qdSoundList;

class qdAnimation;
typedef std::list<qdAnimation*> qdAnimationList;

class qdAnimationSet;
typedef std::list<qdAnimationSet*> qdAnimationSetList;

class qdInventory;
typedef std::list<qdInventory*> qdInventoryList;

class qdTriggerChain;
typedef std::list<qdTriggerChain*> qdTriggerChainList;

class qdGameObjectState;
class qdGameObjectStateStatic;
class qdGameObjectStateWalk;
class qdGameObjectStateMask;
typedef std::vector<qdGameObjectState*> qdGameObjectStateVector;

class qdGameObjectMouse;
class qdGameObjectMoving;
class qdNamedObject;
class qdNamedObjectReference;
class qdScaleInfo;

class qdInterfaceButton;

class qdGameDispatcher;

typedef std::list<std::string> qdFileNameList;

class qdFontInfo;
typedef std::list<qdFontInfo*> qdFontInfoList;

#endif // __QD_FWD_H__
