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

#ifndef MEDIASTATION_ASSET_HEADER_H
#define MEDIASTATION_ASSET_HEADER_H

#include "common/str.h"
#include "common/rect.h"
#include "common/hashmap.h"
#include "graphics/palette.h"

#include "mediastation/chunk.h"
#include "mediastation/mediascript/eventhandler.h"

namespace MediaStation {

struct MovieChunkReference {
	uint32 headerChunkId;
	uint32 audioChunkId;
	uint32 videoChunkId;
};

typedef uint32 ChunkReference;

typedef uint32 AssetId;

enum AssetType {
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
	kAssetTypeImageSet = 0x001d,
	kAssetTypeCursor = 0x000c, // CSR
	kAssetTypePrinter = 0x0019, // PRT
	kAssetTypeMovie = 0x0016, // MOV
	kAssetTypePalette = 0x0017,
	kAssetTypeText = 0x001a, // TXT
	kAssetTypeFont = 0x001b, // FON
	kAssetTypeCamera = 0x001c, // CAM
	kAssetTypeCanvas = 0x001e, // CVS
	// TODO: Discover how the XSND differs from regular sounds.
	// Only appears in Ariel.
	kAssetTypeXsnd= 0x001f,
	kAssetTypeXsndMidi = 0x0020,
	// TODO: Figure out what this is. Only appears in Ariel.
	kAssetTypeRecorder = 0x0021,
	kAssetTypeFunction = 0x0069 // FUN
};

enum AssetStartupType {
	kAssetStartupInactive = 0,
	kAssetStartupActive = 1
};

enum AssetHeaderSectionType {
	kAssetHeaderEmptySection = 0x0000,
	kAssetHeaderSoundEncoding1 = 0x0001,
	kAssetHeaderSoundEncoding2 = 0x0002,
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
	kAssetHeaderPathUnk1 = 0x0610,
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
	kAssetHeaderTextCharacterClass = 0x0266
};

enum TextJustification {
    kTextJustificationLeft = 0x25c,
    kTextJustificationRight = 0x25d,
    kTextJustificationCenter = 0x25e
};

enum TextPosition {
    kTextPositionMiddle = 0x25e,
    kTextPositionTop = 0x260,
    kTextPositionBotom = 0x261
};

struct CharacterClass {
    uint firstAsciiCode = 0;
    uint lastAsciiCode = 0;
};

enum SoundEncoding {
	PCM_S16LE_MONO_22050 = 0x0010, // Uncompressed linear PCM
	IMA_ADPCM_S16LE_MONO_22050 = 0x0004 // IMA ADPCM encoding, must be decoded
};

class AssetHeader {
public:
	AssetHeader(Chunk &chunk);
	~AssetHeader();

	uint32 _fileNumber = 0;
	AssetType _type;
	AssetId _id;

	ChunkReference _chunkReference = 0;
	// These two are only used in movies.
	ChunkReference _audioChunkReference = 0;
	ChunkReference _animationChunkReference = 0;
	Common::Rect *_boundingBox = nullptr;
	Common::Array<Common::Point *> _mouseActiveArea;
	int _zIndex = 0;
	uint32 _assetReference = 0;
	uint32 _startup = 0;
	bool _transparency = false;
	bool _hasOwnSubfile = false;
	uint32 _cursorResourceId = 0;
	uint32 _frameRate = 10; // This is the default for sprites, which are the only ones that use this field.
	uint32 _loadType = 0;
	uint32 _rate = 0;
	bool _editable = 0;
	Graphics::Palette *_palette = nullptr;
	bool _getOffstageEvents = 0;
	uint32 _x = 0; // Image only.
	uint32 _y = 0; // Image only.
	Common::String *_name = nullptr;
	uint32 _stageId = 0;
	SoundEncoding _soundEncoding;
	uint32 _chunkCount = 0;

	// PATH FIELDS.
	uint32 _dissolveFactor = 0;
	Common::Point *_startPoint = nullptr;
	Common::Point *_endPoint = nullptr;
	uint32 _stepRate = 0;
	uint32 _duration = 0;

	// EVENT HANDLER FIELDS.
	Common::HashMap<uint, EventHandler *> _eventHandlers;
	Common::Array<EventHandler *> _timeHandlers;
	Common::HashMap<uint, EventHandler *> _keyDownHandlers;
	Common::Array<EventHandler *> _inputHandlers;
	Common::Array<EventHandler *> _loadCompleteHandlers;

	// TEXT FIELDS.
    Common::String *_text = nullptr;
    uint _maxTextLength = 0;
    uint _fontAssetId = 0;
    TextJustification _justification;
    TextPosition _position;
    Common::Array<CharacterClass> _acceptedInput;

private:
	void readSection(AssetHeaderSectionType sectionType, Chunk &chunk);
	AssetHeaderSectionType getSectionType(Chunk &chunk);
};

} // End of namespace MediaStation

#endif