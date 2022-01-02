/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_SCENE_VIEW_H
#define BURIED_SCENE_VIEW_H

#include "buried/aidata.h"
#include "buried/animdata.h"
#include "buried/global_flags.h"
#include "buried/sprtdata.h"
#include "buried/window.h"

#include "common/array.h"

namespace Graphics {
struct Surface;
}

namespace Buried {

class AVIFrames;
class SceneBase;
class VideoWindow;

enum Direction {
	kDirectionUp = 0,
	kDirectionLeft = 1,
	kDirectionRight = 2,
	kDirectionDown = 3,
	kDirectionForward = 4
};

class SceneViewWindow : public Window {
public:
	SceneViewWindow(BuriedEngine *vm, Window *parent);
	~SceneViewWindow();

	bool _paused;
	bool _disableArthur;

	bool startNewGame(bool walkthrough = false);
	bool startNewGameIntro(bool walkthrough = false);
	bool startNewGame(const Location &startingLocation);
	bool showDeathScene(int deathSceneIndex);
	bool showCompletionScene();

	bool getSceneStaticData(const Location &location, LocationStaticData &sceneStaticData);
	GlobalFlags &getGlobalFlags() { return _globalFlags; }
	bool getCurrentSceneLocation(Location &curLocation);

	bool jumpToScene(const Location &newLocation);
	bool jumpToSceneRestore(const Location &newLocation);
	bool moveInDirection(Direction direction);
	bool moveToDestination(const DestinationScene &destinationData);
	bool timeSuitJump(int destination);

	bool playTransition(const DestinationScene &destinationData, int navFrame);
	bool videoTransition(const Location &location, DestinationScene destinationData, int navFrame);
	bool walkTransition(const Location &location, const DestinationScene &destinationData, int navFrame);
	bool pushTransition(Graphics::Surface *curBackground, Graphics::Surface *newBackground, int direction, uint stripSize, int totalTime);
	bool pushNewTransition(Graphics::Surface *newBackground, int direction, int stripSize, int totalTime);
	bool slideInTransition(Graphics::Surface *newBackground, int direction, int stripSize, int totalTime);
	bool slideOutTransition(Graphics::Surface *newBackground, int direction, int stripSize, int totalTime);

	bool changeStillFrameMovie(const Common::String &fileName = "");
	bool changeCycleFrameMovie(const Common::String &fileName = "");

	Graphics::Surface *getStillFrameCopy(int frameIndex);
	const Graphics::Surface *getStillFrame(int frameIndex);
	Graphics::Surface *getCycleFrameCopy(int frameIndex);
	const Graphics::Surface *getCycleFrame(int frameIndex);
	bool enableCycleFrameCache(bool enable);
	bool flushCycleFrameCache();
	bool enableCycling(bool enable);
	bool forceEnableCycling(bool enable);
	bool getCyclingStatus() const { return _cycleEnabled; }
	bool isCyclingEnabled() const { return _cycleEnabled || _forceCycleEnabled; }
	bool closeCycleFrameMovie();

	byte getGlobalFlagByte(int offset);
	bool setGlobalFlagByte(int offset, byte value);
	bool addNumberToGlobalFlagTable(int offset, int curItemCountOffset, int maxItems, byte numberToAdd);
	byte getNumberFromGlobalFlagTable(int offset, int tableIndex);
	bool isNumberInGlobalFlagTable(int offset, int curItemCountOffset, byte numberToCheck);

	bool playSynchronousAnimation(int animationID);
	bool playSynchronousAnimationExtern(int animationID);
	bool playPlacedSynchronousAnimation(int animationID, int left, int top);
	bool playClippedSynchronousAnimation(int animationID, int left, int top, int right, int bottom);

	bool startAsynchronousAnimation(int animationID, bool loopAnimation);
	bool startAsynchronousAnimation(int fileNameID, int startPosition, int playStartPosition, int frameCount, bool loopAnimation);
	bool startAsynchronousAnimationExtern(int fileNameID, int startPosition, int playStartPosition, int frameCount, bool loopAnimation);
	bool stopAsynchronousAnimation();
	bool isAsynchronousAnimationStillPlaying();
	int getAsynchronousAnimationCurrentPosition();
	bool asynchronousAnimationTimerCallback();
	bool startPlacedAsynchronousAnimation(int left, int top, int width, int height, int animationID, bool loopAnimation);
	bool startPlacedAsynchronousAnimation(int left, int top, int width, int height, int fileNameID, int startPosition, int playStartPosition, int frameCount, bool loopAnimation);
	bool startPlacedAsynchronousAnimationExtern(int left, int top, int width, int height, int fileNameID, int startPosition, int playStartPosition, int frameCount, bool loopAnimation);

	bool retrieveAICommentEntry(const Location &commentLocation, int commentType, const Common::Array<AIComment> &commentDatabase, int &lastFoundEntry, AIComment &currentCommentData);
	bool checkAICommentDependencies(const Location &commentLocation, const AIComment &commentData);
	bool playAICommentFromData(const AIComment &commentData);
	bool playAIComment(int commentType);
	bool playAIComment(const Location &commentLocation, int commentType);
	bool checkForAIComment(int commentType);
	bool checkForAIComment(const Location &commentLocation, int commentType);

	bool infoWindowDisplayed(bool flag);
	bool bioChipWindowDisplayed(bool flag);
	bool burnedLetterWindowDisplayed(bool flag);
	bool isAuxWindowDisplayed();

	void onPaint();
	void onTimer(uint timer);
	bool onSetCursor(uint message);
	void onEnable(bool enable);

	void onLButtonDown(const Common::Point &point, uint flags);
	void onLButtonUp(const Common::Point &point, uint flags);
	void onMouseMove(const Common::Point &point, uint flags);

	void onKeyUp(const Common::KeyState &key, uint flags);

	bool isScenePresent() { return _currentScene != 0; }
	int draggingItem(int itemID, const Common::Point &pointLocation, int itemFlags);
	int droppedItem(int itemID, const Common::Point &pointLocation, int itemFlags);

	bool updatePrebufferWithSprite(Sprite &spriteData);
	bool changeSpriteStatus(bool status);
	bool resetCursor();

	bool displayLiveText(const Common::String &text = "", bool notifyUser = true);
	bool displayTranslationText(const Common::String &text);

	bool resetNavigationArrows();

	bool startEnvironmentAmbient(int oldTimeZone = -1, int oldEnvironment = -1, int timeZone = -1, int environment = -1, bool fade = true);
	bool checkCustomAICommentDependencies(const Location &commentLocation, const AIComment &commentData);

	const SceneBase *getCurrentScene() const { return _currentScene; }

	Common::Array<AIComment> getAICommentDatabase(int timeZone, int environment);

private:
	Graphics::Surface *_preBuffer;
	SceneBase *_currentScene;
	Common::Array<LocationStaticData> _currentNavigationDatabase;
	GlobalFlags _globalFlags;
	VideoWindow *_walkMovie;

	Common::String _walkMovieFileName;

	AVIFrames *_stillFrames;
	AVIFrames *_cycleFrames;
	Common::Point _curMousePos;
	int _curCursor;
	Sprite _currentSprite;
	int _currentAmbient;
	bool _useScenePaint;
	bool _useSprite;
	bool _cycleEnabled, _forceCycleEnabled;
	uint _timer;

	bool _infoWindowDisplayed;
	bool _bioChipWindowDisplayed;
	bool _burnedLetterDisplayed;

	VideoWindow *_asyncMovie;
	Common::String _asyncMovieFileName;
	int _asyncMovieStartFrame;
	int _asyncMovieFrameCount;
	bool _loopAsyncMovie;

	Common::String _lastAICommentFileName;

	// Special sound handling for the demo
	uint _demoSoundTimer;
	int _demoSoundEffectHandle;
	void startDemoAmbientSound();

	void handleCyclingChange();

	bool initializeTimeZoneAndEnvironment(Window *viewWindow, int timeZone, int environment);
	SceneBase *constructSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);

	Common::Array<AnimEvent> getAnimationDatabase(int timeZone, int environment);

	bool moveToDestination(const DestinationScene &destinationData, int navFrame);

	// AI Lab
	bool initializeAILabTimeZoneAndEnvironment(Window *viewWindow, int environment);
	bool startAILabAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade);
	SceneBase *constructAILabSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	bool checkCustomSpaceStationAICommentDependencies(const Location &commentLocation, const AIComment &commentData);

	// Castle
	bool initializeCastleTimeZoneAndEnvironment(Window *viewWindow, int environment);
	bool startCastleAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade);
	SceneBase *constructCastleSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	bool checkCustomCastleAICommentDependencies(const Location &commentLocation, const AIComment &commentData);

	// Da Vinci's Studio
	bool initializeDaVinciTimeZoneAndEnvironment(Window *viewWindow, int environment);
	bool startDaVinciAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade);
	SceneBase *constructDaVinciSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	bool checkCustomDaVinciAICommentDependencies(const Location &commentLocation, const AIComment &commentData);

	// Future Apartment
	bool startFutureApartmentAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade);
	SceneBase *constructFutureApartmentSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);

	// Mayan
	bool initializeMayanTimeZoneAndEnvironment(Window *viewWindow, int environment);
	bool startMayanAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade);
	SceneBase *constructMayanSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	bool checkCustomMayanAICommentDependencies(const Location &commentLocation, const AIComment &commentData);

	// Agent 3's Lair
	bool initializeAgent3LairTimeZoneAndEnvironment(Window *viewWindow, int environment);
	bool startAgent3LairAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade);
	SceneBase *constructAgent3LairSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);

	// Alien
	bool initializeAlienTimeZoneAndEnvironment(Window *viewWindow, int environment);
	bool startAlienAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade);
	SceneBase *constructAlienSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
};

struct AICommentInfo {
	byte timeZone;
	byte environment;
	const char *filePath;
};

// timeZone, environment, filePath
static const AICommentInfo s_aiCommentInfo[] {
	// Castle
	{ 1,  1, "CASTLE/CGTT"  }, { 1,  2, "CASTLE/CGTS"  }, { 1,  3, "CASTLE/CGMW"  },
	{ 1,  4, "CASTLE/CGMB"  }, { 1,  5, "CASTLE/CGBS"  }, { 1,  6, "CASTLE/CGKC"  },
	{ 1,  7, "CASTLE/CGST"  }, { 1,  8, "CASTLE/CGKS"  }, { 1,  9, "CASTLE/CGSR"  },
	{ 1, 10, "CASTLE/CGTR"  },
	// Mayan
	{ 2,  1, "MAYAN/MYTP"   }, { 2,  2, "MAYAN/MYMC"   }, { 2,  3, "MAYAN/MYWG"   },
	{ 2,  4, "MAYAN/MYWT"   }, { 2,  5, "MAYAN/MYAG"   }, { 2,  6, "MAYAN/MYDG"   },
	// Future Apartment
	{ 4,  1, "FUTAPT/FAKI"  }, { 4,  2, "FUTAPT/FAER"  }, { 4,  3, "FUTAPT/FAMN"  },
	// Da Vinci
	{ 5,  1, "DAVINCI/DSPT" }, { 5,  2, "DAVINCI/DSCT" }, { 5,  3, "DAVINCI/DSGD" },
	{ 5,  4, "DAVINCI/DSWS" }, { 5,  5, "DAVINCI/DSCY" },
	// Space Station
	{ 6,  1, "AILAB/AIHW"   }, { 6,  2, "AILAB/AICR"   }, { 6,  3, "AILAB/AIDB"   },
	{ 6,  4, "AILAB/AISC"   }, { 6,  5, "AILAB/AINX"   }, { 6,  6, "AILAB/AIIC"   },
	{ 6,  7, "AILAB/AISW"   }, { 6,  8, "AILAB/AIMR"   }, { 6, 10, "AILAB/AIHW"   },	// there is no 9
	{ 0,  0, "" }
};

} // End of namespace Buried

#endif
