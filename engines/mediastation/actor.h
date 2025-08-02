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

#include "common/keyboard.h"

#include "mediastation/datafile.h"
#include "mediastation/mediascript/eventhandler.h"
#include "mediastation/mediascript/scriptconstants.h"
#include "mediastation/mediascript/scriptvalue.h"

namespace MediaStation {

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
	kActorTypeImageSet = 0x001d,
	kActorTypeCursor = 0x000c, // CSR
	kActorTypePrinter = 0x0019, // PRT
	kActorTypeMovie = 0x0016, // MOV
	kActorTypePalette = 0x0017,
	kActorTypeText = 0x001a, // TXT
	kActorTypeFont = 0x001b, // FON
	kActorTypeCamera = 0x001c, // CAM
	kActorTypeCanvas = 0x001e, // CVS
	kActorTypeXsnd = 0x001f,
	kActorTypeXsndMidi = 0x0020,
	kActorTypeRecorder = 0x0021,
	kActorTypeFunction = 0x0069 // FUN
};

enum ActorHeaderSectionType {
	kActorHeaderEmptySection = 0x0000,
	kActorHeaderEventHandler = 0x0017,
	kActorHeaderStageId = 0x0019,
	kActorHeaderActorId = 0x001a,
	kActorHeaderChunkReference = 0x001b,
	kActorHeaderMovieAnimationChunkReference = 0x06a4,
	kActorHeaderMovieAudioChunkReference = 0x06a5,
	kActorHeaderActorReference = 0x077b,
	kActorHeaderBoundingBox = 0x001c,
	kActorHeaderMouseActiveArea = 0x001d,
	kActorHeaderZIndex = 0x001e,
	kActorHeaderStartup = 0x001f,
	kActorHeaderTransparency = 0x0020,
	kActorHeaderHasOwnSubfile = 0x0021,
	kActorHeaderCursorResourceId = 0x0022,
	kActorHeaderFrameRate = 0x0024,
	kActorHeaderLoadType = 0x0032,
	kActorHeaderSoundInfo = 0x0033,
	kActorHeaderMovieLoadType = 0x0037,
	kActorHeaderSpriteChunkCount = 0x03e8,
	kActorHeaderPalette = 0x05aa,
	kActorHeaderDissolveFactor = 0x05dc,
	kActorHeaderGetOffstageEvents = 0x05dd,
	kActorHeaderX = 0x05de,
	kActorHeaderY = 0x05df,

	// PATH FIELDS.
	kActorHeaderStartPoint = 0x060e,
	kActorHeaderEndPoint = 0x060f,
	kActorHeaderPathTotalSteps = 0x0610,
	kActorHeaderStepRate = 0x0611,
	kActorHeaderDuration = 0x0612,

	// CAMERA FIELDS.
	kActorHeaderViewportOrigin = 0x076f,
	kActorHeaderLensOpen = 0x0770,

	// STAGE FIELDS.
	kActorHeaderStageUnk1 = 0x0771,
	kActorHeaderCylindricalX = 0x0772,
	kActorHeaderCylindricalY = 0x0773,
	kActorHeaderActorName = 0x0bb8,

	// TEXT FIELDS.
	kActorHeaderEditable = 0x03eb,
	kActorHeaderFontId = 0x0258,
	kActorHeaderInitialText = 0x0259,
	kActorHeaderTextMaxLength = 0x25a,
	kActorHeaderTextJustification = 0x025b,
	kActorHeaderTextPosition = 0x25f,
	kActorHeaderTextUnk1 = 0x262,
	kActorHeaderTextUnk2 = 0x263,
	kActorHeaderTextCharacterClass = 0x0266,

	// SPRITE FIELDS.
	kActorHeaderSpriteClip = 0x03e9,
	kActorHeaderCurrentSpriteClip = 0x03ea
};

class Actor {
public:
	Actor(ActorType type) : _type(type) {};
	virtual ~Actor();

	// Does any needed frame drawing, audio playing, event handlers, etc.
	virtual void process() { return; }

	// Runs built-in bytecode methods.
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args);

	virtual bool isSpatialActor() const { return false; }

	virtual void initFromParameterStream(Chunk &chunk);
	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType);

	// These are not pure virtual so if an actor doesnʻt read any chunks or
	// subfiles it doesnʻt need to just implement these with an error message.
	virtual void readChunk(Chunk &chunk);
	virtual void readSubfile(Subfile &subfile, Chunk &chunk);

	void processTimeEventHandlers();
	void runEventHandlerIfExists(EventType eventType, const ScriptValue &arg);
	void runEventHandlerIfExists(EventType eventType);

	ActorType type() const { return _type; }
	uint id() const { return _id; }
	uint contextId() const { return _contextId; }
	void setId(uint id) { _id = id; }
	void setContextId(uint id) { _contextId = id; }

	uint32 _chunkReference = 0;
	uint _actorReference = 0;

protected:
	ActorType _type = kActorTypeEmpty;
	uint _id = 0;
	uint _contextId = 0;

	uint _startTime = 0;
	uint _lastProcessedTime = 0;
	uint _duration = 0;
	Common::HashMap<uint, Common::Array<EventHandler *> > _eventHandlers;
};

class SpatialEntity : public Actor {
public:
	SpatialEntity(ActorType type) : Actor(type) {};

	virtual void draw(const Common::Array<Common::Rect> &dirtyRegion) { return; }
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;
	virtual void readParameter(Chunk &chunk, ActorHeaderSectionType paramType) override;

	virtual bool isSpatialActor() const override { return true; }
	virtual bool isVisible() const { return _isVisible; }
	virtual Common::Rect getBbox() const { return _boundingBox; }
	int zIndex() const { return _zIndex; }

protected:
	uint _stageId = 0;
	int _zIndex = 0;
	double _dissolveFactor = 0.0;
	Common::Rect _boundingBox;
	bool _isVisible = false;
	bool _hasTransparency = false;
	bool _getOffstageEvents = false;

	void moveTo(int16 x, int16 y);
	void moveToCentered(int16 x, int16 y);
	void setBounds(const Common::Rect &bounds);
	void setZIndex(int zIndex);

	virtual void setDissolveFactor(double dissolveFactor);
	virtual void invalidateLocalBounds();
	virtual void invalidateLocalZIndex();
};

} // End of namespace MediaStation

#endif
