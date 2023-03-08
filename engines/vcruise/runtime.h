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

#ifndef VCRUISE_RUNTIME_H
#define VCRUISE_RUNTIME_H

#include "common/hashmap.h"
#include "common/keyboard.h"

#include "vcruise/detection.h"

class OSystem;

namespace Common {

class RandomSource;
class ReadStream;
class WriteStream;

} // End of namespace Commom

namespace Graphics {

struct PixelFormat;
struct WinCursorGroup;
class ManagedSurface;

} // End of namespace Graphics

namespace Video {

class AVIDecoder;

} // End of namespace Video

namespace VCruise {

static const uint kNumDirections = 8;

class AudioPlayer;
class TextParser;
struct ScriptSet;
struct Script;
struct Instruction;

enum GameState {
	kGameStateBoot,					// Booting the game
	kGameStateWaitingForAnimation,	// Waiting for a blocking animation to complete, then resuming script
	kGameStateWaitingForFacing,		// Waiting for a blocking animation to complete, then playing _postFacingAnimDef and switching to kGameStateWaitingForAnimation
	kGameStateQuit,					// Quitting
	kGameStateIdle,					// Waiting for input events
	kGameStateScript,				// Running a script
	kGameStateGyroIdle,				// Waiting for mouse movement to run a gyro
	kGameStateGyroAnimation,		// Animating a gyro

	kGameStatePanLeft,
	kGameStatePanRight,
};

struct AnimationDef {
	AnimationDef();

	int animNum;	// May be negative if reversed
	uint firstFrame;
	uint lastFrame;	// Inclusive

	Common::Rect constraintRect;
};

struct RoomDef {
	Common::HashMap<Common::String, AnimationDef> animations;
	Common::HashMap<Common::String, uint> vars;
	Common::HashMap<Common::String, int> values;
	Common::HashMap<Common::String, Common::String> texts;
	Common::HashMap<Common::String, int> sounds;

	Common::String name;
};

struct InteractionDef {
	InteractionDef();

	Common::Rect rect;
	uint16 interactionID;
	uint16 objectType;
};

struct MapScreenDirectionDef {
	Common::Array<InteractionDef> interactions;
};

struct MapDef {
	static const uint kNumScreens = 96;
	static const uint kFirstScreen = 0xa0;

	Common::SharedPtr<MapScreenDirectionDef> screenDirections[kNumScreens][kNumDirections];

	void clear();
	const MapScreenDirectionDef *getScreenDirection(uint screen, uint direction);
};

struct ScriptEnvironmentVars {
	ScriptEnvironmentVars();

	bool lmb;
	bool lmbDrag;
	uint panInteractionID;
	uint fpsOverride;
};

class Runtime {
public:
	Runtime(OSystem *system, Audio::Mixer *mixer, const Common::FSNode &rootFSNode, VCruiseGameID gameID);
	virtual ~Runtime();

	void initSections(Common::Rect gameRect, Common::Rect menuRect, Common::Rect trayRect, const Graphics::PixelFormat &pixFmt);

	void loadCursors(const char *exeName);
	void setDebugMode(bool debugMode);

	bool runFrame();
	void drawFrame();

	void onLButtonDown(int16 x, int16 y);
	void onLButtonUp(int16 x, int16 y);
	void onMouseMove(int16 x, int16 y);
	void onKeyDown(Common::KeyCode keyCode);

	bool canSave() const;
	bool canLoad() const;

	void saveGame(Common::WriteStream *stream) const;
	bool loadGame(Common::ReadStream *stream);

	bool bootGame(bool newGame);

private:
	enum IndexParseType {
		kIndexParseTypeNone,
		kIndexParseTypeRoom,
		kIndexParseTypeRRoom,	// Rectangle room (constrains animation areas)
		kIndexParseTypeYRoom,	// Yes room (variable/ID mappings)
		kIndexParseTypeVRoom,	// Value room (value/ID mappings?)
		kIndexParseTypeTRoom,	// Text
		kIndexParseTypeCRoom,	// Const
		kIndexParseTypeSRoom,	// Sound
		kIndexParseTypeNameRoom,
	};

	enum AnimDecoderState {
		kAnimDecoderStateStopped,
		kAnimDecoderStatePlaying,
		kAnimDecoderStatePaused,
	};

	struct IndexPrefixTypePair {
		const char *prefix;
		IndexParseType parseType;
	};

	struct RenderSection {
		Common::SharedPtr<Graphics::ManagedSurface> surf;
		Common::Rect rect;

		void init(const Common::Rect &paramRect, const Graphics::PixelFormat &fmt);
	};

	struct Gyro {
		int32 currentState;
		int32 requiredState;

		Gyro();

		void reset();
	};

	struct GyroState {
		GyroState();

		void reset();

		static const uint kNumGyros = 4;

		Gyro gyros[kNumGyros];

		uint completeInteraction;
		uint failureInteraction;
		uint frameSeparation;

		uint activeGyro;
		uint dragMargin;
		uint maxValue;

		AnimationDef negAnim;
		AnimationDef posAnim;
		bool isVertical;

		Common::Point dragBasePoint;
		uint dragBaseState;
		bool isWaitingForAnimation;
	};

	enum OSEventType {
		kOSEventTypeInvalid,

		kOSEventTypeMouseMove,
		kOSEventTypeLButtonDown,
		kOSEventTypeLButtonUp,

		kOSEventTypeKeyDown,
	};

	enum PanoramaCursorFlags {
		kPanCursorDraggableHoriz	= (1 << 0),
		kPanCursorDraggableUp		= (1 << 1),
		kPanCursorDraggableDown		= (1 << 2),

		kPanCursorDirectionUp		= (0 << 3),
		kPanCursorDirectionLeft		= (1 << 3),
		kPanCursorDirectionRight	= (2 << 3),
		kPanCursorDirectionDown		= (3 << 3),

		kPanCursorMaxCount			= (1 << 5),
	};

	enum PanoramaState {
		kPanoramaStateInactive,

		kPanoramaStatePanningUncertainDirection,

		kPanoramaStatePanningLeft,
		kPanoramaStatePanningRight,
		kPanoramaStatePanningUp,
		kPanoramaStatePanningDown,
	};

	struct OSEvent {
		OSEvent();

		OSEventType type;
		Common::Point pos;
		Common::KeyCode keyCode;
		uint32 timestamp;
	};

	typedef int32 ScriptArg_t;
	typedef int32 StackValue_t;

	bool runIdle();
	bool runHorizontalPan(bool isRight);
	bool runScript();
	bool runWaitForAnimation();
	bool runWaitForFacing();
	bool runGyroIdle();
	bool runGyroAnimation();
	void exitGyroIdle();
	void continuePlayingAnimation(bool loop, bool useStopFrame, bool &outEndedAnimation);
	void drawSectionToScreen(const RenderSection &section, const Common::Rect &rect);
	void commitSectionToScreen(const RenderSection &section, const Common::Rect &rect);
	void terminateScript();

	void startTerminatingHorizontalPan(bool isRight);

	bool popOSEvent(OSEvent &evt);
	void queueOSEvent(const OSEvent &evt);

	void loadIndex();
	void changeToScreen(uint roomNumber, uint screenNumber);
	void returnToIdleState();
	void changeToCursor(const Common::SharedPtr<Graphics::WinCursorGroup> &cursor);
	bool dischargeIdleMouseMove();
	bool dischargeIdleMouseDown();
	bool dischargeIdleClick();
	void loadMap(Common::SeekableReadStream *stream);

	void changeMusicTrack(int musicID);
	void changeAnimation(const AnimationDef &animDef, bool consumeFPSOverride);
	void changeAnimation(const AnimationDef &animDef, uint initialFrame, bool consumeFPSOverride);

	AnimationDef stackArgsToAnimDef(const StackValue_t *args) const;
	void pushAnimDef(const AnimationDef &animDef);

	void activateScript(const Common::SharedPtr<Script> &script, const ScriptEnvironmentVars &envVars);

	bool parseIndexDef(IndexParseType parseType, uint roomNumber, const Common::String &key, const Common::String &value);
	void allocateRoomsUpTo(uint roomNumber);

	void drawDebugOverlay();

	Common::SharedPtr<Script> findScriptForInteraction(uint interactionID) const;

	void detectPanoramaDirections();
	void detectPanoramaMouseMovement(uint32 timestamp);
	void panoramaActivate();

	bool computeFaceDirectionAnimation(uint desiredDirection, const AnimationDef *&outAnimDef, uint &outInitialFrame, uint &outStopFrame);

	// Script things
	void scriptOpNumber(ScriptArg_t arg);
	void scriptOpRotate(ScriptArg_t arg);
	void scriptOpAngle(ScriptArg_t arg);
	void scriptOpAngleGGet(ScriptArg_t arg);
	void scriptOpSpeed(ScriptArg_t arg);
	void scriptOpSAnimL(ScriptArg_t arg);
	void scriptOpChangeL(ScriptArg_t arg);

	void scriptOpAnimR(ScriptArg_t arg);
	void scriptOpAnimF(ScriptArg_t arg);
	void scriptOpAnimN(ScriptArg_t arg);
	void scriptOpAnimG(ScriptArg_t arg);
	void scriptOpAnimS(ScriptArg_t arg);
	void scriptOpAnim(ScriptArg_t arg);

	void scriptOpStatic(ScriptArg_t arg);
	void scriptOpVarLoad(ScriptArg_t arg);
	void scriptOpVarStore(ScriptArg_t arg);

	void scriptOpItemCheck(ScriptArg_t arg);
	void scriptOpItemCRSet(ScriptArg_t arg);
	void scriptOpItemSRSet(ScriptArg_t arg);
	void scriptOpItemRSet(ScriptArg_t arg);

	void scriptOpSetCursor(ScriptArg_t arg);
	void scriptOpSetRoom(ScriptArg_t arg);
	void scriptOpLMB(ScriptArg_t arg);
	void scriptOpLMB1(ScriptArg_t arg);
	void scriptOpSoundS1(ScriptArg_t arg);
	void scriptOpSoundS2(ScriptArg_t arg);
	void scriptOpSoundS3(ScriptArg_t arg);
	void scriptOpSoundL1(ScriptArg_t arg);
	void scriptOpSoundL2(ScriptArg_t arg);
	void scriptOpSoundL3(ScriptArg_t arg);
	void scriptOp3DSoundL2(ScriptArg_t arg);
	void scriptOpRange(ScriptArg_t arg);
	void scriptOpAddXSound(ScriptArg_t arg);
	void scriptOpClrXSound(ScriptArg_t arg);
	void scriptOpStopSndLA(ScriptArg_t arg);
	void scriptOpStopSndLO(ScriptArg_t arg);

	void scriptOpMusic(ScriptArg_t arg);
	void scriptOpMusicUp(ScriptArg_t arg);
	void scriptOpMusicDn(ScriptArg_t arg);
	void scriptOpParm0(ScriptArg_t arg);
	void scriptOpParm1(ScriptArg_t arg);
	void scriptOpParm2(ScriptArg_t arg);
	void scriptOpParm3(ScriptArg_t arg);
	void scriptOpParmG(ScriptArg_t arg);
	void scriptOpSParmX(ScriptArg_t arg);
	void scriptOpSAnimX(ScriptArg_t arg);

	void scriptOpVolumeDn4(ScriptArg_t arg);
	void scriptOpVolumeUp3(ScriptArg_t arg);
	void scriptOpRandom(ScriptArg_t arg);
	void scriptOpDrop(ScriptArg_t arg);
	void scriptOpDup(ScriptArg_t arg);
	void scriptOpSay3(ScriptArg_t arg);
	void scriptOpSay3Get(ScriptArg_t arg);
	void scriptOpSetTimer(ScriptArg_t arg);
	void scriptOpGetTimer(ScriptArg_t arg);
	void scriptOpDelay(ScriptArg_t arg);
	void scriptOpLoSet(ScriptArg_t arg);
	void scriptOpLoGet(ScriptArg_t arg);
	void scriptOpHiSet(ScriptArg_t arg);
	void scriptOpHiGet(ScriptArg_t arg);

	void scriptOpNot(ScriptArg_t arg);
	void scriptOpAnd(ScriptArg_t arg);
	void scriptOpOr(ScriptArg_t arg);
	void scriptOpAdd(ScriptArg_t arg);
	void scriptOpSub(ScriptArg_t arg);
	void scriptOpCmpEq(ScriptArg_t arg);
	void scriptOpCmpLt(ScriptArg_t arg);
	void scriptOpCmpGt(ScriptArg_t arg);

	void scriptOpBitLoad(ScriptArg_t arg);
	void scriptOpBitSet0(ScriptArg_t arg);
	void scriptOpBitSet1(ScriptArg_t arg);

	void scriptOpDisc1(ScriptArg_t arg);
	void scriptOpDisc2(ScriptArg_t arg);
	void scriptOpDisc3(ScriptArg_t arg);

	void scriptOpEscOn(ScriptArg_t arg);
	void scriptOpEscOff(ScriptArg_t arg);
	void scriptOpEscGet(ScriptArg_t arg);
	void scriptOpBackStart(ScriptArg_t arg);

	void scriptOpAnimName(ScriptArg_t arg);
	void scriptOpValueName(ScriptArg_t arg);
	void scriptOpVarName(ScriptArg_t arg);
	void scriptOpSoundName(ScriptArg_t arg);
	void scriptOpCursorName(ScriptArg_t arg);

	void scriptOpCheckValue(ScriptArg_t arg);
	void scriptOpJump(ScriptArg_t arg);

	void scriptOpVerticalPanSet(bool *flags);
	void scriptOpVerticalPanGet();

	Common::Array<Common::SharedPtr<Graphics::WinCursorGroup> > _cursors;		// Cursors indexed as CURSOR_CUR_##
	Common::Array<Common::SharedPtr<Graphics::WinCursorGroup> > _cursorsShort;	// Cursors indexed as CURSOR_#

	uint _panCursors[kPanCursorMaxCount];

	Common::HashMap<Common::String, StackValue_t> _namedCursors;
	Common::HashMap<StackValue_t, uint> _scriptCursorIDToResourceIDOverride;

	OSystem *_system;
	uint _roomNumber;	// Room number can be changed independently of the loaded room, the screen doesn't change until a command changes it
	uint _screenNumber;
	uint _direction;

	GyroState _gyros;

	AnimationDef _panLeftAnimationDef;
	AnimationDef _panRightAnimationDef;
	bool _havePanAnimations;
	bool _havePanUpFromDirection[kNumDirections];
	bool _havePanDownFromDirection[kNumDirections];

	AnimationDef _idleAnimations[kNumDirections];
	bool _haveIdleAnimations[kNumDirections];

	AnimationDef _postFacingAnimDef;

	Common::HashMap<uint32, int32> _variables;
	Common::HashMap<uint, uint32> _timers;

	static const uint kPanLeftInteraction = 1;
	static const uint kPanDownInteraction = 2;
	static const uint kPanRightInteraction = 3;
	static const uint kPanUpInteraction = 4;

	static const uint kPanoramaLeftFlag = 1;
	static const uint kPanoramaRightFlag = 2;
	static const uint kPanoramaUpFlag = 4;
	static const uint kPanoramaDownFlag = 8;
	static const uint kPanoramaHorizFlags = (kPanoramaLeftFlag | kPanoramaRightFlag);

	uint _panoramaDirectionFlags;

	uint _loadedRoomNumber;
	uint _activeScreenNumber;
	bool _havePendingScreenChange;
	bool _havePendingReturnToIdleState;
	GameState _gameState;

	bool _escOn;
	bool _debugMode;

	VCruiseGameID _gameID;

	Common::Array<Common::SharedPtr<RoomDef> > _roomDefs;
	Common::SharedPtr<ScriptSet> _scriptSet;

	Common::SharedPtr<Script> _activeScript;
	uint _scriptNextInstruction;
	Common::Array<StackValue_t> _scriptStack;
	ScriptEnvironmentVars _scriptEnv;

	Common::SharedPtr<Common::RandomSource> _rng;

	Common::SharedPtr<AudioPlayer> _musicPlayer;

	Common::SharedPtr<Video::AVIDecoder> _animDecoder;
	AnimDecoderState _animDecoderState;
	uint _animPendingDecodeFrame;
	uint _animDisplayingFrame;
	uint _animFirstFrame;
	uint _animLastFrame;
	uint _animStopFrame;
	uint _animFrameRateLock;
	Common::Rect _animConstraintRect;
	uint32 _animStartTime;
	uint32 _animFramesDecoded;
	uint _loadedAnimation;
	bool _animPlayWhileIdle;

	bool _idleIsOnInteraction;
	bool _idleHaveClickInteraction;
	bool _idleHaveDragInteraction;
	uint _idleInteractionID;

	Audio::Mixer *_mixer;

	MapDef _map;

	RenderSection _gameSection;
	RenderSection _gameDebugBackBuffer;
	RenderSection _menuSection;
	RenderSection _traySection;

	Common::Point _mousePos;
	Common::Point _lmbDownPos;
	uint32 _lmbDownTime;
	bool _lmbDown;
	bool _lmbDragging;
	bool _lmbReleaseWasClick;	// If true, then the mouse didn't move at all since the LMB down

	PanoramaState _panoramaState;
	// The anchor point behavior is kind of weird.  The way it works in Reah is that if the camera is panning left or right and the mouse is moved
	// back across the anchor threshold at all, then the anchor point is reset to the mouse position.  This means it can drift, because the center
	// is moved by roughly the margin amount.
	// Vertical panning on the other hand resets the anchor vertical coordinate every time.
	Common::Point _panoramaAnchor;

	Common::Array<OSEvent> _pendingEvents;

	static const uint kAnimDefStackArgs = 7;

	static const uint kCursorArrow = 0;

	static const int kPanoramaPanningMarginX = 11;
	static const int kPanoramaPanningMarginY = 11;

	static const uint kSaveGameIdentifier = 0x53566372;
	static const uint kSaveGameCurrentVersion = 1;
	static const uint kSaveGameEarliestSupportedVersion = 1;
};

} // End of namespace VCruise

#endif
