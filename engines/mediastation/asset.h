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

#ifndef MEDIASTATION_ASSET_H
#define MEDIASTATION_ASSET_H

#include "common/keyboard.h"

#include "mediastation/datafile.h"
#include "mediastation/mediascript/eventhandler.h"
#include "mediastation/mediascript/scriptconstants.h"
#include "mediastation/mediascript/scriptvalue.h"

namespace MediaStation {

enum AssetType {
	kAssetTypeEmpty = 0x0000,
	kAssetTypeScreen = 0x0001, // SCR
	kAssetTypeStage = 0x0002, // STG
	kAssetTypePath = 0x0004, // PTH
	kAssetTypeSound = 0x0005, // SND
	kAssetTypeTimer = 0x0006, // TMR
	kAssetTypeImage = 0x0007, // IMG
	kAssetTypeHotspot = 0x000b, // HSP
	kAssetTypeSprite = 0x000e, // SPR
	kAssetTypeLKZazu = 0x000f,
	kAssetTypeLKConstellations = 0x0010,
	kAssetTypeDocument = 0x0011,
	kAssetTypeImageSet = 0x001d,
	kAssetTypeCursor = 0x000c, // CSR
	kAssetTypePrinter = 0x0019, // PRT
	kAssetTypeMovie = 0x0016, // MOV
	kAssetTypePalette = 0x0017,
	kAssetTypeText = 0x001a, // TXT
	kAssetTypeFont = 0x001b, // FON
	kAssetTypeCamera = 0x001c, // CAM
	kAssetTypeCanvas = 0x001e, // CVS
	kAssetTypeXsnd = 0x001f,
	kAssetTypeXsndMidi = 0x0020,
	kAssetTypeRecorder = 0x0021,
	kAssetTypeFunction = 0x0069 // FUN
};

enum AssetHeaderSectionType {
	kAssetHeaderEmptySection = 0x0000,
	kAssetHeaderEventHandler = 0x0017,
	kAssetHeaderStageId = 0x0019,
	kAssetHeaderAssetId = 0x001a,
	kAssetHeaderChunkReference = 0x001b,
	kAssetHeaderMovieAnimationChunkReference = 0x06a4,
	kAssetHeaderMovieAudioChunkReference = 0x06a5,
	kAssetHeaderAssetReference = 0x077b,
	kAssetHeaderBoundingBox = 0x001c,
	kAssetHeaderMouseActiveArea = 0x001d,
	kAssetHeaderZIndex = 0x001e,
	kAssetHeaderStartup = 0x001f,
	kAssetHeaderTransparency = 0x0020,
	kAssetHeaderHasOwnSubfile = 0x0021,
	kAssetHeaderCursorResourceId = 0x0022,
	kAssetHeaderFrameRate = 0x0024,
	kAssetHeaderLoadType = 0x0032,
	kAssetHeaderSoundInfo = 0x0033,
	kAssetHeaderMovieLoadType = 0x0037,
	kAssetHeaderSpriteChunkCount = 0x03e8,
	kAssetHeaderPalette = 0x05aa,
	kAssetHeaderDissolveFactor = 0x05dc,
	kAssetHeaderGetOffstageEvents = 0x05dd,
	kAssetHeaderX = 0x05de,
	kAssetHeaderY = 0x05df,

	// PATH FIELDS.
	kAssetHeaderStartPoint = 0x060e,
	kAssetHeaderEndPoint = 0x060f,
	kAssetHeaderPathTotalSteps = 0x0610,
	kAssetHeaderStepRate = 0x0611,
	kAssetHeaderDuration = 0x0612,

	// CAMERA FIELDS.
	kAssetHeaderViewportOrigin = 0x076f,
	kAssetHeaderLensOpen = 0x0770,

	// STAGE FIELDS.
	kAssetHeaderStageUnk1 = 0x0771,
	kAssetHeaderCylindricalX = 0x0772,
	kAssetHeaderCylindricalY = 0x0773,
	kAssetHeaderAssetName = 0x0bb8,

	// TEXT FIELDS.
	kAssetHeaderEditable = 0x03eb,
	kAssetHeaderFontId = 0x0258,
	kAssetHeaderInitialText = 0x0259,
	kAssetHeaderTextMaxLength = 0x25a,
	kAssetHeaderTextJustification = 0x025b,
	kAssetHeaderTextPosition = 0x25f,
	kAssetHeaderTextUnk1 = 0x262,
	kAssetHeaderTextUnk2 = 0x263,
	kAssetHeaderTextCharacterClass = 0x0266,

	// SPRITE FIELDS.
	kAssetHeaderSpriteClip = 0x03e9,
	kAssetHeaderCurrentSpriteClip = 0x03ea
};

class Asset {
public:
	Asset(AssetType type) : _type(type) {};
	virtual ~Asset();

	// Does any needed frame drawing, audio playing, event handlers, etc.
	virtual void process() { return; }

	// Runs built-in bytecode methods.
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args);

	virtual bool isSpatialActor() const { return false; }

	virtual void initFromParameterStream(Chunk &chunk);
	virtual void readParameter(Chunk &chunk, AssetHeaderSectionType paramType);

	// These are not pure virtual so if an asset doesn't read any chunks or
	// subfiles it doesn't need to just implement these with an error message.
	virtual void readChunk(Chunk &chunk);
	virtual void readSubfile(Subfile &subfile, Chunk &chunk);

	void processTimeEventHandlers();
	void runEventHandlerIfExists(EventType eventType, const ScriptValue &arg);
	void runEventHandlerIfExists(EventType eventType);

	AssetType type() const { return _type; }
	uint id() const { return _id; }
	uint contextId() const { return _contextId; }
	void setId(uint id) { _id = id; }
	void setContextId(uint id) { _contextId = id; }

	uint32 _chunkReference = 0;
	uint _assetReference = 0;

protected:
	AssetType _type = kAssetTypeEmpty;
	uint _id = 0;
	uint _contextId = 0;

	uint _startTime = 0;
	uint _lastProcessedTime = 0;
	uint _duration = 0;
	Common::HashMap<uint, Common::Array<EventHandler *> > _eventHandlers;
};

class SpatialEntity : public Asset {
public:
	SpatialEntity(AssetType type) : Asset(type) {};

	virtual void draw(const Common::Array<Common::Rect> &dirtyRegion) { return; }
	virtual ScriptValue callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) override;
	virtual void readParameter(Chunk &chunk, AssetHeaderSectionType paramType) override;

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
