/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

class SceneViewWindow : public Window {
public:
	SceneViewWindow(BuriedEngine *vm, Window *parent);
	~SceneViewWindow();

	bool _paused;
	bool _disableArthur;

	bool startNewGame(bool walkthrough = false);
	bool startNewGameIntro(bool walkthrough = false);
	bool startNewGame(const Location &startingLocation);
	bool startNewGame(const Common::String &restoreFile);
	bool showDeathScene(int deathSceneIndex);
	bool showCompletionScene();

	bool getSceneStaticData(const Location &location, LocationStaticData &sceneStaticData);
	GlobalFlags &getGlobalFlags() { return _globalFlags; }
	bool getCurrentSceneLocation(Location &curLocation);

	bool jumpToScene(const Location &newLocation);
	bool jumpToSceneRestore(const Location &newLocation);
	bool moveInDirection(int direction);
	bool moveToDestination(const DestinationScene &destinationData);
	bool timeSuitJump(int destination);

	bool playTransition(const DestinationScene &destinationData, int navFrame);
	bool videoTransition(const Location &location, DestinationScene destinationData, int navFrame);
	bool walkTransition(const Location &location, const DestinationScene &destinationData, int navFrame);
	bool pushTransition(Graphics::Surface *curBackground, Graphics::Surface *newBackground, int direction, int stripSize, int totalTime);
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
	bool getCyclingStatus() { return _cycleEnabled; }
	bool closeCycleFrameMovie();

	int getGlobalFlag(int offset);
	byte getGlobalFlagByte(int offset);
	bool setGlobalFlag(int offset, int value);
	bool setGlobalFlagByte(int offset, byte value);
	bool setGlobalFlagDWord(int offset, uint32 value);
	uint32 getGlobalFlagDWord(int offset);
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
	bool _cycleEnabled;
	uint _timer;
	uint _soundTimer;

	bool _infoWindowDisplayed;
	bool _bioChipWindowDisplayed;
	bool _burnedLetterDisplayed;

	VideoWindow *_asyncMovie;
	Common::String _asyncMovieFileName;
	int _asyncMovieStartFrame;
	int _asyncMovieFrameCount;
	bool _loopAsyncMovie;

	Common::String _lastAICommentFileName;

	bool _useWaitCursor;
	int _oldCursorForWait;

	bool initializeTimeZoneAndEnvironment(Window *viewWindow, int timeZone, int environment);
	SceneBase *constructSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);

	Common::Array<AnimEvent> getAnimationDatabase(int timeZone, int environment);
	Common::Array<AIComment> getAICommentDatabase(int timeZone, int environment);

	// Castle
	bool initializeCastleTimeZoneAndEnvironment(Window *viewWindow, int environment);
	SceneBase *constructCastleSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
};

} // End of namespace Buried

#endif
