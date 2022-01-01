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

#ifndef BURIED_SCENE_COMMON_H
#define BURIED_SCENE_COMMON_H

#include "buried/avi_frames.h"
#include "buried/bookdata.h"
#include "buried/inndata.h"
#include "buried/environ/scene_base.h"

namespace Buried {

class BasicDoor : public SceneBase {
public:
	BasicDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1, int timeZone = -1, int environment = -1,
			int node = -1, int facing = -1, int orientation = -1, int depth = -1, int transitionType = -1, int transitionData = -1,
			int transitionStartFrame = -1, int transitionLength = -1, int openingSoundID = -1);

	int mouseDown(Window *viewWindow, const Common::Point &pointLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

	bool _clicked;
	Common::Rect _clickable;
	DestinationScene _destData;
	int _openingSoundID;
};

class TurnDepthPreChange : public SceneBase {
public:
	TurnDepthPreChange(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			byte &flag, int upDepth = -1, int leftDepth = -1, int rightDepth = -1, int downDepth = -1, int forwardDepth = -1);
};

class GenericItemAcquire : public SceneBase {
public:
	GenericItemAcquire(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left, int top, int right, int bottom, int itemID, int clearStillFrame, byte &itemFlag);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation);
	virtual int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

protected:
	bool _itemPresent;
	Common::Rect _acquireRegion;
	int _fullFrameIndex;
	int _clearFrameIndex;
	int _itemID;
	byte &_itemFlag;
};

class PlaySoundExitingFromScene : public SceneBase {
public:
	PlaySoundExitingFromScene(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int soundFileNameID = -1);
	int postExitRoom(Window *viewWindow, const Location &newLocation);

private:
	int _soundFileNameID;
};

class PlaySoundExitingFromSceneDeux : public SceneBase {
public:
	PlaySoundExitingFromSceneDeux(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int soundFileNameID = -1);
	int postExitRoom(Window *viewWindow, const Location &newLocation);

private:
	int _soundFileNameID;
};

class PlaySoundEnteringScene : public SceneBase {
public:
	PlaySoundEnteringScene(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int soundFileNameID, byte &flag);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);

private:
	int _soundFileNameID;
	byte &_flag;
};

class ClickChangeScene : public SceneBase {
public:
	ClickChangeScene(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1, int cursorID = 0,
			int timeZone = -1, int environment = -1, int node = -1, int facing = -1, int orientation = -1, int depth = -1,
			int transitionType = -1, int transitionData = -1, int transitionStartFrame = -1, int transitionLength = -1);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _cursorID;
	Common::Rect _clickRegion;
	DestinationScene _clickDestination;
};

class ClickPlayVideoSwitchAI : public SceneBase {
public:
	ClickPlayVideoSwitchAI(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int animID, int cursorID, byte &flag, int left = 0, int top = 0, int right = 0, int bottom = 0);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _cursorID;
	int _animID;
	Common::Rect _clickRegion;
	byte &_flag;
};

class ClickChangeSceneSetFlag : public ClickChangeScene {
public:
	ClickChangeSceneSetFlag(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left, int top, int right, int bottom, int cursorID,
			int timeZone, int environment, int node, int facing, int orientation, int depth,
			int transitionType, int transitionData, int transitionStartFrame, int transitionLength, byte &flag);
};

class PlayStingers : public SceneBase {
public:
	PlayStingers(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int stingerVolume, byte &lastStingerFlag, byte &effectIDFlag, int firstStingerFileID, int lastStingerFileID);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);

private:
	int _stingerVolume;
	byte &_lastStingerFlag;
	byte &_effectIDFlag;
	int _firstStingerFileID;
	int _lastStingerFileID;
};

class ClickPlaySound : public SceneBase {
public:
	ClickPlaySound(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			byte &flag, int soundID, int cursorID, int left, int top, int right, int bottom);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _cursorID;
	int _soundID;
	Common::Rect _clickRegion;
	byte &_flag;
};

class ClickZoom : public SceneBase {
public:
	ClickZoom(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int animInID = -1, int stillInID = -1, int animOutID = -1, int stillOutID = -1,
			int cursorID = 0, int left = 0, int top = 0, int right = 0, int bottom = 0);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _cursorID;
	Common::Rect _clickRegion;
	int _animInID;
	int _stillInID;
	int _animOutID;
	int _stillOutID;
	bool _zoomedIn;
	LocationStaticData _savedNavData;
};

class PlaySoundEnteringFromScene : public SceneBase {
public:
	PlaySoundEnteringFromScene(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int soundFileNameID = -1, int timeZone = -1, int environment = -1, int node = -1, int facing = -1, int orientation = -1, int depth = -1);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);

private:
	Location _soundLocation;
	int _soundFileNameID;
};

class SetFlagOnEntry : public SceneBase {
public:
	SetFlagOnEntry(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			byte &flag, byte flagNewValue);
};

class InteractiveNewsNetwork : public SceneBase {
public:
	InteractiveNewsNetwork(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int enterTransition = -1, int timeZone = -1, int environment = -1, int node = -1, int facing = -1, int orientation = -1, int depth = -1,
			int transitionType = -1, int transitionData = -1, int transitionStartFrame = -1, int transitionLength = -1);
	~InteractiveNewsNetwork();
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	int preExitRoom(Window *viewWindow, const Location &newLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int paint(Window *viewWindow, Graphics::Surface *preBuffer);
	int movieCallback(Window *viewWindow, VideoWindow *movie, int animationID, int status);
	int timerCallback(Window *viewWindow);

private:
	int _enterTransition;
	int _currentMovieFrame;
	DestinationScene _returnDestination;
	AVIFrames _stillFrames;
	Common::Array<INNFrame> _frameDatabase;
	Common::Array<INNMediaElement> _movieDatabase;
	Common::Array<byte> _hyperLinkHistory;
	bool _playingMovie;
	bool _loopingMovie;
	bool _playingAudio;
	int _audioChannel;

	void loadFrameDatabase();
	void loadMovieDatabase();
};

class DisplayMessageWithEvidenceWhenEnteringNode : public SceneBase {
public:
	DisplayMessageWithEvidenceWhenEnteringNode(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int evidenceID = -1, int messageBoxTextID = -1);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);

private:
	int _messageBoxTextID;
	byte _evidenceID;
};

class ClickPlayLoopingVideoClip : public SceneBase {
public:
	ClickPlayLoopingVideoClip(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int cursorID, int animID, int left, int top, int right, int bottom, byte &flag, int newFlagValue);
	int preExitRoom(Window *viewWindow, const Location &newLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _cursorID;
	int _animID;
	Common::Rect _clickRegion;
	byte &_flag;
	int _flagValue;
	bool _playing;
};

class OneShotEntryVideoWarning : public SceneBase {
public:
	OneShotEntryVideoWarning(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int animID, byte &flag, int warningMessageID);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);

private:
	int _animID;
	byte &_flag;
	int _warningMessageID;
};

class DisableForwardMovement : public SceneBase {
public:
	DisableForwardMovement(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int flagValue = 1);
};

class CycleEntryVideoWarning : public SceneBase {
public:
	CycleEntryVideoWarning(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int animIDA = 0, int animIDB = 0, int warningMessageID = -1);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);

private:
	int _animIDA;
	int _animIDB;
	int _warningMessageID;
};

class ClickPlayVideoSwitch : public SceneBase {
public:
	ClickPlayVideoSwitch(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int animID, int cursorID, byte &flag, int left = 0, int top = 0, int right = 0, int bottom = 0);

	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _cursorID;
	int _animID;
	Common::Rect _clickRegion;
	byte &_flag;
};

class ClickPlayVideo : public SceneBase {
public:
	ClickPlayVideo(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int animID = 0, int cursorID = -1, int left = 0, int top = 0, int right = 0, int bottom = 0);

	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _cursorID;
	int _animID;
	Common::Rect _clickRegion;
};

class VideoDeath : public SceneBase {
public:
	VideoDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation, int deathID = 0, int messageTextID = -1);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	int postExitRoom(Window *viewWindow, const Location &newLocation);

private:
	int _deathID;
	int _messageTextID;
};

class ClickChangeDepth : public SceneBase {
public:
	ClickChangeDepth(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int newDepth = 0, int cursorID = 0, int left = -1, int top = -1, int right = -1, int bottom = -1);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _newDepth;
	int _cursorID;
	Common::Rect _clickableRegion;
};

class OpenFirstItemAcquire : public SceneBase {
public:
	OpenFirstItemAcquire(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int openLeft = 0, int openTop = 0, int openRight = 0, int openBottom = 0, int getLeft = 0, int getTop = 0, int getRight = 0,
			int getBottom = 0, int animOpenWith = 0, int animOpenWithout = 0, int itemID = 0, int fullStillFrame = 0, int clearStillFrame = 0);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	bool _itemPresent;
	bool _open;
	Common::Rect _openClickRegion;
	Common::Rect _acquireRegion;
	int _fullFrameIndex;
	int _clearFrameIndex;
	int _itemID;
	int _animOpenWith;
	int _animOpenWithout;
};

class BrowseBook : public SceneBase {
public:
	BrowseBook(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int bookResID = 0, int textStartResID = -1, int startingPageID = 0, int timeZone = -1, int environment = -1,
			int node = -1, int facing = -1, int orientation = -1, int depth = -1, int transitionType = -1, int transitionData = -1,
			int transitionStartFrame = -1, int transitionLength = -1);
	int gdiPaint(Window *viewWindow);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);
	int mouseMove(Window *viewWindow, const Common::Point &pointLocation);

private:
	int pageChanged(Window *viewWindow);
	int textTranslated(Window *viewWindow);

	Common::Array<BookPage> _bookDatabase;
	int _curPage;
	Common::Rect _top, _bottom, _left, _right, _putDown;
	DestinationScene _putDownDestination;
	int _translatedTextResourceID;
	int _curLineIndex;
};

class ClickPlaySoundSynchronous : public SceneBase {
public:
	ClickPlaySoundSynchronous(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int soundID = 0, int cursorID = 0, int left = 0, int top = 0, int right = 0, int bottom = 0);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _cursorID;
	int _soundID;
	Common::Rect _clickRegion;
};

class TrialRecallScene : public SceneBase {
public:
	TrialRecallScene(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
};

} // End of namespace Buried

#endif
