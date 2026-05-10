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

#ifndef MEDIASTATION_ACTOR_H
#define MEDIASTATION_ACTOR_H

#include "common/events.h"
#include "common/keyboard.h"

#include "mediastation/datafile.h"
#include "mediastation/events.h"
#include "mediastation/mediascript/scriptresponse.h"
#include "mediastation/mediascript/scriptconstants.h"
#include "mediastation/mediascript/scriptvalue.h"

namespace MediaStation {

class DisplayContext;
class SpatialEntity;
class StageActor;

enum ActorType {
	kActorTypeEmpty = 0x0000,
	kActorTypeScreen = 0x0001, // SCR
	kActorTypeStage = 0x0002, // STG
	kActorTypePath = 0x0004, // PTH
	kActorTypeSound = 0x0005, // SND
	kActorTypeTimer = 0x0006, // TMR
	kActorTypeImage = 0x0007, // IMG
	kActorTypeHotspot = 0x000b, // HSP
	kActorTypeSprite = 0x000e, // SPR
	kActorTypeLKZazu = 0x000f,
	kActorTypeLKConstellations = 0x0010,
	kActorTypeDocument = 0x0011,
	kActorTypeDiskImage = 0x001d,
	kActorTypeCursor = 0x000c, // CSR
	kActorTypePrinter = 0x0019, // PRT
	kActorTypeMovie = 0x0016, // MOV
	kActorTypePalette = 0x0017,
	kActorTypeStreamMovieProxy = 0x18,
	kActorTypeText = 0x001a, // TXT
	kActorTypeFont = 0x001b, // FON
	kActorTypeCamera = 0x001c, // CAM
	kActorTypeDiskImageActor = 0x001d,
	kActorTypeCanvas = 0x001e, // CVS
	kActorTypeXsnd = 0x001f,
	kActorTypeXsndMidi = 0x0020,
	kActorTypeRecorder = 0x0021,
	kActorTypeFunction = 0x0069 // FUN
};
const char *actorTypeToStr(ActorType type);

enum ActorHeaderSectionType {
	kActorHeaderEmptySection = 0x0000,
	kActorHeaderScriptResponse = 0x0017,
	kActorHeaderChildActorId = 0x0019,
	kActorHeaderActorId = 0x001a,
	kActorHeaderChannelIdent = 0x001b,
	kActorHeaderMovieAnimationChannelIdent = 0x06a4,
	kActorHeaderMovieAudioChannelIdent = 0x06a5,
	kActorHeaderActorReference = 0x077b,
	kActorHeaderBoundingBox = 0x001c,
	kActorHeaderMouseActiveArea = 0x001d,
	kActorHeaderZIndex = 0x001e,
	kActorHeaderStartup = 0x001f,
	kActorHeaderTransparency = 0x0020,
	kActorHeaderDiscardAfterUse = 0x0021,
	kActorHeaderCursorResourceId = 0x0022,
	kActorHeaderFrameRate = 0x0024,
	kActorHeaderLoadType = 0x0032,
	kActorHeaderSoundInfo = 0x0033,
	kActorHeaderCachingEnabled = 0x0034,
	kActorHeaderInstallType = 0x0037,
	kActorHeaderSpriteChunkCount = 0x03e8,
	kActorHeaderPalette = 0x05aa,
	kActorHeaderDissolveFactor = 0x05dc,
	kActorHeaderGetOffstageEvents = 0x05dd,
	kActorHeaderX = 0x05de,
	kActorHeaderY = 0x05df,
	kActorHeaderScaleXAndY = 0x77a,
	kActorHeaderScaleX = 0x77c,
	kActorHeaderScaleY = 0x77d,
	kActorHeaderActorName = 0x0bb8,
	kStreamMovieProxyInfo = 0x06ac,

	// DISK IMAGE ACTOR FIELDS.
	kActorHeaderDiskImageMaxStrips = 0x774,
	kActorHeaderDiskImageStripWidth = 0x775,
	kActorHeaderDiskImageUnk1 = 0x776,
	kActorHeaderDiskImageMaxImages = 0x777,
	kActorHeaderDiskImageStripInfo = 0x778,
	kActorHeaderDiskImageUnkRect = 0x779,

	// PATH FIELDS.
	kActorHeaderStartPoint = 0x060e,
	kActorHeaderEndPoint = 0x060f,
	kActorHeaderPathTotalSteps = 0x0610,
	kActorHeaderStepRate = 0x0611,
	kActorHeaderDuration = 0x0612,

	// CAMERA FIELDS.
	kActorHeaderCameraViewportOrigin = 0x076f,
	kActorHeaderCameraLensOpen = 0x0770,
	kActorHeaderCameraImageActor = 0x77b,

	// CANVAS FIELDS.
	kActorHeaderCanvasDissolveFactor = 0x493,
	kActorHeaderCanvasTransparency = 0x7d0,

	// STAGE FIELDS.
	kActorHeaderStageExtent = 0x0771,
	kActorHeaderCylindricalX = 0x0772,
	kActorHeaderCylindricalY = 0x0773,

	// TEXT FIELDS.
	kActorHeaderEditable = 0x03eb,
	kActorHeaderFontActorId = 0x0258,
	kActorHeaderInitialText = 0x0259,
	kActorHeaderTextMaxLength = 0x25a,
	kActorHeaderTextJustification = 0x025b,
	kActorHeaderTextPosition = 0x25f,
	kActorHeaderTextCursorIsVisible = 0x262,
	kActorHeaderTextConstrainToWidth = 0x263,
	kActorHeaderTextOverwriteMode = 0x264,
	kActorHeaderTextAcceptedCharRangeWithOffset = 0x265,
	kActorHeaderTextAcceptedCharRange = 0x0266,

	// SPRITE FIELDS.
	kActorHeaderSpriteClip = 0x03e9,
	kActorHeaderDefaultSpriteClip = 0x03ea
};

enum CylindricalWrapMode : int;

struct MouseActorState {
	SpatialEntity *keyDown = nullptr;
	// There is no key up event.
	SpatialEntity *mouseDown = nullptr;
	SpatialEntity *mouseUp = nullptr;
	SpatialEntity *mouseMoved = nullptr;
	SpatialEntity *mouseExit = nullptr;
	SpatialEntity *mouseEnter = nullptr;
	SpatialEntity *mouseOutOfFocus = nullptr;
};

enum MouseEventFlag {
	kNoFlag = 0x00,
	kMouseDownFlag = 0x01,
	kMouseUpFlag = 0x02,
	kMouseMovedFlag = 0x04,
	kMouseExitFlag = 0x10,
	kMouseEnterFlag = 0x08,
	kMouseUnk1Flag = 0x20,
	kMouseOutOfFocusFlag = 0x40,
	kKeyDownFlag = 0x80,
	// There is no key up event.
};

class Polygon {
public:
	void loadFromParameterStream(Chunk &chunk);
	bool containsPoint(const Common::Point &pointToCheck) const;

private:
	Common::Array<Common::Point> _polygon;
};

// Argument count validation macros for built-in script methods.
// For exact argument count.
#define ARGCOUNTCHECK(n) \
	if (args.size() != (n)) { \
		warning("%s: Expected %d arguments, got %d", builtInMethodToStr(methodId), (n), args.size()); \
	}

// For a range of valid argument counts (min to max).
#define ARGCOUNTRANGE(min, max) \
	if ((int64)(min) > args.size() || args.size() > (int64)(max)) { \
		warning("%s: Expected %d to %d arguments, got %d", builtInMethodToStr(methodId), (min), (max), args.size()); \
	}

// For minimum argument count (no maximum).
#define ARGCOUNTMIN(min) \
	if (args.size() < (min)) { \
		warning("%s: Expected at least %d arguments, got %d", builtInMethodToStr(methodId), (min), args.size()); \
	}

class Actor : public TimerEventReceiver {
public:
	Actor(ActorType type) : _type(type), _timer(this) {};
	virtual ~Actor();

	virtual void timerEvent(const TimerEvent &event) { return; }

	// Runs built-in bytecode methods.
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args);
	virtual void initFromParameterStream(Chunk &chunk);
	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType);
	virtual void loadIsComplete();

	virtual void onEvent(const ActorEvent &event);
	ScriptResponse *findNextTimeScriptResponseAfter(uint32 after) const;
	void runScriptResponseIfExists(EventType eventType, const ScriptValue &arg);
	void runScriptResponseIfExists(EventType eventType);

	ActorType type() const { return _type; }
	uint id() const { return _id; }
	uint contextId() const { return _contextId; }
	void setId(uint id);
	void setContextId(uint id) { _contextId = id; }
	virtual bool isSpatialActor() const { return false; }

	const char *debugName() const;

protected:
	ActorType _type = kActorTypeEmpty;
	bool _loadIsComplete = false;
	uint _id = 0;
	uint _contextId = 0;
	Common::String _debugName;

	uint _duration = 0;
	Common::HashMap<uint, Common::Array<ScriptResponse *> > _scriptResponses;

	// The original had these fields duplicated across several actors, but it made more
	// sense to consolidate it into the main Actor in the reimplementation.
	TimerEntry _timer;
	uint _startTime = 0;
	uint _lastProcessedTime = 0;
	bool setupNextScriptResponseTimer();
	void triggerRemainingTimerEvents();
	void processTimeScriptResponses();
};

class SpatialEntity : public Actor {
public:
	SpatialEntity(ActorType type) : Actor(type) {};
	~SpatialEntity();

	virtual void draw(DisplayContext &displayContext) { return; }
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;
	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;
	virtual void loadIsComplete() override;
	virtual void preload(const Common::Rect &rect, bool fireStepEvent = true) {};
	virtual bool isRectInMemory(const Common::Rect &rect) { return true; }
	virtual bool isReadyToDraw(DisplayContext &displayContext) { return true; }
	virtual bool isLoading() { return false; }

	virtual bool isSpatialActor() const override { return true; }
	virtual bool isVisible() const { return _isVisible; }
	virtual Common::Rect getBbox() const { return _boundingBox; }
	int zIndex() const { return _zIndex; }
	void moveTo(int16 x, int16 y);

	virtual void currentMousePosition(Common::Point &point);
	virtual void invalidateMouse();
	virtual bool interactsWithMouse() const { return false; }

	virtual uint16 findActorToAcceptMouseEvents(
		const Common::Point &point,
		uint16 eventMask,
		MouseActorState &state,
		bool clipMouseEvents) { return kNoFlag; }
	virtual uint16 findActorToAcceptKeyboardEvents(
		uint16 asciiCode,
		uint16 eventMask,
		MouseActorState &state) { return kNoFlag; }

	virtual void mouseDownEvent(const MouseEvent &event) { return; }
	virtual void mouseUpEvent(const MouseEvent &event) { return; }
	virtual void mouseEnteredEvent(const MouseEvent &event) { return; }
	virtual void mouseExitedEvent(const MouseEvent &event) { return; }
	virtual void mouseMovedEvent(const MouseEvent &event) { return; }
	virtual void mouseOutOfFocusEvent(const MouseEvent &event) { return; }
	virtual void keyboardEvent(const KeyboardEvent &event) { return; }

	void setParentStage(StageActor *parentStage) { _parentStage = parentStage; }
	void setToNoParentStage() { _parentStage = nullptr; }
	StageActor *getParentStage() const { return _parentStage; }

	virtual void invalidateLocalBounds();
	virtual void setAdjustedBounds(CylindricalWrapMode wrapMode);

protected:
	uint _stageId = 0;
	int _zIndex = 0;
	double _dissolveFactor = 1.0;

	double _parallaxFactorX = 0.0;
	double _parallaxFactorY = 0.0;
	Common::Rect _boundingBox;
	Common::Rect _originalBoundingBox;
	bool _isVisible = false;
	bool _hasTransparency = false;
	StageActor *_parentStage = nullptr;

	void moveToCentered(int16 x, int16 y);
	void setBounds(const Common::Rect &bounds);
	void setZIndex(int zIndex);
	virtual void setMousePosition(int16 x, int16 y);

	virtual void setDissolveFactor(double dissolveFactor);
	virtual void invalidateLocalZIndex();
};

} // End of namespace MediaStation

#endif
